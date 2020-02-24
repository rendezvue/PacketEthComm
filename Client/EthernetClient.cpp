#include "EthernetClient.h"
#include "ErrorType.h"
#include <string>
#include <chrono>


CEthernetClient::CEthernetClient(void) :
	m_s(NULL)
	, m_timer(NULL)
{
}

CEthernetClient::~CEthernetClient(void)
{
	Release() ;
}

void CEthernetClient::Release(void) 
{
	if (m_s != NULL)
	{
		delete m_s;
		m_s = NULL;
	}

	if (m_timer != NULL)
	{
		delete m_timer;
		m_timer = NULL;
	}
}

std::string CEthernetClient::GetClinetIpAddress(void)
{
	std::string str_ip ;

	if( m_s )
	{
		str_ip =m_s->remote_endpoint().address().to_string() ;
	}

	return str_ip ;
}

int CEthernetClient::Open(const char* ip, unsigned int port)
{
	boost::system::error_code ec;

    boost::asio::io_service* p_io_service = CEthernetGetInfo::getInstance()->GetIoService() ;

	if( m_s == NULL )       m_s = new tcp::socket(*p_io_service);
    if( m_timer == NULL )   m_timer = new deadline_timer(*p_io_service);
	
	if( !m_s->is_open() )
    {
    	std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
		 
        do
        {
            try
            {
                if( m_s->is_open() )
                {
                    break ;
                }

                m_s->async_connect(tcp::endpoint(ip::address::from_string(ip), port), boost::bind(&CEthernetClient::handle_connect, this, boost::asio::placeholders::error));
                //m_s->async_connect(tcp::endpoint(ip::address::from_string(ip), port), boost::bind(&CEthernetClient::handle_connect));

            }
            catch (boost::system::system_error const &e)
            {
                //cout << "Warning : could not connect : " << e.what() << endl;
                //Close();
                Release() ;

                return ENSEMBLE_ERROR_SOCKET_CONNECT;
            }

			
			std::chrono::system_clock::time_point end_time = std::chrono::system_clock::now();
			std::chrono::seconds sec = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);

			if( sec.count() > 3 )		//3 second
			{
				Release() ;

                return ENSEMBLE_ERROR_SOCKET_CONNECT;
			}
        }while(1) ;
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

	Shutdown() ;
}

tcp::socket *CEthernetClient::GetSocketPointer(void)
{
	return  m_s; 
}

deadline_timer *CEthernetClient::GetTimerPointer(void)
{
    return m_timer ;
}

void CEthernetClient::Shutdown(void)
{
	if( m_s )
	{
		//m_s->shutdown(boost::asio::socket_base::shutdown_both);
		
		if( m_s->is_open() ) m_s->close();
		Release() ;
	}
}
