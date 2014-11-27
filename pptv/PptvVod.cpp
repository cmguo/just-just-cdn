// PptvVod.cpp

#include "just/cdn/Common.h"
#include "just/cdn/CdnError.h"
#include "just/cdn/pptv/PptvVod.h"

#include <framework/string/Url.h>
#include <framework/string/Format.h>
#include <framework/logger/StreamRecord.h>
#include <framework/system/LogicError.h>
using namespace framework::string;
using namespace framework::system;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("just.cdn.PptvVod", framework::logger::Debug);

namespace just
{
    namespace cdn
    {

        PptvVod::PptvVod(
            boost::asio::io_service & io_svc,
            framework::string::Url const & url)
            : PptvMedia(io_svc, url)
            , segments_(NULL)
        {
            just::data::MediaBasicInfo info;
            info.flags = just::data::MediaBasicInfo::f_segment;
            info.flags |= just::data::SegmentMediaFlags::f_segment_seek;
            // info.format = "mp4"; // auto detect
            set_basic_info(info);
        }

        PptvVod::~PptvVod()
        {
        }

        bool PptvVod::get_url(
            framework::string::Url & url,
            boost::system::error_code & ec) const
        {
            url = url_;
            url.svc("80");
            url.param("w", "1");
            url.param("z", "1");
            url.param("key", get_key());
            ec.clear();
            return true;
        }

        size_t PptvVod::segment_count() const
        {
            size_t ret = size_t(-1);
            if (segments_)
                ret = segments_->size();
            return ret;
        }

        bool PptvVod::segment_url(
            size_t segment, 
            framework::string::Url & url,
            boost::system::error_code & ec) const
        {
            if (segments_ && segment >= segments_->size()) {
                ec = logic_error::item_not_exist;
                return false;
            }
            url = url_;
            url.protocol("http");
            url.path("/" + format(segment) + url.path());
            url.param("key", get_key());
            ec.clear();
            return true;
        }

        bool PptvVod::segment_info(
            size_t segment,
            just::data::SegmentInfo & info,
            boost::system::error_code & ec) const
        {
            ec.clear();
            if (segments_ && segment < segments_->size()) {
                info.head_size = segments_->at(segment).head_length;
                info.size = segments_->at(segment).file_length;
                info.offset = segments_->at(segment).offset;
                info.duration = segments_->at(segment).duration;
            } else {
                info.head_size = boost::uint64_t(-1);
                info.size = boost::uint64_t(-1);
                info.offset = boost::uint64_t(-1);
                info.duration = boost::uint64_t(-1);
            }
            return true;
        }

        void PptvVod::set_segments(
            std::vector<VodSegment> & segments)
        {
            if (segments_ == NULL) {
                segments_ = &segments;
                LOG_INFO("[set segments] count: " << segments.size());
            }
        }

    } //cdn
} //just
