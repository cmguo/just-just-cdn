//TripMediaInfo.cpp

#include "just/cdn/Common.h"
#include "just/cdn/trip/TripMediaInfo.h"

namespace just
{
    namespace cdn
    {

        void meta_to_video(
            trip::client::ResourceMeta const & meta, 
            P2pVideo & video)
        {
            video.duration = meta.duration;
            video.file_size = meta.bytesize;
        }

        void index_to_video(
            trip::client::ResourceInfo const & index, 
            P2pVideo & video)
        {
            meta_to_video(index.meta, video);
        }

        void index_to_jump(
            trip::client::ResourceInfo const & index, 
            P2pJump & jump)
        {
            if (index.urls.is_initialized()) {
                std::vector<framework::string::Url> const & urls(index.urls.get());
                if (urls.size() > 0) {
                    jump.url = urls[0];
                }
                if (urls.size() > 1) {
                    jump.url2 = urls[1];
                }
            }
        }

    } // cdn
} // just
