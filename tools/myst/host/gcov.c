#include <string.h>
#include <unistd.h>

#include "myst_u.h"

char* myst_gcov_getenv_ocall(const char* name)
{
    return getenv(name);
}

int myst_gcov_getpid_ocall(void)
{
    return getpid();
}

int myst_gcov_open_ocall(const char* pathname, int flags, mode_t mode)
{
    return open(pathname, flags, mode);
}

int myst_gcov_fcntl_ocall(int fd, int cmd, long arg)
{
    return fcntl(fd, cmd, arg);
}

int myst_gcov___errno_location_ocall(void)
{
    return 123; // TODO: __errno_location();
}

void myst_gcov_abort_ocall(void)
{
    abort();

    for (;;)
        ;
}

// TODO: change long to FILE*; conflict with edl types
long myst_gcov_fopen_ocall(const char* pathname, const char* mode)
{
    return (long)fopen(pathname, mode);
}

// TODO: change long to FILE*; conflict with edl types
long myst_gcov_fdopen_ocall(int fd, const char* mode)
{
    return (long)fdopen(fd, mode);
}

// TODO: change long to FILE*; conflict with edl types
size_t myst_gcov_fread_ocall(void* ptr, size_t size, size_t nmemb, long stream)
{
    // TODO: does not recognize the macro
    // if (stream == MYST_GCOV_STDERR)
    //     stream = stderr;

    return fread(ptr, size, nmemb, (FILE*)stream);
}

size_t myst_gcov_fwrite_ocall(
    const void* ptr,
    size_t size,
    size_t nmemb,
    long stream)
{
    // TODO: does not recognize the macro
    // if (stream == MYST_GCOV_STDERR)
    //     stream = stderr;

    return fwrite(ptr, size, nmemb, (FILE*)stream);
}

// int myst_gcov_fseek_ocall(FILE* stream, long offset, int whence)
// {
//     if (stream == MYST_GCOV_STDERR)
//         stream = stderr;

//     return fseek(stream, offset, whence);
// }

// long myst_gcov_ftell(FILE* stream)
// {
//     if (stream == MYST_GCOV_STDERR)
//         stream = stderr;

//     return ftell(stream);
// }

// int myst_gcov_fclose(FILE* stream)
// {
//     if (stream == MYST_GCOV_STDERR)
//         stream = stderr;

//     return fclose(stream);
// }

// void myst_gcov_setbuf(FILE* stream, char* buf)
// {
//     if (stream == MYST_GCOV_STDERR)
//         stream = stderr;

//     return setbuf(stream, buf);
// }

// int myst_gcov_close(int fd)
// {
//     return close(fd);
// }

// long int myst_gcov_strtol(const char* nptr, char** endptr, int base)
// {
//     return strtol(nptr, endptr, base);
// }

// int myst_gcov_access(const char* pathname, int mode)
// {
//     return access(pathname, mode);
// }

// int myst_gcov_mkdir(const char* pathname, mode_t mode)
// {
//     return mkdir(pathname, mode);
// }

// int myst_gcov_vfprintf(FILE* stream, const char* format, va_list ap)
// {
//     if (stream == MYST_GCOV_STDERR)
//         stream = stderr;

//     return vfprintf(stream, format, ap);
// }

// int myst_gcov_fprintf(FILE* stream, const char* format, ...)
// {
//     if (stream == MYST_GCOV_STDERR)
//         stream = stderr;

//     va_list ap;
//     va_start(ap, format);
//     int r = vfprintf(stream, format, ap);
//     va_end(ap);
//     return r;
// }

// int myst_gcov___popcountdi2(unsigned long a)
// {
//     size_t nbits = 0;

//     /* Count the number of bits that are set */
//     for (unsigned long i = 0; i < 64; i++)
//     {
//         if ((a & (1LU << i)))
//             nbits++;
//     }

//     /* Return 1 if the nbits is odd; return 0 if nbits is event */
//     return (nbits % 2) ? 1 : 0;
// }

// int myst_gcov___vfprintf_chk(
//     FILE* stream,
//     int flag,
//     const char* format,
//     va_list ap)
// {
//     (void)flag;

//     if (stream == MYST_GCOV_STDERR)
//         stream = myst_gcov_stderr;

//     return vfprintf(stream, format, ap);
// }

// int myst_gcov___fprintf_chk(FILE* stream, int flag, const char* format, ...)
// {
//     va_list ap;
//     (void)flag;

//     if (stream == MYST_GCOV_STDERR)
//         stream = stderr;

//     va_start(ap, format);
//     int n = vfprintf(stream, format, ap);
//     va_end(ap);

//     return n;
// }
