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

        void meta_to_video(
            ::trip::client::ResourceMeta const & meta, 
            P2pVideo & video);

        void index_to_video(
            ::trip::client::ResourceInfo const & index, 
            P2pVideo & video);

        void index_to_jump(
            ::trip::client::ResourceInfo const & index, 
            P2pJump & video);

    } // namespace cdn
} // namespace just

#endif // _JUST_CDN_TRIP_TRIP_MEDIA_INFO_H_
