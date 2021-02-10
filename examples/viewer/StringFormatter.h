// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------
#pragma once
#include <algorithm>
#include <cmath>
#include <ios>
#include <iosfwd>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

namespace fx::common
{
    namespace detail
    {
        // clang-format off
        template <typename T>
        using FormatType =
            typename std::conditional_t<std::is_same_v<bool, T>, bool,
            typename std::conditional_t<std::is_same_v<short, T> || std::is_same_v<int, T> || std::is_same_v<long, T>, long,
            typename std::conditional_t<std::is_same_v<unsigned short, T> || std::is_same_v<unsigned int, T> || std::is_same_v<unsigned long, T>, unsigned long,
            typename std::conditional_t<std::is_same_v<long long, T>, long long,
            typename std::conditional_t<std::is_same_v<unsigned long long, T>, unsigned long long,
            typename std::conditional_t<std::is_same_v<float, T> || std::is_same_v<double, T>, double,
            typename std::conditional_t<std::is_same_v<long double, T>, long double, T>>>>>>>;
        // clang-format on
    } // namespace detail

    // Custom formatting for user-defined types
    template <typename Type, typename T>
    struct Formatter final : std::false_type
    {
    };

    // Formats all placeholders of the form: {index[,alignment][:formatString]}
    template <typename T>
    class BasicStringFormatter final
    {
    public:
        using StringType = std::basic_string<T>;
        using StringViewType = std::basic_string_view<T>;

        template <typename... Args>
        static StringType Format(StringViewType format, Args &&... args)
        {
            static thread_local StringType buffer;

            buffer.clear();
            BasicStringFormatter::FormatInto(buffer, format, std::forward<Args>(args)...);
            return buffer;
        }

        template <typename... Args>
        static void FormatInto(StringType & buffer, StringViewType format, Args &&... args)
        {
            // Provide space for the format string and the args...
            buffer.reserve(buffer.length() + format.length() + sizeof...(args) * 4);

            FormatState formatState(buffer);

            T ch = '\x0';
            std::size_t pos = 0;
            std::size_t lastPos = 0;
            const std::size_t len = format.length();
            while (pos < len)
            {
                while (pos < len)
                {
                    ch = format[pos];

                    pos++;
                    if (ch == '}')
                    {
                        if (pos < len && format[pos] == '}') // Treat as escape character for }}
                        {
                            buffer.append(&format[lastPos], pos - lastPos);

                            pos++;
                            lastPos = pos;
                        }
                        else
                        {
                            FormatErrorIf(true);
                        }
                    }

                    if (ch == '{')
                    {
                        if (pos < len && format[pos] == '{') // Treat as escape character for {{
                        {
                            buffer.append(&format[lastPos], pos - lastPos);

                            pos++;
                            lastPos = pos;
                        }
                        else
                        {
                            pos--;

                            buffer.append(&format[lastPos], pos - lastPos);
                            break;
                        }
                    }
                }

                if (pos == len)
                    break;

                pos++;

                int index = 0;
                FormatErrorIf(!TryExtractInteger(format, pos, index));

                FormatErrorIf(index >= sizeof...(args), "Format_IndexOutOfRange");

                while (pos < len && (ch = format[pos]) == ' ')
                    pos++;

                // Alignment specification...
                int width = 0;
                bool leftJustify = false;
                if (ch == ',')
                {
                    pos++;
                    while (pos < len && format[pos] == ' ')
                        pos++;

                    FormatErrorIf(pos == len);

                    ch = format[pos];
                    if (ch == '-')
                    {
                        leftJustify = true;
                        pos++;
                    }

                    FormatErrorIf(!TryExtractInteger(format, pos, width));
                }

                while (pos < len && (ch = format[pos]) == ' ')
                    pos++;

                // Format specification...
                StringViewType formatSpec;
                if (ch == ':')
                {
                    pos++;

                    const std::size_t startPos = pos;
                    while (true)
                    {
                        FormatErrorIf(pos == len);

                        ch = format[pos];
                        pos++;
                        if (ch == '}' || ch == '{')
                        {
                            if (ch == '{')
                            {
                                FormatErrorIf(true); // escaped braces not supported
                            }
                            else
                            {
                                FormatErrorIf(pos < len && format[pos] == '}'); // escaped braces not supported

                                formatSpec = format.substr(startPos, pos - startPos - 1);

                                pos--;
                                break;
                            }
                        }
                    }
                }

                FormatErrorIf(ch != '}');

                pos++;
                lastPos = pos;

                // Perform the actual substitution...
                formatState.Prepare(index, width, leftJustify, formatSpec);
                ArgSearch(formatState, 0, std::forward<Args>(args)...);
            }

            if (lastPos != len)
            {
                buffer.append(&format[lastPos], pos - lastPos);
            }
        }

    private:
        class FormatState final
        {
        public:
            explicit FormatState(StringType & buffer) noexcept
                : ios_(), buffer_(buffer), index_(0), width_(0), leftJustify_(false)
            {
            }

            int Index() const noexcept
            {
                return index_;
            }

            void Prepare(int index, std::size_t width, bool leftJustify, StringViewType formatSpec)
            {
                SetDefaults();

                index_ = index;
                width_ = width;
                leftJustify_ = leftJustify;
                formatSpec_ = formatSpec;
            }

            void Format(StringViewType arg)
            {
                Append(arg);
            }

            template <typename Arg, typename = std::enable_if_t<Formatter<std::remove_cv_t<Arg>, T>::value>>
            void Format(Arg & arg)
            {
                Formatter<std::remove_cv_t<Arg>, T>::Format(arg, formatSpec_, [this](StringViewType view) { Append(view); });
            }

            template <typename Arg, typename = std::enable_if_t<std::is_arithmetic_v<Arg>>>
            void Format(Arg arg)
            {
                static const auto & facet = std::use_facet<std::num_put<T, T *>>(std::locale());
                T buf[64];
                T * begin = std::begin(buf);
                T * end = begin;
                std::unique_ptr<T[]> scratch;

                BasicStringFormatter<T>::FormatErrorIf(!ApplySpecForNumbers());

#pragma warning(push)
#pragma warning(disable : 4127) // conditional expression is constant
                if constexpr (std::is_floating_point_v<Arg>)
                {
                    ios_.precision(std::min(static_cast<int>(ios_.precision()), std::numeric_limits<Arg>::max_digits10));
                    const auto argAbs = std::fabs(arg);
                    if (argAbs > 1e46)
                    {
                        // Large numbers may exceed the size of our stack buffer...
                        const std::size_t num = static_cast<std::size_t>(std::ceil(std::log10(argAbs))) + ios_.precision() + 2;
                        if (num > 64)
                        {
                            scratch = std::make_unique<T[]>(num);
                            begin = std::addressof(scratch[0]);
                            end = begin;
                        }
                    }
                }
#pragma warning(pop)

                end = facet.put(begin, ios_, ' ', static_cast<typename detail::FormatType<Arg>>(arg));

                Append(StringViewType{ begin, static_cast<std::size_t>(end - begin) });
            }

        private:
            class IosBase final : public std::basic_ios<T>
            {
            public:
                IosBase() noexcept
                    : std::basic_ios<T>()
                {
                    std::basic_ios<T>::init(nullptr);
                    std::basic_ios<T>::setf(std::ios_base::boolalpha);
                    std::basic_ios<T>::setf(std::ios_base::fixed, std::ios_base::floatfield);
                }
            };

            IosBase ios_;
            StringType & buffer_;

            int index_;
            std::size_t width_;
            bool leftJustify_;
            StringViewType formatSpec_;

            bool ApplySpecForNumbers()
            {
                bool requiresFill = false;
                bool isFloating = false;

                if (formatSpec_.empty())
                    return true;

                std::size_t index = 0;
                switch (formatSpec_[index])
                {
                case 'N':
                case 'n':
                    requiresFill = true;
                    break;

                case 'X':
                    ios_.setf(std::ios_base::uppercase);
                case 'x':
                    requiresFill = true;
                    ios_.setf(std::ios_base::hex, std::ios_base::basefield);
                    break;

                case 'F':
                case 'f':
                    isFloating = true;
                    break;
                }

                index++;

                int number = 0;
                if (!TryExtractInteger(formatSpec_, index, number))
                {
                    return false;
                }

                if (requiresFill)
                {
                    ios_.fill('0');
                    width_ = number;
                }
                else if (isFloating)
                {
                    ios_.precision(number);
                }

                return true;
            }

            void Append(StringViewType arg)
            {
                const bool shouldPad = width_ > arg.length();
                const std::size_t pad = width_ - arg.length();
                if (shouldPad && !leftJustify_)
                    buffer_.append(pad, ios_.fill());

                buffer_.append(arg);

                if (shouldPad && leftJustify_)
                    buffer_.append(pad, ios_.fill());
            }

            void SetDefaults()
            {
                ios_.unsetf(std::ios_base::uppercase);
                ios_.setf(std::ios_base::dec, std::ios_base::basefield);
                ios_.precision(2);
                ios_.fill(' ');
            }
        };

        static bool TryExtractInteger(StringViewType view, std::size_t & pos, int & out) noexcept
        {
            if (pos == view.length())
                return false;

            out = 0;
            const std::size_t start = pos;

            T ch = view[pos];
            while (ch >= '0' && ch <= '9' && out < 1000000)
            {
                out = out * 10 + ch - '0';
                pos++;

                if (pos == view.length())
                    break;

                ch = view[pos];
            }

            return pos != start;
        }

        static void FormatErrorIf(bool isError, char const * const message = "Format_InvalidString")
        {
            if (isError)
            {
                throw std::runtime_error(message);
            }
        }

        template <typename Arg>
        static void ArgSearch(FormatState & formatState, int currentIndex, Arg && arg)
        {
            if (currentIndex == formatState.Index())
            {
                formatState.Format(arg);
            }
        }

        template <typename Arg, typename... Args>
        static void ArgSearch(FormatState & formatState, int currentIndex, Arg && arg, Args &&... args)
        {
            if (currentIndex == formatState.Index())
            {
                formatState.Format(arg);
            }
            else
            {
                ArgSearch(formatState, currentIndex + 1, std::forward<Args>(args)...);
            }
        }
    };

    using StringFormatter = BasicStringFormatter<char>;
    using WStringFormatter = BasicStringFormatter<wchar_t>;
} // namespace fx::common
