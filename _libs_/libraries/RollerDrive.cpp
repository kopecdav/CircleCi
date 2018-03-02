/*
 * RollerDrive.cpp
 *
 *  Created on: 7. 12. 2017
 *      Author: Viktor
 */


#include "RollerDrive.h"

REGISTER_DEBUG(RD);

RollerDrive::RollerDrive(void){
	_rm_up = new DigitalIn(Y02);
	_rm_down = new DigitalIn(Y03);
	_rm_xxx = new DigitalIn(Y04);
	_rm_onoff = new DigitalIn(Y05);

	_rm_connected = new DigitalIn(Y06);
	_rm_automan = new DigitalIn(Y07);
	_sens_light = new DigitalIn(Y09);

	ul1 = new DigitalOut(Y12, 0);
	ul2 = new DigitalOut(Y13, 0);
	ul3 = new DigitalOut(Y14, 0);

	_expander = new PCA9536(X07, X06);
	_ticker = new Ticker();
	_thread = new Thread(osPriorityLow4, 1024);
	_edge.timer = new Timer;


	for(uint8_t i = 0; i < ROLLERDRIVE_LIGHT_AVG; i++){
		_sens_light_array[i] = false;	// Sensor output is active in logic low (false value)
	}
	_sens_light_cntr = 0;

	_state_now = STOP;
	_state_last = STOP;
	_event = 0;

	_poster_cnt = 0;
	_poster_dst = 1;
	_poster_now  = 1;
	_mode = MODE_MAN;
	_ignore_white_mark = false;

	_auto_poster_num = 1;
	_auto_dir = 1;
	_auto_poster_from = 0;
	_auto_poster_to = 0;
	_auto_wait_period = ROLLERDRIVE_AUTO_MODE_PERIOD_DEFAULT/ROLLERDRIVE_TIME_UNIT;

	_calib_done = false;

	_thread->start(callback(this, &RollerDrive::_thread_function));


	//__LOG("Test rele shield:\n");
	_expander->set_all_outputs();
	Thread::wait(100);
	_expander->set_value(0x0F);
	Thread::wait(100);
	_expander->set_value(0x00);
	Thread::wait(500);
}


void RollerDrive::_thread_function(void){
	static uint8_t _edge_type;
	static osEvent _evt;

	__DEBUG("RollerDrive: thread started\n");

	// Start ticker for periodic light sensor measurement
	_ticker->attach(callback(this, &RollerDrive::_sens_light_read), 0.007);

	// Reset and start timer.
	_edge.timer->reset();
	_edge.timer->start();

	while(1){

		// Light sensor edge found?
		_edge_type = ROLLERDRIVE_LIGHT_NONE;
		_evt = Thread::signal_wait(0, ROLLERDRIVE_TIME_UNIT);
		if((_evt.status == osEventSignal) && (_evt.value.signals & SIGNAL_LIGHT_SENSOR)){
			__INFO("Light sensor: change %d to %d, pulse lenght=%dms\n", _edge.level_last, _edge.level_this, _edge.time_between);
			_edge_type = _proccess_time_edge((int32_t) _edge.time_between);
			__INFO("Pulse type: %d\n", _edge_type);

			// Create new events based on light sensor detection
			if(_mode != MODE_MAN){
				switch(_edge_type){
					case ROLLERDRIVE_LIGHT_NONE:
						break;

					case ROLLERDRIVE_LIGHT_POSTER_MARK:
						_event_add(SYSTEM_EVENT_POSTER_MARK);
						__TRACE("_event |= SYSTEM_EVENT_POSTER_MARK;\n");
						break;

					case ROLLERDRIVE_LIGHT_POSTER_TOP:
						break;

					case ROLLERDRIVE_LIGHT_WHITE_PAPER_MARK:
						_event_add(SYSTEM_EVENT_WHITE_PAPER_MARK);
						break;

					case ROLLERDRIVE_LIGHT_POSTER_NEXT:
						break;

					default:
						__NOP();
				}
			}
		}

		// Timeout expired?
		if(_evt.status == osEventTimeout){
			// Is remote controller connected? Remote controller has the highest priority.
			if(_rm_connected->read() == 1){
				// Automatic mode = 1, manual = 0
				if(_rm_onoff->read() == 0){
					//_mode = MODE_MAN;
					__DEBUG("_mode = MODE_MAN - HARD REMOTE;\n");

					if(_rm_up->read()){
						__TRACE("_motors_up(true, true): %d\n", _motors_up(true, true));
					} else if(_rm_down->read()){
						__TRACE("_motors_down(true, true): %d\n", _motors_down(true, true));
					} else {
						__TRACE("_motors_stop(): %d\n", _motors_stop());
					}

					// Wait and run the loop again.
					continue;

				}else if (_mode == MODE_POSTER){
					if(_rm_up->read()){
						set_poster(_poster_dst + 1);
					} else if(_rm_down->read()){
						set_poster(_poster_dst - 1);
					}
				}
			}


			// Create new events based on mode of operation
			switch(_mode){
				case MODE_AUTO:
					__DEBUG("MODE_AUTO: wait=%d/%d, _auto_poster_num=%d, range <%d, %d>\n", _auto_wait_cntr, _auto_wait_period, _auto_poster_num, _auto_poster_from, _auto_poster_to);

					// If poster change period overflows, change poster.
					if(_auto_wait_cntr >= _auto_wait_period){

						// Check direction change inversion.
						/*if((_auto_poster_num == _auto_poster_to) || (_auto_poster_num == _auto_poster_from)){
							_auto_dir *= -1;
							__INFO("MODE_AUTO: Direction changed to: %d\n", _auto_dir);
						}*/
						// If first poster achieved, increment required poster position.
						if((_auto_poster_num == _auto_poster_from)){
							_auto_dir = +1;
							__INFO("MODE_AUTO: Direction changed to: %d\n", _auto_dir);
						// If last poster achieved, decrement required poster position.
						} else if((_auto_poster_num == _auto_poster_to)){
							_auto_dir = -1;
							__INFO("MODE_AUTO: Direction changed to: %d\n", _auto_dir);
						}


						// Reset wait counter, increment poster number and set it.
						_auto_wait_cntr = 0;
						_auto_poster_num += _auto_dir;
						set_poster(_auto_poster_num);
						__INFO("MODE_AUTO: _auto_poster_num=%d, range <%d, %d>, _poster_now=%d, _poster_dst=%d, _poster_cnt=%d\n", _auto_poster_num, _auto_poster_from, _auto_poster_to, _poster_now, _poster_dst, _poster_cnt);
					}
					_auto_wait_cntr++;
					// break;	// NO BREAK HERE - poster mode also necessary for auto mode

				case MODE_POSTER:
					// Run to final poster destination
					if(_poster_dst != _poster_now){
						if(_poster_dst > _poster_now){
							__DEBUG("MODE_POSTER: RUN_UP (now/dst %d/%d)\n", _poster_now, _poster_dst);
							_event_add(SYSTEM_EVENT_RUN_UP);
						} else {
							__DEBUG("MODE_POSTER: RUN_DOWN (now/dst %d/%d)\n", _poster_now, _poster_dst);
							_event_add(SYSTEM_EVENT_RUN_DOWN);
						}
					} else{
						__DEBUG("MODE_POSTER: STOP (now/dst %d/%d)\n", _poster_now, _poster_dst);
						_event_add(SYSTEM_EVENT_STOP);
					}
					break;

				case MODE_CALIB:
					__DEBUG("MODE_CALIB: white_marks=%d\n", _calib_white_marks_cnt);
					if(_calib_white_marks_cnt == 0){
						_event_add(SYSTEM_EVENT_RUN_UP);
					} else if(_calib_white_marks_cnt == 1){
						_event_add(SYSTEM_EVENT_RUN_DOWN);
						_calib_poster_cnt = 0;
					} else if(_calib_white_marks_cnt == 2){
						_event_add(SYSTEM_EVENT_RUN_DOWN);
					} else if(_calib_white_marks_cnt == 3){
						_event_add(SYSTEM_EVENT_RUN_UP);
					} else if(_calib_white_marks_cnt == 4){
						_event_add(SYSTEM_EVENT_POSTER_MARK);
						_event_add(SYSTEM_EVENT_RUN_UP);

						/*
						 * Calibration done. Switch to auto mode.
						 */
						_mode = MODE_AUTO;
						_poster_now = 0;
						_poster_dst = 1;
						_poster_cnt = _calib_poster_cnt;

						// If auto range not defined explicitly than you full range for auto mode.
						if((_auto_poster_from == 0) && (_auto_poster_to == 0)){
							_auto_poster_from = 1;

							// -1 because of emergency poster
							if(_poster_cnt>1){
								_auto_poster_to = _poster_cnt - 1;
							} else {
								__WARNING("not enough posters\n");
								_auto_poster_to = 1;
							}
						}
						_calib_done = true;
						__LOG("RollerDrive: Calibration finished (_poster_cnt=%d)\n", _poster_cnt);
					}
					break;

				case MODE_MAN:
					__DEBUG("_mode = MODE_MAN - SOFT;\n");

					/*if(_rm_up->read()){
						__TRACE("_motors_up(true, true): %d\n", _motors_up(true, true));
					} else if(_rm_down->read()){
						__TRACE("_motors_down(true, true): %d\n", _motors_down(true, true));
					} else {
						__TRACE("_motors_stop(): %d\n", _motors_stop());
					}*/

					//Thread::wait(ROLLERDRIVE_TIME_UNIT);
					//break; //continue;
			}

			// Debug print
			__TRACE("Event START: 0b" PRINTF_BINARY_PATTERN_INT16 "\n", PRINTF_BYTE_TO_BINARY_INT16(_event));
			__TRACE("(0x%02X)\n", _event);

			// Run motors up
			if(_event_check(SYSTEM_EVENT_RUN_UP)){

				__TRACE("SYSTEM_EVENT_RUN_UP\n");

				_event_remove(SYSTEM_EVENT_RUN_UP);
				_state_last = _state_now;
				_run_up();
			}

			// Run motors down
			if(_event_check(SYSTEM_EVENT_RUN_DOWN)){

				__TRACE("SYSTEM_EVENT_RUN_DOWN\n");

				_event_remove(SYSTEM_EVENT_RUN_DOWN);
				_state_last = _state_now;
				_run_down();
			}

			// Stop motors
			if(_event_check(SYSTEM_EVENT_STOP)){

				__TRACE("SYSTEM_EVENT_RUN_STOP\n");

				_event_remove(SYSTEM_EVENT_STOP);
				_state_last = _state_now;
				_stop();
			}

			// Individual motor control (for adding/removing posters), necessary to do it after stop motors cmd
			if(_event_check(SYSTEM_EVENT_IND_UP_BOTH)){

				__TRACE("SYSTEM_EVENT_IND_UP_BOTH\n");

				_event_remove(SYSTEM_EVENT_IND_UP_BOTH);
				_motors_up(true, true);
			}
			if(_event_check(SYSTEM_EVENT_IND_UP_TOP)){

				__TRACE("SYSTEM_EVENT_IND_UP_TOP\n");

				_event_remove(SYSTEM_EVENT_IND_UP_TOP);
				_motors_up(true, false);
			}

			if(_event_check(SYSTEM_EVENT_IND_UP_BOTTOM)){

				__TRACE("SYSTEM_EVENT_IND_UP_BOTTOM\n");

				_event_remove(SYSTEM_EVENT_IND_UP_BOTTOM);
				_motors_up(false, true);
			}

			if(_event_check(SYSTEM_EVENT_IND_DOWN_BOTH)){

				__TRACE("SYSTEM_EVENT_IND_DOWN_BOTH\n");

				_event_remove(SYSTEM_EVENT_IND_DOWN_BOTH);
				_motors_down(true, true);
			}

			if(_event_check(SYSTEM_EVENT_IND_DOWN_TOP)){

				__TRACE("SYSTEM_EVENT_IND_DOWN_TOP\n");

				_event_remove(SYSTEM_EVENT_IND_DOWN_TOP);
				_motors_down(true, false);
			}

			if(_event_check(SYSTEM_EVENT_IND_DOWN_BOTTOM)){

				__TRACE("SYSTEM_EVENT_IND_DOWN_BOTTOM\n");

				_event_remove(SYSTEM_EVENT_IND_DOWN_BOTTOM);
				_motors_down(false, true);
			}

			// Poster mark was detected - calculate waiting time for stopping motors
			if(_event_check(SYSTEM_EVENT_POSTER_MARK)){


				__TRACE("SYSTEM_EVENT_POSTER_MARK\n");

				_event_remove(SYSTEM_EVENT_POSTER_MARK);

				// In calibration mode, count all posters only.
				if(_mode == MODE_CALIB){
					_calib_poster_cnt++;
					__DEBUG("MODE_CALIB: _calib_poster_cnt=%d\n", _calib_poster_cnt);

				// In poster and auto mode is still necessary to scroll some more time to reach final position
				} else if((_mode == MODE_POSTER) || (_mode == MODE_AUTO)){
					// In
					if(_state_now == RUN_UP){
						_wait_cntr = 6;
						_event_add(SYSTEM_EVENT_POSTER_WAIT);
					} else if(_state_now == RUN_DOWN){
						_wait_cntr = 26;
						_event_add(SYSTEM_EVENT_POSTER_WAIT);
					} else {
						__ERROR("SYSTEM_EVENT_POSTER_MARK: error - motors not runnig\n");
					}
				}
			}


			// Poster mark was detected but is still necessary to scroll some more time
			if(_event_check(SYSTEM_EVENT_POSTER_WAIT)){
				__TRACE("_wait_cntr=%d\n", _wait_cntr);

				// If waiting finished, increment to next poster.
				if(_wait_cntr == 0){
					if(_state_now == RUN_UP){
						_poster_now++;
					} else if(_state_now == RUN_DOWN){
						_poster_now--;
					}

					// Do not ignore white paper mark. During calibration is necessary to ignore white mark, but in other mode not.
					_ignore_white_mark = false;

					__INFO("Poster destination %d reached. (%d/%d)\n\n", _poster_dst, _poster_now, _poster_cnt);
					_event_remove(SYSTEM_EVENT_POSTER_WAIT);
					_state_last = _state_now;
					_stop();
				} else {
					_wait_cntr--;	// Decrement waiting counter.
				}
			}

			// What to do with white paper mark.
			if(_event_check(SYSTEM_EVENT_WHITE_PAPER_MARK)){
				_event_remove(SYSTEM_EVENT_WHITE_PAPER_MARK);

				// White mark found = some error happened -> necessary to scroll to defined position (= first/last poster)
				if((_mode == MODE_POSTER) || (_mode == MODE_AUTO)){
					// Clear all events to get clear defined state
					_event_remove(SYSTEM_EVENT_RUN_UP);
					_event_remove(SYSTEM_EVENT_RUN_DOWN);
					_event_remove(SYSTEM_EVENT_POSTER_MARK);
					_event_remove(SYSTEM_EVENT_POSTER_WAIT);

					// Backup this state and stop motors
					_state_last = _state_now;
					_stop();

					// First white mark = poster overflow detected -> go back (roll back through white mark back to posters).
					if(_ignore_white_mark == false){
						_ignore_white_mark = true;

						// Go back to first or last poster
						if(_state_last == RUN_UP){
							_poster_dst = _poster_cnt;
							_poster_now = _poster_cnt + 1;
						} else{
							_poster_dst = 1;
							_poster_now = 0;
						}
					// Second white mark = scrolling back completed -> white mark simulate ROLLERDRIVE_LIGHT_POSTER_MARK
					} else {
						_event_add(SYSTEM_EVENT_POSTER_MARK);
					}
					__WARNING("SYSTEM_EVENT_WHITE_PAPER_MARK: stop, new allowed dir %s (poster dst=%d, now=%d)\n", _state_last == RUN_UP ? "RUN_DOWN" : "RUN_UP", _poster_dst, _poster_now);

				// In calibration is white paper mark useful event.
				} else if(_mode == MODE_CALIB){
					// Wait to generate longer light pulse (without wait is time between two white paper marks recognized also as white paper mark)
					Thread::wait(300);
					_calib_white_marks_cnt++;
				}
			}
			// Debug print
			__TRACE("Event END:   0b" PRINTF_BINARY_PATTERN_INT16 "\n", PRINTF_BYTE_TO_BINARY_INT16(_event));
			__TRACE("(0x%02X)\n", _event);
		}

	}
}



void RollerDrive::_event_add(uint32_t event){

	__TRACE("begin\n");

	//__TRACE("event_set: 0x%04X (now=0x%04X)\n", event, _event);
	_event |= event;
}
void RollerDrive::_event_remove(uint32_t event){

	__TRACE("begin\n");

	//__TRACE("event_remove: 0x%04X (now=0x%04X)\n", event, _event);
	_event &= ~event;
}

bool RollerDrive::_event_check(uint32_t event){

	//__TRACE("begin\n");

	//__TRACE("event_check: 0x%04X (now=0x%04X)\n", event, _event);
	if(_event & event){
		return true;
	} else {
		return false;
	}
}


/**
 * Was calibration successfully finished?
 */
bool RollerDrive::is_calibrated(void){

	if(_lock.lock(100)!=osOK){
		__ERROR("roller locker was not locked\n");
		return 0;
	}

	__TRACE("begin\n");

	if(_calib_done == false){
		__DEBUG("Calibration not done.\n");
		_lock.unlock();
		return false;
	}

	_lock.unlock();

	return true;
}


/**
 * Returns number of posters. Return zero if calibration wasn't finished.
 */
uint8_t RollerDrive::get_poster_count(void){


	if(_lock.lock(100)!=osOK){
		__ERROR("roller locker was not locked\n");
		return 0;
	}

	__TRACE("begin\n");

	// Is calibration finished?
	if(!is_calibrated()){
		__WARNING("get_poster_count: Calibration not done.n");
		_lock.unlock();
		return 0;
	}

	_lock.unlock();

	return _poster_cnt;
}


/**
 * Set poster you want do display. Range is 1 to N (N = posters count).
 */
uint8_t RollerDrive::set_poster(uint8_t dst){

	if(_lock.lock(100)!=osOK){
		__ERROR("roller locker was not locked\n");
		return 0;
	}

	__TRACE("begin\n");

	// Is calibration finished?
	if(!is_calibrated()){
		__WARNING("set_poster: Calibration not done.\n");
		_lock.unlock();
		return 2;
	}

	/*
	if(_mode != MODE_POSTER){
		__WARNING("set_poster: poster mode not enabled\n");
		_lock.unlock();
		return 4;
	}
	*/

	// Is value invalid?
	if((dst == 0) || (dst > _poster_cnt)){
		__WARNING("set_poster: invalid value %d (_poster_cnt=%d)\n", dst, _poster_cnt);
		_lock.unlock();
		return 3;
	}

	// Is roller ready to change poster?
	if(_poster_dst == _poster_now){
		_poster_dst = dst;
		__INFO("set_poster(%d): _poster dst=%d, now=%d, cnt=%d\n", dst, _poster_dst, _poster_now, _poster_cnt);
		_lock.unlock();
		return 0;
	}
	__WARNING("set_poster(%d): roller NOT ready - _poster dst=%d, now=%d, cnt=%d\n", dst,  _poster_dst, _poster_now, _poster_cnt);

	_lock.unlock();
	return 1;
}

/**
 * Select mode of operation. Some mode requires successfully finished calibration.
 */
uint8_t RollerDrive::set_mode(RollerMode mode){

	if(_lock.lock(100)!=osOK){
		__ERROR("roller locker was not locked\n");
		return 0;
	}

	__TRACE("begin\n");

	int rc = 0;

	// Is calibration finished?
	//if(!is_calibrated() && ((mode != MODE_CALIB) || (mode != MODE_MAN))){

	if(!is_calibrated() && (mode != MODE_CALIB) && (mode != MODE_MAN)){
		__WARNING("calibration not done, mode=%d\n", mode);
		_lock.unlock();
		return 1;
	}

	// Should I start calibration?
	if(mode == MODE_CALIB){
		rc = calibration();
		_lock.unlock();
		return rc;
	}

	__TRACE("setting mode=%d\n", (uint32_t) mode);

	_mode = mode;

	_lock.unlock();

	return 0;
}


/**
 * Auto mode changes poster after some time. Set this time in miliseconds.
 */
uint8_t	RollerDrive::set_auto_mode_period(uint32_t period_ms){

	int rc = 0;

	// Clear actual counter.
	_auto_wait_cntr = 0;

	// Set new period (calculate period from time in miliseconds).
	if(period_ms < ROLLERDRIVE_AUTO_MODE_PERIOD_MIN){
		__TRACE("correcting period (underflow)\n");
		_auto_wait_period = ROLLERDRIVE_AUTO_MODE_PERIOD_MIN/ROLLERDRIVE_TIME_UNIT;
		rc = 1;
	} else 	if(period_ms > ROLLERDRIVE_AUTO_MODE_PERIOD_MAX){
		__TRACE("correcting period (overflow)\n");
		_auto_wait_period = ROLLERDRIVE_AUTO_MODE_PERIOD_MAX/ROLLERDRIVE_TIME_UNIT;
		rc = 2;
	} else {
		__TRACE("calculating period (ok)\n");
		_auto_wait_period = period_ms/ROLLERDRIVE_TIME_UNIT;
		rc = 3;
	}

	__TRACE("_auto_wait_period = %d, rc = %d\n", _auto_wait_period, rc);
}


/**
 * 	Set auto mode range, ie roll from poster 'start' to poster 'to' and repeat.
 */
uint8_t	RollerDrive::set_auto_mode_range(uint8_t from, uint8_t to){

	__TRACE("begin\n");

	// Check input data validity
	if((from > to) || (from == 0) || (to > _poster_cnt)){
		__WARNING("set_auto_mode_range: invalid input - %d, %d\n", from, to);
		return 4;
	}

	// Save input data
	_auto_poster_from = from;
	_auto_poster_to = to;

	// Clear actual counter.
	// NOTE: large amount of poster and short _auto_wait_period time can results in one-time poster skipping.
	_auto_wait_cntr = 0;

	// Go to first poster in defined range, save it in auto mode variable and set positive increment for next poster.
	_auto_poster_num = _auto_poster_from;
	_auto_dir = +1;
	return set_poster(_auto_poster_num);
}

/**
 * 	Init and start calibration.
 */
uint8_t	RollerDrive::calibration(void){

	__TRACE("begin\n");

	// Is calibration in progress right now?
	if(_mode == MODE_CALIB){
		__DEBUG("calibration: calibration already in progress\n");
		return 1;
	}

	_calib_white_marks_cnt = 0;
	_calib_poster_cnt = 0;
	_mode = MODE_CALIB;
	_calib_done = false;
	return 0;
}


uint8_t	RollerDrive::run_ind_motors_up(bool top, bool bottom){

	__TRACE("begin\n");

	if((_mode != MODE_MAN)){
		__WARNING("run_ind_motor_up: Not MODE_MAN\n");
		return 1;
	}

	if(top && bottom){
		_event_add(SYSTEM_EVENT_IND_UP_BOTH);
	} else if(top){
		_event_add(SYSTEM_EVENT_IND_UP_TOP);
	} else if(bottom){
		_event_add(SYSTEM_EVENT_IND_UP_BOTTOM);
	}
	return 0;
}

uint8_t	RollerDrive::run_ind_motors_down(bool top, bool bottom){

	__TRACE("begin\n");

	if((_mode != MODE_MAN)){
		__WARNING("run_ind_motor_down: Not MODE_MAN\n");
		return 1;
	}

	if(top && bottom){
		__WARNING("_DOWN_BOTH\n");
		_event_add(SYSTEM_EVENT_IND_DOWN_BOTH);
	} else if(top){
		__WARNING("_DOWN_TOP\n");
		_event_add(SYSTEM_EVENT_IND_DOWN_TOP);
	} else if(bottom){
		__WARNING("_DOWN__BOTTOM\n");
		_event_add(SYSTEM_EVENT_IND_DOWN_BOTTOM);
	} else {
		__WARNING("_DOWN_xx - nothing\n");
	}

	__TRACE("end\n");

	return 0;
}


uint8_t	RollerDrive::run_ind_motors_stop(void){

	__TRACE("begin\n");

	if((_mode != MODE_MAN)){
		__WARNING("run_ind_motor_stop: Not MODE_MAN\n");
		return 1;
	}
	_event_add(SYSTEM_EVENT_STOP);
	return 0;
}



uint8_t RollerDrive::_proccess_time_edge(int32_t num){

	__DEBUG("Delay after ticker pulse event=%dms\n", _edge.timer->read_ms());

	// Direction - down
	if((num > 30) && (num < 60)){	// 75ms pro originalni plakaty
		__INFO(">>> Sensor on the poster mark.\n");
		return ROLLERDRIVE_LIGHT_POSTER_MARK;
	} else if((num > 95) && (num < 135)){
		__INFO(">>> Sensor on top of the poster.\n");
		return ROLLERDRIVE_LIGHT_POSTER_TOP;
	} else if((num > 1350) && (num < 1750)){	// puvodne 2900ms horni hranice
		__INFO(">>> White paper mark detected.\n");
		return ROLLERDRIVE_LIGHT_WHITE_PAPER_MARK;
	} else if((num > 2600) && (num < 3200)){
		__INFO(">>> Next poster.\n");
		return ROLLERDRIVE_LIGHT_POSTER_NEXT;
	}

	return ROLLERDRIVE_LIGHT_NONE;
}



/**
 * Read state of the light sensor.
 * Sensor output is active in logic low (0 value).
 *
 * Do some averaging in this function, than return value.
 *
 */
void RollerDrive::_sens_light_read(void){
	uint8_t _avg = 0;
	bool _out_now = 0;
	static bool _out_last = 0;

	//__TRACE("ticker runs\n");
	//printf("ticker runs - printf\n");
	// Read and save value
	_sens_light_array[_sens_light_cntr] = _sens_light->read() ? true : false;

	// Debug print
	/*if(last != _sens_light_array[_sens_light_cntr]){
		//_edge_time_previos = _edge_time;
		//_edge_time = _timer->read_ms();

		__DEBUG("Ticker: %d/%dms\n", _timer->read_ms(););
		_thread->signal_set(0x01);
	}
	last = _sens_light_array[_sens_light_cntr];*/



	// Found average value
	for(uint8_t i = 0; i < ROLLERDRIVE_LIGHT_AVG; i++){
		_avg += (uint8_t)(_sens_light_array[i]);
	}

	// Increment array position counter.
	_sens_light_cntr++;

	// If counter overflows, reset the counter.
	if(_sens_light_cntr >= ROLLERDRIVE_LIGHT_AVG){
		_sens_light_cntr = 0;
	}

	// Is sensor output active after averaging? (active = 0, inactive = 1)
	_out_now = (_avg < (ROLLERDRIVE_LIGHT_AVG/2)) ? false : true;

	// Does sensor's output change (after filtering/averaging)?
	 if(_out_now != _out_last) {

		 // Public data to structure
		 _edge.level_last = _out_last;
		 _edge.level_this = _out_now;
		 _edge.time_between = _edge.timer->read_ms();

		 // Save this event locally and reset timer
		 _out_last = _out_now;
		 _edge.timer->reset();

		 // TODO: remove this debug printf (ByzanceLogger can't be used - mutex in logger is probably unaccessible from ticker routine)
		 // printf("Light sensor: change %d to %d, pulse lenght=%dms\n", _edge.level_last, _edge.level_this, _edge.time_between);
		 _thread->signal_set(SIGNAL_LIGHT_SENSOR);

	 }
}






uint8_t  RollerDrive::_run_up(void){

	if(_lock.lock(100)!=osOK){
		__ERROR("roller locker was not locked\n");
		return 0;
	}

	int rc = 0;

	__TRACE("begin\n");

	_state_now = RUN_UP;

	rc = _motors_up(true, true);

	_lock.unlock();

	return rc;
}

uint8_t  RollerDrive::_run_down(void){

	if(_lock.lock(100)!=osOK){
		__ERROR("roller locker was not locked\n");
		return 0;
	}

	int rc = 0;

	__TRACE("begin\n");

	_state_now = RUN_DOWN;

	rc = _motors_down(true, true);

	_lock.unlock();

	return rc;
}

uint8_t  RollerDrive::_stop(void){

	if(_lock.lock(100)!=osOK){
		__ERROR("roller locker was not locked\n");
		return 0;
	}

	int rc = 0;

	__TRACE("begin\n");

	_state_now = STOP;

	rc = _motors_stop();

	_lock.unlock();

	return rc;
}



/**
 * Run poster up (possible to drive each motor separately). Useful for manual mode.
 */
uint8_t  RollerDrive::_motors_up(bool top, bool bottom){

	if(_lock.lock(100)!=osOK){
		__ERROR("roller locker was not locked\n");
		return 0;
	}

	__TRACE("motors up\n");

	uint8_t tmp = 0;

	*ul1 = 1;
	*ul2 = 0;
	*ul3 = 0;

	// Prepare value and set all realys at one time
	tmp = _expander->set_value((((1 << ROLLERDRIVE_TOP_RELAY_DIR) + (1 << ROLLERDRIVE_TOP_RELAY_RUN)) & (bottom ? 0xFF : 0x00)) +
			(((1 << ROLLERDRIVE_BOTTOM_RELAY_DIR) + (1 << ROLLERDRIVE_BOTTOM_RELAY_RUN)) & (top ? 0xFF : 0x00)));

	if(tmp){
		__ERROR("motors up -> ERROR!\n");
	}

	_lock.unlock();

	return tmp;
}

/**
 * Run poster down (possible to drive each motor separately). Useful for manual mode.
 */
uint8_t  RollerDrive::_motors_down(bool top, bool bottom){

	if(_lock.lock(100)!=osOK){
		__ERROR("roller locker was not locked\n");
		return 0;
	}

	__TRACE("motors down\n");

	uint8_t tmp = 0;

	*ul1 = 0;
	*ul2 = 0;
	*ul3 = 1;

	// Prepare value and set all realys at one time
	tmp = _expander->set_value((((0 << ROLLERDRIVE_TOP_RELAY_DIR) + (1 << ROLLERDRIVE_TOP_RELAY_RUN)) & (bottom ? 0xFF : 0x00)) +
			(((0 << ROLLERDRIVE_BOTTOM_RELAY_DIR) + (1 << ROLLERDRIVE_BOTTOM_RELAY_RUN)) & (top ? 0xFF : 0x00)));

	if(tmp){
		__ERROR("motors down -> ERROR!\n");
	}

	_lock.unlock();

	return tmp;
}

/**
 * Stop both motors. Useful for manual mode.
 */
uint8_t RollerDrive::_motors_stop(void){

	if(_lock.lock(100)!=osOK){
		__ERROR("roller locker was not locked\n");
		return 0;
	}

	__TRACE("motors stop\n");

	uint8_t tmp = 0;

	*ul1 = 0;
	*ul2 = 1;
	*ul3 = 0;

	// Turn off all relays -> stop all drives
	tmp = _expander->set_value(0b0000);

	if(tmp){
		__ERROR("motors stop -> ERROR!\n");
	}

	_lock.unlock();

	return tmp;

}






uint32_t RollerDrive::test_get_state_now(void){
	return _state_now;
}

uint32_t RollerDrive::test_get_state_last(void){
	return _state_last;
}


void RollerDrive::test_set_event(uint32_t event){
	__INFO("SetEvent: 0x%02X (now=0x%02X)\n", event, _event);
	_event = event;
}

/**
 * Test function. Test rele function.
 */
void RollerDrive::test_rele(uint8_t val){

	if(val == 0){
		_expander->set_value(0x00);
	} else {
		_expander->set_value(0x0F);
	}
	Thread::wait(1000);
}

/**
 * Debug function. Read and print all IOs.
 */
void RollerDrive::test_io(void){
	__ERROR("_rm_up =        %d\n", _rm_up->read());
	__ERROR("_rm_down =      %d\n", _rm_down->read());
	__ERROR("_rm_xxx =       %d\n", _rm_xxx->read());
	__ERROR("_rm_onoff =     %d\n", _rm_onoff->read());;
	__ERROR("_rm_connected = %d\n", _rm_connected->read());
	__ERROR("_rm_automan =   %d\n", _rm_automan->read());

	__ERROR("_sens_light =   %d\n\n", _sens_light->read());
}

