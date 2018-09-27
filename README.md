# cppy3

## Embed Python 3 into your C++ app in 10 minutes

### Minimalistic library for embedding CPython 3.x scripting language into C++ application

No additional dependencies. Linux, Windows platforms supported.

### Features

* Inject variables from C++ code into Python namespace
* Use reference-counted smart pointer for PyObject*
* Manage init/shutdown of interpreter in 1 line of code
* Manage GIL with scoped lock/unlock guards
* Translate/forward exceptions from Python to to C++ layer
* Nice C++ abstractions for Python native types list, dict and numpy.ndarray
* Support Numpy ndarray via tiny C++ wrappers

### Dependencies

* C++14 compatible compiler (Clang 3.4+, GCC 4.9+, MSVC 19.0+ (Visual Studio 2015+), Intel C++ Compiler 17+)
* CMake (3+, build only)
* python3-dev package (build only)
* python3-numpy package (optional, build only)

#### Installing dependencies

##### Debian / Ubuntu

```bash
sudo apt-get install g++ python3-dev
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

### Build & run example interactive python console

```bash
cd build && cmake .. && make && ./console
```

### License

[MIT License](LICENSE). Feel free to use
