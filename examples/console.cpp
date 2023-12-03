#include <iostream>
#include "cppy3/cppy3.hpp"

static PyObject* hello(PyObject *self, PyObject *args, PyObject *keywds)
{
    /*
    printf("%s", "hello world\n");
    return Py_BuildValue("s", "Pls Work");
    */
    
    const char *name = "";    
    static char *kwlist[] = {"name", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, keywds, "s", kwlist, &name))
        return NULL;

    printf("Hello! I am %s.\n", name);

    Py_RETURN_NONE;
}

static PyMethodDef EmbMethods[] = {
    {"hello", (PyCFunction)(void(*)(void))hello, METH_VARARGS | METH_KEYWORDS, "Say hello."},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyModuleDef EmbModule = {
    PyModuleDef_HEAD_INIT, "emb", NULL, -1, EmbMethods
};

static PyObject*
PyInit_emb(void)
{
    return PyModule_Create(&EmbModule);
}

int main(int argc, char *argv[])
{
    cppy3::PythonVM instance("emb", PyInit_emb);

    std::cout << "Hey, type in command line, e.g. print(2+2*2)" << std::endl
              << std::endl;
 
    size_t i = 0;
    for (std::string line; std::getline(std::cin, line); i++)
    {
        try
        {

            const cppy3::Var result = cppy3::eval(line.c_str());
            std::cout << std::endl
                      << "Out[#" << i << " " << result.typeName() << "] " << result.toUTF8String() << std::endl;
        }
        catch (const cppy3::PythonException &e)
        {

            std::cerr << e.what() << std::endl;
        }
    }

    return 0;
}
