#include "utility.h"

#include "hv//htime.h"

#include <thread>
#include <codecvt>
#include <iomanip>

namespace std
{
    ostream &operator<<(ostream &os, const u8string &str)
    {
        os << reinterpret_cast<const char *>(str.data());
        return os;
    }

    std::u8string to_u8str(const std::string &src)
    {
        return std::u8string{reinterpret_cast<const char8_t *>(src.data())};
    }

    std::string to_string(const std::u8string &src)
    {
        return std::string{reinterpret_cast<const char *>(src.data())};
    }
}

namespace ayan
{
    void clear_screen() noexcept
    {
#ifdef _WIN32
        std::system("chcp 65001 & cls");
#else
        std::system("clear");
#endif
    }

    void block_here() noexcept
    {
        using namespace std::literals::chrono_literals;
        while (true)
        {
            std::this_thread::sleep_for(16ms);
        }
    }

    std::wstring utf8_to_wstr(const std::string &src)
    {
        static std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.from_bytes(src);
    }

    std::string wstr_to_utf8(const std::wstring &src)
    {
        static std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.to_bytes(src);
    }

    std::string_view time_now()
    {
        static char buf[DATETIME_FMT_BUFLEN];
        datetime_t dt = datetime_now();
        datetime_fmt(&dt, buf);
        return buf;
    }

    std::string url_encode(const std::string &utf8)
    {
        static std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex;

        for (auto &c : utf8)
        {
            /// not encoded
            if (c >= 0 && std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            {
                escaped << c;
                continue;
            }

            /// other characters are encoded
            escaped << std::uppercase
                    << '%' << std::setw(2) << int((unsigned char)c)
                    << std::nouppercase;
        }

        std::string ret = escaped.str();
        escaped.str(""); /// clear stream
        return ret;
    }

    std::string srcinfo(std::source_location &&loc)
    {
        return fmt::format("file:{} line: {} fn:{}", loc.file_name(), loc.line(), loc.function_name());
    }
}
