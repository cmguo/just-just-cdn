//VodCoreSegment.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/VodCoreSegment.h"

#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::logger;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("VodCoreSegment", 0);
namespace ppbox
{
    namespace cdn
    {
        VodCoreSegment::VodCoreSegment(
            boost::asio::io_service & ios)
            : JDSegment(ios)
        {
        }

        VodCoreSegment::~VodCoreSegment()
        {
        }

        boost::system::error_code VodCoreSegment::get_request(
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

//VodCoreSegment.cpp
