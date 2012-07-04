//PeerCoreSegment.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/PeerCoreSegment.h"

#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::logger;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("PeerCoreSegment", 0);
namespace ppbox
{
    namespace cdn
    {
        PeerCoreSegment::PeerCoreSegment(
            boost::asio::io_service & ios)
            : JDSegment(ios)
        {
        }

        PeerCoreSegment::~PeerCoreSegment()
        {
        }

        boost::system::error_code PeerCoreSegment::get_request(
            size_t segment, 
            boost::uint64_t& beg, 
            boost::uint64_t& end, 
            std::string& url,
            boost::system::error_code & ec)
        {
            JDSegment::get_request(segment, beg, end , url , ec);
            return ec;
        }

    }//cdn
}//ppbox


//PeerCoreSegment.cpp