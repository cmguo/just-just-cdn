//PptvVod.h

#ifndef _PPBOX_CDN_SEGMENT_VOD_H_
#define _PPBOX_CDN_SEGMENT_VOD_H_

#include "ppbox/cdn/PptvMedia.h"
#include "ppbox/cdn/PptvVodInfo.h"

namespace ppbox
{
    namespace cdn
    {

        class PptvVod
            : public PptvMedia
        {
        public:
            PptvVod(
                boost::asio::io_service & io_svc,
                framework::string::Url const & url);

            ~PptvVod();

        public:
            virtual bool get_url(
                framework::string::Url & url,
                boost::system::error_code & ec) const;

        public:
            virtual size_t segment_count() const;

            virtual bool segment_url(
                size_t segment, 
                framework::string::Url & url,
                boost::system::error_code & ec) const;

            virtual void segment_info(
                size_t segment, 
                ppbox::data::SegmentInfo & info) const;

        protected:
            void set_video(
                Video & video);

            void set_segments(
                std::vector<VodSegment> & segments);

        protected:
            std::vector<VodSegment> * segments_;
        };

    } // cdn
} // ppbox

#endif//_PPBOX_CDN_VOD_SEGMENT_H_
