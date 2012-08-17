// Cdn.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/Cdn.h"
#include "ppbox/cdn/VodSegments.h"
#include "ppbox/cdn/Vod2Segments.h"
#include "ppbox/cdn/Live2Segment.h"

#include <framework/logger/LoggerStreamRecord.h>

namespace ppbox
{
    namespace cdn
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("Cdn", 0)

        PPBOX_REGISTER_SEGMENT(ppvod, VodSegments);
        PPBOX_REGISTER_SEGMENT(ppvod2, Vod2Segments);
        PPBOX_REGISTER_SEGMENT(pplive2, Live2Segment);

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
