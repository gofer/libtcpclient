#include <iostream>
#include <string>
#include <tcp_client.hpp>

void normal_tcp() {
	const uint32_t port = 80;
	const std::string host = "www.google.com";
	const std::string ip_addr = hostname2ipaddr(host.c_str());
	
	TCPClient *tcp_client = new TCPClient();
	tcp_client->connect(
		port,
		ip_addr
	);
	
	tcp_client->send(std::string("GET / HTTP/1.1\nHost: ") + host + "\nConnection: close\n\n");
	
	std::string *message = new std::string();
	const uint32_t buf_size = 0xFFFF;
	char* buf = new char[buf_size+1];
	while(true) {
		uint32_t recv_size = tcp_client->recive(buf, buf_size);
		buf[recv_size] = '\0';
		message->append(buf);
		if(recv_size == 0) break;
	}
	delete buf;
	std::cout << *message << std::endl;
	delete message;
	
	tcp_client->disconnect();
	delete tcp_client;
}

void ssl_tcp() {
	const uint32_t port = 443;
	const std::string host = "www.google.com";
	const std::string ip_addr = hostname2ipaddr(host.c_str());
	
	SSL_TCPClient *ssl_tcp_client = new SSL_TCPClient();
	
	if(ssl_tcp_client->connect(port, ip_addr) != 0) return;
	
	ssl_tcp_client->send(std::string("GET / HTTP/1.1\nHost: ") + host + "\nConnection: close\n\n");
	
	std::string *message = new std::string();
	const uint32_t buf_size = 0xFFFF;
	char* buf = new char[buf_size+1];
	while(true) {
		uint32_t recv_size = ssl_tcp_client->recive(buf, buf_size);
		buf[recv_size] = '\0';
		message->append(buf);
		if(recv_size == 0) break;
	}
	delete buf;
	std::cout << *message << std::endl;
	delete message;
	
	ssl_tcp_client->disconnect();
	delete ssl_tcp_client;
}

int main(void) {
	normal_tcp();
	
	ssl_tcp();
	
	return 0;
}
