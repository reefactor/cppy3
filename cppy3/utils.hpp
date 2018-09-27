#pragma once

#include <string>
#include <locale>

#ifdef CPPY3_USE_BOOST_CONVERT
    #include <boost/locale/encoding_utf.hpp>
#else
    #include <codecvt>
#endif

#ifndef _NDEBUG
 #define DLOG(MSG) {std::cerr<<MSG<<std::endl;}
 #define DWLOG(MSG) {std::wcerr<<MSG<<std::endl;}
#else
 #define DLOG(MSG) {}
 #define DWLOG(MSG) {}
#endif

namespace cppy3 {

std::wstring UTF8ToWide(const std::string& text);
std::string WideToUTF8(const std::wstring& text);

}
