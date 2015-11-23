// P2pMediaInfo.h

#ifndef _JUST_CDN_P2P_P2P_MEDIA_INFO_H_
#define _JUST_CDN_P2P_P2P_MEDIA_INFO_H_

#include "just/cdn/UtcTime.h"
#include "just/cdn/Serialize.h"

#include <just/data/base/MediaBase.h>
#include <just/data/segment/SegmentInfo.h>

#include <util/serialization/Optional.h>

#include <framework/network/NetName.h>

#ifndef JUST_CDN_PARAM_DELIM
#  define JUST_CDN_PARAM_DELIM ","
#endif

namespace just
{
    namespace cdn
    {

        using just::data::invalid_size;

        struct P2pVideo
            : just::data::MediaInfo
        {
            std::string rid;

            P2pVideo()
            {
                flags |= f_segment;
                flags |= just::data::SegmentMediaFlags::f_segment_seek;
            }

            friend bool operator==(
                P2pVideo const & l, 
                P2pVideo const & r)
            {
                return l.rid == r.rid
                    && l.bitrate == r.bitrate
                    && l.duration == r.duration
                    && l.file_size == r.file_size;
            }
        };

        struct P2pJump
        {
            framework::string::Url url;
            framework::string::Url url2;
            UtcTime server_time;
            size_t bw_type;

            P2pJump()
                : bw_type(0)
            {
            }

            friend bool operator==(
                P2pJump const & l, 
                P2pJump const & r)
            {
                return l.url == r.url
                    && l.url2 == r.url2
                    && l.bw_type == r.bw_type;
            }
        };

    } // namespace cdn
} // namespace just

#endif // _JUST_CDN_P2P_P2P_MEDIA_INFO_H_
