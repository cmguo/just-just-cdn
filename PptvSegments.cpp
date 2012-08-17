//PptvSegments.cpp
#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/CdnError.h"
#include "ppbox/cdn/PptvSegments.h"

#include <framework/string/Url.h>
#include <framework/string/Format.h>
#include <framework/string/StringToken.h>
using namespace framework::string;
#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::logger;

#include <util/daemon/Daemon.h>
#include <util/protocol/pptv/Url.h>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("PptvSegments", 0);

namespace ppbox
{
    namespace cdn
    {

        PptvSegments::PptvSegments(
            boost::asio::io_service & io_svc)
            : common::SegmentBase(io_svc)
            , open_total_time_(0)
            , fetch_(io_svc)
        {
        }

        PptvSegments::~PptvSegments()
        {
        }


        void PptvSegments::set_url(
            framework::string::Url const &url)
        {
            /*
             * 补充vvid、type、platform
             * cdn.jump.bwtype也在PptvSegments统一处理
             * cdn.jump.server_host 也在PptvSegments统一处理
             * cdn.drag.* 由派生类处理
             * cdn.* 
             * p2p.* 
             */
            framework::string::Url temp_url = url;
            framework::string::Url cdn_jump_param("http://localhost/");

            std::string bwtype = temp_url.param("bwtype");
            if (bwtype != "") {
                cdn_jump_param.param("bwtype", bwtype);
                temp_url.param("bwtype", "");
                temp_url.param("cdn.jump", cdn_jump_param.to_string());
            } 

            common::SegmentBase::set_url(temp_url);

            boost::system::error_code ec;
            std::string playlink = url.path().substr(1);

            if (playlink.size() > 4 && playlink.substr(playlink.size() - 4) == ".mp4") {
                if (playlink.find('%') == std::string::npos) {
                    playlink = Url::encode(playlink, ".");
                }
            } else {
                playlink = util::protocol::pptv::url_decode(playlink, "kioe257ds");
                StringToken st(playlink, "||");
                if (!st.next_token(ec)) {
                    playlink = st.remain();
                }
            }

            jdp_url_.from_string("http://localhost/");
            jdp_url_.path(playlink);
            cdn_url_.from_string("http://localhost/");
            cdn_url_.path(playlink);

            if (jdp_url_.param("vvid").empty()) {
                size_t vvid = rand();
                jdp_url_.param("vvid", format(vvid));
            }
            if (jdp_url_.param("platform").empty()) {
                jdp_url_.param("platform", "ppbox");
            }
            if (jdp_url_.param("type").empty()) {
                jdp_url_.param("type", "ppbox");
            }

        }

        void PptvSegments::cancel()
        {
            fetch_.cancel();
        }

        void PptvSegments::close()
        {
            fetch_.close();
        }

        void PptvSegments::set_response(
            SegmentBase::response_type const & resp)
        {
            resp_ = resp;
        }

        void PptvSegments::response(
            boost::system::error_code const & ec)
        {
            SegmentBase::response_type resp;
            resp.swap(resp_);
            resp(ec);
        }

        void PptvSegments::open_logs_end(
            int index, 
            boost::system::error_code const & ec)
        {
            HttpStatistics const & http_stat = fetch_.http_stat(), 
            if (&http_stat != &open_logs_[index]) {
                open_logs_[index] = http_stat;
            }
            open_logs_[index].total_elapse = open_logs_[index].elapse();
            open_logs_[index].last_last_error = ec;
        }



    }//cdn
}//ppbox
