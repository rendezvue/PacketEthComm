#include "EthernetClientControlData.h"
#include "ErrorType.h"
#include <string>

bool CEthernetClientControlData::instanceFlag = false;
CEthernetClientControlData* CEthernetClientControlData::instance = NULL;


CEthernetClientControlData::CEthernetClientControlData(void) :
	m_p_command(NULL)
	, m_ui_command_size(0)

{
	m_ui_command_size = DATA_BUF_SIZE ;
	m_p_command = new unsigned char[m_ui_command_size] ;	
}

CEthernetClientControlData::~CEthernetClientControlData(void)
{
	if(m_p_command != NULL )
	{
		delete m_p_command ;
		m_p_command = NULL ;
	}
}

// 싱글턴 인스턴스를 반환할 멤버함수
CEthernetClientControlData* CEthernetClientControlData::getInstance()
{
	if(instance == NULL)
	{
		instance = new CEthernetClientControlData();
		instanceFlag = true;
	}
	return instance;
}


int CEthernetClientControlData::SendNoData(tcp::socket *soc, const unsigned int command, const std::string id)
{
	std::vector<float> vec_send_data ;
	return Send(soc, command, id, &vec_send_data) ;	
}

int CEthernetClientControlData::SendString(tcp::socket *soc, const unsigned int command, const std::string id, const std::string str)
{
	std::vector<float> vec_send_data ;
	int send_data_size = str.size() ;
	for( int i=0 ; i<send_data_size ; i++ )
	{
		vec_send_data.push_back(str[i]) ;
	}
	
	return Send(soc, command, id, &vec_send_data) ;	
}

int CEthernetClientControlData::Send(tcp::socket *soc, const unsigned int command, const std::string id, std::vector<float> *p_vec_send_data)
{
	if (soc == NULL)
	{
		return ENSEMBLE_ERROR_INVALID_MEMORY;
	}

	m_mutex.lock();

	int ret = ENSEMBLE_SUCCESS ;
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
   //m_p_command[index++] = 0;

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
       int ret = soc->send(boost::asio::buffer(m_p_command, sizeof(unsigned char)*(index))) ;

       qDebug("send ret = %d, index= %d, command=%d", ret, index, command) ;

        //boost::asio::write(*soc, boost::asio::buffer(m_p_command, sizeof(unsigned char)*(index)));
    }
    catch(exception& e)
    {
        ret = ENSEMBLE_ERROR_SOCKET_WRITE;
    }

#endif
	m_mutex.unlock();

	return  ret ;
}

int CEthernetClientControlData::Receive(tcp::socket *soc, std::string *out_str_id, std::vector<float>* out_receive_data) 
{
	//printf("receive\n") ;
	
	if (soc == NULL)
	{
		return ENSEMBLE_ERROR_INVALID_MEMORY;
	}

	m_mutex.lock();
	
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

    std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();

	//do
	//{
        //int rev_count = 0 ;
		do
		{
			//cnt = recv(client_socket, m_buf, DEFAULT_BUFLEN, 0);
			//cnt = recv(client_socket, m_buf, DEFAULT_BUFLEN, 0);
            try{
			    boost::system::error_code error;
                //soc->async_receive(boost::asio::buffer(buf, 1024), boost::bind(&CEthernetClientControlData::OnAsyncReceive, this, boost::asio::placeholders::error));
                soc->async_read_some(boost::asio::buffer(buf, DEFAULT_BUFLEN2), boost::bind(&CEthernetClientControlData::OnAsyncReceive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
                //cnt = soc->read_some(boost::asio::buffer(m_buf), error);
    			//cnt = soc->read_some(boost::asio::buffer(m_buf), error);
                //qDebug("receive") ;
			
                //cnt = soc->receive(boost::asio::buffer(m_buf));
					
                //printf("receive size = %d(%d)\n", cnt, rev_count++) ;
		    	//cnt = soc->async_read_some(boost::asio::buffer(m_buf), error);

                std::chrono::system_clock::time_point end_time = std::chrono::system_clock::now();
                std::chrono::seconds sec = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);

                if( sec.count() > 10 )
                {
                    m_cls_check_data.init_variable();

                    m_mutex.unlock();

                    //LOG(LOG_VISION, "Exception Receive : %s", e.what()) ;
                    return ENSEMBLE_ERROR_SOCKET_READ;
                }


				if (error == boost::asio::error::eof)
    				break; // Connection closed cleanly by peer.
	    		else if (error)
                {
                    //qDebug("read_some error 2") ;
                    throw boost::system::system_error(error); // Some other error.

                    break ;
                }
            }
            catch(exception& e)
            {
                m_cls_check_data.init_variable();

				m_mutex.unlock();

				//LOG(LOG_VISION, "Exception Receive : %s", e.what()) ;
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

	
	//ID length
	int i_id_length = 0 ;
	i_get_data = (unsigned int)buf[index++];
	i_id_length = (i_get_data << 24) & 0xFF000000;
	i_get_data = (unsigned int)buf[index++];
	i_id_length |= (i_get_data << 16) & 0x00FF0000;
	i_get_data = (unsigned int)buf[index++];
	i_id_length |= (i_get_data << 8) & 0x0000FF00;
	i_get_data = (unsigned int)buf[index++];
	i_id_length |= (i_get_data) & 0x000000FF;

	std::string str_id ;
	if( i_id_length > 0 )
	{
		str_id.resize(i_id_length) ;

		for( int i=0 ; i<i_id_length ; i++ )
		{
			int i_char = (unsigned int)buf[index++];			
			str_id[i] = (char)i_char ;
		}
	}
	
	if( out_str_id )	(*out_str_id) = str_id ;

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
			out_receive_data->resize(data_length) ;

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

	m_mutex.unlock();
	
    return  get_command ;
}

void CEthernetClientControlData::OnAsyncReceive(const boost::system::error_code& ErrorCode, std::size_t bytes_transferred )
{
    cout << "receiving..." << endl;
    if (ErrorCode == 0)
    {
        qDebug("Receive size = %d", bytes_transferred) ;
    }
    else
    {
        cout << "ERROR! OnReceive..." << endl;
    }
}

int CEthernetClientControlData::Receive(tcp::socket *soc, const unsigned int command, std::vector<float>* out_receive_data) 
{	
	if (soc == NULL)
	{
		return ENSEMBLE_ERROR_INVALID_MEMORY;
	}

    qDebug("Receive 1") ;

	m_mutex.lock();
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

    std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();

	do
	{
        //int rev_count = 0 ;
		do
		{
			//cnt = recv(client_socket, m_buf, DEFAULT_BUFLEN, 0);
			//cnt = recv(client_socket, m_buf, DEFAULT_BUFLEN, 0);
            try{
			    boost::system::error_code error;
                //cnt = soc->read_some(boost::asio::buffer(m_buf), error);
    			//cnt = soc->read_some(boost::asio::buffer(m_buf), error);
                //qDebug("receive") ;
			
                cnt = soc->receive(boost::asio::buffer(m_buf));

                //soc->async_read_some(boost::asio::buffer(buf, DEFAULT_BUFLEN2), boost::bind(&CEthernetClientControlData::OnAsyncReceive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

                std::chrono::system_clock::time_point end_time = std::chrono::system_clock::now();
                std::chrono::seconds sec = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);

                //qDebug("receive time out , sec=%d", sec) ;

                if( sec.count() > 10 )
                {
                    qDebug("receive time out , sec=%d", sec) ;

                    m_cls_check_data.init_variable();

                    m_mutex.unlock();

                    //LOG(LOG_VISION, "Exception Receive : %s", e.what()) ;
                    return ENSEMBLE_ERROR_SOCKET_READ;
                }


					
                //printf("receive size = %d(%d)\n", cnt, rev_count++) ;
		    	//cnt = soc->async_read_some(boost::asio::buffer(m_buf), error);
				
				
				if (error == boost::asio::error::eof)
				{
                    qDebug("Exception Receive : EOF\n") ;
    				break; // Connection closed cleanly by peer.
				}
	    		else if (error)
	    		{
                    //break ;
                    qDebug("read_some error") ;
                    throw boost::system::system_error(error); // Some other error.
	    		}
            }
            catch(exception& e)
            {
                m_cls_check_data.init_variable();
				m_mutex.unlock();

                qDebug("Exception Receive : %s", e.what()) ;
				
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

	//ID length
	int i_id_length = 0 ;
	i_get_data = (unsigned int)buf[index++];
	i_id_length = (i_get_data << 24) & 0xFF000000;
	i_get_data = (unsigned int)buf[index++];
	i_id_length |= (i_get_data << 16) & 0x00FF0000;
	i_get_data = (unsigned int)buf[index++];
	i_id_length |= (i_get_data << 8) & 0x0000FF00;
	i_get_data = (unsigned int)buf[index++];
	i_id_length |= (i_get_data) & 0x000000FF;

	std::string str_id ;
	if( i_id_length > 0 )
	{
		str_id.resize(i_id_length) ;

		for( int i=0 ; i<i_id_length ; i++ )
		{
			int i_char = (unsigned int)buf[index++];
			str_id[i] = (char)i_char ;
		}
	}
	
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
			out_receive_data->resize(data_length) ;

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
	m_mutex.unlock();
	
    return  ENSEMBLE_SUCCESS ;
}

int CEthernetClientControlData::SendImage(tcp::socket *soc, const unsigned int command, const int width, const int height, const int image_type, unsigned char* image_buf, const int buf_len)
{
	if (soc == NULL)
	{
		return ENSEMBLE_ERROR_INVALID_MEMORY;
	}

	m_mutex.lock();
	
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
    //m_p_command[index++] = 0;

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
   		boost::asio::write(*soc, boost::asio::buffer(m_p_command, sizeof(unsigned char)*(index)));
    }
    catch(exception& e)
    {
    	m_mutex.unlock();
        return ENSEMBLE_ERROR_SOCKET_WRITE;
    }

#endif

	m_mutex.unlock();
	return  ENSEMBLE_SUCCESS ;
}

int CEthernetClientControlData::ReceiveImage(tcp::socket *soc, const unsigned int command, int& width, int& height, unsigned char** out_data)
{
	if (soc == NULL)
	{
		return ENSEMBLE_ERROR_INVALID_MEMORY;
	}

	m_mutex.lock();
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
                cnt = soc->read_some(boost::asio::buffer(m_buf), error);
    			//cnt = soc->read_some(boost::asio::buffer(m_buf), error);
                //qDebug("receive") ;
			
                //cnt = soc->receive(boost::asio::buffer(m_buf));
					
                //printf("receive size = %d(%d)\n", cnt, rev_count++) ;
		    	//cnt = soc->async_read_some(boost::asio::buffer(m_buf), error);

				if (error == boost::asio::error::eof)
    				break; // Connection closed cleanly by peer.
	    		else if (error)
		    		throw boost::system::system_error(error); // Some other error.
            }
            catch(exception& e)
            {
                m_cls_check_data.init_variable();
				m_mutex.unlock();
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

	m_mutex.unlock();
    return  ENSEMBLE_SUCCESS ;
}

int CEthernetClientControlData::Send(tcp::socket *soc, unsigned int command, unsigned char* send_data, const unsigned int send_data_size, const unsigned int send_scalefactor, unsigned char** out_data, int* out_data_size, unsigned int* out_scalefactor )
{
	if (soc == NULL)
	{
		*out_data_size = 0;
		return ENSEMBLE_ERROR_INVALID_MEMORY;
	}

	m_mutex.lock();

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
    //m_p_command[index++] = 0;

	//send(client_socket, &command, sizeof(command), 0);
	//printf("sizeof(m_command) = %d\n", sizeof(m_command));

#if 0
	send(client_socket, m_command, sizeof(char)*(2+(*len)), 0);
#else
	//printf("send len = %d\n", sizeof(char)*(2 + (*len)));
	//printf("m_command[0] = %d\n", m_command[0]);
	//printf("m_command[1] = %d\n", m_command[1]);
    try{
   		boost::asio::write(*soc, boost::asio::buffer(m_p_command, sizeof(unsigned char)*(index)));
    }
    catch(exception& e)
    {
    	m_mutex.unlock();
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
                cnt = soc->read_some(boost::asio::buffer(m_buf), error);
    			//cnt = soc->read_some(boost::asio::buffer(m_buf), error);
                //qDebug("receive") ;
			
                //3cnt = soc->receive(boost::asio::buffer(m_buf));
					
                //printf("receive size = %d(%d)\n", cnt, rev_count++) ;
		    	//cnt = soc->async_read_some(boost::asio::buffer(m_buf), error);

				if (error == boost::asio::error::eof)
    				break; // Connection closed cleanly by peer.
	    		else if (error)
		    		throw boost::system::system_error(error); // Some other error.
            }
            catch(exception& e)
            {
                m_cls_check_data.init_variable();
				m_mutex.unlock();
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

	m_mutex.unlock();
    return ENSEMBLE_SUCCESS;
}
