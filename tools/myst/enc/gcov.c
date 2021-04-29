#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>

#include "myst_t.h"

long myst_gcov(const char* func, long params[6])
{
    printf("myst_gcov: enclave: func=%s\n", func);

    if (strcmp(func, "myst_gcov_abort") == 0)
    {
        assert("oops" == NULL);
    }
    else if (strcmp(func, "myst_gcov_fopen") == 0)
    {
        assert("oops" == NULL);
    }
    else if (strcmp(func, "myst_gcov_fdopen") == 0)
    {
        assert("oops" == NULL);
    }
    else if (strcmp(func, "myst_gcov_fread") == 0)
    {
        assert("oops" == NULL);
    }
    else if (strcmp(func, "myst_gcov_fwrite") == 0)
    {
        assert("oops" == NULL);
    }
    else if (strcmp(func, "myst_gcov_fseek") == 0)
    {
        assert("oops" == NULL);
    }
    else if (strcmp(func, "myst_gcov_ftell") == 0)
    {
        assert("oops" == NULL);
    }
    else if (strcmp(func, "myst_gcov_fclose") == 0)
    {
        assert("oops" == NULL);
    }
    else if (strcmp(func, "myst_gcov_setbuf") == 0)
    {
        assert("oops" == NULL);
    }
    else if (strcmp(func, "myst_gcov_open") == 0)
    {
        const char* pathname = (const char*)params[0];
        int flags = (int)params[1];
        mode_t mode = (mode_t)params[2];
        int retval;

        if (myst_gcov_open_ocall(&retval, pathname, flags, mode) != OE_OK)
            assert(0);

        return retval;
    }
    else if (strcmp(func, "myst_gcov_close") == 0)
    {
        assert("oops" == NULL);
    }
    else if (strcmp(func, "myst_gcov_fcntl") == 0)
    {
        assert("oops" == NULL);
    }
    else if (strcmp(func, "myst_gcov_getenv") == 0)
    {
        const char* name = (const char*)params[0];
        char* retval = NULL;

        if (myst_gcov_getenv_ocall(&retval, name) != OE_OK)
            assert(0);

        return (long)retval;
    }
    else if (strcmp(func, "myst_gcov___errno_location") == 0)
    {
        assert("oops" == NULL);
    }
    else if (strcmp(func, "myst_gcov_getpid") == 0)
    {
        int retval;

        if (myst_gcov_getpid_ocall(&retval) != OE_OK)
            assert(0);

        return retval;
    }
    else if (strcmp(func, "myst_gcov_strtol") == 0)
    {
        assert("oops" == NULL);
    }
    else if (strcmp(func, "myst_gcov_access") == 0)
    {
        assert("oops" == NULL);
    }
    else if (strcmp(func, "myst_gcov_mkdir") == 0)
    {
        assert("oops" == NULL);
    }
    else if (strcmp(func, "myst_gcov___popcountdi2") == 0)
    {
        assert("oops" == NULL);
    }
    else
    {
        fprintf(stderr, "unhandled gcov function: %s\n", func);
        fflush(stderr);
        abort();
    }

    return -1;
}
