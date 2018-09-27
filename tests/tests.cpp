#include <iostream>
#include <clocale>

#include <cppy3/cppy3.hpp>
#if CPPY3_BUILT_WITH_NUMPY
#include <cppy3/cppy3_numpy.hpp>
#endif

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>


TEST_CASE( "Utils", "" ) {
  SECTION( "unicode converters" ) {
    const std::string utf8Str("зачем вы посетили нас в глуши забытого селенья");
    const std::wstring unicodeStr(L"зачем вы посетили нас в глуши забытого селенья");

#if DEBUG_UNICODE_CONVERTERS
#if 1
    constexpr char locale_name[] = "en_US.UTF-8";
    setlocale( LC_ALL, locale_name );
    std::locale::global(std::locale(locale_name));
    std::wcin.imbue(std::locale());
    std::wcout.imbue(std::locale());
    std::ios_base::sync_with_stdio(false);
#else
#include <codecvt>
    std::ios_base::sync_with_stdio(false);
    std::locale utf8(std::locale(), new std::codecvt_utf8<wchar_t>);
    std::wcout.imbue(utf8);
#endif
    std::cout << utf8Str << std::endl;
    std::cout << "Unicode->UTF8:" << cppy3::WideToUTF8(unicodeStr) << std::endl;
    
    std::wcout << unicodeStr << std::endl;
    std::wcout << "UTF8->Unicode:" << cppy3::UTF8ToWide(utf8Str) << std::endl;
#endif

    REQUIRE(cppy3::WideToUTF8(unicodeStr) == utf8Str);
    REQUIRE(cppy3::UTF8ToWide(utf8Str) == unicodeStr);
  }
}

TEST_CASE( "cppy3 public functionality", "main funcs" ) {
  // create interpreter
  cppy3::PythonVM instance;

  SECTION("c++ -> python -> c++ variables injection/extraction") {
    // inject
    cppy3::Main().injectVar<int>("a", 2);
    cppy3::Main().injectVar<int>("b", 2);
    cppy3::exec("assert a + b == 4");
    cppy3::exec("print('sum is', a + b)");
    // extract
    const cppy3::Var sum = cppy3::eval("a + b");
    REQUIRE(sum.type() == cppy3::Var::LONG);
    REQUIRE(sum.toLong() == 4);
    REQUIRE(sum.toString() == L"4");
    REQUIRE(!cppy3::error());
  }

  SECTION("python -> c++ exception forwarding") {
    try {
      // throw excepton in python
      cppy3::exec("raise Exception('test-exception')");
      REQUIRE( false );  // unreachable code

    } catch (const cppy3::PythonException& e) {
      // catch in c++
      REQUIRE(e.info.type == L"<class 'Exception'>");
      REQUIRE(e.info.reason == L"test-exception");
      REQUIRE(e.info.trace.size() > 0);
      REQUIRE(std::string(e.what()).size() > 0);
    }
    // exception has been poped from python layer
    REQUIRE(!cppy3::error());
  }

#if CPPY3_BUILT_WITH_NUMPY
  SECTION("numpy ndarray support") {

    cppy3::importNumpy();
    cppy3::exec("import numpy");
    cppy3::exec("print('numpy version {}'.format(numpy.version.full_version))");

    // create numpy ndarray in C
    double data[2] = {3.14, 42};
    // create copy
    cppy3::NDArray<double> a(data, 2, 1);
    // wrap data without copying
    cppy3::NDArray<double> b;
    b.wrap(data, 2, 1);
    REQUIRE(a(1, 0) == data[1]);
    REQUIRE(b(1, 0) == data[1]);

    // inject into python __main__ namespace
    cppy3::Main().inject("a", a);
    cppy3::Main().inject("b", b);
    cppy3::exec("print('a: {} {}'.format(type(a), a))");
    cppy3::exec("print('b: {} {}'.format(type(b), b))");
    cppy3::exec("assert type(a) == numpy.ndarray, 'expect injected instance'");
    cppy3::exec("assert numpy.all(a == b), 'expect data'");

    // modify b from python
    cppy3::exec("b[0] = 100500");
    REQUIRE(b(0, 0) == 100500);
  }
#endif

}
