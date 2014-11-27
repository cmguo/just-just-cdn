// PptvLive.cpp

#include "just/cdn/Common.h"
#include "just/cdn/CdnError.h"
#include "just/cdn/pptv/PptvLive.h"

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>

#include <framework/string/Format.h>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("just.cdn.PptvLive", framework::logger::Debug);

namespace just
{
    namespace cdn
    {

        PptvLive::PptvLive(
            boost::asio::io_service & io_svc,
            framework::string::Url const & url)
            : PptvMedia(io_svc, url)
            , segment_(NULL)
            , begin_time_(0)
        {
            just::data::MediaBasicInfo info;
            info.type = just::data::MediaBasicInfo::live;
            info.flags = just::data::MediaBasicInfo::f_segment;
            info.flags |= just::data::SegmentMediaFlags::f_segment_seek;
            info.flags |= just::data::SegmentMediaFlags::f_fix_duration;
            info.flags |= just::data::SegmentMediaFlags::f_time_smoth;
            //info.format = "flv"; // auto detect
            set_basic_info(info);

            if (parse_segment_param(parsed_segment_, url_.param("cdn.segment"))) {
                segment_ = &parsed_segment_;
            }
            url_.param("cdn.segment", "");
        }

        PptvLive::~PptvLive()
        {
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
            url = url_; //����ʹ��ԭʼ����Ĳ���url
            url.host(jump_->server_host.host());
            url.svc(jump_->server_host.svc());
            url.path("/live/" + video_->rid + "/" + framework::string::format(file_time) + ".block");
            LOG_DEBUG("[segment_url] url:" << url.to_string());
            return true;
        }

        bool PptvLive::segment_info(
            size_t segment, 
            just::data::SegmentInfo & info, 
            boost::system::error_code & ec) const
        {
            ec.clear();
            info.head_size = invalid_size;
            info.size = invalid_size;
            info.duration = segment_->interval * 1000;
            return true;
        }

        void PptvLive::set_segment(
            LiveSegment & segment)
        {
            if (segment_ == NULL) {
                segment_ = &segment;
                LOG_INFO("[set segment] interval: " << segment.interval);
            }

            if (jump_ && video_) {
                begin_time_ = jump_->server_time.to_time_t() - video_->shift / 1000;
                begin_time_ = begin_time_ / segment_->interval * segment_->interval;
                LOG_INFO("[set segment] begin_time: " << begin_time_);
            }
        }

        bool PptvLive::parse_segment_param(
            LiveSegment & segment, 
            std::string const & param)
        {
            boost::system::error_code ec = 
                framework::string::map_find(param, "interval", segment.interval, JUST_CDN_PARAM_DELIM);
            return !ec;
        }

    } // namespace cdn
} // namespace just
