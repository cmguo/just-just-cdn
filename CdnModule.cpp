// CdnModule.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/CdnModule.h"

#ifndef PPBOX_CDN_DISABLE_PPTV
#  include "ppbox/cdn/pptv/PptvMediaTypes.h"
#  include "ppbox/cdn/pptv/P2pSource.h"
#endif

#ifndef PPBOX_CDN_DISABLE_TRIP
#  include "ppbox/cdn/trip/TripMediaTypes.h"
#endif

#include <framework/logger/StreamRecord.h>

namespace ppbox
{
    namespace cdn
    {

        //FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.cdn.CdnModule", Debug)

        CdnModule::CdnModule(
            util::daemon::Daemon & daemon)
            : ppbox::common::CommonModuleBase<CdnModule>(daemon, "CdnModule")
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
} // namespace ppbox
