#include "cp_otp.h"
#include <Python.h>

void fail(char * msg) {
	printf("Test Failed: %s\n", msg);
	getchar();
	exit(1);
}

void main (int argc, char * argv)
{
	char * msg = "makes";
	char * key = "sense";
    char * out = NULL;
    char * ok = {0x1e, 0x04, 0x05, 0x16, 0x16, 0x00};

    PyObject * pyMsg = PyBytes_FromStringAndSize(msg, strlen(msg));
    PyObject * pyKey = PyBytes_FromStringAndSize(key, strlen(key));
    PyObject * pyOut = NULL;

    pyOut = cp_otp_strxor(pyMsg, pyKey);
    out = PyString_AsString(pyOut);

	getchar();
}
