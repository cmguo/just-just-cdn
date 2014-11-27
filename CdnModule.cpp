// CdnModule.cpp

#include "just/cdn/Common.h"
#include "just/cdn/CdnModule.h"

#ifndef JUST_CDN_DISABLE_PPTV
#  include "just/cdn/pptv/PptvMediaTypes.h"
#  include "just/cdn/pptv/P2pSource.h"
#endif

#ifndef JUST_CDN_DISABLE_TRIP
#  include "just/cdn/trip/TripMediaTypes.h"
#endif

#include <framework/logger/StreamRecord.h>

namespace just
{
    namespace cdn
    {

        //FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("just.cdn.CdnModule", Debug)

        CdnModule::CdnModule(
            util::daemon::Daemon & daemon)
            : just::common::CommonModuleBase<CdnModule>(daemon, "CdnModule")
        {
        }

        CdnModule::~CdnModule()
        {
        }

        boost::system::error_code CdnModule::startup()
        {
            boost::system::error_code ec;
            return ec;
        }

        void CdnModule::shutdown()
        {
        }

    } // namespace cdn
} // namespace just
