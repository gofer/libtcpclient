#include <cstdio>
#include <cstring>
#include <errno.h>
#include "tcp_client.hpp"

TCPClient::TCPClient(){
#ifdef __MS_WIN32__
	WSAStartup(MAKEWORD(2,0), &wsaData);
#endif
	_init_flag = false;
	_err_mesg = "No error";
	memset(&_dst_addr_in, 0, sizeof(struct sockaddr_in));
	_dst_addr_in.sin_family = AF_INET;
}

TCPClient::~TCPClient(){
	if(_init_flag) this->disconnect();
}

int TCPClient::create_socket(uint32_t port, const char* ip_addr_str) {
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(_sock == -1) {
		_err_mesg = "Cannot open socket.";
		_err_no = errno;
		return -1;
	}
	
	_dst_addr_in.sin_port = htons((uint16_t)port);
	_dst_addr_in.sin_addr.s_addr = inet_addr(ip_addr_str);
	
	return 0;
}

int TCPClient::connect(uint32_t port, std::string ip_addr_str) {
	return connect(port, ip_addr_str.c_str());
}

int TCPClient::connect(uint32_t port, const char* ip_addr_str) {
	if(this->create_socket(port, ip_addr_str) == -1) {
		return -1;
	}
	
	if(::connect(_sock, (struct sockaddr*)&_dst_addr_in, sizeof(struct sockaddr_in)) == -1) {
		_err_mesg = "Cannot connect socket.";
		_err_no = errno;
		return -1;
	}
	
	_init_flag = true;
	
	return 0;
}

int TCPClient::disconnect() {
#ifdef __GNU_LINUX__
	close(_sock);
#endif
#ifdef __MS_WIN32__
	WSACleanup();
#endif
	_init_flag = false;
	return 0;
}

int TCPClient::send(const std::string buffer, int flags) {
	return send(buffer.c_str(), buffer.length(), flags);
}

int TCPClient::send(const char* buffer, int flags) {
	if(!_init_flag) {
		_err_mesg = "Access before initalized.";
		return -1;
	}
	
	ssize_t buf_size = strlen(buffer), send_size = 0;
	
	while(send_size < buf_size) {
		ssize_t ret = ::send(_sock, buffer, buf_size, flags);
		if(ret == -1) {
			_err_mesg = "Cannot send buffer.";
			_err_no = errno;
			return -1;
		}
		send_size += ret;
	}
	
	return 0;
}

int TCPClient::send(const char* buffer, int length, int flags) {
	if(!_init_flag) {
		_err_mesg = "Access before initalized.";
		return -1;
	}
	
	ssize_t buf_size = length, send_size = 0;
	
	while(send_size < buf_size) {
		ssize_t ret = ::send(_sock, buffer+send_size, buf_size-send_size, flags);
		if(ret == -1) {
			_err_mesg = "Cannot send buffer.";
			_err_no = errno;
			return -1;
		}
		send_size += ret;
	}
	
	return send_size;
}

int TCPClient::recive(char* buffer, int length, int flags) {
	if(!_init_flag) {
		_err_mesg = "Access before initalized.";
		return -1;
	}
	
	ssize_t buf_size = length, recive_size = 0;
	
	while(recive_size < buf_size) {
		ssize_t ret = ::recv(_sock, buffer+recive_size, buf_size-recive_size, flags);
		if(ret == -1) {
			_err_mesg = "Cannot recive buffer.";
			_err_no = errno;
			return -1;
		} else if(ret == 0) {
			break;
		}
		recive_size += ret;
	}
	
	return recive_size;
}

int TCPClient::get_socket() {return _sock;}

// for debug use only
#ifdef __DEBUG__
int TCPClient::get_error_no() {return _err_no;}
std::string TCPClient::get_error_mesg() {return _err_mesg;}
#endif

int hostname2ipaddr(const char* hostname, char* ipaddr) {
	struct addrinfo hints, *response = new struct addrinfo;
	
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	
	int ret = getaddrinfo(hostname, "", &hints, &response);
	if(ret != 0) {
#ifdef __DEBUG__
		fprintf(stderr, "Cannot convert host to IP address.\n");
#endif
		return -1;
	}
	
	char* _addr_str = inet_ntoa(((struct sockaddr_in*)response->ai_addr)->sin_addr);
#ifdef __GNU_LINUX__
	strncpy(ipaddr, _addr_str, strlen(_addr_str)+1);
#endif
#ifdef __MS_WIN32__
	//strncpy_s(ipaddr, _addr_str, strlen(_addr_str)+1);
	strncpy_s(ipaddr, 0xFF, (const char*)_addr_str, strlen(_addr_str)+1);
#endif
	freeaddrinfo(response);
	
	return 0;
}

std::string hostname2ipaddr(const char* hostname) {
	char ipaddr[0xFF];
	if(hostname2ipaddr(hostname, ipaddr) != 0)
		return std::string();
	return std::string(ipaddr);
}

std::string hostname2ipaddr(const std::string hostname) {
	char ipaddr[0xFF];
	if(hostname2ipaddr(hostname.c_str(), ipaddr) != 0)
		return std::string();
	return hostname2ipaddr(ipaddr);
}
