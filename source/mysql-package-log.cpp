#include <cstring>
#include <iostream>
#include "mysql-package-log.hpp"

void proxy::mysql_log::fill(uint8_t *_data, size_t _size)
{
	switch (m_package.comm) {
		case COM_QUERY:
			m_package.body.append(reinterpret_cast<const char*>(_data), _size);
			m_bodyWritten += _size;
			break;
		default:
			//If package does not contain query, it is necessary to reach the end of packet for correct clearing call
			m_bodyWritten += _size;
			break;
	}
}

proxy::mysql_log::mysql_log(std::ostream &_os) :
	m_os(_os)
{
}

void proxy::mysql_log::put(uint8_t *_data, size_t _size, bool _isClient)
{
	//Only client package contains query
	if (!_isClient) return;

	//The first MySQL package does not contain any structured information processed by this object.
	if (m_firstLog) {
		m_firstLog = false;
		return;
	}

	//Fill first 5 bytes of package
	//This bytes contains information about size and mysql command
	while (!isHeaderFilled() && _size > 0) {
		m_package.byte(m_headerWritten++) = *(_data++);
		_size--;
	}
	m_package.size--; //Remove command byte

	if (isHeaderFilled()) fill(_data, _size);

	//Output a query only when the package is received in full
	//Otherwise keep filling the buffer
	if (isBodyFilled())
	{
		//If package contains query, it will be logged
		if (m_package.comm == COM_QUERY) flush();
		//Clearing is required after any type of incoming package
		clear();
	}
}

void proxy::mysql_log::flush()
{
	m_os << m_prefix << m_package.body << std::endl;
}

void proxy::mysql_log::clear()
{
	m_package.size = 0;
	m_package.comm = 0;
	m_package.body.clear();

	m_headerWritten = 0;
	m_bodyWritten   = 0;
}
