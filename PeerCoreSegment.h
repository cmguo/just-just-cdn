//PeerCoreSegment.h

#ifndef _PPBOX_CDN_PEER_COER_SEGMNET_H_
#define _PPBOX_CDN_PEER_COER_SEGMNET_H_

#include "ppbox/cdn/JDSegment.h"

namespace ppbox
{

    namespace cdn
    {
        class PeerCoreSegment
            : public JDSegment
        {
        public:
            PeerCoreSegment(boost::asio::io_service & ios);

            ~PeerCoreSegment();

            boost::system::error_code get_request(
                size_t segment, 
                boost::uint64_t& beg, 
                boost::uint64_t& end, 
                std::string& url,
                boost::system::error_code & ec);

            std::string get_class_name()
            {
                return "PeerCoreSegment";
            }


        };//PeerCoreSegment

    }//cdn
}//ppbox


#endif//_PPBOX_CDN_PEER_COER_SEGMNET_H_
