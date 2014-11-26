// PptvLiveInfo.h

#ifndef _PPBOX_CDN_PPTV_PPTV_LIVE_INFO_H_
#define _PPBOX_CDN_PPTV_PPTV_LIVE_INFO_H_

#include "ppbox/cdn/pptv/PptvMediaInfo.h"

namespace ppbox
{
    namespace cdn
    {

        struct LiveVideo
            : Video
        {
            LiveVideo()
            {
                flags |= ppbox::data::SegmentMediaFlags::f_time_smoth;
                flags |= ppbox::data::SegmentMediaFlags::f_fix_duration;
            }
        };

        struct LiveSegment
        {
            boost::uint16_t interval;

            LiveSegment()
                : interval(0)
            {
            }

            template <
                typename Archive
            >
            void serialize(
                Archive & ar)
            {
                assert(ar.version() == live_2_play);
                ar  & SERIALIZATION_NVP(interval);
            }
        };

    } // namespace cdn
} // namespace ppbox

#endif//_PPBOX_CDN_PPTV_PPTV_LIVE_INFO_H_
