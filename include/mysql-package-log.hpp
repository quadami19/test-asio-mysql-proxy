#pragma once
#include <ios>
#include "mysql-package.hpp"

namespace proxy {

class mysql_log
{
		mysql_package m_package;
		std::string   m_prefix;
		std::ostream& m_os;
		uint32_t      m_headerWritten = 0;
		uint32_t      m_bodyWritten = 0;
		bool          m_firstLog = true;

		void fill(uint8_t* _data, size_t _size);

	public:

		mysql_log(std::ostream& _os);
		mysql_log(const mysql_log*) = delete;
		~mysql_log() {}

		std::string& prefix() { return m_prefix; }

		void put(uint8_t* _data, size_t _size, bool _isClient);
		void flush();
		void clear();

		bool isHeaderFilled() const { return mysql_package::headerSize() == m_headerWritten; }
		bool isBodyFilled()   const { return m_bodyWritten == m_package.size && m_bodyWritten; }
};

}
