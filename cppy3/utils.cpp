#include "utils.hpp"
#if __cplusplus >= 201703L
#include <cstdlib>
#include <cstring>
#ifdef _WIN32
#include <wchar.h>
#endif
#endif

namespace cppy3
{
    std::wstring UTF8ToWide(const std::string &text)
    {
#ifdef CPPY3_USE_BOOST_CONVERT
        using boost::locale::conv::utf_to_utf;
        return utf_to_utf<wchar_t>(text.c_str(), text.c_str() + text.size())
#elif __cplusplus >= 201703L 
        std::mbstate_t state{};
        const char *narrow = text.c_str();
        std::size_t len = std::strlen(narrow) * 2;
        wchar_t *wide = (wchar_t *)malloc(sizeof(wchar_t) * (len + 1));
#ifdef _WIN32
        mbstowcs_s(&len, wide, len, narrow, len);
#else
        std::mbsrtowcs(wide, &narrow, len, &state);
#endif
        std::wstring result(wide);
        free(wide);
        return result;
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
#elif __cplusplus >= 201703L
        const wchar_t *wide = text.c_str();
        std::size_t len = text.size() * 2;
        char *narrow = (char *)malloc(sizeof(char) * (len + 1));
#ifdef _WIN32
        wcstombs_s(&len, narrow, len, wide, len);
#else
        std::wcstombs(narrow, wide, len);
#endif
        std::string result(narrow);
        free(narrow);
        return result;
#else
        std::wstring_convert<std::codecvt_utf8<wchar_t, 0x10ffff, std::little_endian>, wchar_t> converter;
        return converter.to_bytes(text);
#endif
    }
}
