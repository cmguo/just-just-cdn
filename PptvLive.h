//PptvLive.h

#ifndef _PPBOX_CDN_LIVE_SEGMENTS_H_
#define _PPBOX_CDN_LIVE_SEGMENTS_H_

#include "ppbox/cdn/PptvMedia.h"
#include "ppbox/cdn/PptvLiveInfo.h"

namespace ppbox
{
    namespace cdn
    {

        class PptvLive
            : public PptvMedia
        {
        public:
            PptvLive(
                boost::asio::io_service & io_svc);

            ~PptvLive();

        public:
            void set_url(
                framework::string::Url const & url);

        public:
            boost::system::error_code get_info(
                ppbox::data::MediaInfo & info,
                boost::system::error_code & ec);

        public:
            size_t segment_count() const;

            boost::system::error_code segment_url(
                size_t segment, 
                framework::string::Url & url, 
                boost::system::error_code & ec);

            void segment_info(
                size_t segment, 
                ppbox::data::SegmentInfo & info) const;

        public:
            LiveSegment const & segment() const
            {
                return segment_;
            }

        protected:
            void set_segment(
                LiveSegment const & seg);

        protected:
            time_t begin_time_;
            LiveSegment segment_;
        };

    } // namespace cdn
} // namespace ppbox

#endif // _PPBOX_CDN_LIVE_SEGMENTS_H_
