//JDSegment.h

#ifndef _PPBOX_CDN_JD_SEGMENT_H_
#define _PPBOX_CDN_JD_SEGMENT_H_

#include "ppbox/cdn/VodSegments.h"

namespace ppbox
{
    namespace cdn
    {
        class JDSegment
            : public VodSegments
        {
        public:

            JDSegment(boost::asio::io_service & ios);

            ~JDSegment();

            boost::system::error_code get_request(
                size_t segment, 
                boost::uint64_t& beg, 
                boost::uint64_t& end, 
                std::string& url,
                boost::system::error_code & ec);

            std::string get_class_name()
            {
                return "JDSegment";
            }
        };
    }
}

#endif//_PPBOX_CDN_JD_SEGMENT_H_
