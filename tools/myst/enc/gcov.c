#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myst_t.h"

long myst_gcov(const char* func, long params[6])
{
    printf("myst_gcov: enclave: func=%s\n", func);

    if (strcmp(func, "myst_gcov_abort") == 0)
    {
        int retval;

        if (myst_gcov_abort_ocall(&retval) != OE_OK)
            assert(0);

        return retval;
    }
    else if (strcmp(func, "myst_gcov_fopen") == 0)
    {
        const char* pathname = (const char*)params[0];
        const char* mode = (const char*)params[1];
        long retval;

        if (myst_gcov_fopen_ocall(&retval, pathname, mode) != OE_OK)
            assert(0);

        return retval;
    }
    else if (strcmp(func, "myst_gcov_fdopen") == 0)
    {
        int fd = (int)params[0];
        const char* mode = (const char*)params[1];
        long retval;

        if (myst_gcov_fdopen_ocall(&retval, fd, mode) != OE_OK)
            assert(0);

        return retval;
    }
    else if (strcmp(func, "myst_gcov_fread") == 0)
    {
        void* ptr = (void*)params[0];
        size_t size = (size_t)params[1];
        size_t nmemb = (size_t)params[2];
        long stream = (long)params[3]; /*TODO: long to FILE* */
        size_t retval;

        if (myst_gcov_fread_ocall(&retval, ptr, size, nmemb, stream) != OE_OK)
            assert(0);

        return (long)retval; /*TODO: returning long not size_t*/
    }
    else if (strcmp(func, "myst_gcov_fwrite") == 0)
    {
        const void* ptr = (const void*)params[0];
        size_t size = (size_t)params[1];
        size_t nmemb = (size_t)params[2];
        long stream = (long)params[3]; /*TODO: long to FILE* */
        size_t retval;

        if (myst_gcov_fwrite_ocall(&retval, ptr, size, nmemb, stream) != OE_OK)
            assert(0);

        return (long)retval; /*TODO: returning long not size_t*/
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
        int fd = (int)params[0];
        int cmd = (int)params[1];
        long arg = (long)params[2];
        int retval;

        if (myst_gcov_fcntl_ocall(&retval, fd, cmd, arg) != OE_OK)
            assert(0);

        return retval;
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
        int retval;

        if (myst_gcov___errno_location_ocall(&retval) != OE_OK)
            assert(0);

        return retval;
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
