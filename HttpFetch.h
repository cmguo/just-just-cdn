// HttpFetch.h

#ifndef _JUST_VOD_HTTPFETCH_H_
#define _JUST_VOD_HTTPFETCH_H_

#include <just/cdn/HttpStatistics.h>

#include <util/protocol/http/HttpClient.h>

#include <framework/network/NetName.h>

#include <boost/asio/io_service.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace just
{
    namespace cdn
    {
        class HttpFetch
            : public boost::enable_shared_from_this<HttpFetch>
        {
        public:
            typedef boost::function<void (
                boost::system::error_code const &)
            > response_type;

            HttpFetch(
                boost::asio::io_service & io_svc);

            ~HttpFetch();

        public:
            void async_fetch(
                framework::string::Url const & url, 
                framework::network::NetName const & server_host, 
                response_type const & resp);

            boost::asio::streambuf & data()
            {
                return http_.response_data();
            }

            void cancel(
                boost::system::error_code & ec);

            void detach(); // cancel but not response

            void close(
                boost::system::error_code & ec);

        public:
            just::cdn::HttpStatistics const & http_stat();

            util::protocol::HttpRequest const & http_request()
            {
                return http_.request();
            }

        private:
            void handle_fetch(
                boost::system::error_code const & ec);

            void response(
                boost::system::error_code const & ec);

        private:
            util::protocol::HttpClient http_;
            just::cdn::HttpStatistics http_stat_;
            framework::network::NetName server_host_;

            boost::mutex mutex_;
            bool canceled_;
            response_type resp_;
            size_t try_times_;
        };

    } // namespace cdn
} // namespace just

#endif // _JUST_VOD_HTTPFETCH_H_
