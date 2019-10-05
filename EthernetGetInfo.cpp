#include "EthernetGetInfo.h"
#include "ErrorType.h"
#include <string>

bool CEthernetGetInfo::instanceFlag = false;
CEthernetGetInfo* CEthernetGetInfo::instance = NULL;

CEthernetGetInfo::CEthernetGetInfo(void) :
	m_acceptor(NULL)
	, m_io_service(NULL)
{	
	m_io_service = new io_service();
	m_acceptor = new tcp::acceptor(*m_io_service, tcp::endpoint(tcp::v4(), NETWORK_PORT_CON));
}

CEthernetGetInfo::~CEthernetGetInfo(void)
{
	if( m_io_service != NULL )
	{
		delete m_io_service ;
		m_io_service = NULL;
	}
		
	if( m_acceptor != NULL )
	{
		delete m_acceptor ;
		m_acceptor = NULL;
	}
}

CEthernetGetInfo* CEthernetGetInfo::getInstance()
{
	if(instance == NULL)
	{
		instance = new CEthernetGetInfo();
		instanceFlag = true;
	}
	return instance;
}

tcp::acceptor* CEthernetGetInfo::GetAcceptoer(void)
{
	return m_acceptor ;
}

boost::asio::io_service* CEthernetGetInfo::GetIoService(void)
{
	return m_io_service ;
}


