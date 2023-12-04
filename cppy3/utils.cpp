#include "utils.hpp"

namespace cppy3
{
    std::wstring UTF8ToWide(const std::string &text)
    {
#ifdef CPPY3_USE_BOOST_CONVERT
        using boost::locale::conv::utf_to_utf;
        return utf_to_utf<wchar_t>(text.c_str(), text.c_str() + text.size());
#else
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t, 0x10ffff, std::little_endian>> converter;
        return converter.from_bytes(text);
#endif
    }

    std::string WideToUTF8(const std::wstring &text)
    {
#ifdef CPPY3_USE_BOOST_CONVERT
        using boost::locale::conv::utf_to_utf;
        return utf_to_utf<char>(text.c_str(), text.c_str() + text.size());
#else
        std::wstring_convert<std::codecvt_utf8<wchar_t, 0x10ffff, std::little_endian>, wchar_t> converter;
        return converter.to_bytes(text);
#endif
    }
}
