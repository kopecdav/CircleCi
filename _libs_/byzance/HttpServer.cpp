#include "HttpServer.h"

/*
 * Pointery na fyzickou vrstvu wifiny a ethernetu a prepinac, kterym reknu, co se ma pouzit
 */
NetworkInterface*		HttpServer::_itf;

TCPSocket*				HttpServer::_http_socket;
TCPServer*				HttpServer::_http_server;
ExtMem*					HttpServer::_extmem;

char*					HttpServer::_buffer;

Thread*					HttpServer::_thread_http;

#define THREAD_SIGNAL_SERVER_EVENT		0x01
#define THREAD_SIGNAL_SOCKET_EVENT		0x02

#define HTTP_SERVER_STREAM_BUFFER_SIZE	16
#define HTTP_SERVER_REQUEST_LIMIT		1024

REGISTER_DEBUG(HttpServer);

void HttpServer::init(ExtMem* extmem) {

	HttpServer::_extmem = extmem;
	// todo: udelat pres malloc a vracet navratovou hodnotu init funkci, jestli se malloc podaril
	// todo: define velikosti bufferu
	_buffer = new char[HTTP_SERVER_STREAM_BUFFER_SIZE];

	// vlákno zatím vždy využilo max 1152 bytů 20.6. 2017
	// todo: pořešit priority vláken
	HttpServer::_thread_http	= new Thread(osPriorityLow, 2048,NULL,"HTTP");

	HTMLParser::init(extmem);
}

void HttpServer::init_itf(NetworkInterface* itf) {
	HttpServer::_itf = itf;
}

void HttpServer::run() {
	if(_itf == NULL){
		__INFO("running HTTP server before calling init_itf\n");
		return;
	}
	if(_thread_http == NULL){
		__ERROR("running HTTP server before calling init\n");
		return;
	}
	HttpServer::_thread_http->start(&HttpServer::_thread_http_function);
}

void HttpServer::stop() {
	HttpServer::_thread_http->terminate();
}

void HttpServer::_thread_http_function() {
	int rc;
	__INFO("pustil se HTTP thread\n");

	if (_http_socket == NULL) {
		_http_socket = new TCPSocket(_itf);
	}

	if (_http_server == NULL) {
		_http_server = new TCPServer(_itf);
	}

	_http_socket->set_blocking(true);
	_http_socket->set_timeout(20);

	//_http_socket->sigio(&_state_changed_sock);

	uint32_t tmp32 = 0;
	uint8_t tries=0;
	do{
		rc = Configurator::get_webport(&tmp32);
		tries++;
	} while(rc!=CONFIG_OK && tries < 5);
	if(rc==-1){
		__ERROR("could not lock config 5 times, server not initiated\n");
		return;
	}
	if (_http_server->bind((uint16_t) tmp32)) {
		__ERROR("bind at port %d failed\n", tmp32);
		return;
	} else {
		__LOG("bind at port %d ok\n", tmp32);
	}

	if (_http_server->listen(10) < 0) {
		__ERROR("tcp server listen failed.\n");
		return;
	} else {
		__INFO("tcp server is listening...\n");
	}

	// timeout musi byt dostatecne dlouhy, aby se request stihl vycist
	// pokud je MCU vyuzity, trva to klidne sekundu (a je to blokujici), coz zbytecne zere procesor a zdrzuje uzivatele
	// pokud timeout vyprsi, funkce accept vraci casto chybu -3001

	// todo: bylo by dobre to predelat na neblokujici a attachnout callback pri zmene stavu k http_server socketu
	// ktery by nastavil vlaknu signal a vlakno by si zpravu vycetlo
	// misto kretenskeho vycitani v cyklu furt dokola
	_http_server->set_blocking(false);
	//_http_server->set_timeout(2000);

	_http_server->sigio(&_state_changed);

	__TRACE("server prosel skrz inicializaci\n");



	string request;
	string response_data;
	string response_header;

	while (true) {

		__DEBUG("ceka se na prichozi pozadavek\n");

		/*
		 * Pokusi se zjistit, jestli je na ETH nejaky pozadavek a pripadne ho vyzvedne
		 */

		osEvent evt = Thread::signal_wait(0, osWaitForever);
		if ((evt.value.signals & THREAD_SIGNAL_SERVER_EVENT) && evt.status == osEventSignal){

			__DEBUG("server signal processing\n", rc);

			rc = _http_server->accept(_http_socket);
			if (!rc) {

				__DEBUG("server accept ok\n", rc);

				request.clear();

				while(1){
					rc = _http_socket->recv(_buffer, HTTP_SERVER_STREAM_BUFFER_SIZE);

					// some data to read
					if(rc>0){

						// request is too big
						if((request.size()+rc)>HTTP_SERVER_REQUEST_LIMIT){
							__DEBUG("too big (>%d)\n", HTTP_SERVER_REQUEST_LIMIT);
							request.clear();
							break;

						// request size is fine
						} else {
							request.append(_buffer, 0, rc);
							//__DEBUG("accepted, total size = %d\n", request.size());
						}

					// no data to read or error
					} else {
						//__DEBUG("no data red (err=%d)\n", rc);
						break;
					}
				}

				if (request.size()) {
					//request.assign(_buffer, rc);
					__DEBUG("request accepted, size = %d\n", request.size());

					struct_http_request request_str;

					string arg;
					bool rc;

					rc = _find_label(request, "GET ", arg);
					if(rc){
						request_str.method = HTTP_METHOD_GET;
						strcpy(request_str.url, arg.c_str());
					}

					rc = _find_label(request, "POST ", arg);
					if(rc){
						request_str.method = HTTP_METHOD_POST;
						strcpy(request_str.url, arg.c_str());
					}

					// assign unknown when not set
					if(!request_str.method){
						request_str.method = HTTP_METHOD_UNKNOWN;
					}

					rc = _find_label(request, "Content-Length: ", arg);
					if(rc){
						__TRACE("label content lenght found, arg=%s\n", arg.c_str());
						request_str.length = atoi(arg.c_str());
					} else {
						__TRACE("label content lenght missing\n");
						request_str.length = 0;
					}

					rc = _find_label(request, "Content-Type: ", arg);
					if(rc){

						__TRACE("label content type found, arg=%s\n", arg.c_str());

						if(arg.compare("application/x-www-form-urlencoded")){
							request_str.type = HTTP_TYPE_APPLICATION_X_WWW_FORM_URLENCODED;
						} else {
							request_str.type = HTTP_TYPE_UNKNOWN;
						}

					} else {
						__TRACE("label content type missing\n");
						request_str.type = HTTP_TYPE_UNKNOWN;
					}

					_print_request(&request_str);

					HttpError_t code;
					code = HTMLParser::parse(request_str.url, response_data);
					_create_header(code, response_header, response_data.size());

					__LOG("responding code=%d, HEADER=%d bytes, DATA=%d bytes\n", code, response_header.size(), response_data.size());

					_send_response(response_header);
					_send_response(response_data);

					response_data.clear();
					response_header.clear();

				} else {
					__WARNING("request invalid\n");
				}

				rc = _http_socket->close();
				if(rc){
					__ERROR("socket close error = %d\n", rc);
				}

			} else {
				__ERROR("accept error = %d\n", rc);
			}
		}
	}
}

bool HttpServer::_find_label(string& request, const char* label, string& result){

	std::size_t pos;
	std::size_t end;
	//string arg;
	//uint32_t i = 0;

	pos = request.find(label);
	if(pos!= string::npos){
		pos += strlen(label);
		//__TRACE("_find: content-len arg begins at pos=%d\n", pos);

		// find \n from starting position
		end = request.find(" ", pos);
		if(end!= string::npos){
			//__TRACE("_find: content-len ends at pos=%d\n", end);
			result.assign(request, pos, end-pos);
			return true;
		} else {
			//__TRACE("_find: content-len end missing\n");
		}

	} else {
		//__TRACE("_find: content-len unknown\n");
	}

	return false;
}

void HttpServer::_create_header(HttpError_t code, string& header, size_t len){

	char buff[8];

	header.clear();

	header.assign("HTTP/1.1 ");

	if (code == HTTP_200_OK) {
		header+="200 OK\r\n";
	} else if (code == HTTP_404_NOT_FOUND) {
		// vyrobim hlavicku
		header+="404 Not Found\r\n";
	}

	header+="Content-Length: ";
	sprintf(buff, "%d", len);
	header+=buff;
	header+="\r\nContent-Type: text/html\r\nConnection: Close\r\n\r\n";

}

void HttpServer::_send_response(string& data) {

	uint32_t tmp32 = 0;

	int rc = Configurator::get_netsource(&tmp32);
	if(rc){
		__ERROR("could not get netsource from Configurator rc = %d\n",rc);
		return;
	}
	// posle se odpoved
	if (tmp32 == NETSOURCE_ETHERNET) {
		int rc = 0;
		unsigned int remaining;
		unsigned int sent = 0;

		remaining = data.size();

		Timer t;
		t.start();

		// for some reason library sent only 1072 bytes if buffer was bigger
		while(remaining>0){

			rc = _http_socket->send((const char*) data.substr(sent).data(), remaining);
			// sent successfully
			if(rc>0){
				sent += rc;
				remaining -= rc;
				__DEBUG("rc=%d, total sent %d bytes, remaining %d bytes\n", rc, sent, remaining);
			} else {
				//__ERROR("response send failed, err=%d\n", rc);
			}

			if(t.read_ms()>1000){
				__ERROR("response send timeout\n");
				break;
			}
		}

		__DEBUG("response send in %d ms\n", t.read_ms());

	}

}

uint32_t HttpServer::get_stack_size(void) {
	return _thread_http->stack_size();
}

uint32_t HttpServer::get_free_stack(void) {
	return _thread_http->free_stack();
}

uint32_t HttpServer::get_used_stack(void) {
	return _thread_http->used_stack();
}

uint32_t HttpServer::get_max_stack(void) {
	return _thread_http->max_stack();
}

Thread::State HttpServer::get_state(void){
	return _thread_http->get_state();
}

osPriority HttpServer::get_priority(void){
	return _thread_http->get_priority();
}

void HttpServer::_print_request(struct_http_request* r){

	__TRACE("start\n");

	switch(r->method){
	case  HTTP_METHOD_GET:
		__TRACE("metod GET\n");
		break;
	case HTTP_METHOD_POST:
		__TRACE("metod POST\n");
		break;
	default:
		__TRACE("metod UNKNOWN\n");
	}

	__TRACE("URL = %s\n", r->url);

	__TRACE("length = %u\n", r->length);

	switch(r->type){
	case HTTP_TYPE_APPLICATION_X_WWW_FORM_URLENCODED:
		__TRACE("type APPLICATION_X_WWW_FORM_URLENCODED\n");
		break;
	default:
		__TRACE("type unknown\n");
	}

	__TRACE("end\n");

}

void HttpServer::_state_changed(void){
	//SERVER_INFO("server: new server event\n");
	_thread_http->signal_set(THREAD_SIGNAL_SERVER_EVENT);

}
