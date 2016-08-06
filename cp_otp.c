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


static void strxor(char **dest, char *message, char *key, int len, int change) {

    int i;
    char *output;
    output = malloc(sizeof(char) * (len + 1));

    for (i = 0; i < len; i ++) {

        output[i] = message[i] ^ key[i];

        // Some black magic to avoid NULL-bytes in our output.
        if (change && output[i] == 0x00) {
            key[i] += 1;
            i--;
        }

    }

    output[i] = 0x00;
    *dest = output;
}


#define CPOTP_STRXOR_SUMMARY "string strxor(string message, string key)   XOR string and key"
static PyObject *cp_otp_strxor(PyObject *self, PyObject *args) {

    char * message = NULL;
    int message_length;
    char * key = NULL;
    int key_length;
    char * output = NULL;

    if (!PyArg_ParseTuple(args, "s#s#", &message, &message_length, &key, &key_length)) {
        PyErr_SetString(PyExc_ValueError, "The strxor method was called with invalid arguments.");
        return NULL;
    }

    // TODO -- we're using strlen, does this work well with keys that contain the NULL character?
    if (message_length != key_length) {
        PyErr_SetString(PyExc_ValueError, "The message and the key need to be of the same length.");
        return NULL;
    }

    strxor(&output, message, key, message_length, 0);
    return PyBytes_FromStringAndSize(output, message_length);
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

#define ERR_MSG_MAX_LEN     512

#define SECRET_MESSAGE      "Online=1; UserIsPresident=0; ActivateSuperMassiveBlackHole=0;"
#define SANITY_CHECK        "Online=1;"
#define SANITY_CHECK_MSG    "The message seems to be corrupted, or the party seems to be offline."
#define MESSAGE_OK_A        "ActivateSuperMassiveBlackHole=1;"
#define MESSAGE_OK_A_MSG    "Message received, nothing to do."
#define MESSAGE_OK_B        "UserIsPresident=1;"
#define MESSAGE_OK_B_MSG    "Sorry, only the President is authorised to do this."

#define TAG_SENDER          ">   SENDER >>"
#define TAG_RECEIVER        "< RECEIVER <<"

char * secretKey = NULL;

#define CPOTP_INTERCEPT_IN_SUMMARY "string intercept_in()   Get the encrypted secret message"
static PyObject *cp_otp_intercept_in(PyObject *self, PyObject *args, PyObject *kw) {

    int silent = 0;
    static char *kwlist[] = {"silent", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kw, "|p", kwlist, &silent)) {
        return NULL;
    }

    char * message = SECRET_MESSAGE;
    if (!silent) printf("%s '%s'\n", TAG_SENDER, message);

    int message_length;
    message_length = strlen(message);

    secretKey = malloc(sizeof(char) * (message_length+1));
    rand_str(secretKey, message_length);

    if (DEBUG && !silent) printf("%s Secret key: %s\n", TAG_SENDER, secretKey);

    char * encrypted = NULL;
    strxor(&encrypted, message, secretKey, message_length, 1);

    return PyBytes_FromStringAndSize(encrypted, message_length);
}



#define CPOTP_INTERCEPT_OUT_SUMMARY "bool intercept_out()   Check the encrypted secret message"
static PyObject *cp_otp_intercept_out(PyObject *self, PyObject *args, PyObject *kw) {

    if (secretKey == NULL) {
        PyErr_SetString(PyExc_ValueError, "You need to call intercept_in first.");
        return NULL;
    }

    char errMessage[ERR_MSG_MAX_LEN];

    char *encrypted = NULL;
    char *decrypted;
    int size;
    int silent = 0;

    static char *kwlist[] = {"encrypted", "silent", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kw, "s#|p", kwlist, &encrypted, &size, &silent)) {
        PyErr_SetString(PyExc_ValueError, "You need to pass the encrypted message as an argument.");
        return NULL;
    }

    strxor(&decrypted, encrypted, secretKey, size, 0);

    if (strlen(decrypted) != strlen(secretKey)) {
        sprintf(errMessage,
                "The size of the received message is unexpected. Expected %d, received %d.",
                (int) strlen(secretKey), (int) strlen(decrypted));
        PyErr_SetString(PyExc_ValueError, errMessage);
        return NULL;
    }

    if (!silent) printf("%s '%s'\n", TAG_RECEIVER, decrypted);

    if (strstr(decrypted, SANITY_CHECK) == NULL) {
        if (!silent) printf("%s (!) %s", TAG_SENDER, SANITY_CHECK_MSG);
        Py_RETURN_FALSE;
    }

    if (strstr(decrypted, MESSAGE_OK_A) == NULL) {
        if (!silent) printf("%s (!) %s", TAG_SENDER, MESSAGE_OK_A_MSG);
        Py_RETURN_FALSE;
    }

    if (strstr(decrypted, MESSAGE_OK_B) == NULL) {
        if (!silent) printf("%s (!) %s", TAG_SENDER, MESSAGE_OK_B_MSG);
        Py_RETURN_FALSE;
    }

    Py_RETURN_TRUE;
}



//******************************************************************
// Formal class/module specs
//******************************************************************

static PyMethodDef cp_otp_module_methods[] = {
        { "version", cp_otp_version, METH_NOARGS, VERSION_SUMMARY },
        { "strxor", cp_otp_strxor, METH_VARARGS, CPOTP_STRXOR_SUMMARY },
        { "get_random_key", cp_otp_get_random_key, METH_VARARGS, CPOTP_GET_RANDOM_KEY_SUMMARY },
        { "intercept_in", (PyCFunction) cp_otp_intercept_in, METH_VARARGS|METH_KEYWORDS, CPOTP_INTERCEPT_IN_SUMMARY },
        { "intercept_out", (PyCFunction) cp_otp_intercept_out, METH_VARARGS|METH_KEYWORDS, CPOTP_INTERCEPT_OUT_SUMMARY },
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
    srand(time(NULL));
    return PyModule_Create(&cp_otpmodule);
}
