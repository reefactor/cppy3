# cppy3

## Embed Python 3 into your C++ app in 10 minutes

#### Minimalistic library for embedding [CPython](https://github.com/python/cpython) 3.x scripting language into C++ application

Lightweight simple and clean alternative to heavy [boost.python](https://github.com/boostorg/python).
No additional dependencies. Crossplatform -- Linux, Windows platforms supported.

cppy3 is sutable for [embedding Python in C++ application](https://docs.python.org/3/extending/index.html) while boost.python is evolved around [extending Python with C++ module](https://docs.python.org/3/extending/index.html) and [it's embedding capabilities are somehow limited for now](https://www.boost.org/doc/libs/1_63_0/libs/python/doc/html/tutorial/tutorial/embedding.html).


### Features

* Inject variables from C++ code into Python namespace
* Use reference-counted smart pointer for PyObject*
* Manage init/shutdown of interpreter in 1 line of code
* Manage GIL with scoped lock/unlock guards
* Translate/forward exceptions from Python to to C++ layer
* Nice C++ abstractions for Python native types list, dict and numpy.ndarray
* Support Numpy ndarray via tiny C++ wrappers

#### Inject/extract variables C++ -> Python -> C++

```c++
// create interpreter
cppy3::PythonVM instance;

// inject
cppy3::Main().injectVar<int>("a", 2);
cppy3::Main().injectVar<int>("b", 2);
cppy3::exec("assert a + b == 4");
cppy3::exec("print('sum is', a + b)");

// extract
const cppy3::Var sum = cppy3::eval("a + b");
assert(sum.type() == cppy3::Var::LONG);
assert(sum.toLong() == 4);
assert(sum.toString() == L"4)";
```

#### Forward exceptions Python -> C++


```c++
// create interpreter
cppy3::PythonVM instance;

try {

  // throw excepton in python
  cppy3::exec("raise Exception('test-exception')");
  assert(false && "not supposed to be here");

} catch (const cppy3::PythonException& e) {

  // catch in c++
  assert(e.info.type == L"<class 'Exception'>");
  assert(e.info.reason == L"test-exception");
  assert(e.info.trace.size() > 0);
  assert(std::string(e.what()).size() > 0);

}
```

#### Support numpy ndarray support


```c++
// create interpreter
cppy3::PythonVM instance;
cppy3::importNumpy();

// create numpy ndarray in C
double cData[2] = {3.14, 42};

// create copy
cppy3::NDArray<double> a(cData, 2, 1);

// wrap cData without copying
cppy3::NDArray<double> b;
b.wrap(data, 2, 1);

REQUIRE(a(1, 0) == cData[1]);
REQUIRE(b(1, 0) == cData[1]);

// inject into python __main__ namespace
cppy3::Main().inject("a", a);
cppy3::Main().inject("b", b);
cppy3::exec("import numpy");
cppy3::exec("assert numpy.all(a == b), 'expect cData'");

// modify b from python (b is a shared ndarray over cData)
cppy3::exec("b[0] = 100500");
assert(b(0, 0) == 100500);
assert(cData[0] == 100500);
```


### Dependencies

* C++11 compatible compiler (GCC 5+, MSVC, Intel C++, Clang)
* CMake (3+, build only)
* python3-dev package (build only)
* python3-numpy package (optional, build only)

#### Installing build dependencies

##### Debian / Ubuntu

```bash
sudo apt-get install cmake g++ python3-dev
```

Numpy is very much desired but optional
```bash
sudo apt-get install python3-numpy
```

##### Windows

TODO

### Build

#### Testdrive

```bash
cd cppy3 && mkdir build && cd build
cmake .. && make && ./tests
```

#### Release build

```bash
cd cppy3 && mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Example interactive python console

```bash
cd cppy3 && mkdir build && cd build
cmake .. && make && ./console
```

### License

[MIT License](LICENSE). Feel free to use
