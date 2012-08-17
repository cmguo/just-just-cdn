//LiveSegment.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/CdnError.h"
#include "ppbox/cdn/LiveSegment.h"

#include <ppbox/common/HttpFetchManager.h>

#include <util/protocol/pptv/Url.h>
#include <util/protocol/pptv/TimeKey.h>
#include <util/protocol/pptv/Base64.h>
#include <util/archive/XmlIArchive.h>
#include <util/archive/ArchiveBuffer.h>

#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::logger;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("LiveSegment", 0);

#ifndef PPBOX_DNS_LIVE_JUMP
#define PPBOX_DNS_LIVE_JUMP "(tcp)(v4)livejump.150hi.com:80"
#endif

namespace ppbox
{
    namespace cdn
    {
        static const  framework::network::NetName dns_live_jump_server(PPBOX_DNS_LIVE_JUMP);

    public:
        LiveSegment::LiveSegment(
            boost::asio::io_service & io_svc)
            : 
        {
            
        }

        LiveSegment::~LiveSegment()
        {

        }

    public:
        void LiveSegment::async_open(
            OpenMode mode,
            response_type const & resp)
        {
            
        }

        void LiveSegment::cancel(
            boost::system::error_code & ec) 
        {
            fetch_mgr_.cancel(handle_);
        }

        void LiveSegment::close(
            boost::system::error_code & ec)
        {
            fetch_mgr_.close(handle_);
        }

        bool LiveSegment::is_open()
        {
            
        }

        framework::string::Url LiveSegment::get_jump_url() const
        {
            framework::string::Url url("http://localhost/");
            url.host(dns_live_jump_server.host());
            url.svc(dns_live_jump_server.svc());
            url.path("/live1/" + Url::encode(channel_));

            return url;
        }

        void LiveSegment::set_url(
            std::string const & url)
        {
            std::string::size_type slash = name.find('|');
            if (slash == std::string::npos) 
            {
                return;
            } 
            key_ = name.substr(0, slash);
            std::string url = name.substr(slash + 1);
            url = framework::string::Url::decode(url);
            framework::string::Url request_url(url);
            url = request_url.path().substr(1);
            url_ = pptv::base64_decode(url, key_);
            if (!url_.empty()) {
                map_find(url_, "name", name_, "&");
                map_find(url_, "channel", channel_, "&");
            }
        }

    }//cdn
}//ppbox



