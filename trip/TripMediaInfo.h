// PptvMediaInfo.h

#ifndef _JUST_CDN_TRIP_TRIP_MEDIA_INFO_H_
#define _JUST_CDN_TRIP_TRIP_MEDIA_INFO_H_

#include "just/cdn/p2p/P2pMediaInfo.h"

#include <trip/client/Common.h>
#include <trip/client/proto/MessageResource.h>

namespace just
{
    namespace cdn
    {

        void info_to_video(
            ::trip::client::ResourceInfo const & info, 
            P2pVideo & video);

        void info_to_jump(
            ::trip::client::ResourceInfo const & info, 
            P2pJump & video);

    } // namespace cdn
} // namespace just

#endif // _JUST_CDN_TRIP_TRIP_MEDIA_INFO_H_
