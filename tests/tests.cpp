#include <iostream>
#include <clocale>

#include <cppy3/cppy3.hpp>
#if CPPY3_BUILT_WITH_NUMPY
#include <cppy3/cppy3_numpy.hpp>
#endif

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#define DEBUG_UNICODE_CONVERTERS 1


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

TEST_CASE( "cppy3: Embedding Python into C++ code", "main funcs" ) {
  // create interpreter
  cppy3::PythonVM instance;

  SECTION("c++ -> python -> c++ variables injection/extraction") {

    // inject C++ -> python
    cppy3::Main().injectVar<int>("a", 2);
    cppy3::Main().injectVar<int>("b", 2);
    cppy3::exec("assert a + b == 4");
    cppy3::exec("print('sum is', a + b)");

    // extract python -> C++
    const cppy3::Var sum = cppy3::eval("a + b");
    REQUIRE(sum.type() == cppy3::Var::LONG);
    REQUIRE(sum.toLong() == 4);
    REQUIRE(sum.toString() == L"4");
    REQUIRE(!cppy3::error());

    try {
      // extract casting integer as double is forbidden
      sum.toDouble();
      REQUIRE(false);  // unreachable code, expect an exception
    } catch (const cppy3::PythonException& e) {
      REQUIRE(e.info.reason == L"variable is not a real type");
    }

    // python var assign name from c++ -> python
    cppy3::Main().inject("sum_var", sum);
    cppy3::exec("assert sum_var == 4");

    // cast to float in python
    cppy3::eval("sum_var = float(sum_var)");
    // can extract in c++ as double
    double sum_var = 0;
    cppy3::Main().getVar<double>("sum_var", sum_var);
    REQUIRE(abs(sum_var - 4.0) < 10e-10);

    // unicode strings inject / extract via exec / eval
    const std::wstring unicodeStr = L"юникод smile ☺";
    cppy3::exec(L"uu = '" + unicodeStr + L"'");
    const cppy3::Var uVar = cppy3::eval("uu");
    REQUIRE(uVar.toString() == unicodeStr);

    // unicode string inject / extract via converters
    cppy3::Main().injectVar<std::wstring>("uVar2", unicodeStr);
    cppy3::exec("print('uVar2:', uVar2)");
    std::wstring uVar2;
    cppy3::Main().getVar<std::wstring>("uVar2", uVar2);
    REQUIRE(uVar2 == unicodeStr);
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
    double cData[2] = {3.14, 42};
    // create copy
    cppy3::NDArray<double> a(cData, 2, 1);
    // wrap cData without copying
    cppy3::NDArray<double> b;
    b.wrap(cData, 2, 1);
    REQUIRE(a(1, 0) == cData[1]);
    REQUIRE(b(1, 0) == cData[1]);

    // inject into python __main__ namespace
    cppy3::Main().inject("a", a);
    cppy3::Main().inject("b", b);
    cppy3::exec("print('a: {} {}'.format(type(a), a))");
    cppy3::exec("print('b: {} {}'.format(type(b), b))");
    cppy3::exec("assert type(a) == numpy.ndarray, 'expect injected instance'");
    cppy3::exec("assert numpy.all(a == b), 'expect cData'");

    // modify b from python (b is a shared ndarray over cData)
    cppy3::exec("b[0] = 100500");
    REQUIRE(b(0, 0) == 100500);
    REQUIRE(cData[0] == 100500);
  }
#endif


  SECTION("test Scoped GIL Lock / Release") {

    // initially Python GIL is locked
    REQUIRE(cppy3::GILLocker::isLocked());

    // add variable
    cppy3::exec("a = []");
    cppy3::List a = cppy3::List(cppy3::lookupObject(cppy3::getMainModule(), L"a"));
    REQUIRE(a.size() == 0);

    // create thread that changes the variable a in a different thread
    const std::string threadScript = R"(
import threading
def thread_main():
  global a
  a.append(42)

t = threading.Thread(target=thread_main, daemon=True)
t.start()
)";
    std::cout << threadScript << std::endl;
    cppy3::exec(threadScript);

    {
      // release GIL on this thread
      cppy3::ScopedGILRelease gilRelease;
      REQUIRE(!cppy3::GILLocker::isLocked());
      // and wait thread changes the variable
      sleep(0.1F);
      {
        // lock GIL again before accessing python objects
        cppy3::GILLocker locker;
        REQUIRE(cppy3::GILLocker::isLocked());

        // ensure that variable has been changed
        cppy3::exec("assert a == [42], a");
        REQUIRE(a.size() == 1);
        REQUIRE((a[0]).toLong() == 42);
      }

      // GIL is released again
      REQUIRE(!cppy3::GILLocker::isLocked());
    }
  }

}
