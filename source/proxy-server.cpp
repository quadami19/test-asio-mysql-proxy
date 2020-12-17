#include <iostream>
#include <functional>
#include "proxy-server.hpp"

proxy::proxy_server::proxy_server(std::string _server_address,
								  uint16_t _server_port, uint16_t _client_port) :
	m_context(asio::io_context()),
	m_endpoint(asio::ip::tcp::endpoint(asio::ip::tcp::v4(), _client_port)),
	m_acceptor(m_context, m_endpoint),
	m_address(_server_address),
	m_loggingOs(LOG_FILE)
{
	m_port = std::to_string(_server_port);
}

void proxy::proxy_server::start()
{
	accept();
	std::cout << "[SERVER] Start listening on port " << m_endpoint.port() << "." << std::endl;
	m_context.run();
}

void proxy::proxy_server::accept()
{
	auto callback = [this] (std::error_code _ec, asio::ip::tcp::socket _socket)
	{
		if (!_ec)
		{
			auto newconn = newConnection(_socket);
			m_conSet.insert(newconn);
			std::cout << "[SERVER] " + m_address + ":" + m_port + " New connection established (ID: "
					  << m_connectionEnumerator - 1 << "). Total " << m_conSet.size() << std::endl;
		}
		else
		{
			std::cerr << "[SERVER] New Connection failed. Error: " << _ec.message() << "\n";
		}
		accept();
	};

	m_acceptor.async_accept(callback);
}

proxy::connection_ptr proxy::proxy_server::newConnection(asio::ip::tcp::socket &_socket)
{
	//Functor for freeing connection object memory
	auto deleter = std::bind(&proxy_server::disconnect, this, std::placeholders::_1);
	auto newconn = connection::create(m_context, std::move(_socket), deleter, m_connectionEnumerator++);

	auto log = std::make_unique<mysql_log>(m_loggingOs);
	//Prefix will be added before each logged query
	log->prefix() = std::string("[CONN ") + std::to_string(newconn->id()) + "] " + m_address + ":" + m_port + " Query: ";

	newconn->logging(std::move(log));
	newconn->start(m_address, m_port);
	return newconn;
}

void proxy::proxy_server::disconnect(proxy::connection_ptr _ptr)
{
	std::cout << "[SERVER] Connection (ID: " << _ptr->id() << ") disconnected. Total " << m_conSet.size() - 1 << std::endl;
	m_conSet.erase(_ptr);
}
