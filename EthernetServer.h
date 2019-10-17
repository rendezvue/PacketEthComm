#ifndef ETHERNETSERVER_H
#define ETHERNETSERVER_H

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

#include "EthernetGetInfo.h" ;

#define PORT "4000"

using namespace boost::asio;
using boost::asio::ip::tcp;
using namespace std;

class  CEthernetServer
{
public:
	static CEthernetServer* getInstance();
	virtual ~CEthernetServer() ;
	
	int Accept(tcp::socket *p_socket) ;

	void Close();
	void Release(void) ;
	int handle_connect(const boost::system::error_code& err);

private:
	CEthernetServer(void) ;
	static bool instanceFlag;
	static CEthernetServer* instance;
	
	//int client_socket;
	//SOCKET client_socket;
	//struct sockaddr_in server_addr;
	//boost::asio::io_service m_io_service;
	tcp::acceptor *m_acceptor ; 
	deadline_timer *m_timer;
	
	int TimeOut = 0;

};

#endif
