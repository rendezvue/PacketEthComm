#include "EthernetServer.h"
#include "ErrorType.h"
#include <string>

bool CEthernetServer::instanceFlag = false;
CEthernetServer* CEthernetServer::instance = NULL;

CEthernetServer::CEthernetServer(void) :
	m_acceptor(NULL)
	, m_timer(NULL)
{
	boost::asio::io_service* p_io_service = CEthernetGetInfo::getInstance()->GetIoService() ;
	m_acceptor = new tcp::acceptor(*p_io_service, tcp::endpoint(tcp::v4(), NETWORK_PORT_CON));
	m_timer = new deadline_timer(*p_io_service);
}

CEthernetServer::~CEthernetServer(void)
{
	Release() ;
}

CEthernetServer* CEthernetServer::getInstance()
{
	if(instance == NULL)
	{
		instance = new CEthernetServer();
		instanceFlag = true;
	}
	return instance;
}


void CEthernetServer::Release(void) 
{
	if (m_timer != NULL)
	{
		delete m_timer;
		m_timer = NULL;
	}

	if( m_acceptor != NULL )
	{
		delete m_acceptor ;
		m_acceptor = NULL;
	}
}

int CEthernetServer::Accept(tcp::socket *p_socket) 
{
	if( p_socket == NULL )
	{
		return ENSEMBLE_ERROR_INVALID_MEMORY ;
	}

	try
	{
#if 0	
		m_acceptor->async_accept((*m_s), boost::bind(&CEthernetServer::handle_connect, this, boost::asio::placeholders::error)) ;

		m_timer->expires_from_now(boost::posix_time::seconds(3));
		m_timer->async_wait(boost::bind(&CEthernetServer::Close, this));

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
		m_acceptor->accept((*p_socket)) ;		//inf
		cout << "Connection IP : " <<  p_socket->remote_endpoint().address().to_string() << endl;
#endif
	}
	catch (boost::system::system_error const &e)
	{
		//cout << "Warning : could not connect : " << e.what() << endl;
		//Close();
		//Release() ;
		
		return ENSEMBLE_ERROR_SOCKET_CONNECT;
	}

	return ENSEMBLE_SUCCESS;
}

int CEthernetServer::handle_connect(const boost::system::error_code& err)
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


void CEthernetServer::Close()
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

	//Release() ;
}
