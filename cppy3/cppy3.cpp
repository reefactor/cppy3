#include "cppy3.hpp"

#include <cassert>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <fstream>
#include <streambuf>

#include "utils.hpp"

namespace cppy3
{

  PythonVM::PythonVM()
  {

    setenv("PYTHONDONTWRITEBYTECODE", "1", 0);

#ifdef _WIN32
    // force utf-8 on windows
    setenv("PYTHONIOENCODING", "UTF-8");
#endif

    // create CPython instance without registering signal handlers
    Py_InitializeEx(0);

    // initialize GIL
    PyEval_InitThreads();
  }
  
  PythonVM::PythonVM(const std::string &name, ModuleInitializer module)
  {

    setenv("PYTHONDONTWRITEBYTECODE", "1", 0);

#ifdef _WIN32
    // force utf-8 on windows
    setenv("PYTHONIOENCODING", "UTF-8");
#endif

    // register the module
    PyImport_AppendInittab(name.c_str(), module);

    // create CPython instance without registering signal handlers
    Py_InitializeEx(0);

    // initialize GIL
    PyEval_InitThreads();
  }

  PythonVM::~PythonVM()
  {
    if (!PyImport_AddModule("dummy_threading"))
    {
      PyErr_Clear();
    }
    Py_Finalize();
  }

  void setArgv(const std::list<std::wstring> &argv)
  {
    GILLocker lock;

    std::vector<const wchar_t *> cargv;
    for (std::list<std::wstring>::const_iterator it = argv.begin(); it != argv.end(); ++it)
    {
      cargv.push_back(it->data());
    }
    PySys_SetArgvEx(argv.size(), (wchar_t **)(&cargv[0]), 0);
  }

  Var createClassInstance(const std::wstring &callable)
  {
    GILLocker lock;
    Var instance;

    instance.newRef(call(lookupCallable(getMainModule(), callable)));
    rethrowPythonException();
    if (instance.none())
    {
      std::wstringstream ss;
      ss << L"error instantiating '" << callable << "': " << getErrorObject().toString();
      throw PythonException(ss.str());
    }
    return instance;
  }

  void appendToSysPath(const std::vector<std::wstring> &paths)
  {
    GILLocker lock;

    Var sys = import("sys");
    List sysPath(lookupObject(sys, L"path"));
    for (auto path : paths)
    {
      Var pyPath(convert(path));
      if (!sysPath.contains(pyPath))
      {
        // append into the 'sys.path'
        sysPath.append(pyPath);
      }
    }
  }

  void interrupt()
  {
    PyErr_SetInterrupt();
  }

  Var exec(const char *pythonScript)
  {
    GILLocker lock;
    PyObject *mainDict = getMainDict();
    Var result;

    result.newRef(PyRun_String(pythonScript, Py_file_input, mainDict, mainDict));
    if (result.data() == NULL)
    {
      rethrowPythonException();
    }
    return result;
  }

  LIB_API Var eval(const char *pythonScript)
  {
    GILLocker lock;
    PyObject *mainDict = getMainDict();
    Var result;

    result.newRef(PyRun_String(pythonScript, Py_eval_input, mainDict, mainDict));
    if (result.data() == NULL)
    {
      const PyExceptionData excData = getErrorObject(false);
      if (excData.type == L"<class 'SyntaxError'>")
      {
        // eval() throws SyntaxError when called with expressions
        // use exec() for expressions
        getErrorObject(true);
        return exec(pythonScript);
      }
      else
      {
        rethrowPythonException();
      }
    }
    return result;
  }

  Var exec(const std::string &pythonScript)
  {
    return exec(pythonScript.data());
  }

  Var exec(const std::wstring &pythonScript)
  {
    ScopedGILLock lock;

    // encode unicode std::wstring to utf8
    std::wstring script = L"# -*- coding: utf-8 -*-\n";
    script += pythonScript;
    return exec(WideToUTF8(script).c_str());
  }

  Var execScriptFile(const std::wstring &path)
  {
    std::ifstream t("file.txt");

    if (!t.is_open())
    {
      throw PythonException(L"canot open file " + path);
    }

    std::string script((std::istreambuf_iterator<char>(t)),
                       std::istreambuf_iterator<char>());
    return exec(script.c_str());
  }

  bool error()
  {
    GILLocker lock;
    return Py_IsInitialized() && PyErr_Occurred();
  }

  void rethrowPythonException()
  {
    if (error())
    {
      const PyExceptionData excData = getErrorObject(true);
      throw PythonException(excData);
    }
  }

  std::wstring pyUnicodeToWstring(PyObject *object)
  {
    std::wstring result;
    if (PyUnicode_Check(object))
    {
      PyObject *bytes = PyUnicode_AsEncodedString(object, "UTF-8", "strict"); // Owned reference
      if (bytes != NULL)
      {
        char *utf8String = PyBytes_AS_STRING(bytes); // Borrowed pointer
        result = UTF8ToWide(std::string(utf8String));
        Py_DECREF(bytes);
      }
    }
    return result;
  }

  PyExceptionData getErrorObject(const bool clearError)
  {
    GILLocker lock;
    std::wstring exceptionType;
    std::wstring exceptionMessage;
    std::vector<std::wstring> exceptionTrace;
    if (PyErr_Occurred())
    {
      // get error context
      PyObject *excType = NULL;
      PyObject *excValue = NULL;
      PyObject *excTraceback = NULL;
      PyErr_Fetch(&excType, &excValue, &excTraceback);
      PyErr_NormalizeException(&excType, &excValue, &excTraceback);

      // get traceback module
      PyObject *name = PyUnicode_FromString("traceback");
      PyObject *tracebackModule = PyImport_Import(name);
      Py_DECREF(name);

      // write text type of exception
      exceptionType = pyUnicodeToWstring(PyObject_Str(excType));

      // write text message of exception
      exceptionMessage = pyUnicodeToWstring(PyObject_Str(excValue));

      if (excTraceback != NULL && tracebackModule != NULL)
      {
        // get traceback.format_tb() function ptr
        PyObject *tbDict = PyModule_GetDict(tracebackModule);
        PyObject *format_tbFunc = PyDict_GetItemString(tbDict, "format_tb");
        if (format_tbFunc && PyCallable_Check(format_tbFunc))
        {
          // build argument
          PyObject *excTbTupleArg = PyTuple_New(1);
          PyTuple_SetItem(excTbTupleArg, 0, excTraceback);
          Py_INCREF(excTraceback); // because PyTuple_SetItem() steals reference
          // call traceback.format_tb(excTraceback)
          PyObject *list = PyObject_CallObject(format_tbFunc, excTbTupleArg);
          if (list != NULL)
          {
            // parse list and extract traceback text lines
            const int len = PyList_Size(list);
            for (int i = 0; i < len; i++)
            {
              PyObject *tt = PyList_GetItem(list, i);
              PyObject *t = Py_BuildValue("(O)", tt);
              char *buffer = NULL;
              if (PyArg_ParseTuple(t, "s", &buffer))
              {
                exceptionTrace.push_back(UTF8ToWide(buffer));
              }
              Py_XDECREF(t);
            }
            Py_DECREF(list);
          }
          Py_XDECREF(excTbTupleArg);
        }
      }
      Py_XDECREF(tracebackModule);

      if (clearError)
      {
        Py_XDECREF(excType);
        Py_XDECREF(excValue);
        Py_XDECREF(excTraceback);
      }
      else
      {
        PyErr_Restore(excType, excValue, excTraceback);
      }
    }
    return PyExceptionData(exceptionType, exceptionMessage, exceptionTrace);
  }

  PyObject *convert(const int &value)
  {
    PyObject *o = PyLong_FromLong(value);
    assert(o);
    return o;
  }

  PyObject *convert(const double &value)
  {
    PyObject *o = PyFloat_FromDouble(value);
    assert(o);
    return o;
  }

  PyObject *convert(const char *value)
  {
    PyObject *o = PyUnicode_FromString(value);
    return o;
  }

  PyObject *convert(const std::wstring &value)
  {
    PyObject *o = PyUnicode_FromWideChar(value.data(), value.size());
    return o;
  }

  void extract(PyObject *o, std::wstring &value)
  {
    Var str(o);
    if (!PyUnicode_Check(o))
    {
      // try cast to string
      str.newRef(PyObject_Str(o));
      if (!str.data())
      {
        throw PythonException(L"variable has no string representation");
      }
    }
    value = PyUnicode_AsUnicode(str);
  }

  void extract(PyObject *o, double &value)
  {
    if (PyFloat_Check(o))
    {
      value = PyFloat_AsDouble(o);
    }
    else
    {
      throw PythonException(L"variable is not a real type");
    }
  }

  void extract(PyObject *o, long &value)
  {
    if (PyLong_Check(o))
    {
      value = PyLong_AsLong(o);
    }
    else
    {
      throw PythonException(L"variable is not a long type");
    }
  }

  std::wstring Var::toString() const
  {
    return toString(_o);
  }

  std::wstring Var::toString(PyObject *val)
  {
    assert(val);
    std::wstringstream result;
    // try str() operator
    PyObject *str = PyObject_Str(val);
    if (!str)
    {
      // try repr() operator
      str = PyObject_Repr(val);
    }
    if (str)
    {
      result << pyUnicodeToWstring(str);
    }
    else
    {
      result << "< type='" << val->ob_type->tp_name << L"' has no string representation >";
    }
    return result.str();
  }

  long Var::toLong() const
  {
    long value = 0;
    extract(_o, value);
    return value;
  }

  double Var::toDouble() const
  {
    long value = 0;
    extract(_o, value);
    return value;
  }

  Var::Type Var::type() const
  {
    if (PyLong_Check(_o))
    {
      return LONG;
    }
    else if (PyFloat_Check(_o))
    {
      return FLOAT;
    }
    else if (PyUnicode_Check(_o))
    {
      return STRING;
    }
    else if (PyTuple_Check(_o))
    {
      return TUPLE;
    }
    else if (PyDict_Check(_o))
    {
      return DICT;
    }
    else if (PyList_Check(_o))
    {
      return LIST;
    }
    else if (PyBool_Check(_o))
    {
      return BOOL;
    }
#ifdef NPY_NDARRAYOBJECT_H
    else if (PyArray_Check(_o))
    {
      return TYPE_NUMPY_NDARRAY;
    }
#endif
    else if (PyModule_Check(_o))
    {
      return MODULE;
    }
    else
    {
      return UNKNOWN;
    }
  }

  Var import(const char *moduleName, PyObject *globals, PyObject *locals)
  {
    Var module;
    module.newRef(PyImport_ImportModuleEx(const_cast<char *>(moduleName), globals, locals, NULL));
    if (module.null())
    {
      const PyExceptionData excData = getErrorObject();
      throw PythonException(excData);
    }
    return module;
  }

  Var lookupObject(PyObject *module, const std::wstring &name)
  {
    std::wstring temp;
    std::vector<std::wstring> items;
    std::wstringstream wss(name);
    while (std::getline(wss, temp, L'.'))
      items.push_back(temp);

    Var p(module);
    // Var prev;  // (1) cause refcount bug
    std::string itemName;
    for (auto it = items.begin(); it != items.end() && !p.null(); ++it)
    {

      // prev = p; // (2) cause refcount bug
      itemName = WideToUTF8(*it);
      if (PyDict_Check(p))
      {
        p = Var(PyDict_GetItemString(p, itemName.data()));
      }
      else
      {
        // PyObject_GetAttrString returns new reference
        p.newRef(PyObject_GetAttrString(p, itemName.data()));
      }

      if (p.null())
      {
        std::wstringstream wss;
        wss << L"lookup " << name << L" failed: no item " << UTF8ToWide(itemName);
        throw PythonException(wss.str());
      }
    }
    return p;
  }

  Var lookupCallable(PyObject *module, const std::wstring &name)
  {
    Var p = lookupObject(module, name);

    if (!PyCallable_Check(p))
    {
      std::wstringstream wss;
      wss << L"PyObject " << name << L" is not callable";
      throw PythonException(wss.str());
    }

    return p;
  }

  PyObject *call(PyObject *callable, const arguments &args)
  {
    assert(callable);
    if (!PyCallable_Check(callable))
    {
      std::wstringstream wss;
      wss << L"PyObject " << callable << L" is not callable";
      throw PythonException(wss.str());
    }

    PyObject *result = NULL;
    Var argsTuple;
    const int argsCount = args.size();
    if (argsCount > 0)
    {
      argsTuple.newRef(PyTuple_New(argsCount));
      for (int i = 0; i < argsCount; i++)
      {
        // steals reference
        PyTuple_SetItem(argsTuple, i, args[i]);
      }
    }

    PyErr_Clear();
    result = PyObject_CallObject(callable, argsTuple);
    rethrowPythonException();

    return result;
  }

  PyObject *call(const char *callable, const arguments &args)
  {
    return call(lookupCallable(getMainModule(), UTF8ToWide(callable)), args);
  }

  GILLocker::GILLocker() : _locked(false)
  {
    // autolock GIL in scoped_lock style
    lock();
  }

  GILLocker::~GILLocker()
  {
    release();
  }

  void GILLocker::release()
  {
    if (_locked)
    {
      assert(Py_IsInitialized());
      PyGILState_Release(_pyGILState);
      _locked = false;
    }
  }

  void GILLocker::lock()
  {
    if (!_locked)
    {
      assert(Py_IsInitialized());
      _pyGILState = PyGILState_Ensure();
      _locked = true;
    }
  }

  PyObject *getMainModule()
  {
    PyObject *mainModule = PyImport_AddModule("__main__");
    assert(mainModule);
    return mainModule;
  }

  PyObject *getMainDict()
  {
    PyObject *mainDict = PyModule_GetDict(getMainModule());
    assert(mainDict);
    return mainDict;
  }

}
