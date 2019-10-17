#include "EthernetGetInfo.h"
#include "ErrorType.h"
#include <string>

bool CEthernetGetInfo::instanceFlag = false;
CEthernetGetInfo* CEthernetGetInfo::instance = NULL;

CEthernetGetInfo::CEthernetGetInfo(void) :
	m_io_service(NULL)
{	
	m_io_service = new io_service();
}

CEthernetGetInfo::~CEthernetGetInfo(void)
{
	if( m_io_service != NULL )
	{
		delete m_io_service ;
		m_io_service = NULL;
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

boost::asio::io_service* CEthernetGetInfo::GetIoService(void)
{
	return m_io_service ;
}


