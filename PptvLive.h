//PptvLive.h

#ifndef _PPBOX_CDN_LIVE_SEGMENTS_H_
#define _PPBOX_CDN_LIVE_SEGMENTS_H_

#include "ppbox/cdn/PptvLiveInfo.h"
#include "ppbox/cdn/PptvMedia.h"

namespace ppbox
{
    namespace cdn
    {

        class PptvLive
            : public PptvMedia
        {
        public:
            PptvLive(
                boost::asio::io_service & io_svc,
                framework::string::Url const & url);

            ~PptvLive();

        public:
            virtual void on_error(
                boost::system::error_code & ec) const;

        public:
            virtual size_t segment_count() const;

            virtual bool segment_url(
                size_t segment, 
                framework::string::Url & url, 
                boost::system::error_code & ec) const;

            virtual bool segment_info(
                size_t segment, 
                ppbox::data::SegmentInfo & info, 
                boost::system::error_code & ec) const;

        public:
            LiveSegment const & segment() const
            {
                return *segment_;
            }

        protected:
            void set_segment(
                LiveSegment & segment);

        private:
            static bool parse_segment_param(
                LiveSegment & segment, 
                std::string const & param);

        protected:
            LiveSegment * segment_;

            LiveSegment parsed_segment_;
            time_t begin_time_;
        };

    } // namespace cdn
} // namespace ppbox

#endif // _PPBOX_CDN_LIVE_SEGMENTS_H_
