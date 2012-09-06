// PptvLiveInfo2.h

#ifndef _PPBOX_CDN_LIVE2_INFO_H_
#define _PPBOX_CDN_LIVE2_INFO_H_

#include "ppbox/cdn/PptvLiveInfo.h"

namespace ppbox
{
    namespace cdn
    {

        struct Live2JumpInfo
        {
            Jump jump;
            Video video;
            boost::uint16_t delay_play_time;

            template <
                typename Archive
            >
            void serialize(
            Archive & ar)
            {
                ar.version(live_2);
                ar & jump
                    & video
                    & SERIALIZATION_NVP(delay_play_time);
            }
        };

    } // namespace cdn
} // namespace ppbox

#endif//_PPBOX_CDN_LIVE2_INFO_H_
