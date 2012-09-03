// Cdn.h

#ifndef _PPBOX_CDN_CDN_H_
#define _PPBOX_CDN_CDN_H_

#include "ppbox/cdn/VodSegments.h"
#include "ppbox/cdn/Vod2Segments.h"
#include "ppbox/cdn/Live2Segment.h"

#include <ppbox/common/CommonModuleBase.h>

namespace ppbox
{
    namespace cdn
    {

        PPBOX_REGISTER_SEGMENT(ppvod, VodSegments);
        PPBOX_REGISTER_SEGMENT(ppvod2, Vod2Segments);
        PPBOX_REGISTER_SEGMENT(pplive2, Live2Segment);

        class Cdn
            : public ppbox::common::CommonModuleBase<Cdn>
        {
        public:
            Cdn(
                util::daemon::Daemon & daemon);

            ~Cdn();

        public:
            virtual boost::system::error_code startup();

            virtual void shutdown();
        };

    } // namespace cdn
} // namespace ppbox

#endif // _PPBOX_CDN_CDN_H_
