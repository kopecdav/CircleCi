#include "MQTTSocket.h"

REGISTER_DEBUG(MQTTSocket);

Callback<void(void)>	MQTTSocket::_socket_changed_callback;

MQTTSocket::MQTTSocket(){
	connected = 0;
}

MQTTSocket::~MQTTSocket(){
	mysock->close();
	delete mysock;
}

void MQTTSocket::init(NetworkInterface *itf) {

	mysock = new TCPSocket(itf);

	mysock->sigio(&change);
}

int MQTTSocket::connect(char* hostname, int port, int timeout) {

	int rc = -1;

	//mysock->set_blocking(false);
	mysock->set_blocking(true);
	mysock->set_timeout(timeout);

	// 0 on success, negative error code on failure
	rc = mysock->connect(hostname, port);
	if(rc==0){
		connected = 1;
		mysock->set_blocking(false);
	} else {
		connected = 0;
	}

	mysock->set_blocking(false);

	return rc;

}

int MQTTSocket::connect(const SocketAddress &address, int timeout) {
	int rc = -1;

	//mysock->set_blocking(false);
	mysock->set_blocking(true);
	mysock->set_timeout(timeout);

	// 0 on success, negative error code on failure
	rc = mysock->connect(address);
	if(rc==0){
		connected = 1;
		mysock->set_blocking(false);
	} else {
		connected = 0;
	}

	mysock->set_blocking(false);

	return rc;
}

int MQTTSocket::read(unsigned char* buffer, int len, int timeout) {

	//mysock->set_blocking(false);
	//mysock->set_blocking(true);
	//mysock->set_timeout(timeout);
	__TRACE("trying to read %d in %d ms\n",len,timeout);
	int rc=-1;
	int bytes=0;
	if(timeout <= 1){
		return mysock->recv((char*)buffer, len);;
	}
	Timer t;
	t.start();
	do{
		//try to recv
		rc = mysock->recv((char*)buffer+bytes, len-bytes);
		if(rc < 0){	//in case of error
			if (rc != NSAPI_ERROR_WOULD_BLOCK)
			{	//error - other error
				__ERROR("socket read error %d\n",rc);
				bytes = rc;
				break;
			}else{	//error - would block
				__TRACE("reading would block, waiting\n");
				Thread::wait(50);	//wait and try it once again
			}
		}else{	//data were read
			__INFO("read %d bytes in time %d\n",rc,t.read_ms());
			bytes += rc;
		}
		if(t.read_ms() > timeout){	//check timeout
			__ERROR("reading timeout expired in %d from %d\n",t.read_ms(),timeout);
			break;
		}
	}while(bytes < len);
	__TRACE("returning %d\n",bytes);
	return bytes;	//return num of bytes read or error code
}

int MQTTSocket::write(unsigned char* buffer, int len, int timeout) {

	//mysock->set_blocking(false);
	//mysock->set_blocking(true);
	//mysock->set_timeout(timeout);

	__TRACE("trying to write %d in %d ms\n",len,timeout);
	int rc=-1;
	int bytes=0;
	if(timeout <= 1){
		return mysock->send((char*)buffer, len);;
	}
	Timer t;
	t.start();
	do{
		rc = mysock->send((char*)buffer+bytes, len-bytes);
		if(rc < 0){
			if (rc != NSAPI_ERROR_WOULD_BLOCK)
			{
				__ERROR("socket send error %d\n",rc);
				bytes = rc;
				break;
			}else{
				__TRACE("sending would block, waiting\n");
				Thread::wait(50);
			}
		}else{
			__INFO("sending %d bytes in time %d\n",bytes,t.read_ms());
			bytes += rc;
		}
		if(t.read_ms() > timeout){
			__ERROR("sending %d from %d bytes timeout expired in %d from %d\n",bytes,len,t.read_ms(),timeout);
			break;
		}
	}while(bytes < len);

	return bytes;

}

bool MQTTSocket::is_connected(){
      return connected;
}

int MQTTSocket::disconnect(){

	int rc = 0;

	//0 on success, negative error code on failure
	rc = mysock->close();

	if(!rc) connected = 0;

	return rc;
}

int MQTTSocket::bind(uint16_t port){
  return mysock->bind(port);
}

void MQTTSocket::change(void){

	if(_socket_changed_callback){
		_socket_changed_callback.call();
	}

}

void MQTTSocket::attach_changed(void (*function)(void)){

	_socket_changed_callback = callback(function);
}
