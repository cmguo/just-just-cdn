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

        PptvLive::PptvLive(
            boost::asio::io_service & io_svc)
            : PptvMedia(io_svc)
            , begin_time_(0)
        {
        }

        PptvLive::~PptvLive()
        {
        }

        void PptvLive::set_url(
            framework::string::Url const & url)
        {
            PptvMedia::set_url(url);

            if (parse_segment_param(parsed_segment_, url_.param("cdn.segment"))) {
                segment_ = &parsed_segment_;
            }
            url_.param("cdn.segment", "");
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
            time_t file_time = begin_time_ + (segment * segment_->interval);
            url = url_; //����ʹ��ԭʼ����Ĳ���url
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
            info.duration = segment_->interval;
        }

        boost::system::error_code PptvLive::get_info(
            ppbox::data::MediaInfo & info,
            boost::system::error_code & ec)
        {
            PptvMedia::get_info(info, ec);
            if (!ec) {
                info.is_live = true;
            }
            return ec;
        }

        void PptvLive::set_video(
            Video & video)
        {
            if (video.duration == 0)
                video.duration = video.delay;

            PptvMedia::set_video(video);
        }

        void PptvLive::set_segment(
            LiveSegment & seg)
        {
            if (jump_ == NULL)
                segment_ = &seg;

            if (jump_ && video_) {
                begin_time_ = jump_->server_time.to_time_t() - video_->duration;
                begin_time_ = begin_time_ / segment_->interval * segment_->interval;
            }
        }

        bool parse_segment_param(
            LiveSegment & segment, 
            std::string const & param)
        {
            boost::system::error_code ec = 
                map_find(param, "interval", segment.interval, "&");
            return !ec;
        }

    } // namespace cdn
} // namespace ppbox
