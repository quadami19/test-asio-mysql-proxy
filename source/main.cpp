#include <iostream>

#include "proxy-server.hpp"

using namespace std;

int main()
{
	try {
		proxy::proxy_server mysqlServer("127.0.0.1", 3306, 3305);
		mysqlServer.start();
	}
	catch (std::exception& _e) {
		std::cout << _e.what() << std::endl;
	}
}
