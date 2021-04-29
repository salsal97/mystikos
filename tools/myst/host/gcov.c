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
