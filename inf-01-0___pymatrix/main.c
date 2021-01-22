#include <stdio.h>
#include <Python.h>

double MatrixElement(PyObject* matrix, int line_index, int column_index) {
    if(line_index < PyList_Size(matrix) &&
       column_index < PyList_Size(PyList_GetItem(matrix, line_index))) {
        return PyFloat_AsDouble(PyList_GetItem(PyList_GetItem(matrix, line_index), column_index));
    }
    return 0.0;
}

PyObject* MultiplicateMatrixes(int size, PyObject* matrix_a, PyObject* matrix_b) {
    double sum = 0;
    PyObject* result = PyList_New(0);
    for(int i = 0; i < size; ++i) {
        PyObject* matrix_line = PyList_New(0);
        for(int j = 0; j < size; ++j) {
            sum = 0;
            for(int k = 0; k < size; ++k) {
                sum += MatrixElement(matrix_a, i, k) * MatrixElement(matrix_b, k, j);
            }
            PyList_Append(matrix_line, Py_BuildValue("d", sum));
        }
        PyList_Append(result, matrix_line);
    }

    return result;
}

static PyObject* dot_on_c(PyObject* self, PyObject* args) {
    if(PyTuple_Size(args) != 3) {
        PyErr_SetString(PyExc_TypeError, "wrong args count");
        return NULL;
    }
    int size;
    PyObject* matrix_a;
    PyObject* matrix_b;

    if(!PyArg_ParseTuple(args, "iOO", &size, &matrix_a, &matrix_b)) {
        return NULL;
    }

    return MultiplicateMatrixes(size, matrix_a, matrix_b);
}

static PyMethodDef methods[] = {
        {"dot", dot_on_c, METH_VARARGS, "help matrix multiplication"},
        {NULL, NULL, 0, NULL}
};

static struct PyModuleDef mod = {
        PyModuleDef_HEAD_INIT, "matrix", "matrix", -1, methods
};

PyMODINIT_FUNC PyInit_matrix(void) {
    return PyModule_Create(&mod);
}
