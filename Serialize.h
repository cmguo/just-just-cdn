// Serialize.h

#ifndef _PPBOX_CDN_SERIALIZE_H_
#define _PPBOX_CDN_SERIALIZE_H_

#include <util/serialization/Serialization.h>

namespace util
{
    namespace serialization
    {
        template <
            typename Archive
        >
        void serialize(
            Archive & ar, 
            framework::network::NetName & t)
        {
            std::string value;
            ar & value;
            t.from_string(value);
        }

    }
}

#endif // _PPBOX_CDN_SERIALIZE_H_
