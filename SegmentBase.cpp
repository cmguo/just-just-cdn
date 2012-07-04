//SegmentBase.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/SegmentBase.h"
#include "ppbox/cdn/CdnError.h"

#include <framework/logger/LoggerStreamRecord.h>

using namespace framework::logger;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("SegmentBase", 0);

namespace ppbox
{
    namespace cdn
    {

        SegmentBase::SegmentBase(
            boost::asio::io_service & io_svc)
            : ios_(io_svc)
        {
        }

        SegmentBase::~SegmentBase()
        {
        }

        void SegmentBase::set_url(std::string const &url)
        {
            url_ = url;
        }

        boost::asio::io_service& SegmentBase::ios_service()
        {
            return ios_;
        }


    }//cdn
}//ppbox


//SegmentBase.cpp