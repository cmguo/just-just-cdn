// PptvVod.h

#ifndef _PPBOX_CDN_PPTV_PPTV_VOD_H_
#define _PPBOX_CDN_PPTV_PPTV_VOD_H_

#include "ppbox/cdn/pptv/PptvVodInfo.h"
#include "ppbox/cdn/pptv/PptvMedia.h"

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

            virtual bool segment_info(
                size_t segment, 
                ppbox::data::SegmentInfo & info,
                boost::system::error_code & ec) const;

        protected:
            void set_segments(
                std::vector<VodSegment> & segments);

        protected:
            std::vector<VodSegment> * segments_;
        };

    } // namespace cdn
} // namespace ppbox

#endif //_PPBOX_CDN_PPTV_PPTV_VOD_H_