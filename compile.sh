#!/bin/sh
g++ -c tcp_client.cpp ssl_tcp_client.cpp -std=c++11 \
	-lssl -lcrypto -ltcpclient -lpthread -lstdc++
ar rsv libtcpclient.a tcp_client.o ssl_tcp_client.o
rm -f tcp_client.o     > /dev/null
rm -f ssl_tcp_client.o > /dev/null
