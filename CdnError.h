// CdnError.h

#ifndef _JUST_CDN_CDN_ERROR_H_
#define _JUST_CDN_CDN_ERROR_H_

namespace just
{
    namespace cdn
    {

        namespace error {

            enum errors
            {
                bad_url_format = 1,
                bad_file_format,
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
                        if (value == error::bad_url_format)
                            return "cdn: bad url";
                        if (value == error::bad_file_format)
                            return "cdn: bad file format";
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
} // namespace just

namespace boost
{
    namespace system
    {

        template<>
        struct is_error_code_enum<just::cdn::error::errors>
        {
            BOOST_STATIC_CONSTANT(bool, value = true);
        };

#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
        using just::cdn::error::make_error_code;
#endif

    }
}

#endif // _JUST_CDN_CDN_ERROR_H_
