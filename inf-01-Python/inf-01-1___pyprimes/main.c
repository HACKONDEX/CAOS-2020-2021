#include <stdio.h>
#include <math.h>
#include <Python.h>

typedef enum {
    Prime = 1,
    NotPrime = 0
} boolean;

unsigned char IsNumberPrime(long long number) {
    if(number == 1) {
        return 0;
    }
    if(number == 2 || number == 3) {
        return 1;
    }
    long long number_square_root = (long long)(sqrt(number));
    for(long long i = 2; i <= number_square_root; ++i) {
        if(number % i == 0) {
            return 0;
        }
    }
    return 1;
}

PyObject* GetFactorizationList(long long number) {
    if(number <= 0) {
        PyErr_SetString(PyExc_ArithmeticError, "Number should be positive");
        return NULL;
    }

    if(IsNumberPrime(number) == Prime) {
        return Py_BuildValue("s", "Prime!");
    }

    PyObject* result = PyList_New(0);
    PyList_Append(result, Py_BuildValue("l", 1));
    if(number != 1) {
        while(number > 1) {
            for(int divisor = 2; divisor <= number; ++divisor) {
                if(number % divisor == 0) {
                    PyList_Append(result, Py_BuildValue("l", divisor));
                    number /= divisor;
                    break;
                }
            }
        }
    }
    return result;
}

static PyObject* factor_out_c(PyObject* self, PyObject* args) {
    if(PyTuple_Size(args) != 1) {
        PyErr_SetString(PyExc_TypeError, "wrong args count");
        return NULL;
    }

    long long number;

    if(!PyArg_ParseTuple(args, "L", &number)) {
        return NULL;
    }

    return GetFactorizationList(number);
}

static PyMethodDef methods[] = {
        {"factor_out", factor_out_c, METH_VARARGS, "help decomposition into prime"},
        {NULL, NULL, 0, NULL}
};

static struct PyModuleDef mod = {
        PyModuleDef_HEAD_INIT, "primes", "primes", -1, methods
};

PyMODINIT_FUNC PyInit_primes(void) {
    return PyModule_Create(&mod);
}
