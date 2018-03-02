/*
 * TimeOut.h
 *
 *  Created on: 28. 6. 2016
 *      Author: Viktor
 */


#ifndef PROTOCOLTIMEOUT_H_
#define PROTOCOLTIMEOUT_H_

#include "mbed.h"
#include <Timer.h>

/** 
* \class ProtocolTimeout
* \brief brief TO DO 
*
* Long brief TO DO
*/
class ProtocolTimeout : public Timer {
protected:
	 int32_t 		last_received_time;				// Cas posledniho prijmu lib. bytu (pcktu u NRF)
	 int32_t 		last_my_received_data_time;		// Cas posledniho prijmu meho data pcktu.
	 int32_t 		last_my_received_ack_time;		// Cas posledniho prijmu meho ACK pcktu.
	 int32_t 		last_clean_time;				// Cas pravidelne mazani bufferu neaktvni linky.
	 int32_t 		last_received_time_with_ack;	// Cas posledni prijmu lib. packetu s validnim ACK.


public:

	/**
	* 
	* \param
	* \param
	* \return
	*/
	inline void save_last_received_time(void){
		last_received_time = read_ms();
	}
	
	/**
	* 
	* \param
	* \param
	* \return
	*/
	inline  int32_t get_last_received_time(void){
		return last_received_time;
	}
	
	/**
	* 
	* \param
	* \param
	* \return
	*/
	inline  bool expired_last_received_time(uint32_t value){
		if(abs(((int32_t)read_ms()) - ((int32_t)last_received_time)) > ((int32_t) value)){
			return true;
		}else{
			return false;
		}
	}

	
	/**
	* 
	* \param
	* \param
	* \return
	*/
	inline void save_my_last_received_data_time(void){
		last_my_received_data_time = read_ms();
	}
	
	
	/**
	* 
	* \param
	* \param
	* \return
	*/
	inline int32_t get_my_last_received_data_time(void){
		return last_my_received_data_time;
	}
	
	/**
	* 
	* \param
	* \param
	* \return
	*/
	inline bool expired_my_last_received_data_time(uint32_t value){
		if(abs(((int32_t)read_ms()) - ((int32_t)last_my_received_data_time)) > ((int32_t) value)){
			return true;
		}else{
			return false;
		}
	}




	/**
	* 
	* \param
	* \param
	* \return
	*/
	inline void save_my_last_received_ack_time(void){
		last_my_received_ack_time = read_ms();
	}
	
	/**
	* 
	* \param
	* \param
	* \return
	*/
	inline  int32_t get_my_last_received_ack_time(void){
		return last_my_received_ack_time;
	}
	
	
	/**
	* 
	* \param
	* \param
	* \return
	*/
	inline  bool expired_my_last_received_ack_time(uint32_t value){
		if(abs(((int32_t)read_ms()) - ((int32_t)last_my_received_ack_time)) > ((int32_t) value)){
			return true;
		}else{
			return false;
		}
	}


	/**
	* 
	* \param
	* \param
	* \return
	*/
	inline void save_last_clean_time(void){
		last_clean_time = read_ms();
	}
	
	/**
	* 
	* \param
	* \param
	* \return
	*/
	inline void save_last_clean_time(int32_t value){
		last_clean_time = value;
	}
	
	/**
	* 
	* \param
	* \param
	* \return
	*/
	inline  int32_t get_last_clean_time(void){
		return last_clean_time;
	}
	
	/**
	* 
	* \param
	* \param
	* \return
	*/
	inline  bool expired_last_clean_time(uint32_t value){
		if(abs(((int32_t)read_ms()) - ((int32_t)last_clean_time)) > ((int32_t) value)){
			return true;
		}else{
			return false;
		}
	}


	/**
	* 
	* \param
	* \param
	* \return
	*/
	inline void save_last_received_time_with_ack(void){
		last_received_time_with_ack = read_ms();
	}
	
	/**
	* 
	* \param
	* \param
	* \return
	*/
	inline void save_last_received_time_with_ack(int32_t value){
		last_received_time_with_ack = value;
	}
	
	/**
	* 
	* \param
	* \param
	* \return
	*/
	inline int32_t get_last_received_time_with_ack(void){
		return last_received_time_with_ack;
	}
	
	/**
	* 
	* \param
	* \param
	* \return
	*/
	inline bool expired_last_received_time_with_ack(uint32_t value){
		if(abs(((int32_t)read_ms()) - ((int32_t)last_received_time_with_ack)) > ((int32_t) value)){
			return true;
		}else{
			return false;
		}
	}


};


#endif	// PROTOCOLTIMEOUT_H_
