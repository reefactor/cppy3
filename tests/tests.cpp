#include "cppy3/cppy3.hpp"
// #include <cppy3/cppy3numpy.hpp>

#include <iostream>

void testInjectSimpleTypes() {
  cppy3::Main().injectVar<int>("a", 2);
  cppy3::Main().injectVar<int>("b", 2);
  cppy3::exec("assert a + b == 4");
  cppy3::exec("print(a + b)");
}

#if 0 // TODO
void TestInjectNumpyArray() {

  cppy3::importNumpy();
  cppy3::exec("import numpy");
  cppy3::exec("print 'hello numpy', numpy.version.full_version");

  // create numpy ndarray in C
  double data[2] = {3.14, 42};
  // create copy
  cppy3::NDArray<double> a(data, 2, 1);
  // wrap data without copying
  cppy3::NDArray<double> b;
  b.wrap(data, 2, 1);
  assert(a(1, 0) == data[1] && "expect data in ndarray");
  assert(b(1, 0) == data[1] && "expect data in ndarray");

  // inject into python __main__ namespace
  cppy3::Main().inject("a", a);
  cppy3::Main().inject("b", b);
  cppy3::exec("print a");
  cppy3::exec("assert type(a) == numpy.ndarray, 'expect injected instance'");
  cppy3::exec("assert numpy.all(a == b), 'expect data'");

  // modify b from python
  cppy3::exec("b[0] = 100500");
  assert(b(0, 0) == 100500 && "expect data is modified");
}
#endif

int main(int argc, char *argv[]) {

  try {
    // create interpreter
    cppy3::PythonVM instance;

    // run bunch of tests
    testInjectSimpleTypes();
    //TestInjectNumpyArray();

  } catch (const cppy3::PythonException& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
