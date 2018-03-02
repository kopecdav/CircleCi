/*
 * Dali.h
 *
 *  Created on: 2. 2. 2018
 *      Author: martin
 */

#ifndef DALI_H_
#define DALI_H_
#include "mbed.h"

#define DALI_0 1
#define DALI_1 0

#define BROADCAST_DP 0b11111110
#define BROADCAST_C 0b11111111
#define ON_DP 0b11111110
#define OFF_DP 0b00000000
#define ON_C 0b00000101
#define OFF_C 0b00000000
#define QUERY_STATUS 0b10010000
#define RESET 0b00100000

class Dali{
public:
	Dali(PinName RX, PinName TX):_rx_int(RX),_out(TX){
		_rx_int.fall(callback(this,&Dali::_recv_fall));
		_rx_int.rise(callback(this,&Dali::_recv_rise));
		_transmitting=false;
		_recv=0x00;
		_recv_cnt=0x00;
	}

	void send(uint8_t addr, uint8_t cmd){
		_transmitting=true;
		_write_bit(1);
		_write_byte(addr);
		_write_byte(cmd);
		_out=DALI_1;
		wait_us(600);
		_transmitting=false;
		_recv_cnt=0;
		_last_state=0;
		_recv_state=2;
		_mask = 0x80;
		_recv_bits=0;
		_recv=0;
		_recieving=true;
	}

	void init() {



		long low_longadd = 0x000000;
		long high_longadd = 0xFFFFFF;
		long longadd = (long)(low_longadd + high_longadd) / 2;
		uint8_t highbyte;
		uint8_t middlebyte;
		uint8_t lowbyte;
		uint8_t short_add = 0;
		uint8_t cmd2;

		Thread::wait(10);
		send(BROADCAST_C, RESET);
		Thread::wait(10);
		send(BROADCAST_C, RESET);
		Thread::wait(10);
		send(BROADCAST_C, OFF_C);
		Thread::wait(10);
		send(0b10100101, 0b00000000); //initialise
		Thread::wait(10);
		send(0b10100101, 0b00000000); //initialise
		Thread::wait(10);
		send(0b10100111, 0b00000000); //randomise
		Thread::wait(10);
		send(0b10100111, 0b00000000); //randomise

		printf("Searching fo long addresses:\n");

		while (longadd <= 0xFFFFFF - 2 and short_add <= 64) {
			while ((high_longadd - low_longadd) > 1) {

				_split_add(longadd, highbyte, middlebyte, lowbyte); //divide 24bit adress into three 8bit adresses
				Thread::wait(10);
				send(0b10110001, highbyte); //search HB
				Thread::wait(10);
				send(0b10110011, middlebyte); //search MB
				Thread::wait(10);
				send(0b10110101, lowbyte); //search LB
				Thread::wait(10);
				send(0b10101001, 0b00000000); //compare


				if (_recieve() == -1)
				{
					printf("low\n");
					low_longadd = longadd;
				}
				else
				{
					printf("high\n");
					high_longadd = longadd;
				}

				longadd = (low_longadd + high_longadd) / 2; //center

				printf("DEC: %d\n",longadd+1);
			} // second while


			if (high_longadd != 0xFFFFFF)
			{
				_split_add(longadd + 1, highbyte, middlebyte, lowbyte);
				send(0b10110001, highbyte); //search HB
				Thread::wait(10);
				send(0b10110011, middlebyte); //search MB
				Thread::wait(10);
				send(0b10110101, lowbyte); //search LB
				Thread::wait(10);
				send(0b10110111, 1 + (short_add << 1)); //program short adress
				Thread::wait(10);
				send(0b10101011, 0b00000000); //withdraw
				Thread::wait(10);
				send(1 + (short_add << 1), ON_C);
				Thread::wait(1000);
				send(1 + (short_add << 1), OFF_C);
				Thread::wait(10);
				short_add++;

				printf("Assigning a short address\n");


				high_longadd = 0xFFFFFF;
				longadd = (low_longadd + high_longadd) / 2;

			}
			else {
				printf("End\n");

			}
		} // first while


		send(0b10100001, 0b00000000);  //terminate
		send(BROADCAST_C, OFF_C);  //broadcast on
	}

private:
	InterruptIn _rx_int;
	DigitalOut _out;
	bool _transmitting,_recieving;
	uint8_t _recv,_recv_cnt,_mask,_recv_bits;
	Timer _recv_timer;
	bool _last_state;
	uint8_t _recv_state;

	void _write_byte(uint8_t b){
		for(int i = 7; i >= 0; i--){
			_write_bit((b>>i)&0x01);
		}
	}

	void _write_bit(bool bit){
		if(bit){
			_out=DALI_0;
			wait_us(416);
			_out=DALI_1;
			wait_us(416);
		}else{
			_out=DALI_1;
			wait_us(416);
			_out=DALI_0;
			wait_us(416);
		}
	}

	void _recv_bit(bool bit){

		if( _recv_bits != 0){	//exclude start bit
			if(bit){
				_recv |= _mask;
				printf("1\n");
			}else{
				printf("0\n");
			}
			_mask = _mask>>1;
		}
		_recv_bits++;
		if(_recv_bits==9){	//8bits + start bit
			printf("received %d\n",_recv);
			_recieving=false;
		}

	}

	int _recieve(){
		Timer tim;
		tim.start();
		while(_recieving){
			if(tim.read_ms() > 100){
				printf("recv timeout\n");
				return -1;	//timeout
			}
			Thread::wait(10);
		}
		printf("recieved %d\n",_recv);
		return _recv;
	}

#define DALI_PERIOD 625
	void _recv_rise(){
		//we received space
		if(_transmitting){
			return;
		}
		if(_recv_cnt==0){
			_recv_timer.reset();
			_recv_timer.start();
			_recv_cnt++;
			//return;
		}

		uint32_t tim_val = _recv_timer.read_us();
		_recv_timer.reset();
		switch(_recv_state){
		case 0:	//last recv was 1 - waiting for short or long pulse
			printf("e1\n");		//pulse is only in _recv_fall - error
			if(tim_val<DALI_PERIOD){
				_recv_state=1;	//short pulse go to state 1
			}else{
				_recv_state=2;	//long pulse - go to state 2
			}
			break;
		case 1:	//waiting for short short space
			if(tim_val<DALI_PERIOD){	//recv short space - go back to state 0
				_recv_state=0;
			}else{
				printf("e2\n");			//long space when short expected
			}
			break;
		case 2:	//last recv was 0 - waiting for short or long space
			//printf("0");
			if(tim_val<DALI_PERIOD){
				_recv_bit(1);
				_recv_state=3;	//short space go to state 3
			}else{
				_recv_bit(0);
				_recv_state=0;	//long space - go to state 0
			}
			break;
		case 3:	//waiting for short pulse
			printf("e3\n");	//no pulse available here
			if(tim_val<DALI_PERIOD){	//recv short pulse - go back to state 2
				_recv_state=2;
			}else{
				//printf("e3\n");
			}
			break;
		}
		_last_state=1;
	}

	void _recv_fall(){
		if(_transmitting){
			return;
		}
		if(_recv_cnt==0){
			_recv_timer.reset();
			_recv_timer.start();
			_recv_cnt++;
			//return;
		}
		uint32_t tim_val = _recv_timer.read_us();
		_recv_timer.reset();
		switch(_recv_state){
		case 0:	//last recv was 1 - waiting for short or long pulse
			if(tim_val<DALI_PERIOD){
				_recv_bit(0);
				_recv_state=1;	//short pulse go to state 1
			}else{
				_recv_bit(1);
				_recv_state=2;	//long pulse - go to state 2
			}
			break;
		case 1:	//waiting for short short space
			printf("e4\n");	//space not avalable here
			if(tim_val<DALI_PERIOD){	//recv short space - go back to state 0
				_recv_state=0;
			}else{
			}
			break;
		case 2:	//last recv was 0 - waiting for short or long space
			printf("e5");
			if(tim_val<DALI_PERIOD){
				_recv_state=3;	//short space go to state 3
			}else{
				_recv_state=0;	//long space - go to state 0
			}
			break;
		case 3:	//waiting for short pulse
			if(tim_val<DALI_PERIOD){	//recv short pulse - go back to state 2
				_recv_state=2;
			}else{
				printf("e6\n");		//shor requred but long recvd
			}
			break;
		}
		_last_state=0;
		_recv_cnt++;
	}

	void _split_add(long input, uint8_t &highbyte, uint8_t &middlebyte, uint8_t &lowbyte)
	{
		highbyte = input >> 16;
		middlebyte = input >> 8;
		lowbyte = input;
	}


};


#endif /* DALI_H_ */
