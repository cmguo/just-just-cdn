//Live2CoreSegment.h

#ifndef _PPBOX_CDN_LIVE2_CORE_SEGMNET_H_
#define _PPBOX_CDN_LIVE2_CORE_SEGMNET_H_

#include "ppbox/cdn/Live2Segment.h"

namespace ppbox
{

    namespace cdn
    {
        class Live2CoreSegment
            : public Live2Segment
        {
        public:
            Live2CoreSegment(boost::asio::io_service & ios);
            ~Live2CoreSegment();

            boost::system::error_code get_request(
                size_t segment, 
                boost::uint64_t& beg, 
                boost::uint64_t& end, 
                std::string& url,
                boost::system::error_code & ec);

            boost::system::error_code get_duration(
                DurationInfo & info,
                boost::system::error_code & ec);

            std::string get_class_name()
            {
                return "Live2CoreSegment";
            }
        };

    }//cdn

}//ppbox

#endif//_PPBOX_CDN_LIVE2_CORE_SEGMNET_H_

//Live2CoreSegment.h