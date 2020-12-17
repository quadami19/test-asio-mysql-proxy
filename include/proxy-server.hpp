#pragma once

#include <set>
#include <fstream>
#include "proxy-connection.hpp"
#include "mysql-package-log.hpp"

const std::string LOG_FILE = "mysql_outstream";

namespace proxy {

class proxy_server
{
		//Single context for all connections
		asio::io_context m_context;
		const asio::ip::tcp::endpoint m_endpoint;
		asio::ip::tcp::acceptor m_acceptor;

		uint32_t m_connectionEnumerator = 0;
		std::string m_address;
		std::string m_port;
		std::set<connection_ptr> m_conSet;

		std::ofstream m_loggingOs;

		void accept();
		connection_ptr newConnection (asio::ip::tcp::socket& _socket);
		//Disconnecting is called from connection object when all its sockets are closed
		void disconnect(connection_ptr _ptr);

	public:

		proxy_server(std::string _server_address,
					 uint16_t _server_port,
					 uint16_t _client_port);
		~proxy_server() { m_loggingOs.close(); }

		void start();
};

}
