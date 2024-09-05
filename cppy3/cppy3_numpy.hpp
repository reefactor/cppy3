/**
 * cppy3 -- embed python3 scripting layer into your c++ app in 10 minutes
 *
 * Adapters for numpy.ndarray
 *
 */

#pragma once

// undef will surpress python warnings
#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif
#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif

#include <Python.h>

// deal with crazy numpy 1.7.x api init procedure
#define PY_ARRAY_UNIQUE_SYMBOL PyArray_API__CPPY3_APP_TOKEN
#if !defined(INCLUDED_FROM_CPPY3_NUMPY_CPP)
#define NO_IMPORT_ARRAY
#endif
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

#include <numpy/arrayobject.h>

#include <cassert>

// fill with zeros by default new NDArray objects
#define SLOWER_AND_CLEARNER false

namespace cppy3
{

  void importNumpy();

  NPY_TYPES toNumpyDType(double);
  NPY_TYPES toNumpyDType(int);

  /**
   * Simple wrapper for numpy.ndarray
   */
  template <typename Type>
  class NDArray
  {
  public:
    NDArray() : _ndarray(NULL) {}

    NDArray(int n) : _ndarray(NULL)
    {
      create(n);
    }

    NDArray(int n1, int n2) : _ndarray(NULL)
    {
      create(n1, n2);
    }

    NDArray(const Type *data, int n) : _ndarray(NULL)
    {
      copy(data, n);
    }

    NDArray(const Type *data, int n1, int n2) : _ndarray(NULL)
    {
      copy(data, n1, n2);
    }

    ~NDArray()
    {
      decref();
    }

    /**
     * Create 1d array of given size
     * @param n - size of dimension
     * @param fillZeros - initialize allocated array with zeros
     */
    void create(int n, bool fillZeros = SLOWER_AND_CLEARNER)
    {

      decref();

      npy_intp dim1[1];
      dim1[0] = n;
      Type impltype = 0;
      if (fillZeros)
      {
        _ndarray = (PyArrayObject *)PyArray_ZEROS(1, dim1, toNumpyDType(impltype), 0);
      }
      else
      {
        _ndarray = (PyArrayObject *)PyArray_SimpleNew(1, dim1, toNumpyDType(impltype));
      }
      assert(_ndarray);
    }

    /**
     * Create 2d array of given size
     * @param n1 - rows size
     * @param n2 - cols size
     * @param fillZeros - initialize allocated array with zeros
     */
    void create(size_t n1, size_t n2, bool fillZeros = SLOWER_AND_CLEARNER)
    {

      decref();

      npy_intp dim2[2];
      dim2[0] = n1;
      dim2[1] = n2;
      Type impltype = 0;
      if (fillZeros)
      {
        _ndarray = (PyArrayObject *)PyArray_ZEROS(2, dim2, toNumpyDType(impltype), 0);
      }
      else
      {
        _ndarray = (PyArrayObject *)PyArray_SimpleNew(2, dim2, toNumpyDType(impltype));
      }
      assert(_ndarray);
    }

    bool isset()
    {
      return (_ndarray);
    }

    /**
     * Wrap an existing 1d array, pointed to by a single Type* pointer, and wraps it in a Numpy ndarray instance
     * @param data - points to allocated array
     * @param n - number of elements of type Type in array
     */
    void wrap(Type *data, int n)
    {
      npy_intp dim1[1];
      dim1[0] = n;
      _ndarray = (PyArrayObject *)PyArray_SimpleNewFromData(1, dim1, toNumpyDType(*data), (void *)&data);
    }

    /**
     * Wrap an existing 2d array, pointed to by a single Type* pointer, and wraps it in a Numpy ndarray instance
     * @param data - points to allocated array
     * @param[in] n1 - number of elements of type Type in array's row
     * @param[in] n2 - number of elements of type Type in array's column
     * @param[in] data - array data
     */
    void wrap(Type *data, int n1, int n2)
    {
      npy_intp dim2[2];
      dim2[0] = n1;
      dim2[1] = n2;
      _ndarray = (PyArrayObject *)PyArray_SimpleNewFromData(2, dim2, toNumpyDType(*data), (void *)data);
    }

    /**
     * Create a Numpy ndarray copy of data
     * @param data - 1d array
     * @param n - size
     */
    void copy(const Type *data, int n)
    {
      create(n, false);

      for (int i = 0; i < n; i++)
      {
        *(Type *)(PyArray_GETPTR1(_ndarray, i)) = data[i];
      }
    }

    /**
     * Create a Numpy ndarray copy of data
     * @param[in] data - 2d array data
     * @param[in] n1 - rows count
     * @param[in] n2 - columns count
     */
    void copy(const Type *data, size_t n1, size_t n2)
    {
      create(n1, n2, false);
      for (size_t r = 0; r < n1; ++r)
      {
        size_t rowOffset = r * n2;
        for (size_t c = 0; c < n2; ++c)
        {
          *(Type *)PyArray_GETPTR2(_ndarray, r, c) = data[rowOffset + c];
        }
      }
    }

    Type &operator()(int i)
    {
      assert(_ndarray);
      assert(PyArray_NDIM(_ndarray) == 1 && i >= 0 && i < PyArray_DIM(_ndarray, 0));
      return *((Type *)PyArray_GETPTR1(_ndarray, i));
    }

    Type &operator()(int i, int j)
    {
      assert(_ndarray);
      assert(PyArray_NDIM(_ndarray) == 2);
      assert(i >= 0 && i < PyArray_DIM(_ndarray, 0));
      assert(j >= 0 && j < PyArray_DIM(_ndarray, 1));
      return *((Type *)PyArray_GETPTR2(_ndarray, i, j));
    }

    operator PyObject *()
    {
      assert(_ndarray);
      return (PyObject *)_ndarray;
    }
    operator PyArrayObject *()
    {
      assert(_ndarray);
      return _ndarray;
    }

    /**
     * @return number of dimensions
     */
    int nd() const
    {
      assert(_ndarray);
      return PyArray_NDIM(_ndarray);
    }

    /**
     * @return size of dimension n
     */
    int dim(size_t n) const
    {
      assert(_ndarray);
      assert(PyArray_NDIM(_ndarray) > n);
      return PyArray_DIM(_ndarray, n);
    }

    int dim1() const
    {
      return dim(1);
    }

    int dim2() const
    {
      return dim(2);
    }

    /**
     * @return raw pointer to data array
     */
    Type *getData()
    {
      assert(_ndarray);
      return PyArray_DATA(_ndarray);
    }

  private:
    PyArrayObject *_ndarray;

    void decref()
    {
      Py_XDECREF(_ndarray);
    }
  };

}
