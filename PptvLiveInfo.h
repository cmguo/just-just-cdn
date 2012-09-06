// PptvLiveInfo.h

#ifndef _PPBOX_CDN_LIVE_INFO_H_
#define _PPBOX_CDN_LIVE_INFO_H_

#include "ppbox/cdn/PptvMeidaInfo.h"

namespace ppbox
{
    namespace cdn
    {

        struct LiveSegment
        {
            boost::uint16_t delay;
            boost::uint16_t interval;
        };

    } // namespace cdn
} // namespace ppbox

#endif//_PPBOX_CDN_LIVE_INFO_H_
