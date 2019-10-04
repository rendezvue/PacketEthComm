#ifndef ETHERNETCLIENT_H
#define ETHERNETCLIENT_H

#include <string>
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <stdint.h>

//#include <qdebug.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#define PORT "4000"
#define IPADDR "192.168.56.1"

using namespace boost::asio;
using boost::asio::ip::tcp;
using namespace std;

class  CEthernetClient
{
public:
	CEthernetClient(void);
	~CEthernetClient(void);

	int Accept(const int port) ;
	//OR
    int Open(const char* ip, unsigned int port);

	void Close();
	void Release(void) ;
	int handle_connect(const boost::system::error_code& err);

	tcp::socket *GetSocketPointer(void) ;

private:
	//int client_socket;
	//SOCKET client_socket;
	//struct sockaddr_in server_addr;
	//boost::asio::io_service m_io_service;
	boost::asio::io_service *m_io_service;
	tcp::socket *m_s;
	tcp::resolver *m_resolver;
	deadline_timer *m_timer;
	tcp::acceptor *m_acceptor ;
	
	int TimeOut = 0;

};

#endif
