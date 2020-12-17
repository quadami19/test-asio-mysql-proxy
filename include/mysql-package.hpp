#pragma once

#include <string>
#include "oracle/my_command.h"
#include <bitset>

namespace proxy
{

struct mysql_package
{
	uint32_t    size;
	uint8_t     comm;
	std::string body;

	static uint32_t headerSize() { return sizeof(size) + sizeof(comm); }
	uint8_t& byte(uint32_t _pos) { return *(reinterpret_cast<uint8_t*>(this) + _pos); }
};

}
