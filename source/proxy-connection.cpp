#include <iostream>
#include "proxy-connection.hpp"

namespace proxy {

connection::connection(asio::io_context& _context, asio::ip::tcp::socket &&_clientSocket, stop_transfer_func&& _func, uint32_t _id) :
	m_context(_context),
	m_clientSocket(std::move(_clientSocket)),
	m_serverSocket(asio::ip::tcp::socket(m_context)),
	m_resolver(m_context),
	m_stopTransferFunc(std::move(_func)),
	m_id(_id)
{
}

void connection::connect(const asio::ip::tcp::resolver::results_type& _endpoints)
{
	auto callback = [this] (std::error_code _ec, asio::ip::tcp::endpoint _endpoint)
	{
		if (!_ec) {
			read (asio::buffer(m_clientBuffer), true);
			read (asio::buffer(m_serverBuffer), false);
		} else {
			error(_ec, false);
		}
	};

	//Trying connect to server socket
	//Client socket is already connected
	asio::async_connect(m_serverSocket, _endpoints, callback);
}

void connection::read(asio::mutable_buffer _buffer, bool _isClient)
{
	asio::ip::tcp::socket& sFrom = _isClient ? m_clientSocket : m_serverSocket;
	auto callback = [this, _buffer, _isClient] (std::error_code _ec, size_t _transferred)
	{
		if (!_ec) {
			//If the package contains a request, it is logged
			if (m_log_ptr) m_log_ptr->put((uint8_t*)_buffer.data(), _transferred, _isClient);

			//Transferring a received packet to another socket
			transfer(_buffer, _transferred, _isClient);
		} else {
			error(_ec, _isClient);
		}
	};

	sFrom.async_receive(_buffer, callback);
}

void connection::transfer(asio::mutable_buffer _buffer, std::size_t _transferred, bool _isClient)
{
	asio::ip::tcp::socket& sTo = _isClient ? m_serverSocket : m_clientSocket;

	auto callback = [this, _buffer, _isClient](std::error_code _ec, size_t _transferred)
	{
		//After transferring a data, start further reading from the socket
		if (!_ec) read(_buffer, _isClient);
		else error(_ec, _isClient);
	};

	sTo.async_send(asio::buffer(_buffer, _transferred), callback);
}

void connection::error(std::error_code _ec, bool _isClient)
{
	switch (_ec.value())
	{
		case asio::error::eof:
			//Shutdown the connection if there is no more data in the socket
			shutdown(_isClient);
			break;
		default:
			//Shutdown the connection in case of unexpected error
			std::cerr << "[CONN " << m_id << "] Unhandled error (id " << _ec.value() << "): " << _ec.message() << "\n";
			shutdown(_isClient);
	}
}

void connection::shutdown(bool _isClient)
{
	//Due to the processing of two asynchronous transfers,
	//sockets must be closed sequentially in two function calls.
	if (_isClient)
	{
		m_clientSocket.shutdown(asio::ip::tcp::socket::shutdown_both);
		m_clientSocket.close();
	}
	else
	{
		m_serverSocket.shutdown(asio::ip::tcp::socket::shutdown_both);
		m_serverSocket.close();
	}

	if (!m_serverSocket.is_open() && !m_clientSocket.is_open())
		m_stopTransferFunc(shared_from_this());
}

void connection::start(std::string_view _serverAddress, std::string_view _port)
{
	asio::ip::tcp::resolver::results_type endpoints = m_resolver.resolve(_serverAddress, _port);
	connect(endpoints);
}

}
