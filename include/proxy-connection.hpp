#pragma once

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/internet.hpp>
#include <asio/ts/buffer.hpp>
#include <memory>
//#include <vector>

#include "mysql-package-log.hpp"

namespace proxy {

class connection : public std::enable_shared_from_this<connection>
{
	public:
		typedef std::shared_ptr<connection> connection_sptr;
		typedef std::function<void(std::shared_ptr<connection>)> stop_transfer_func;
		typedef std::unique_ptr<mysql_log> mysql_log_uptr;

	private:
		static const uint32_t BUFFER_LENGTH = 0x2000;

		std::array<uint8_t, BUFFER_LENGTH> m_clientBuffer;
		std::array<uint8_t, BUFFER_LENGTH> m_serverBuffer;

		//Reference to context in case one need to use several contexts for different connections
		asio::io_context&       m_context;
		asio::ip::tcp::socket   m_clientSocket;
		asio::ip::tcp::socket   m_serverSocket;
		asio::ip::tcp::resolver m_resolver;
		stop_transfer_func      m_stopTransferFunc;
		mysql_log_uptr          m_log_ptr;
		const uint32_t          m_id;

		connection (asio::io_context& _context, asio::ip::tcp::socket&& _clientSocket, stop_transfer_func&& _func, uint32_t _id);

		void connect(const asio::ip::tcp::resolver::results_type &_endpoints);
		void read(asio::mutable_buffer _buffer, bool _isClient);
		void transfer(asio::mutable_buffer _buffer, std::size_t _transferred, bool _isClient);
		void error(std::error_code _ec, bool _isClient);
		void shutdown(bool _isClient);

	public:

		//Connection must exist as shared object
		static connection_sptr create(asio::io_context& _context, asio::ip::tcp::socket&& _clientSocket, stop_transfer_func&& _func, uint32_t _id)
			{ return connection_sptr(new connection (_context, std::move(_clientSocket), std::move(_func), _id)); }

		void start(std::string_view _serverAddress, std::string_view _port);
		void logging(mysql_log_uptr&& _ptr) { m_log_ptr = std::move(_ptr); }

		uint32_t id() const { return m_id; }

		connection (const connection&) = delete;
		~connection() {}
};

typedef std::shared_ptr<connection> connection_ptr;

}
