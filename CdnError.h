// CdnError.h

#ifndef _PPBOX_CDN_CDN_ERROR_H_
#define _PPBOX_CDN_CDN_ERROR_H_

namespace ppbox
{
    namespace cdn
    {

        namespace error {

            enum errors
            {
                jump_error = 1,
                drag_error,
                play_error,
                bad_file_format,
                item_not_exist,
                not_support,
                not_open,
                bad_url,
                start_failed, 
                bad_xml_format,
                write_mp4_head_error,
                bad_ft_param

            };

            namespace detail {

                class demux_category
                    : public boost::system::error_category
                {
                public:
                    demux_category()
                    {
                        register_category(*this);
                    }

                    const char* name() const
                    {
                        return "cdn";
                    }

                    std::string message(int value) const
                    {
                        if (value == error::jump_error)
                            return "cdn: jump failed";
                        if (value == error::drag_error)
                            return "cdn: drag failed";
                        if (value == error::play_error)
                            return "cdn: play failed";
                        if (value == error::bad_file_format)
                            return "cdn: bad file format";
                        if (value == error::item_not_exist)
                            return "cdn: segment not exist";
                        if (value == error::not_support)
                            return "cdn: not support";
                        if (value == error::not_open)
                            return "cdn: not open";
                        if (value == error::bad_url)
                            return "cdn: bad url";
                        if (value == start_failed)
                            return "Vod start faield";
                        if (value == bad_xml_format)
                            return "bad xml format";
                        if (value == write_mp4_head_error)
                            return "save mp4 head failed";
                        if (value == bad_ft_param)
                            return "ft param is worning";
                        return "cdn: unknown error";
                    }
                };

            } // namespace detail

            inline const boost::system::error_category & get_category()
            {
                static detail::demux_category instance;
                return instance;
            }

            inline boost::system::error_code make_error_code(
                errors e)
            {
                return boost::system::error_code(
                    static_cast<int>(e), get_category());
            }

        } // namespace demux_error

    } // namespace cdn
} // namespace ppbox

namespace boost
{
    namespace system
    {

        template<>
        struct is_error_code_enum<ppbox::cdn::error::errors>
        {
            BOOST_STATIC_CONSTANT(bool, value = true);
        };

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
        using ppbox::cdn::error::make_error_code;
#endif

    }
}

#endif // _PPBOX_CDN_CDN_ERROR_H_
