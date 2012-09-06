// PptvLive.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/CdnError.h"
#include "ppbox/cdn/PptvLive.h"

#include <framework/string/Format.h>
using namespace framework::string;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("PptvLive", 0);

namespace ppbox
{
    namespace cdn
    {

        static const boost::uint32_t LIVE_CACHE_TIME = 1800; // 秒

        PptvLive::PptvLive(
            boost::asio::io_service & io_svc)
            : PptvMedia(io_svc)
            , begin_time_(0)
        {
        }

        PptvLive::~PptvLive()
        {
        }

        size_t PptvLive::segment_count() const
        {
            return size_t(-1);
        }

        boost::system::error_code PptvLive::segment_url(
            size_t segment, 
            framework::string::Url & url,
            boost::system::error_code & ec)
        {
            ec.clear();
            time_t file_time = begin_time_ + (segment * segment_.interval);
            url = url_; //这里使用原始传入的播放url
            url.host(jump_->server_host.host());
            url.svc(jump_->server_host.svc());
            url.path("/live/" + video_->rid + "/" + format(file_time) + ".block");
            LOG_DEBUG("[segment_url] url:" << url.to_string());
            return ec;
        }

        void PptvLive::segment_info(
            size_t segment, 
            ppbox::data::SegmentInfo & info) const
        {
            info.head_size = 0;
            info.size = 0;
            info.duration = segment_.interval;
        }

        boost::system::error_code PptvLive::get_info(
            ppbox::data::MediaInfo & info,
            boost::system::error_code & ec)
        {
            PptvMedia::get_info(info, ec);
            if (!ec) {
                info.is_live = true;
                info.delay = segment_.delay;
            }
            return ec;
        }

        void PptvLive::set_segment(
            LiveSegment const & seg)
        {
            segment_ = seg;

            begin_time_ = jump_->server_time.to_time_t() - LIVE_CACHE_TIME;
            begin_time_ = begin_time_ / segment_.interval * segment_.interval;
        }

    } // cdn
} // ppbox
