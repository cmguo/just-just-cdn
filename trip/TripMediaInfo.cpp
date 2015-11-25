//TripMediaInfo.cpp

#include "just/cdn/Common.h"
#include "just/cdn/trip/TripMediaInfo.h"

namespace just
{
    namespace cdn
    {

        void info_to_video(
            trip::client::ResourceInfo const & info, 
            P2pVideo & video)
        {
            video.duration = info.meta.duration;
            video.file_size = info.meta.bytesize;
            video.format_type = info.meta.file_extension.substr(1);
            if (video.format_type == "ts") {
                video.flags |= just::data::SegmentMediaFlags::f_smoth;
                video.flags |= just::data::SegmentMediaFlags::f_time_smoth;
            }
        }

        void info_to_jump(
            trip::client::ResourceInfo const & info, 
            P2pJump & jump)
        {
            if (info.urls.is_initialized()) {
                std::vector<framework::string::Url> const & urls(info.urls.get());
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
