// TripMeidaTypes.h

#ifndef _JUST_CDN_TRIP_TRIP_MEDIA_TYPES_H_
#define _JUST_CDN_TRIP_TRIP_MEDIA_TYPES_H_

#include <util/tools/ClassRegister.h>

#include "just/cdn/trip/TripLive.h"
#include "just/cdn/trip/TripMedia.h"

#ifdef JUST_DISABLE_TRIP

namespace just
{
    namespace cdn
    {

        UTIL_REGISTER_URL_SOURCE("trip", P2pSource);

    } // namespace cdn
} // namespace just

#endif

#endif // _JUST_CDN_TRIP_TRIP_MEDIA_TYPES_H_
