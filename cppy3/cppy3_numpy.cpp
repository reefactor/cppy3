
#include "cppy3.hpp"

#define INCLUDED_FROM_CPPY3_NUMPY_CPP
#include "cppy3_numpy.hpp"
#undef INCLUDED_FROM_CPPY3_NUMPY_CPP

namespace cppy3
{
  // workaround numpy & python3 https://github.com/boostorg/python/issues/214
  // return NULL to avoid UB https://wanzenbug.xyz/boost-numpy/
  static void *wrap_import_array() { import_array(); return NULL; }

  void importNumpy()
  {
    static bool imported = false;
    if (!imported)
    {
      // @todo double-lock-singleton pattern against multithreaded race condition
      imported = true;
      wrap_import_array();
      rethrowPythonException();
    }
  }

  NPY_TYPES toNumpyDType(double)
  {
    return NPY_DOUBLE;
  }

  NPY_TYPES toNumpyDType(int)
  {
    return NPY_INT;
  }

}
