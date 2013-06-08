#pragma once

#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <socket/socket_helpers.hpp>
#include <socket/connection.hpp>
#include <strEx.h>

namespace socket_helpers {

	namespace server {
		namespace str = nscp::helpers;
		
		class server_exception : public std::exception {
			std::string error;
		public:
			//////////////////////////////////////////////////////////////////////////
			/// Constructor takes an error message.
			/// @param error the error message
			///
			/// @author mickem
			//server_exception(std::wstring error) : error(to_string(error)) {}
			server_exception(std::string error) : error(error) {}
			~server_exception() throw() {}

			//////////////////////////////////////////////////////////////////////////
			/// Retrieve the error message from the exception.
			/// @return the error message
			///
			/// @author mickem
			const char* what() const throw() { return error.c_str(); }

		};

		namespace ip = boost::asio::ip;

		template<class protocol_type, std::size_t N>
		class server : private boost::noncopyable {


			typedef socket_helpers::server::connection<protocol_type, N> connection_type;
			typedef socket_helpers::server::tcp_connection<protocol_type, N> tcp_connection_type;
#ifdef USE_SSL
			typedef socket_helpers::server::ssl_connection<protocol_type, N> ssl_connection_type;
#endif

			boost::asio::io_service io_service_;
			boost::asio::ip::tcp::acceptor acceptor_v4;
			boost::asio::ip::tcp::acceptor acceptor_v6;
			boost::asio::strand accept_strand_;
			socket_helpers::connection_info info_;
			typename protocol_type::handler_type handler_;
			boost::shared_ptr<protocol_type> logger_;
#ifdef USE_SSL
			boost::asio::ssl::context context_;
#endif

			boost::shared_ptr<connection_type> new_connection_;
			boost::thread_group thread_group_;
		public:
			server(socket_helpers::connection_info info, typename protocol_type::handler_type handler)
				: info_(info)
				, handler_(handler)
				, acceptor_v4(io_service_)
				, acceptor_v6(io_service_)
				, accept_strand_(io_service_)
				, logger_(protocol_type::create(info_, handler_))
#ifdef USE_SSL
				, context_(io_service_, boost::asio::ssl::context::sslv23)
#endif
			{
			}
			~server() {

			}

			bool start() {
				boost::system::error_code er;
				ip::tcp::resolver resolver(io_service_);
				ip::tcp::resolver::iterator endpoint_iterator;
				if (info_.back_log == connection_info::backlog_default)
					info_.back_log = boost::asio::socket_base::max_connections;

				if (info_.address.empty()) {
					endpoint_iterator = resolver.resolve(ip::tcp::resolver::query(info_.get_port()));
				} else {
					endpoint_iterator = resolver.resolve(ip::tcp::resolver::query(info_.get_address(), info_.get_port()));
				}
				ip::tcp::resolver::iterator end;
				if (endpoint_iterator == end) {
					logger_->log_error(__FILE__, __LINE__, "Failed to lookup: " + info_.get_endpoint_string());
					return false;
				}
				if (info_.ssl.enabled) {
#ifdef USE_SSL
					std::list<std::string> errors;
					info_.ssl.configure_ssl_context(context_, errors);
					BOOST_FOREACH(const std::string &e, errors) {
						logger_->log_error(__FILE__, __LINE__, e);
					}
#else
					logger_->log_error(__FILE__, __LINE__, "Not compiled with SSL");
					return false;
#endif
				}
				new_connection_.reset(create_connection());

				for (;endpoint_iterator != end;++endpoint_iterator) {
					ip::tcp::endpoint endpoint = *endpoint_iterator;

					std::stringstream ss;
					ss << endpoint;
					if (endpoint.address().is_v4()) {
						ss << "(ipv4)";
						logger_->log_debug(__FILE__, __LINE__, "Binding to: " + ss.str());
						if (!setup_acceptor(acceptor_v4, endpoint))
							return false;
					} else if (endpoint.address().is_v6()) {
						ss << "(ipv6)";
						logger_->log_debug(__FILE__, __LINE__, "Binding to: " + ss.str());
						if (!setup_acceptor(acceptor_v6, endpoint))
							return false;
					} else {
						logger_->log_error(__FILE__, __LINE__, "Invalid protocol (ignoring): " + ss.str());
						continue;
					}
					ss << endpoint;
				}

				if (acceptor_v4.is_open())
					acceptor_v4.async_accept(new_connection_->get_socket(),accept_strand_.wrap(
						boost::bind(&server::handle_accept, this, false, boost::asio::placeholders::error)
					));
				if (acceptor_v6.is_open())
					acceptor_v6.async_accept(new_connection_->get_socket(),accept_strand_.wrap(
						boost::bind(&server::handle_accept, this, true, boost::asio::placeholders::error)
						));

				for (std::size_t i = 0; i < info_.thread_pool_size; ++i) {
					thread_group_.create_thread(boost::bind(&boost::asio::io_service::run, &io_service_));
				}
			}

			bool setup_acceptor(boost::asio::ip::tcp::acceptor &acceptor, ip::tcp::endpoint &endpoint) {
				boost::system::error_code er;
				if (acceptor.is_open()) {
					logger_->log_error(__FILE__, __LINE__, "Acceptor is already open (cant bind multiple interfaces)");
					return true;
				}
				acceptor.open(endpoint.protocol(), er);
				if (er) {
					logger_->log_error(__FILE__, __LINE__, "Failed to set reuse on socket: " + er.message());
					return false;
				}

				logger_->log_debug(__FILE__, __LINE__, "Attempting to bind to: " + info_.get_endpoint_string());
				acceptor.bind(endpoint, er);
				if (er) {
					logger_->log_error(__FILE__, __LINE__, "Failed to bind: " + er.message());
					return false;
				}

				acceptor.listen(info_.back_log, er);
				if (er) {
					logger_->log_error(__FILE__, __LINE__, "Failed to open: " + er.message());
					return false;
				}
				return true;
			}


// 			std::string get_password() const
// 			{
// 				logger_->log_error(__FILE__, __LINE__, "Getting password...");
// 				return "test";
// 			}
			void stop() {
				io_service_.stop();
				thread_group_.join_all();
			}

		private:
			void handle_accept(bool ipv6, const boost::system::error_code& e) {
				if (!e) {
					std::list<std::string> errors;
					if (logger_->on_accept(new_connection_->get_socket())) {
						new_connection_->start();
					} else {
						new_connection_->on_done(false);
					}
					new_connection_.reset(create_connection());

					if (ipv6)
 						acceptor_v6.async_accept(new_connection_->get_socket(),
 							accept_strand_.wrap(
 							boost::bind(&server::handle_accept, this, ipv6, boost::asio::placeholders::error)
 							)
 							);
					else
						acceptor_v4.async_accept(new_connection_->get_socket(),
						accept_strand_.wrap(
						boost::bind(&server::handle_accept, this, ipv6, boost::asio::placeholders::error)
						)
						);
				} else {
					logger_->log_error(__FILE__, __LINE__, "Socket ERROR: " + e.message());
				}
			}

			connection_type* create_connection() {
#ifdef USE_SSL
				if (info_.ssl.enabled) {
					return new ssl_connection_type(io_service_, context_, protocol_type::create(info_, handler_));
				}
#endif
				return new tcp_connection_type(io_service_, protocol_type::create(info_, handler_));
			}
		};
	} // namespace server
} // namespace nrpe
