// PptvMeidaTypes.h

#ifndef _PPBOX_CDN_TRIP_TRIP_MEDIA_H_
#define _PPBOX_CDN_TRIP_TRIP_MEDIA_H_

#include <ppbox/data/base/MediaBase.h>

namespace ppbox
{
    namespace cdn
    {

        ppbox::data::MediaBase * create_trip(
            boost::asio::io_service & io_svc,
            framework::string::Url const & url);

        UTIL_REGISTER_CLASS_FUNC(ppbox::data::MediaProtocolFactory, "trip", create_trip);

    } // cdn
} // ppbox

#endif // _PPBOX_CDN_TRIP_TRIP_MEDIA_H_
