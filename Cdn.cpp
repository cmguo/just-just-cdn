// Cdn.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/Cdn.h"
#include "ppbox/cdn/PptvVod.h"
#include "ppbox/cdn/PptvVod2.h"
#include "ppbox/cdn/PptvLive2.h"

#include <framework/logger/StreamRecord.h>

namespace ppbox
{
    namespace cdn
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("Cdn", 0)

        Cdn::Cdn(
            util::daemon::Daemon & daemon)
            : ppbox::common::CommonModuleBase<Cdn>(daemon, "Cdn")
        {
        }

        Cdn::~Cdn()
        {
        }

        boost::system::error_code Cdn::startup()
        {
            boost::system::error_code ec;
            return ec;
        }

        void Cdn::shutdown()
        {
        }

    } // namespace cdn
} // namespace ppbox
