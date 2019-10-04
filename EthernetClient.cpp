#include "EthernetClient.h"
#include "ErrorType.h"
#include <string>

CEthernetClient::CEthernetClient(void) :
	m_s(NULL)
	,m_resolver(NULL)
	, m_acceptor(NULL)
	, m_io_service(NULL)

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

tcp::socket *CEthernetClient::GetSocketPointer(void)
{
	return  m_s; 
}

