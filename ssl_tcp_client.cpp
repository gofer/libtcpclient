#include <cstdio>
#include <cstring>
#include <errno.h>
#include "tcp_client.hpp"

SSL_TCPClient::SSL_TCPClient() {
	_ssl_init_flag = false;
	SSL_library_init();
}

SSL_TCPClient::~SSL_TCPClient() {
	if(_init_flag) disconnect();
	if(!_ssl_init_flag) _ssl_clear();
}

int SSL_TCPClient::_ssl_init() {
	if(_ssl_init_flag) return -1;
	
	_ssl_ctx = SSL_CTX_new( SSLv23_client_method() );
	if (_ssl_ctx == NULL) {
		return 1;
	}
	SSL_CTX_set_options(_ssl_ctx, SSL_OP_NO_SSLv2);
	
	_ssl = SSL_new(_ssl_ctx);
	if (_ssl == NULL) {
		SSL_CTX_free(_ssl_ctx);
		return 1;
	}
	SSL_set_options(_ssl, SSL_OP_NO_SSLv2);
	
	_ssl_init_flag = true;
	
	return 0;
}

int SSL_TCPClient::_ssl_clear() {
	if(!_ssl_init_flag) return -1;
	
	SSL_free(_ssl); 
	SSL_CTX_free(_ssl_ctx);
	_ssl_init_flag = false;
	
	return 0;
}

int SSL_TCPClient::connect(uint32_t port, std::string ip_addr_str) {
	return connect(port, ip_addr_str.c_str());
}

int SSL_TCPClient::connect(uint32_t port, const char* ip_addr_str) {
	if(create_socket(port, ip_addr_str) == -1) {
		return -1;
	}
	
	if(::connect(_sock, (struct sockaddr*)&_dst_addr_in, sizeof(struct sockaddr_in)) == -1) {
		_err_mesg = "Cannot connect socket.";
		_err_no = errno;
		return -1;
	}
	
	if(_ssl_init() != 0) return -1;
	
	if(SSL_set_fd(_ssl, _sock) == 0) {
		_err_mesg = "SSL_set_fd() error.";
		_err_no = errno;
		return -1;
	}
	
	if(SSL_connect(_ssl) != 1) {
		_err_mesg = "Cannot connect with SSL.";
		_err_no = errno;
		return -1;
	}
	
	//::printf("SSL Cipher    : %s\n", SSL_get_cipher(_ssl));
	//::printf("SSL Cipher Ver: %s\n", SSL_get_cipher_version(_ssl));
	
	_init_flag = true;
	
	return 0;
}

int SSL_TCPClient::disconnect() {
	if(!_ssl_init_flag) {
		SSL_shutdown(_ssl);
		_ssl_clear();
	}
	
#ifdef __GNU_LINUX__
	close(_sock);
#endif
#ifdef __MS_WIN32__
	WSACleanup();
#endif
	_init_flag = false;
	return 0;
}

int SSL_TCPClient::send(const std::string buffer) {
	return send(buffer.c_str(), buffer.size());
}

int SSL_TCPClient::send(const char* buffer, int length) {
	if(!_ssl_init_flag) return -1;
	if(!    _init_flag) return -1;
	return SSL_write(_ssl, buffer, length);
}

int SSL_TCPClient::recive(char* buffer, int length) {
	if(!_ssl_init_flag) return -1;
	if(!    _init_flag) return -1;
	return SSL_read(_ssl, buffer, length);
}
