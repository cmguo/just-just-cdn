// PptvVod.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/CdnError.h"
#include "ppbox/cdn/PptvVod.h"

#include <framework/string/Url.h>
#include <framework/string/Format.h>
#include <framework/logger/StreamRecord.h>
using namespace framework::string;
using namespace framework::logger;
using namespace boost::system;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("PptvVod", 0);

namespace ppbox
{
    namespace cdn
    {

        PptvVod::PptvVod(
            boost::asio::io_service & io_svc)
            : PptvMedia(io_svc)
        {
        }

        PptvVod::~PptvVod()
        {
        }

        void PptvVod::set_url(
            framework::string::Url const & url)
        {
            PptvMedia::set_url(url);
        }

        boost::system::error_code PptvVod::segment_url(
            size_t segment, 
            framework::string::Url & url,
            boost::system::error_code & ec)
        {
            ec.clear();
            if (segment < segments_->size()) {
                url = url_;
                url.host(jump_->server_host.host());
                url.svc(jump_->server_host.svc());
                url.path("/" + format(segment) + std::string("/") + url.path());
                url.param("key", get_key());
                LOG_DEBUG("[segment_url] url:"<< url.to_string());
            } else {
                ec = error::item_not_exist;
            }
            return ec;
        }

        size_t PptvVod::segment_count() const
        {
            size_t ret = size_t(-1);
            ret = segments_->size();
            return ret;
        }

        void PptvVod::segment_info(
            size_t segment,
            ppbox::data::SegmentInfo & info) const
        {
            if (segments_->size() > segment) {
                info.head_size = segments_->at(segment).head_length;
                info.size = segments_->at(segment).file_length;
                info.offset = segments_->at(segment).offset;
                info.duration = segments_->at(segment).duration;
            } else {
                info.head_size = boost::uint64_t(-1);
                info.size = boost::uint64_t(-1);
                info.duration = boost::uint64_t(-1);
            }
        } 

        boost::system::error_code PptvVod::get_info(
            ppbox::data::MediaInfo & info,
            boost::system::error_code & ec)
        {
            return PptvMedia::get_info(info, ec);
        }

        void PptvVod::set_segments(
            std::vector<VodSegment> & segments)
        {
            segments_ = &segments;
        }

    } //cdn
} //ppbox
