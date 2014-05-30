// CdnModule.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/CdnModule.h"
#include "ppbox/cdn/PptvMediaTypes.h"
#include "ppbox/cdn/P2pSource.h"

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
