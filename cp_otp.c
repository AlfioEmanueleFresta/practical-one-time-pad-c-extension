/* **********************************************************

    @author: Alfio E. Fresta
    @version: 0.1.0

	Copyright (c) 2016, Alfio E. Fresta
	All rights reserved.

************************************************************ */

#include <Python.h>
#include <time.h>
#include "cp_otp.h"

#define VERSION "1.0"
#define MODULE_SUMMARY "cp_otp provides c exptensions for cyber-practicals one-time pad experiments"

//************************************************************
//	Public Methods in Module
//************************************************************

#define DEBUG 1

#define VERSION_SUMMARY "string version()   Provides version string for module."
static PyObject *cp_otp_version(PyObject *self, PyObject *args) {
    //provide module version string
    return Py_BuildValue("s", VERSION);
}


static void strxor(char **dest, char *message, char *key) {
    int len, i;
    char *output;
    len = strlen(message);
    output = malloc(sizeof(char) * (len + 1));
    for (i = 0; i < len; i ++) {
        output[i] = message[i] ^ key[i];
    }
    output[i] = 0x00;
    *dest = output;
}


#define CPOTP_STRXOR_SUMMARY "string strxor(string message, string key)   XOR string and key"
static PyObject *cp_otp_strxor(PyObject *self, PyObject *args) {

    char * message = NULL;
    char * key = NULL;
    char * output = NULL;

    if (!PyArg_ParseTuple(args, "ss", &message, &key)) {
        PyErr_SetString(PyExc_ValueError, "The strxor method was called with invalid arguments.");
        return NULL;
    }

    // TODO -- we're using strlen, does this work well with keys that contain the NULL character?
    if (strlen(message) != strlen(key)) {
        PyErr_SetString(PyExc_ValueError, "The message and the key need to be of the same length.");
        return NULL;
    }

    int len;
    len = strlen(message);

    strxor(&output, message, key);

    return PyBytes_FromStringAndSize(output, len);
}


void rand_str(char *dest, size_t length) {
    char charset[] = "0123456789"
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}


#define MAX_KEY_LENGTH 1024
#define CPOTP_GET_RANDOM_KEY_SUMMARY "string get_random_key(int key_length)   Get a Random string"
static PyObject *cp_otp_get_random_key(PyObject *self, PyObject *args) {

    char * output = NULL;
    int len = 0;

    if (!PyArg_ParseTuple(args, "i", &len)) {
        PyErr_SetString(PyExc_ValueError, "You need to define a length.");
        return NULL;
    }

    if (len < 1 || len > MAX_KEY_LENGTH) {
        PyErr_SetString(PyExc_ValueError, "The size requested is invalid.");
        return NULL;
    }

    output = malloc(sizeof(char) * (len+1));
    rand_str(output, len);

    return PyBytes_FromStringAndSize(output, len);
}


char * secretKey = NULL;

#define MESSAGE_LENGTH 128
#define CPOTP_INTERCEPT_IN_SUMMARY "string intercept_in()   Get the encrypted secret message"
static PyObject *cp_otp_intercept_in(PyObject *self, PyObject *args) {

    srand(time(NULL));

    // First, generate a secret random key
    secretKey = malloc(sizeof(char) * (MESSAGE_LENGTH+1));
    rand_str(secretKey, MESSAGE_LENGTH);
    if (DEBUG) printf("> Secret key: %s\n", secretKey);

    char * message = NULL;
    message = malloc(sizeof(char) * (MESSAGE_LENGTH+1));
    rand_str(message, MESSAGE_LENGTH);
    if (DEBUG) printf("> Secret message: %s\n", message);

    char * encrypted = NULL;
    strxor(&encrypted, message, secretKey);
    // if (DEBUG) printf("> Encrypted message: %s\n", encrypted);

    return PyBytes_FromStringAndSize(encrypted, MESSAGE_LENGTH);
}


#define CPOTP_INTERCEPT_OUT_SUMMARY "bool intercept_out()   Check the encrypted secret message"
static PyObject *cp_otp_intercept_out(PyObject *self, PyObject *args) {

    if (secretKey == NULL) {
        PyErr_SetString(PyExc_ValueError, "You need to call intercept_in first.");
        return NULL;
    }

    char *encrypted = NULL;
    char *decrypted = NULL;

    if (!PyArg_ParseTuple(args, "s", &encrypted)) {
        PyErr_SetString(PyExc_ValueError, "You need to pass the encrypted message as an argument.");
        return NULL;
    }

    strxor(&decrypted, encrypted, secretKey);

    if (DEBUG) printf("< Received message: %s", decrypted);

    Py_RETURN_FALSE;
    Py_RETURN_TRUE;
}



//******************************************************************
// Formal class/module specs
//******************************************************************

static PyMethodDef cp_otp_module_methods[] = {
        { "version", cp_otp_version, METH_NOARGS, VERSION_SUMMARY },
        { "strxor", cp_otp_strxor, METH_VARARGS, CPOTP_STRXOR_SUMMARY },
        { "get_random_key", cp_otp_get_random_key, METH_VARARGS, CPOTP_GET_RANDOM_KEY_SUMMARY },
        { "intercept_in", cp_otp_intercept_in, METH_VARARGS, CPOTP_INTERCEPT_IN_SUMMARY },
        { "intercept_out", cp_otp_intercept_out, METH_VARARGS, CPOTP_INTERCEPT_OUT_SUMMARY },
        { NULL, NULL,}
};

static struct PyModuleDef cp_otpmodule = {
        PyModuleDef_HEAD_INIT,
        "cp_otp",
        MODULE_SUMMARY,
        -1,
        cp_otp_module_methods,
        NULL,
        NULL,
        NULL,
        NULL
};

PyMODINIT_FUNC
PyInit_cp_otp(void)
{
    return PyModule_Create(&cp_otpmodule);
}
