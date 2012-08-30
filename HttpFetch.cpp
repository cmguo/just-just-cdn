// HttpFetch.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/HttpFetch.h"

#include <util/protocol/http/HttpRequest.h>
#include <framework/logger/StreamRecord.h>
#include <framework/logger/StringRecord.h>
using namespace framework::logger;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("HttpFetch", 0);


namespace ppbox
{
    namespace cdn
    {

        HttpFetch::HttpFetch(
            boost::asio::io_service & io_svc)
            : http_(io_svc)
            ,canceled_(false)
            , try_times_(0)
        {
        }

        HttpFetch::~HttpFetch()
        {
        }

        void HttpFetch::async_fetch(
            framework::string::Url const & url
            ,framework::network::NetName const & server_host
            ,response_type const & resp)
        {
            resp_ = resp;
            server_host_ = server_host;

            util::protocol::HttpRequestHead request_head;
            request_head.method = util::protocol::HttpRequestHead::get;
            request_head.path = url.path_all();
            request_head.host.reset(url.host_svc());
            request_head["Accept"] = "{*/*}";

            std::ostringstream oss;
            request_head.get_content(oss);
            LOG_STR(Trace, ("[async_fetch] request_head", oss.str()));

            http_stat_.begin_try();
            http_.async_fetch(request_head,
                boost::bind(&HttpFetch::handle_fetch, this, _1));
        }


        void HttpFetch::cancel()
        {
            boost::system::error_code ec1;
            http_.cancel_forever(ec1);
            canceled_ = true;
        }

        void HttpFetch::close()
        {
            boost::system::error_code  ec;
            http_.close(ec);
        }

        ppbox::cdn::HttpStatistics const & HttpFetch::http_stat()
        {
            http_stat_.total_elapse = http_stat_.elapse();
            return http_stat_;
        }
        
        void HttpFetch::handle_fetch(
            boost::system::error_code const & ec)
        {
            http_stat_.end_try(http_.stat(), ec);
            if (!ec) {
            } else {
                if (!canceled_ && (++try_times_ == 1 || util::protocol::HttpClient::recoverable(ec))) {
                    LOG_DEBUG("[handle_fetch] ec: " << ec.message());
                    http_stat_.begin_try();
                    http_.request_head().host.reset(server_host_.host_svc());
                    http_.async_fetch(http_.request_head(),
                        boost::bind(&HttpFetch::handle_fetch, this, _1));
                    return;
                }
                LOG_WARN("[handle_fetch] ec: " << ec.message());
            }
            //returned_ = 1;
            response(ec);
        }


        void HttpFetch::response(
            boost::system::error_code const & ec)
        {
            response_type resp;
            resp.swap(resp_);
            resp(ec);
        }
        
    } // namespace cdn
} // namespace ppbox
