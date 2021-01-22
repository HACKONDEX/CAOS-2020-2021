/* 
Very bad but working code.
*/

#include <stdio.h>
#include <Python.h>
#include <string.h>

int main() {
    Py_Initialize();

    char *new_line = NULL;
    size_t buffer_size = 32;
    PyObject *py_table = PyList_New(0);
    PyObject *py_separator = PyUnicode_FromString(";");
    for (int i = 0; i < 9; ++i) {
        if (getline(&new_line, &buffer_size, stdin) == -1) {
            break;
        }
        PyObject *line = PyUnicode_FromStringAndSize(new_line, strlen(new_line) - 1);
        PyList_Append(py_table, PyUnicode_Split(line, py_separator, -1));
        free(new_line);
        new_line = NULL;
        line = NULL;
    }

    int **processed = (int **) malloc(9 * sizeof(int *));
    for (int i = 0; i < 9; ++i) {
        processed[i] = (int *) malloc(26 * sizeof(int));
        for (int j = 0; j < 26; ++j) {
            processed[i][j] = 0;
        }
    }

    PyObject *py_dict_table = PyDict_New();
    PyObject *py_line = NULL;

    int table_size = PyList_Size(py_table);
    int line_size;

    PyObject *py_current_object;
    PyObject *py_number;
    PyObject *py_double;
    PyObject *py_run_result;

    for (int steps_count = 0; steps_count < 234 * 234; ++steps_count) {
        for (int i = 0; i < table_size; ++i) {
            py_line = PyList_GetItem(py_table, i);
            line_size = PyList_Size(py_line);
            for (int j = 0; j < line_size; ++j) {
                if (processed[i][j] == 1) {
                    continue;
                }
                py_current_object = PyList_GetItem(py_line, j);
                py_number = PyLong_FromUnicodeObject(py_current_object, 10);
                if (!PyErr_Occurred()) {
                    PyList_SetItem(py_line, j, py_number);
                    PyDict_SetItem(py_dict_table, PyUnicode_FromFormat("%c%d", j + 'A', i + 1), py_number);
                    processed[i][j] = 1;
                } else {
                    PyErr_Clear();
                    py_double = PyFloat_FromString(py_current_object);
                    if (!PyErr_Occurred()) {
                        PyList_SetItem(py_line, j, py_double);
                        PyDict_SetItem(py_dict_table, PyUnicode_FromFormat("%c%d", j + 'A', i + 1), py_double);
                        processed[i][j] = 1;
                    } else {
                        PyErr_Clear();
                        const char *str_from_table = PyUnicode_AsUTF8(py_current_object);
                        if (str_from_table[0] != '=') {
                            PyDict_SetItem(py_dict_table,
                                           PyUnicode_FromFormat("%c%d", j + 'A', i + 1),
                                           py_current_object);
                            processed[i][j] = 1;
                        } else {
                            py_run_result =
                                PyRun_String(str_from_table + 1, Py_eval_input, py_dict_table, py_dict_table);
                            if (!PyErr_Occurred()) {
                                processed[i][j] = 1;
                                PyList_SetItem(py_line, j, py_run_result);
                                PyDict_SetItem(py_dict_table,
                                               PyUnicode_FromFormat("%c%d", j + 'A', i + 1),
                                               py_run_result);
                            } else {
                                PyErr_Clear();
                            }
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < table_size; ++i) {
        py_line = PyList_GetItem(py_table, i);
        line_size = PyList_Size(py_line);
        for (int j = 0; j < line_size; ++j) {
            PyObject_Print(PyList_GetItem(py_line, j), stdout, Py_PRINT_RAW);
            if (j != line_size - 1) {
                printf(";");
            }
        }
        printf("\n");
    }

    for (int i = 0; i < 9; ++i) {
        free(processed[i]);
    }
    free(processed);
    Py_Finalize();
    return 0;
}
