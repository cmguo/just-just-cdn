// PptvLiveInfo1.h

#ifndef _JUST_CDN_PPTV_PPTV_LIVE_INFO1_H_
#define _JUST_CDN_PPTV_PPTV_LIVE_INFO1_H_

#include "just/cdn/pptv/PptvLiveInfo.h"

#include <util/serialization/stl/vector.h>

namespace just
{
    namespace cdn
    {

        struct Live1JumpInfo
        {
            Video video;
            Jump jump;
            LiveSegment seg;

            std::vector<std::string> server_hosts;
            std::string proto_type;
            size_t buffer_size;

            Live1JumpInfo()
                : buffer_size(0)
            {
            }

            template <
                typename Archive
            >
            void serialize(
                Archive & ar)
            {
                ar.version(live_1);
                ar & SERIALIZATION_NVP(server_hosts)
                    & SERIALIZATION_NVP(proto_type)
                    & SERIALIZATION_NVP(buffer_size);

                if (server_hosts.size()) {
                    jump.server_host.from_string(server_hosts.front());
                }
                if (server_hosts.size() > 1) {
                    jump.back_host.from_string(server_hosts[1]);
                }
            }
        };

    } // namespace cdn
} // namespace just

#endif//_JUST_CDN_PPTV_PPTV_LIVE_INFO1_H_
