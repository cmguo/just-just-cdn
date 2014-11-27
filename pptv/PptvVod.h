// PptvVod.h

#ifndef _JUST_CDN_PPTV_PPTV_VOD_H_
#define _JUST_CDN_PPTV_PPTV_VOD_H_

#include "just/cdn/pptv/PptvVodInfo.h"
#include "just/cdn/pptv/PptvMedia.h"

namespace just
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

            virtual bool segment_info(
                size_t segment, 
                just::data::SegmentInfo & info,
                boost::system::error_code & ec) const;

        protected:
            void set_segments(
                std::vector<VodSegment> & segments);

        protected:
            std::vector<VodSegment> * segments_;
        };

    } // namespace cdn
} // namespace just

#endif //_JUST_CDN_PPTV_PPTV_VOD_H_
