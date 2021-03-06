// TripLive.h

#ifndef _JUST_CDN_TRIP_TRIP_LIVE_H_
#define _JUST_CDN_TRIP_TRIP_LIVE_H_

#include <just/data/base/MediaBase.h>

namespace just
{
    namespace cdn
    {

        just::data::MediaBase * create_trip(
            boost::asio::io_service & io_svc,
            framework::string::Url const & url);

        UTIL_REGISTER_CLASS_FUNC(just::data::MediaProtocolFactory, "tripl", create_trip);

    } // cdn
} // just

#endif // _JUST_CDN_TRIP_TRIP_LIVE_H_
