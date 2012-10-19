// PptvLive.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/CdnError.h"
#include "ppbox/cdn/PptvLive.h"

#include <framework/string/Format.h>
using namespace framework::string;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.cdn.PptvLive", framework::logger::Debug);

namespace ppbox
{
    namespace cdn
    {

        PptvLive::PptvLive(
            boost::asio::io_service & io_svc)
            : PptvMedia(io_svc)
            , segment_(NULL)
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

        void PptvLive::on_error(
            boost::system::error_code & ec) const
        {
            if (ec == util::protocol::http_error::not_found)
                ec = boost::asio::error::would_block;
        }

        size_t PptvLive::segment_count() const
        {
            return size_t(-1);
        }

        bool PptvLive::segment_url(
            size_t segment, 
            framework::string::Url & url,
            boost::system::error_code & ec) const
        {
            ec.clear();
            time_t file_time = begin_time_ + (segment * segment_->interval);
            url = url_; //这里使用原始传入的播放url
            url.host(jump_->server_host.host());
            url.svc(jump_->server_host.svc());
            url.path("/live/" + video_->rid + "/" + format(file_time) + ".block");
            LOG_DEBUG("[segment_url] url:" << url.to_string());
            return true;
        }

        void PptvLive::segment_info(
            size_t segment, 
            ppbox::data::SegmentInfo & info) const
        {
            info.head_size = invalid_size;
            info.size = invalid_size;
            info.duration = segment_->interval * 1000;
        }

        void PptvLive::set_video(
            Video & video)
        {
            video.format = "flv";
            video.is_live = true;
            if (video.duration == invalid_size)
                video.duration = video.delay;
            if (video.current == 0)
                video.duration = video.duration;

            PptvMedia::set_video(video);
        }

        void PptvLive::set_segment(
            LiveSegment & segment)
        {
            if (segment_ == NULL) {
                segment_ = &segment;
                LOG_INFO("[set segment] interval: " << segment.interval);
            }

            if (jump_ && video_) {
                begin_time_ = jump_->server_time.to_time_t() - video_->duration / 1000;
                begin_time_ = begin_time_ / segment_->interval * segment_->interval;
                LOG_INFO("[set segment] begin_time: " << begin_time_);
            }
        }

        bool PptvLive::parse_segment_param(
            LiveSegment & segment, 
            std::string const & param)
        {
            boost::system::error_code ec = 
                map_find(param, "interval", segment.interval, "&");
            return !ec;
        }

    } // namespace cdn
} // namespace ppbox
