#include <gcov.h>
#include <string.h>
#include <unistd.h>

// TODO: couldnt import these from gcov.h
#define MYST_GCOV_STDERR ((FILE*)0x67001b41aafb4224)
#include <errno.h>
#include <sys/stat.h>

#include "myst_u.h"

void myst_gcov_abort_ocall(void)
{
    abort();

    for (;;)
        ;
}

FILE* myst_gcov_fopen_ocall(const char* pathname, const char* mode)
{
    return fopen(pathname, mode);
}

FILE* myst_gcov_fdopen_ocall(int fd, const char* mode)
{
    return fdopen(fd, mode);
}

size_t myst_gcov_fread_ocall(
    void* ptr,
    size_t size,
    size_t nmemb,
    size_t count,
    FILE* stream)
{
    if (stream == MYST_GCOV_STDERR)
        stream = stderr;

    return fread(ptr, size, nmemb, stream);
}

size_t myst_gcov_fwrite_ocall(
    const void* ptr,
    size_t size,
    size_t nmemb,
    size_t count,
    FILE* stream)
{
    if (stream == MYST_GCOV_STDERR)
        stream = stderr;

    return fwrite(ptr, size, nmemb, stream);
}

int myst_gcov_fseek_ocall(FILE* stream, uint64_t offset, int whence)
{
    if (stream == MYST_GCOV_STDERR)
        stream = stderr;

    return fseek(stream, offset, whence);
}

long myst_gcov_ftell_ocall(FILE* stream)
{
    if (stream == MYST_GCOV_STDERR)
        stream = stderr;

    return ftell(stream);
}

int myst_gcov_fclose_ocall(FILE* stream)
{
    if (stream == MYST_GCOV_STDERR)
        stream = stderr;

    return fclose(stream);
}

void myst_gcov_setbuf_ocall(FILE* stream, char* buf)
{
    if (stream == MYST_GCOV_STDERR)
        stream = stderr;

    return setbuf(stream, buf);
}

int myst_gcov_open_ocall(const char* pathname, int flags, mode_t mode)
{
    return open(pathname, flags, mode);
}

int myst_gcov_close_ocall(int fd)
{
    return close(fd);
}

int myst_gcov_fcntl_ocall(int fd, int cmd, uint64_t arg)
{
    return fcntl(fd, cmd, arg);
}

char* myst_gcov_getenv_ocall(const char* name)
{
    return getenv(name);
}

int* myst_gcov___errno_location_ocall(void)
{
    return __errno_location();
}

int myst_gcov_getpid_ocall(void)
{
    return getpid();
}

long int myst_gcov_strtol_ocall(const char* nptr, char** endptr, int base)
{
    return strtol(nptr, endptr, base);
}

int myst_gcov_access_ocall(const char* pathname, int mode)
{
    return access(pathname, mode);
}

int myst_gcov_mkdir_ocall(const char* pathname, mode_t mode)
{
    return mkdir(pathname, mode);
}

int myst_gcov___popcountdi2_ocall(unsigned long a)
{
    size_t nbits = 0;

    /* Count the number of bits that are set */
    for (unsigned long i = 0; i < 64; i++)
    {
        if ((a & (1LU << i)))
            nbits++;
    }

    /* Return 1 if the nbits is odd; return 0 if nbits is event */
    return (nbits % 2) ? 1 : 0;
}
