/*
 * RollerDrive.h
 *
 *  Created on: 7. 12. 2017
 *      Author: Viktor
 */

#ifndef ROLLERDRIVE_H_
#define ROLLERDRIVE_H_


#include "PCA9536.h"
#include "ByzanceLogger.h"
#include "byzance_debug.h"






/* --- PRINTF_BYTE_TO_BINARY macro's --- */
#define PRINTF_BINARY_PATTERN_INT8 "%c%c%c%c%c%c%c%c%c"
#define PRINTF_BYTE_TO_BINARY_INT8(i)    \
    (((i) & 0x80ll) ? '1' : '0'), \
    (((i) & 0x40ll) ? '1' : '0'), \
    (((i) & 0x20ll) ? '1' : '0'), \
    (((i) & 0x10ll) ? '1' : '0'), \
    (((i) & 0x08ll) ? '1' : '0'), \
    (((i) & 0x04ll) ? '1' : '0'), \
    (((i) & 0x02ll) ? '1' : '0'), \
    (((i) & 0x01ll) ? '1' : '0')

#define PRINTF_BINARY_PATTERN_INT16 \
    PRINTF_BINARY_PATTERN_INT8              PRINTF_BINARY_PATTERN_INT8
#define PRINTF_BYTE_TO_BINARY_INT16(i) \
    PRINTF_BYTE_TO_BINARY_INT8((i) >> 8), '_',  PRINTF_BYTE_TO_BINARY_INT8(i),  '\0'
#define PRINTF_BINARY_PATTERN_INT32 \
    PRINTF_BINARY_PATTERN_INT16             PRINTF_BINARY_PATTERN_INT16
#define PRINTF_BYTE_TO_BINARY_INT32(i) \
    PRINTF_BYTE_TO_BINARY_INT16((i) >> 16),  '_',  PRINTF_BYTE_TO_BINARY_INT16(i),  '\0'
#define PRINTF_BINARY_PATTERN_INT64    \
    PRINTF_BINARY_PATTERN_INT32             PRINTF_BINARY_PATTERN_INT32
#define PRINTF_BYTE_TO_BINARY_INT64(i) \
    PRINTF_BYTE_TO_BINARY_INT32((i) >> 32), '_',  PRINTF_BYTE_TO_BINARY_INT32(i),  '\0'
// Usage:
//  long long int flag = 516;
//  printf("My Flag 0b" PRINTF_BINARY_PATTERN_INT32 "\n", PRINTF_BYTE_TO_BINARY_INT32(flag));

/* --- end macros --- */


#define ROLLERDRIVE_TOP_RELAY_RUN			((uint8_t) 0)
#define ROLLERDRIVE_TOP_RELAY_DIR			((uint8_t) 1)

#define ROLLERDRIVE_BOTTOM_RELAY_RUN		((uint8_t) 3)
#define ROLLERDRIVE_BOTTOM_RELAY_DIR		((uint8_t) 2)

#define	ROLLERDRIVE_LIGHT_AVG				((uint8_t) 4)


#define	ROLLERDRIVE_LIGHT_NONE				((uint8_t) 0)
#define	ROLLERDRIVE_LIGHT_POSTER_MARK		((uint8_t) 1)	// Sensor on the poster mark.
#define	ROLLERDRIVE_LIGHT_POSTER_TOP		((uint8_t) 2)	// Sensor on top of the poster.
#define	ROLLERDRIVE_LIGHT_WHITE_PAPER_MARK	((uint8_t) 3)	// Edge of the top white paper mark.
#define	ROLLERDRIVE_LIGHT_POSTER_NEXT		((uint8_t) 4)	// Next poster.


#define SYSTEM_EVENT_RUN_UP					((uint32_t) 0b0000000000001)
#define SYSTEM_EVENT_STOP					((uint32_t) 0b0000000000010)
#define SYSTEM_EVENT_RUN_DOWN				((uint32_t) 0b0000000000100)
#define SYSTEM_EVENT_POSTER_MARK			((uint32_t) 0b0000000001000)
#define SYSTEM_EVENT_WHITE_PAPER_MARK		((uint32_t) 0b0000000010000)
#define SYSTEM_EVENT_POSTER_WAIT			((uint32_t) 0b0000000100000)

#define SYSTEM_EVENT_IND_UP_BOTH			((uint32_t) 0b0000001000000)
#define SYSTEM_EVENT_IND_UP_TOP				((uint32_t) 0b0000010000000)
#define SYSTEM_EVENT_IND_UP_BOTTOM			((uint32_t) 0b0000100000000)
#define SYSTEM_EVENT_IND_DOWN_BOTH			((uint32_t) 0b0001000000000)
#define SYSTEM_EVENT_IND_DOWN_TOP			((uint32_t) 0b0010000000000)
#define SYSTEM_EVENT_IND_DOWN_BOTTOM		((uint32_t) 0b0100000000000)



#define ROLLERDRIVE_TIME_UNIT					((uint32_t) 50)		// Period of roller state machine in ms
#define ROLLERDRIVE_AUTO_MODE_PERIOD_MIN		((uint32_t) 5000)	// ms
#define ROLLERDRIVE_AUTO_MODE_PERIOD_MAX		((uint32_t) 120000)	// ms
#define ROLLERDRIVE_AUTO_MODE_PERIOD_DEFAULT	((uint32_t) 10000)	// ms

#define SIGNAL_LIGHT_SENSOR					0x01


struct EdgeStruct {
    int 		time_between;
    bool 		level_this;
    bool 		level_last;
    Timer *		timer;
};

enum MotorState {RUN_UP, STOP, RUN_DOWN};
enum RollerMode {MODE_MAN, MODE_CALIB, MODE_POSTER, MODE_AUTO};


class RollerDrive{
	public:
		RollerDrive(void);

		uint8_t			set_poster(uint8_t dst);
		uint8_t			set_mode(RollerMode mode);
		uint8_t			set_auto_mode_period(uint32_t period_ms);
		uint8_t			set_auto_mode_range(uint8_t from, uint8_t to);
		uint8_t			calibration(void);
		bool 			is_calibrated(void);
		uint8_t 		get_poster_count(void);
		uint8_t			run_ind_motors_up(bool top = false, bool bottom = false);
		uint8_t			run_ind_motors_down(bool top = false, bool bottom = false);
		uint8_t			run_ind_motors_stop();

		uint32_t 		test_get_state_now(void);
		uint32_t 		test_get_state_last(void);
		void 			test_set_event(uint32_t event);
		void 			test_rele(uint8_t val);
		void 			test_io(void);

	private:
		PCA9536 * 		_expander;
		Ticker *		_ticker;
		Thread *		_thread;


		uint8_t			_proccess_time_edge(int32_t num);
		void 			_sens_light_read(void);
		void			_thread_function(void);

		void 			_event_add(uint32_t event);
		void 			_event_remove(uint32_t event);
		bool 			_event_check(uint32_t event);

		uint8_t  		_run_up(void);
		uint8_t  		_run_down(void);
		uint8_t  		_stop(void);

		uint8_t 		_motors_up(bool top = true, bool bottom = true);
		uint8_t 		_motors_down(bool top = true, bool bottom = true);
		uint8_t 		_motors_stop(void);



		bool			_sens_light_array[ROLLERDRIVE_LIGHT_AVG];
		uint8_t			_sens_light_cntr;
		EdgeStruct 		_edge;
		MotorState		_state_now, _state_last;
		uint32_t		_event;
		RollerMode		_mode;

		uint8_t			_wait_cntr;
		bool			_ignore_white_mark;

		uint8_t			_calib_poster_cnt;
		uint8_t			_calib_white_marks_cnt;
		bool			_calib_done;

		uint16_t		_auto_wait_cntr;
		uint16_t		_auto_wait_period;
		uint8_t			_auto_poster_num;
		int8_t			_auto_dir;
		uint8_t			_auto_poster_from;
		uint8_t			_auto_poster_to;

		uint8_t			_poster_cnt;
		uint8_t			_poster_dst;
		uint8_t			_poster_now;


		DigitalIn * 	_rm_up;			// Pressed = 1, released = 0
		DigitalIn * 	_rm_down;		// Pressed = 1, released = 0
		DigitalIn * 	_rm_xxx;		// Pressed = 1, released = 0
		DigitalIn * 	_rm_onoff;		// On = 1, off = 0
		DigitalIn * 	_rm_connected;	// Remote connected = 1, disconnected = 0
		DigitalIn * 	_rm_automan;	// Auto = 1, man = 0

		DigitalIn * 	_sens_light;	// No reflected light received = 1, else 0

		DigitalOut * 	ul3;
		DigitalOut * 	ul2;
		DigitalOut * 	ul1;

		Mutex			_lock;
};






#endif /* ROLLERDRIVE_H_ */
