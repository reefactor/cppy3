#pragma once

#if (defined(_WIN32) || defined(__CYGWIN__))
 #pragma warning(disable : 4996 4244 4180 4800 4661)
 // if we are building the dynamic library (instead of using it)
 #if defined(cppy2_EXPORTS)
   #define LIB_API __declspec(dllexport)
 #else
   #define LIB_API __declspec(dllimport)
 #endif
#else
 #if defined(cppy2_EXPORTS)
  #define LIB_API __attribute__((visibility("default")))
 #else
  #define LIB_API
 #endif
#endif

