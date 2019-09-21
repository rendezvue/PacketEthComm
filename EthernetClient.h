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

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include "RendezvueCheckData.h"

#define PORT "4000"
#define IPADDR "192.168.56.1"

#define DEFAULT_BUFLEN 4096
#define DEFAULT_BUFLEN2 1024

//#define COMMAND_BUF_SIZE	1024
#define DATA_BUF_SIZE	1000000*100		//100Mega //////20Mega * 3(RGB) = 60Mega

using namespace boost::asio;
using boost::asio::ip::tcp;
using namespace std;

class  CEthernetClient
{
public:
	CEthernetClient(void);
	~CEthernetClient(void);

	int Send(const unsigned int command, const std::string id, std::vector<float> *p_vec_send_data);
	int Receive(const unsigned int command, std::vector<float>* out_receive_data) ;

	int SendImage(const unsigned int command, const int width, const int height, const int image_type, unsigned char* image_buf, const int buf_len);
	int ReceiveImage(const unsigned int command, int& width, int& height, unsigned char** out_data) ;
	
	int Send(unsigned int command, unsigned char* send_data, const unsigned int send_data_size, const unsigned int send_scalefactor, unsigned char** out_data, int* out_data_size, unsigned int* out_scalefactor);
    int Open(const char* ip, unsigned int port);
	void Close();
	int handle_connect(const boost::system::error_code& err);

private:
	//int client_socket;
	//SOCKET client_socket;
	//struct sockaddr_in server_addr;
	//boost::asio::io_service m_io_service;
	boost::asio::io_service *m_io_service;
	tcp::socket *m_s;
	tcp::resolver *m_resolver;
	deadline_timer *m_timer;

	//char *m_buf;
	unsigned char m_buf[DEFAULT_BUFLEN2];

	//CCheckData m_cls_check_data;
	CRendezvueCheckData m_cls_check_data;

	int TimeOut = 0;

	unsigned char *m_p_command ;
	unsigned int m_ui_command_size ;
};

#endif
