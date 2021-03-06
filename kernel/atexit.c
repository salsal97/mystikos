// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <stdlib.h>

#include <myst/atexit.h>
#include <myst/spinlock.h>

typedef struct atexit_entry atexit_entry_t;

struct atexit_entry
{
    atexit_entry_t* next;
    void (*function)(void*);
    void* arg;
};

static atexit_entry_t* _entries;
static myst_spinlock_t _lock;

int myst_atexit(void (*function)(void*), void* arg)
{
    int ret = -1;
    atexit_entry_t* entry;

    if (!function)
        goto done;

    if (!(entry = calloc(1, sizeof(atexit_entry_t))))
        goto done;

    entry->function = function;
    entry->arg = arg;

    myst_spin_lock(&_lock);
    entry->next = _entries;
    _entries = entry;
    myst_spin_unlock(&_lock);

    ret = 0;

done:

    return ret;
}

void myst_call_atexit_functions(void)
{
    atexit_entry_t* p;

    for (p = _entries; p;)
    {
        atexit_entry_t* next = p->next;

        if (p->function)
            (*p->function)(p->arg);

        free(p);

        p = next;
    }

    _entries = NULL;
}
