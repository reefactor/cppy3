#pragma once

#include <string>
#include <locale>

#ifdef USE_BOOST_CONVERT
    #include <boost/locale/encoding_utf.hpp>
#else
    #include <codecvt>
#endif

std::wstring UTF8ToWide(const std::string& text);
std::string WideToUTF8(const std::wstring& text);
