/*
 * MsgIdBuffer.cpp
 *
 *  Created on: 27. 10. 2016
 *      Author: Viktor
 */

#include "MsgIdBuffer.h"


MsgIdBuffer::MsgIdBuffer(){
	_msg_list.clear();
}

/*
 * @return 0 - polozka v bufferu nebyla a pridala se
 * @return 1 - polozka v bufferu uz je 
 */
uint32_t MsgIdBuffer::add(string & input_string){
	uint32_t 		i;
	msg_id_struct 	temp;

	// Projedu vsechny polozku v seznamu
	for(i = 0; i < _msg_list.size(); i++){
		//MESSAGE_ID_BUFF_DBG("list=%s, in=%s\n", _msg_list.at(i).msg_id.c_str(), input_string.c_str());

		// Kontroluju uz existenci dane polozky, tj. exituje uz?
		if(input_string.compare(_msg_list.at(i).msg_id) == 0){
			// Vyrizenoi instrukci inkrementovat nechci, ale u nevyrizene si zvysim pocitadlo
			if(_msg_list.at(i).cnt != 0){
				_msg_list.at(i).cnt++;	// Existovala nevyrizena, zvysim pocitadlo.
			}
			MESSAGE_ID_BUFF_DBG("[MSG_ID_BUFF] ID already \"%s\" exist, cntr=%d.\n", input_string.c_str(), _msg_list.at(i).cnt);
			return 1;
		}
	}


	// Polozka neexistovala, budu ji chtit pridat. Pokud je buffer plny, odmazu posledni prvek.
	if(_msg_list.size() >= MESSAGE_ID_BUFF_CAPACITY){
		MESSAGE_ID_BUFF_DBG("[MSG_ID_BUFF] list full %d/%d, delete last item\n", _msg_list.size(), MESSAGE_ID_BUFF_CAPACITY);
		_msg_list.pop_back();
	}


	// Novy prvek vlozim na zacatek (vytvarim si tak na oko kruh. buffer)
	temp.cnt = 10;
	temp.msg_id = input_string;
	_msg_list.push_front(temp);
	MESSAGE_ID_BUFF_DBG("[MSG_ID_BUFF] create item, ID=\"%s\"\n", input_string.c_str());

	return 0;
}


/*
 * @return 0      - 
 * @return number - 
 */
uint32_t MsgIdBuffer::remove(string & input_string){
	uint32_t 		i;

	// Projedu vsechny polozku v seznamu
	for(i = 0; i < _msg_list.size(); i++){
		// Kontroluju uz existenci dane polozky, tj. exituje uz?
		if(input_string.compare(_msg_list.at(i).msg_id) == 0){
			_msg_list.at(i).cnt = 0;	// Nasel jse, nastavim si, ze toto msg id je uz vyrizene.
			MESSAGE_ID_BUFF_DBG("[MSG_ID_BUFF] remove succ, ID=\"%s\", cntr=%d\n", _msg_list.at(i).msg_id.c_str(), _msg_list.at(i).cnt);
			return 0;
		}
	}
	MESSAGE_ID_BUFF_DBG("[MSG_ID_BUFF] remove failed, ID=\"%s\" not found\n", input_string.c_str());
	return 1;
}


/*
 * @return 0      - 
 * @return 1      - polozka v bufferu neni
 * @return 2      - polozka v bufferu je, ale nebyla vyrizena
 */
uint32_t MsgIdBuffer::state(string & input_string){
	uint32_t 		i;

	// Projedu vsechny polozku v seznamu
	for(i = 0; i < _msg_list.size(); i++){
		MESSAGE_ID_BUFF_DBG("[MSG_ID_BUFF] state: [%d] in=%s, list=%s, cntr=%d\n", i, input_string.c_str(), _msg_list.at(i).msg_id.c_str(), _msg_list.at(i).cnt);

		// Kontroluju uz existenci dane polozky, tj. exituje uz?
		if(input_string.compare(_msg_list.at(i).msg_id) == 0){
			MESSAGE_ID_BUFF_DBG("[MSG_ID_BUFF] state: cntr=%d, index=%d\n", _msg_list.at(i).cnt, i);

			if(_msg_list.at(i).cnt == 0){
				return 0;
			}else{
				return 1;
			}
		}
	}
	MESSAGE_ID_BUFF_DBG("[MSG_ID_BUFF] state: \"ID\" not found, unknown state\n", input_string.c_str());
	return 2;
}




