# cppy3

## Embed Python 3 into your C++ app in 10 minutes

#### Minimalistic library for embedding [CPython](https://github.com/python/cpython) 3.x scripting language into C++ application

Lightweight simple and clean alternative to heavy [boost.python](https://github.com/boostorg/python).
No additional dependencies. Crossplatform -- Linux, Windows platforms are supported.

cppy3 is sutable for [embedding Python in C++ application](https://docs.python.org/3/extending/index.html) while boost.python is evolved around [extending Python with C++ module](https://docs.python.org/3/extending/index.html) and [it's embedding capabilities are somehow limited for now](https://www.boost.org/doc/libs/1_63_0/libs/python/doc/html/tutorial/tutorial/embedding.html).


### Features

* Inject variables from C++ code into Python
* Extract variables from Python to C++ layer
* Reference-counted smart pointer wrapper for PyObject*
* Manage Python init/shutdown with 1 line of code
* Manage GIL with scoped lock/unlock guards
* Forward exceptions (throw in Python, catch in C++ layer)
* Nice C++ abstractions for Python native types list, dict and numpy.ndarray
* Support Numpy ndarray via tiny C++ wrappers
* Example [interactive python console](examples/console.cpp) in 10 lines of code
* Tested on Debian Linux x64 G++ and Mac OSX M1 Clang


#### Features examples code snippets from [tests.cpp](tests/tests.cpp)

##### Inject/extract variables C++ -> Python -> C++

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
assert(sum.toString() == L"4");
```


##### Forward exceptions Python -> C++


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

#### Support numpy ndarray


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


### Requirements

* C++11 compatible compiler
* CMake 3.12+
* python3 dev package (with numpy recommended)


#### Build

##### Prerequisites
###### MacOSX

Brew python package has altogether dev headers and numpy included
```bash
sudo brew install cmake python3
```

###### Linux (Debian)

```bash
sudo apt-get install cmake g++ python3-dev
```

Numpy is very much desired but optional
```bash
sudo apt-get install python3-numpy
```

##### Windows

Cmake, Python with numpy is recommended.

### Build

#### Testdrive

```bash
mkdir build
cd build && cmake ..
make
./tests/tests
```

#### Example interactive python console

```bash
mkdir build
cd build && cmake ..
make
./console
```

#### Release build

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### License

[MIT License](LICENSE). Feel free to use
