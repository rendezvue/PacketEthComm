#include "EthernetClient.h"
#include "ErrorType.h"
#include <string>

CEthernetClient::CEthernetClient(void) :
	m_s(NULL)
	,m_resolver(NULL)
	, m_p_command(NULL)
	, m_acceptor(NULL)
	, m_io_service(NULL)
	, m_ui_command_size(0)

{

	m_ui_command_size = DATA_BUF_SIZE ;
	m_p_command = new unsigned char[m_ui_command_size] ;
	
	//m_s = new tcp::socket(m_io_service);
	//m_resolver = new tcp::resolver(m_io_service);

	//boost::asio::connect(*m_s, m_resolver->resolve({ ip, port }));
#if 0
	client_socket = INVALID_SOCKET;

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
	{
		printf("error WSAStartup\n");
	}

	client_socket = socket(PF_INET, SOCK_STREAM, 0);

	if (client_socket == -1) {
		printf("[eyedea] Socket Open failed\n");
	}
	else {
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(PORT);
		//server_addr.sin_addr.s_addr = inet_addr(IPADDR);
		server_addr.sin_addr.s_addr = inet_addr(IPADDR);

		if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
			printf("[eyedea] Socket connect error\n");
			closesocket(client_socket);
		}
	}
#endif
}

CEthernetClient::~CEthernetClient(void)
{
	if(m_p_command != NULL )
	{
		delete m_p_command ;
		m_p_command = NULL ;
	}

	Release() ;
}

void CEthernetClient::Release(void) 
{
	if (m_s != NULL)
	{
		delete m_s;
		m_s = NULL;
	}

	if( m_acceptor != NULL )
	{
		delete m_acceptor ;
		m_acceptor = NULL;
	}

	if (m_resolver != NULL)
	{
		delete m_resolver;
		m_resolver = NULL;
	}

	if (m_timer != NULL)
	{
		delete m_timer;
		m_timer = NULL;
	}

	if (m_io_service != NULL)
	{
		delete m_io_service;
		m_io_service = NULL;
	}
}

int CEthernetClient::Accept(const int port) 
{
	if( m_s )
	{
		printf("Already Connect\n") ;
		return ENSEMBLE_ERROR_ALREADY_CONNECT ;
	}
	
	m_io_service = new io_service();
	m_s = new tcp::socket(*m_io_service);
	m_resolver = new tcp::resolver(*m_io_service);
	m_timer = new deadline_timer(*m_io_service);
	m_acceptor = new tcp::acceptor(*m_io_service, tcp::endpoint(tcp::v4(), port));

	//a.accept((*m_s));
	
	try
	{
#if 0	
		m_acceptor->async_accept((*m_s), boost::bind(&CEthernetClient::handle_connect, this, boost::asio::placeholders::error)) ;

		m_timer->expires_from_now(boost::posix_time::seconds(3));
		m_timer->async_wait(boost::bind(&CEthernetClient::Close, this));

		do {
			m_io_service->run_one();
		} while (ec == boost::asio::error::would_block);
		if (ec || !m_s->is_open() || TimeOut == 1)
		{
			//cout << "error happend in socket connect" << endl;
			m_s->close();
			Release() ;

			TimeOut = 0;

			return ENSEMBLE_ERROR_SOCKET_CONNECT;
		}
		m_timer->cancel();
#else
		printf("Waiting Client..\n");
		m_acceptor->accept((*m_s)) ;		//inf
		cout << "Connection IP : " <<  m_s->remote_endpoint().address().to_string() << endl;
#endif
	}
	catch (boost::system::system_error const &e)
	{
		//cout << "Warning : could not connect : " << e.what() << endl;
		//Close();
		Release() ;
		
		return ENSEMBLE_ERROR_SOCKET_CONNECT;
	}

	return ENSEMBLE_SUCCESS;
}

int CEthernetClient::Open(const char* ip, unsigned int port)
{
	boost::system::error_code ec;

	if( m_s )
	{
		printf("Already Connect\n") ;
		return ENSEMBLE_ERROR_ALREADY_CONNECT ;
	}

	m_io_service = new io_service();
	m_s = new tcp::socket(*m_io_service);
	m_resolver = new tcp::resolver(*m_io_service);
	m_timer = new deadline_timer(*m_io_service);


	//boost::asio::connect(*m_s, m_resolver->resolve({ ip, "4000" }), ec);
	//boost::asio::connect(*m_s, m_resolver->resolve({ ip, std::to_string(port) }), ec);
	try
	{
		m_s->async_connect(tcp::endpoint(ip::address::from_string(ip), port), boost::bind(&CEthernetClient::handle_connect, this, boost::asio::placeholders::error));
		//m_s->async_connect(tcp::endpoint(ip::address::from_string(ip), port), boost::bind(&CEthernetClient::handle_connect));

		m_timer->expires_from_now(boost::posix_time::seconds(3));
		m_timer->async_wait(boost::bind(&CEthernetClient::Close, this));

		do {
			m_io_service->run_one();
		} while (ec == boost::asio::error::would_block);
		if (ec || !m_s->is_open() || TimeOut == 1)
		{
			//cout << "error happend in socket connect" << endl;
			m_s->close();
			Release() ;

			TimeOut = 0;

			return ENSEMBLE_ERROR_SOCKET_CONNECT;
		}
		m_timer->cancel();
	}
	catch (boost::system::system_error const &e)
	{
		//cout << "Warning : could not connect : " << e.what() << endl;
		//Close();
		Release() ;
		
		return ENSEMBLE_ERROR_SOCKET_CONNECT;
	}

	return ENSEMBLE_SUCCESS;

}

int CEthernetClient::handle_connect(const boost::system::error_code& err)
{
	//cout << "handle_connect call " << err << endl;

	if (err)
	{
		//fail
		throw boost::system::system_error(
			err ? err : boost::asio::error::operation_aborted);
		return -1;
	}
	else
	{
		//success
		return 1;
	}
}


void CEthernetClient::Close()
{
#if 0
	closesocket(client_socket);
#endif

	//cout << "close call" << endl;
	//m_s->cancel();
	TimeOut = 1;
	/*
	m_s->close();
	delete m_timer;
	//delete m_s;
	delete m_resolver;
	delete m_io_service;
	m_timer = NULL;
	//m_s = NULL;
	m_resolver = NULL;
	m_io_service = NULL;
	*/
}

#if 0
int CEthernetClient::Send(unsigned int command, unsigned char* send_data, const unsigned int send_data_size, const unsigned int send_scalefactor, unsigned char** out_data, int* out_data_size, unsigned int* out_scalefactor )
{
	if (m_s == NULL)
	{
		*out_data_size = 0;
		return ENSEMBLE_ERROR_INVALID_MEMORY;
	}

	int cnt = 0;
	//char recvbuf[DEFAULT_BUFLEN];
	//char buf[DEFAULT_BUFLEN];
	//if ((*out_data) == NULL)
	//{
	//	return;
	//}

	//cout << "data send" << ":" << command << " " << sizeof(command) << endl;
	//write(client_socket, &command, sizeof(command));

	unsigned int leng = send_data_size;
	if (leng <= 0) leng = 0;
	
	unsigned int index = 0 ;
	m_p_command[index++] = '[';
	m_p_command[index++] = 'R';
	m_p_command[index++] = 'D';
	m_p_command[index++] = 'V';

	m_p_command[index++] = (command & 0xFF000000) >> 24;
	m_p_command[index++] = (command & 0x00FF0000) >> 16;
	m_p_command[index++] = (command & 0x0000FF00) >> 8;
	m_p_command[index++] = (command & 0x000000FF);

	//scale factor
	m_p_command[index++] = (unsigned char)(((send_scalefactor) & 0xFF000000) >> 24);
	m_p_command[index++] = (unsigned char)(((send_scalefactor) & 0x00FF0000) >> 16);
	m_p_command[index++] = (unsigned char)(((send_scalefactor) & 0x0000FF00) >> 8);
	m_p_command[index++] = (unsigned char)(((send_scalefactor) & 0x000000FF));

	m_p_command[index++] = (unsigned char)((leng & 0xFF000000) >> 24);
	m_p_command[index++] = (unsigned char)((leng & 0x00FF0000) >> 16);
	m_p_command[index++] = (unsigned char)((leng & 0x0000FF00) >> 8);
	m_p_command[index++] = (unsigned char)((leng & 0x000000FF));

	if (leng > 0)
	{
        memcpy(m_p_command + index, (send_data), leng);
        index += leng ;
	}

	m_p_command[index++] = 'E';
	m_p_command[index++] = 'S';
	m_p_command[index++] = 'B';
	m_p_command[index++] = ']';
    //m_p_command[index] = 0;

	//send(client_socket, &command, sizeof(command), 0);
	//printf("sizeof(m_command) = %d\n", sizeof(m_command));

#if 0
	send(client_socket, m_command, sizeof(char)*(2+(*len)), 0);
#else
	//printf("send len = %d\n", sizeof(char)*(2 + (*len)));
	//printf("m_command[0] = %d\n", m_command[0]);
	//printf("m_command[1] = %d\n", m_command[1]);
    try{
   		boost::asio::write(*m_s, boost::asio::buffer(m_p_command, sizeof(unsigned char)*(index)));
    }
    catch(exception& e)
    {
    	m_s->close();
    	m_resolver = NULL;
        return ENSEMBLE_ERROR_SOCKET_WRITE;
    }

#endif

    //int get_size = 0;
	
	//unsigned char get_data[5];
	//int get_data_index = 0;

    unsigned char* buf = NULL;
	int buf_size = 0 ;

	int get_command = -1;
	index = 0 ;
	unsigned int i_get_data = 0 ;

	//-------------------------------------------------------------------------------------------
	// 1. Get Command(Check Command)
	//-----
	m_cls_check_data.init_variable();
	do
	{
        //int rev_count = 0 ;
		do
		{
			//cnt = recv(client_socket, m_buf, DEFAULT_BUFLEN, 0);
			//cnt = recv(client_socket, m_buf, DEFAULT_BUFLEN, 0);
            try{
			    boost::system::error_code error;
				//cnt = m_s->read_some(boost::asio::buffer(m_buf), error);
    			//cnt = m_s->read_some(boost::asio::buffer(m_buf), error);
                //qDebug("receive") ;
			
	    		cnt = m_s->receive(boost::asio::buffer(m_buf));
					
                //printf("receive size = %d(%d)\n", cnt, rev_count++) ;
		    	//cnt = m_s->async_read_some(boost::asio::buffer(m_buf), error);

				if (error == boost::asio::error::eof)
    				break; // Connection closed cleanly by peer.
	    		else if (error)
		    		throw boost::system::system_error(error); // Some other error.
            }
            catch(exception& e)
            {
            	m_s->close();
            	m_resolver = NULL;
                m_cls_check_data.init_variable();
                return ENSEMBLE_ERROR_SOCKET_READ;
            }
			
			//int out_size = 0;
            buf = m_cls_check_data.FindData(m_buf, cnt, &buf_size);
		} while (buf == NULL);

		index = 0 ;
		i_get_data = (unsigned int)buf[index++];
		get_command = (i_get_data << 24) & 0xFF000000;
		i_get_data = (unsigned int)buf[index++];
		get_command |= (i_get_data << 16) & 0x00FF0000;
		i_get_data = (unsigned int)buf[index++];
		get_command |= (i_get_data << 8) & 0x0000FF00;
		i_get_data = (unsigned int)buf[index++];
		get_command |= (i_get_data) & 0x000000FF;
	
	} while (get_command != command);
	
	//-------------------------------------------------------------
	//head data

	//-------------------------------------------------------------------------------------------
	// 2. Scale Factor
	//-----
	unsigned int scale_factor = 0 ;
	i_get_data = (unsigned int)buf[index++];
	scale_factor = (i_get_data << 24) & 0xFF000000;
	i_get_data = (unsigned int)buf[index++];
	scale_factor |= (i_get_data << 16) & 0x00FF0000;
	i_get_data = (unsigned int)buf[index++];
	scale_factor |= (i_get_data << 8) & 0x0000FF00;
	i_get_data = (unsigned int)buf[index++];
	scale_factor |= (i_get_data) & 0x000000FF;

	//-------------------------------------------------------------------------------------------
	// 3. data length
	//-----
	unsigned int data_length = 0 ;
	i_get_data = (unsigned int)buf[index++];
	data_length = (i_get_data << 24) & 0xFF000000;
	i_get_data = (unsigned int)buf[index++];
	data_length |= (i_get_data << 16) & 0x00FF0000;
	i_get_data = (unsigned int)buf[index++];
	data_length |= (i_get_data << 8) & 0x0000FF00;
	i_get_data = (unsigned int)buf[index++];
	data_length |= (i_get_data) & 0x000000FF;
	//head data
	
	//-------------------------------------------------------------

	//-------------------------------------------------------------------------------------------
	// 4. data
	//-----
	#if 1
	if( data_length > 0 )
	{
		if( out_data != NULL )
		{
			if( (*out_data) == NULL )
			{
				(*out_data) = new unsigned char[data_length];
			}
		}
	}
	#endif

	//printf("get_command = %d\n", get_command);

	//-------------------------------------------------------------------------------------------
	// 5. out variable
	//-----
    if (data_length > 0)
    {
    	if( out_data != NULL )
		{
	        if( (*out_data) != NULL )
	        {
	            memcpy((*out_data), buf+index, data_length);
	        }
    	}
    }

	if( out_data_size != NULL )	*out_data_size = data_length;
	if( out_scalefactor != NULL )	*out_scalefactor = scale_factor;

	m_cls_check_data.init_variable();
	
    return ENSEMBLE_SUCCESS;
}
#else
int CEthernetClient::Send(const unsigned int command, const std::string id, std::vector<float> *p_vec_send_data)
{
	if (m_s == NULL)
	{
		return ENSEMBLE_ERROR_INVALID_MEMORY;
	}

	int cnt = 0;

	int data_count = 0 ;
	if( p_vec_send_data )	data_count = p_vec_send_data->size() ;
	unsigned int leng = data_count * 4 ;
	if (leng <= 0) leng = 0;
	
	unsigned int index = 0 ;
	m_p_command[index++] = '[';
	m_p_command[index++] = 'R';
	m_p_command[index++] = 'D';
	m_p_command[index++] = 'V';

	m_p_command[index++] = (command & 0xFF000000) >> 24;
	m_p_command[index++] = (command & 0x00FF0000) >> 16;
	m_p_command[index++] = (command & 0x0000FF00) >> 8;
	m_p_command[index++] = (command & 0x000000FF);

	//ID length
	const int i_id_length = id.size() ;
	m_p_command[index++] = (i_id_length & 0xFF000000) >> 24;
	m_p_command[index++] = (i_id_length & 0x00FF0000) >> 16;
	m_p_command[index++] = (i_id_length & 0x0000FF00) >> 8;
	m_p_command[index++] = (i_id_length & 0x000000FF);

	if( i_id_length > 0 )
	{
		strcpy((char*)(m_p_command + index), id.c_str()) ;
		index += i_id_length ;
	}

	//scale factor
	const int scale_factor = 1000 ;
	m_p_command[index++] = (unsigned char)(((scale_factor) & 0xFF000000) >> 24);
	m_p_command[index++] = (unsigned char)(((scale_factor) & 0x00FF0000) >> 16);
	m_p_command[index++] = (unsigned char)(((scale_factor) & 0x0000FF00) >> 8);
	m_p_command[index++] = (unsigned char)(((scale_factor) & 0x000000FF));

	//data size
	m_p_command[index++] = (unsigned char)((data_count & 0xFF000000) >> 24);
	m_p_command[index++] = (unsigned char)((data_count & 0x00FF0000) >> 16);
	m_p_command[index++] = (unsigned char)((data_count & 0x0000FF00) >> 8);
	m_p_command[index++] = (unsigned char)((data_count & 0x000000FF));

	//data ;
	for( int i=0 ; i<data_count ; i++ )
	{
		float data_value = (*p_vec_send_data)[i] ;
		int i_data_value = (int)(data_value * (float)scale_factor) ;
		m_p_command[index++] = (unsigned char)((i_data_value & 0xFF000000) >> 24);
		m_p_command[index++] = (unsigned char)((i_data_value & 0x00FF0000) >> 16);
		m_p_command[index++] = (unsigned char)((i_data_value & 0x0000FF00) >> 8);
		m_p_command[index++] = (unsigned char)((i_data_value & 0x000000FF));
	}
	
	m_p_command[index++] = 'E';
	m_p_command[index++] = 'S';
	m_p_command[index++] = 'B';
	m_p_command[index++] = ']';
    //m_p_command[index] = 0;

	//send(client_socket, &command, sizeof(command), 0);
	//printf("sizeof(m_command) = %d\n", sizeof(m_command));

#if 0
	send(client_socket, m_command, sizeof(char)*(2+(*len)), 0);
#else
	//printf("send len = %d\n", sizeof(char)*(2 + (*len)));
	//printf("m_command[0] = %d\n", m_command[0]);
	//printf("m_command[1] = %d\n", m_command[1]);
    try
	{
   		boost::asio::write(*m_s, boost::asio::buffer(m_p_command, sizeof(unsigned char)*(index)));
    }
    catch(exception& e)
    {
    	m_s->close();
    	Release() ;
        return ENSEMBLE_ERROR_SOCKET_WRITE;
    }

#endif

	return  ENSEMBLE_SUCCESS ;
}

int CEthernetClient::Receive(std::vector<float>* out_receive_data) 
{
	if (m_s == NULL)
	{
		return ENSEMBLE_ERROR_INVALID_MEMORY;
	}
	
	unsigned char* buf = NULL;
	int buf_size = 0 ;

	int get_command = -1;
	int index = 0 ;
	unsigned int i_get_data = 0 ;
	int cnt = 0;

	//-------------------------------------------------------------------------------------------
	// 1. Get Command(Check Command)
	//-----
	m_cls_check_data.init_variable();

	//do
	//{
        //int rev_count = 0 ;
		do
		{
			//cnt = recv(client_socket, m_buf, DEFAULT_BUFLEN, 0);
			//cnt = recv(client_socket, m_buf, DEFAULT_BUFLEN, 0);
            try{
			    boost::system::error_code error;
				//cnt = m_s->read_some(boost::asio::buffer(m_buf), error);
    			//cnt = m_s->read_some(boost::asio::buffer(m_buf), error);
                //qDebug("receive") ;
			
	    		cnt = m_s->receive(boost::asio::buffer(m_buf));
					
                //printf("receive size = %d(%d)\n", cnt, rev_count++) ;
		    	//cnt = m_s->async_read_some(boost::asio::buffer(m_buf), error);

				if (error == boost::asio::error::eof)
    				break; // Connection closed cleanly by peer.
	    		else if (error)
		    		throw boost::system::system_error(error); // Some other error.
            }
            catch(exception& e)
            {
            	m_s->close();
            	Release() ;
                m_cls_check_data.init_variable();
                return ENSEMBLE_ERROR_SOCKET_READ;
            }
			
			//int out_size = 0;
            buf = m_cls_check_data.FindData(m_buf, cnt, &buf_size);
		} while (buf == NULL);

		index = 0 ;
		i_get_data = (unsigned int)buf[index++];
		get_command = (i_get_data << 24) & 0xFF000000;
		i_get_data = (unsigned int)buf[index++];
		get_command |= (i_get_data << 16) & 0x00FF0000;
		i_get_data = (unsigned int)buf[index++];
		get_command |= (i_get_data << 8) & 0x0000FF00;
		i_get_data = (unsigned int)buf[index++];
		get_command |= (i_get_data) & 0x000000FF;
	
	//} while (get_command != command);
	
	//-------------------------------------------------------------
	//head data

	//-------------------------------------------------------------------------------------------
	// 2. Scale Factor
	//-----
	unsigned int scale_factor = 0 ;
	i_get_data = (unsigned int)buf[index++];
	scale_factor = (i_get_data << 24) & 0xFF000000;
	i_get_data = (unsigned int)buf[index++];
	scale_factor |= (i_get_data << 16) & 0x00FF0000;
	i_get_data = (unsigned int)buf[index++];
	scale_factor |= (i_get_data << 8) & 0x0000FF00;
	i_get_data = (unsigned int)buf[index++];
	scale_factor |= (i_get_data) & 0x000000FF;

	//-------------------------------------------------------------------------------------------
	// 3. data length
	//-----
	unsigned int data_length = 0 ;
	i_get_data = (unsigned int)buf[index++];
	data_length = (i_get_data << 24) & 0xFF000000;
	i_get_data = (unsigned int)buf[index++];
	data_length |= (i_get_data << 16) & 0x00FF0000;
	i_get_data = (unsigned int)buf[index++];
	data_length |= (i_get_data << 8) & 0x0000FF00;
	i_get_data = (unsigned int)buf[index++];
	data_length |= (i_get_data) & 0x000000FF;
	//head data
	
	//-------------------------------------------------------------

	//-------------------------------------------------------------------------------------------
	// 4. data
	//-----
	if( data_length > 0 )
	{
		if( out_receive_data )
		{
			if( out_receive_data->size() > 0 ) out_receive_data->clear() ;
			out_receive_data->reserve(data_length) ;

			for( int i=0 ; i<data_length ; i++ )
			{
				int i_get_data_value = 0 ;
				
				i_get_data = (unsigned int)buf[index++];
				i_get_data_value = (i_get_data << 24) & 0xFF000000;
				i_get_data = (unsigned int)buf[index++];
				i_get_data_value |= (i_get_data << 16) & 0x00FF0000;
				i_get_data = (unsigned int)buf[index++];
				i_get_data_value |= (i_get_data << 8) & 0x0000FF00;
				i_get_data = (unsigned int)buf[index++];
				i_get_data_value |= (i_get_data) & 0x000000FF;

				float f_get_data_value = (float)i_get_data_value / (float)scale_factor ;

				(*out_receive_data)[i] = f_get_data_value ;
			}
		}
	}
	
	m_cls_check_data.init_variable();
	
    return  get_command ;
}

int CEthernetClient::Receive(const unsigned int command, std::vector<float>* out_receive_data) 
{	
	if (m_s == NULL)
	{
		return ENSEMBLE_ERROR_INVALID_MEMORY;
	}
	
	unsigned char* buf = NULL;
	int buf_size = 0 ;

	int get_command = -1;
	int index = 0 ;
	unsigned int i_get_data = 0 ;
	int cnt = 0;

	//-------------------------------------------------------------------------------------------
	// 1. Get Command(Check Command)
	//-----
	m_cls_check_data.init_variable();
	do
	{
        //int rev_count = 0 ;
		do
		{
			//cnt = recv(client_socket, m_buf, DEFAULT_BUFLEN, 0);
			//cnt = recv(client_socket, m_buf, DEFAULT_BUFLEN, 0);
            try{
			    boost::system::error_code error;
				//cnt = m_s->read_some(boost::asio::buffer(m_buf), error);
    			//cnt = m_s->read_some(boost::asio::buffer(m_buf), error);
                //qDebug("receive") ;
			
	    		cnt = m_s->receive(boost::asio::buffer(m_buf));
					
                //printf("receive size = %d(%d)\n", cnt, rev_count++) ;
		    	//cnt = m_s->async_read_some(boost::asio::buffer(m_buf), error);

				if (error == boost::asio::error::eof)
    				break; // Connection closed cleanly by peer.
	    		else if (error)
		    		throw boost::system::system_error(error); // Some other error.
            }
            catch(exception& e)
            {
            	m_s->close();
            	Release() ;
                m_cls_check_data.init_variable();
                return ENSEMBLE_ERROR_SOCKET_READ;
            }
			
			//int out_size = 0;
            buf = m_cls_check_data.FindData(m_buf, cnt, &buf_size);
		} while (buf == NULL);

		index = 0 ;
		i_get_data = (unsigned int)buf[index++];
		get_command = (i_get_data << 24) & 0xFF000000;
		i_get_data = (unsigned int)buf[index++];
		get_command |= (i_get_data << 16) & 0x00FF0000;
		i_get_data = (unsigned int)buf[index++];
		get_command |= (i_get_data << 8) & 0x0000FF00;
		i_get_data = (unsigned int)buf[index++];
		get_command |= (i_get_data) & 0x000000FF;
	
	} while (get_command != command);
	
	//-------------------------------------------------------------
	//head data

	//-------------------------------------------------------------------------------------------
	// 2. Scale Factor
	//-----
	unsigned int scale_factor = 0 ;
	i_get_data = (unsigned int)buf[index++];
	scale_factor = (i_get_data << 24) & 0xFF000000;
	i_get_data = (unsigned int)buf[index++];
	scale_factor |= (i_get_data << 16) & 0x00FF0000;
	i_get_data = (unsigned int)buf[index++];
	scale_factor |= (i_get_data << 8) & 0x0000FF00;
	i_get_data = (unsigned int)buf[index++];
	scale_factor |= (i_get_data) & 0x000000FF;

	//-------------------------------------------------------------------------------------------
	// 3. data length
	//-----
	unsigned int data_length = 0 ;
	i_get_data = (unsigned int)buf[index++];
	data_length = (i_get_data << 24) & 0xFF000000;
	i_get_data = (unsigned int)buf[index++];
	data_length |= (i_get_data << 16) & 0x00FF0000;
	i_get_data = (unsigned int)buf[index++];
	data_length |= (i_get_data << 8) & 0x0000FF00;
	i_get_data = (unsigned int)buf[index++];
	data_length |= (i_get_data) & 0x000000FF;
	//head data
	
	//-------------------------------------------------------------

	//-------------------------------------------------------------------------------------------
	// 4. data
	//-----
	if( data_length > 0 )
	{
		if( out_receive_data )
		{
			if( out_receive_data->size() > 0 ) out_receive_data->clear() ;
			out_receive_data->reserve(data_length) ;

			for( int i=0 ; i<data_length ; i++ )
			{
				int i_get_data_value = 0 ;
				
				i_get_data = (unsigned int)buf[index++];
				i_get_data_value = (i_get_data << 24) & 0xFF000000;
				i_get_data = (unsigned int)buf[index++];
				i_get_data_value |= (i_get_data << 16) & 0x00FF0000;
				i_get_data = (unsigned int)buf[index++];
				i_get_data_value |= (i_get_data << 8) & 0x0000FF00;
				i_get_data = (unsigned int)buf[index++];
				i_get_data_value |= (i_get_data) & 0x000000FF;

				float f_get_data_value = (float)i_get_data_value / (float)scale_factor ;

				(*out_receive_data)[i] = f_get_data_value ;
			}
		}
	}
	
	m_cls_check_data.init_variable();
	
    return  ENSEMBLE_SUCCESS ;
}

int CEthernetClient::SendImage(const unsigned int command, const int width, const int height, const int image_type, unsigned char* image_buf, const int buf_len)
{
	if (m_s == NULL)
	{
		return ENSEMBLE_ERROR_INVALID_MEMORY;
	}

	unsigned int index = 0 ;
	m_p_command[index++] = '[';
	m_p_command[index++] = 'R';
	m_p_command[index++] = 'D';
	m_p_command[index++] = 'V';

	m_p_command[index++] = (command & 0xFF000000) >> 24;
	m_p_command[index++] = (command & 0x00FF0000) >> 16;
	m_p_command[index++] = (command & 0x0000FF00) >> 8;
	m_p_command[index++] = (command & 0x000000FF);

	//image width
	m_p_command[index++] = (width & 0xFF000000) >> 24;
	m_p_command[index++] = (width & 0x00FF0000) >> 16;
	m_p_command[index++] = (width & 0x0000FF00) >> 8;
	m_p_command[index++] = (width & 0x000000FF);

	//image height
	m_p_command[index++] = (unsigned char)(((height) & 0xFF000000) >> 24);
	m_p_command[index++] = (unsigned char)(((height) & 0x00FF0000) >> 16);
	m_p_command[index++] = (unsigned char)(((height) & 0x0000FF00) >> 8);
	m_p_command[index++] = (unsigned char)(((height) & 0x000000FF));

	//image type
	m_p_command[index++] = (unsigned char)((image_type & 0xFF000000) >> 24);
	m_p_command[index++] = (unsigned char)((image_type & 0x00FF0000) >> 16);
	m_p_command[index++] = (unsigned char)((image_type & 0x0000FF00) >> 8);
	m_p_command[index++] = (unsigned char)((image_type & 0x000000FF));

	//buf data len
	m_p_command[index++] = (unsigned char)((buf_len & 0xFF000000) >> 24);
	m_p_command[index++] = (unsigned char)((buf_len & 0x00FF0000) >> 16);
	m_p_command[index++] = (unsigned char)((buf_len & 0x0000FF00) >> 8);
	m_p_command[index++] = (unsigned char)((buf_len & 0x000000FF));

	//image buf
    if( image_buf != NULL && buf_len > 0)
    {				
        memcpy(m_p_command + index, image_buf, buf_len);	
		index += buf_len ;
    }
	
	m_p_command[index++] = 'E';
	m_p_command[index++] = 'S';
	m_p_command[index++] = 'B';
	m_p_command[index++] = ']';
    //m_p_command[index] = 0;

	//send(client_socket, &command, sizeof(command), 0);
	//printf("sizeof(m_command) = %d\n", sizeof(m_command));

#if 0
	send(client_socket, m_command, sizeof(char)*(2+(*len)), 0);
#else
	//printf("send len = %d\n", sizeof(char)*(2 + (*len)));
	//printf("m_command[0] = %d\n", m_command[0]);
	//printf("m_command[1] = %d\n", m_command[1]);
    try
	{
   		boost::asio::write(*m_s, boost::asio::buffer(m_p_command, sizeof(unsigned char)*(index)));
    }
    catch(exception& e)
    {
    	m_s->close();
    	Release() ;
        return ENSEMBLE_ERROR_SOCKET_WRITE;
    }

#endif

	return  ENSEMBLE_SUCCESS ;
}

int CEthernetClient::ReceiveImage(const unsigned int command, int& width, int& height, unsigned char** out_data)
{
	if (m_s == NULL)
	{
		return ENSEMBLE_ERROR_INVALID_MEMORY;
	}
	
	unsigned char* buf = NULL;
	int buf_size = 0 ;

	int get_command = -1;
	int index = 0 ;
	unsigned int i_get_data = 0 ;
	int cnt = 0;

	//-------------------------------------------------------------------------------------------
	// 1. Get Command(Check Command)
	//-----
	m_cls_check_data.init_variable();
	do
	{
        //int rev_count = 0 ;
		do
		{
			//cnt = recv(client_socket, m_buf, DEFAULT_BUFLEN, 0);
			//cnt = recv(client_socket, m_buf, DEFAULT_BUFLEN, 0);
            try{
			    boost::system::error_code error;
				//cnt = m_s->read_some(boost::asio::buffer(m_buf), error);
    			//cnt = m_s->read_some(boost::asio::buffer(m_buf), error);
                //qDebug("receive") ;
			
	    		cnt = m_s->receive(boost::asio::buffer(m_buf));
					
                //printf("receive size = %d(%d)\n", cnt, rev_count++) ;
		    	//cnt = m_s->async_read_some(boost::asio::buffer(m_buf), error);

				if (error == boost::asio::error::eof)
    				break; // Connection closed cleanly by peer.
	    		else if (error)
		    		throw boost::system::system_error(error); // Some other error.
            }
            catch(exception& e)
            {
            	m_s->close();
            	Release() ;
                m_cls_check_data.init_variable();
                return ENSEMBLE_ERROR_SOCKET_READ;
            }
			
			//int out_size = 0;
            buf = m_cls_check_data.FindData(m_buf, cnt, &buf_size);
		} while (buf == NULL);

		index = 0 ;
		i_get_data = (unsigned int)buf[index++];
		get_command = (i_get_data << 24) & 0xFF000000;
		i_get_data = (unsigned int)buf[index++];
		get_command |= (i_get_data << 16) & 0x00FF0000;
		i_get_data = (unsigned int)buf[index++];
		get_command |= (i_get_data << 8) & 0x0000FF00;
		i_get_data = (unsigned int)buf[index++];
		get_command |= (i_get_data) & 0x000000FF;
	
	} while (get_command != command);
	
	//-------------------------------------------------------------
	//head data

	//-------------------------------------------------------------------------------------------
	// 2. image width
	//-----
	unsigned int image_width = 0 ;
	i_get_data = (unsigned int)buf[index++];
	image_width = (i_get_data << 24) & 0xFF000000;
	i_get_data = (unsigned int)buf[index++];
	image_width |= (i_get_data << 16) & 0x00FF0000;
	i_get_data = (unsigned int)buf[index++];
	image_width |= (i_get_data << 8) & 0x0000FF00;
	i_get_data = (unsigned int)buf[index++];
	image_width |= (i_get_data) & 0x000000FF;

	//-------------------------------------------------------------------------------------------
	// 3. image height
	//-----
	unsigned int image_height = 0 ;
	i_get_data = (unsigned int)buf[index++];
	image_height = (i_get_data << 24) & 0xFF000000;
	i_get_data = (unsigned int)buf[index++];
	image_height |= (i_get_data << 16) & 0x00FF0000;
	i_get_data = (unsigned int)buf[index++];
	image_height |= (i_get_data << 8) & 0x0000FF00;
	i_get_data = (unsigned int)buf[index++];
	image_height |= (i_get_data) & 0x000000FF;

	//-------------------------------------------------------------------------------------------
	// 4. image type
	//-----
	unsigned int image_cv_type = 0 ;
	i_get_data = (unsigned int)buf[index++];
	image_cv_type = (i_get_data << 24) & 0xFF000000;
	i_get_data = (unsigned int)buf[index++];
	image_cv_type |= (i_get_data << 16) & 0x00FF0000;
	i_get_data = (unsigned int)buf[index++];
	image_cv_type |= (i_get_data << 8) & 0x0000FF00;
	i_get_data = (unsigned int)buf[index++];
	image_cv_type |= (i_get_data) & 0x000000FF;

	//-------------------------------------------------------------------------------------------
	// 5. image raw data length
	//-----
	unsigned int data_length = 0 ;
	i_get_data = (unsigned int)buf[index++];
	data_length = (i_get_data << 24) & 0xFF000000;
	i_get_data = (unsigned int)buf[index++];
	data_length |= (i_get_data << 16) & 0x00FF0000;
	i_get_data = (unsigned int)buf[index++];
	data_length |= (i_get_data << 8) & 0x0000FF00;
	i_get_data = (unsigned int)buf[index++];
	data_length |= (i_get_data) & 0x000000FF;
	
	//-------------------------------------------------------------

	//-------------------------------------------------------------------------------------------
	// 7. data copy
	//-----
	if( data_length > 0 )
	{
		if( out_data != NULL )
		{
			if( (*out_data) )
			{
				if( image_width != width || image_height != height )
				{
					delete [] (*out_data) ;
					(*out_data) = NULL ;
				}
			}

			if( (*out_data) == NULL )
			{
				(*out_data) = new unsigned char[data_length];
			}

			memcpy((*out_data), buf+index, data_length);

			width = image_width ;
			height = image_height ;
		}
	}
	
	m_cls_check_data.init_variable();
	
    return  ENSEMBLE_SUCCESS ;
}

int CEthernetClient::Send(unsigned int command, unsigned char* send_data, const unsigned int send_data_size, const unsigned int send_scalefactor, unsigned char** out_data, int* out_data_size, unsigned int* out_scalefactor )
{
	if (m_s == NULL)
	{
		*out_data_size = 0;
		return ENSEMBLE_ERROR_INVALID_MEMORY;
	}

	int cnt = 0;
	//char recvbuf[DEFAULT_BUFLEN];
	//char buf[DEFAULT_BUFLEN];
	//if ((*out_data) == NULL)
	//{
	//	return;
	//}

	//cout << "data send" << ":" << command << " " << sizeof(command) << endl;
	//write(client_socket, &command, sizeof(command));

	unsigned int leng = send_data_size;
	if (leng <= 0) leng = 0;
	
	unsigned int index = 0 ;
	m_p_command[index++] = '[';
	m_p_command[index++] = 'R';
	m_p_command[index++] = 'D';
	m_p_command[index++] = 'V';

	m_p_command[index++] = (command & 0xFF000000) >> 24;
	m_p_command[index++] = (command & 0x00FF0000) >> 16;
	m_p_command[index++] = (command & 0x0000FF00) >> 8;
	m_p_command[index++] = (command & 0x000000FF);

	//scale factor
	m_p_command[index++] = (unsigned char)(((send_scalefactor) & 0xFF000000) >> 24);
	m_p_command[index++] = (unsigned char)(((send_scalefactor) & 0x00FF0000) >> 16);
	m_p_command[index++] = (unsigned char)(((send_scalefactor) & 0x0000FF00) >> 8);
	m_p_command[index++] = (unsigned char)(((send_scalefactor) & 0x000000FF));

	m_p_command[index++] = (unsigned char)((leng & 0xFF000000) >> 24);
	m_p_command[index++] = (unsigned char)((leng & 0x00FF0000) >> 16);
	m_p_command[index++] = (unsigned char)((leng & 0x0000FF00) >> 8);
	m_p_command[index++] = (unsigned char)((leng & 0x000000FF));

	if (leng > 0)
	{
        memcpy(m_p_command + index, (send_data), leng);
        index += leng ;
	}

	m_p_command[index++] = 'E';
	m_p_command[index++] = 'S';
	m_p_command[index++] = 'B';
	m_p_command[index++] = ']';
    //m_p_command[index] = 0;

	//send(client_socket, &command, sizeof(command), 0);
	//printf("sizeof(m_command) = %d\n", sizeof(m_command));

#if 0
	send(client_socket, m_command, sizeof(char)*(2+(*len)), 0);
#else
	//printf("send len = %d\n", sizeof(char)*(2 + (*len)));
	//printf("m_command[0] = %d\n", m_command[0]);
	//printf("m_command[1] = %d\n", m_command[1]);
    try{
   		boost::asio::write(*m_s, boost::asio::buffer(m_p_command, sizeof(unsigned char)*(index)));
    }
    catch(exception& e)
    {
    	m_s->close();
    	Release() ;
        return ENSEMBLE_ERROR_SOCKET_WRITE;
    }

#endif

    //int get_size = 0;
	
	//unsigned char get_data[5];
	//int get_data_index = 0;

    unsigned char* buf = NULL;
	int buf_size = 0 ;

	int get_command = -1;
	index = 0 ;
	unsigned int i_get_data = 0 ;

	//-------------------------------------------------------------------------------------------
	// 1. Get Command(Check Command)
	//-----
	m_cls_check_data.init_variable();
	do
	{
        //int rev_count = 0 ;
		do
		{
			//cnt = recv(client_socket, m_buf, DEFAULT_BUFLEN, 0);
			//cnt = recv(client_socket, m_buf, DEFAULT_BUFLEN, 0);
            try{
			    boost::system::error_code error;
				//cnt = m_s->read_some(boost::asio::buffer(m_buf), error);
    			//cnt = m_s->read_some(boost::asio::buffer(m_buf), error);
                //qDebug("receive") ;
			
	    		cnt = m_s->receive(boost::asio::buffer(m_buf));
					
                //printf("receive size = %d(%d)\n", cnt, rev_count++) ;
		    	//cnt = m_s->async_read_some(boost::asio::buffer(m_buf), error);

				if (error == boost::asio::error::eof)
    				break; // Connection closed cleanly by peer.
	    		else if (error)
		    		throw boost::system::system_error(error); // Some other error.
            }
            catch(exception& e)
            {
            	m_s->close();
            	Release() ;
                m_cls_check_data.init_variable();
                return ENSEMBLE_ERROR_SOCKET_READ;
            }
			
			//int out_size = 0;
            buf = m_cls_check_data.FindData(m_buf, cnt, &buf_size);
		} while (buf == NULL);

		index = 0 ;
		i_get_data = (unsigned int)buf[index++];
		get_command = (i_get_data << 24) & 0xFF000000;
		i_get_data = (unsigned int)buf[index++];
		get_command |= (i_get_data << 16) & 0x00FF0000;
		i_get_data = (unsigned int)buf[index++];
		get_command |= (i_get_data << 8) & 0x0000FF00;
		i_get_data = (unsigned int)buf[index++];
		get_command |= (i_get_data) & 0x000000FF;
	
	} while (get_command != command);
	
	//-------------------------------------------------------------
	//head data

	//-------------------------------------------------------------------------------------------
	// 2. Scale Factor
	//-----
	unsigned int scale_factor = 0 ;
	i_get_data = (unsigned int)buf[index++];
	scale_factor = (i_get_data << 24) & 0xFF000000;
	i_get_data = (unsigned int)buf[index++];
	scale_factor |= (i_get_data << 16) & 0x00FF0000;
	i_get_data = (unsigned int)buf[index++];
	scale_factor |= (i_get_data << 8) & 0x0000FF00;
	i_get_data = (unsigned int)buf[index++];
	scale_factor |= (i_get_data) & 0x000000FF;

	//-------------------------------------------------------------------------------------------
	// 3. data length
	//-----
	unsigned int data_length = 0 ;
	i_get_data = (unsigned int)buf[index++];
	data_length = (i_get_data << 24) & 0xFF000000;
	i_get_data = (unsigned int)buf[index++];
	data_length |= (i_get_data << 16) & 0x00FF0000;
	i_get_data = (unsigned int)buf[index++];
	data_length |= (i_get_data << 8) & 0x0000FF00;
	i_get_data = (unsigned int)buf[index++];
	data_length |= (i_get_data) & 0x000000FF;
	//head data
	
	//-------------------------------------------------------------

	//-------------------------------------------------------------------------------------------
	// 4. data
	//-----
	#if 1
	if( data_length > 0 )
	{
		if( out_data != NULL )
		{
			if( (*out_data) == NULL )
			{
				(*out_data) = new unsigned char[data_length];
			}
		}
	}
	#endif

	//printf("get_command = %d\n", get_command);

	//-------------------------------------------------------------------------------------------
	// 5. out variable
	//-----
    if (data_length > 0)
    {
    	if( out_data != NULL )
		{
	        if( (*out_data) != NULL )
	        {
	            memcpy((*out_data), buf+index, data_length);
	        }
    	}
    }

	if( out_data_size != NULL )	*out_data_size = data_length;
	if( out_scalefactor != NULL )	*out_scalefactor = scale_factor;

	m_cls_check_data.init_variable();
	
    return ENSEMBLE_SUCCESS;
}

#endif

