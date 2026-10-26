/*
 * Q57(a-g): Query system limitations with sysconf.
 */
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

// Ask sysconf for one limit. A return value of -1 needs errno to tell an error from an unknown
// limit.
static int show(const char *label, int name)
{
    errno = 0;
    long value = sysconf(name);
    if (value == -1)
    {
        if (errno)
        {
            perror(label);
            return 1;
        }
        printf("%s: indeterminate or unlimited\n", label);
    }
    else
    {
        printf("%s: %ld\n", label, value);
    }
    return 0;
}

int main(void)
{
    // Check every requested value, even if one query reports an error.
    int result = 0;

    // ARG_MAX includes the argument strings and environment passed to a new program.
    result |= show("a. exec argument + environment limit (bytes)", _SC_ARG_MAX);

    // This is the configured per-user process limit; other limits can also restrict process
    // creation.
    result |= show("b. Processes per real UID (configured limit)", _SC_CHILD_MAX);
    /* _SC_CLK_TCK is the userspace accounting tick rate, not necessarily
       the kernel's configured scheduler HZ. */
    result |= show("c. Clock/accounting ticks per second", _SC_CLK_TCK);
    result |= show("d. Open files per process", _SC_OPEN_MAX);

    // Memory is managed in pages, so page size and page counts are reported separately.
    result |= show("e. Page size (bytes)", _SC_PAGESIZE);
    result |= show("f. Physical memory pages", _SC_PHYS_PAGES);

    // The available-page count is a snapshot and can change while other programs are running.
    result |= show("g. Currently available physical pages", _SC_AVPHYS_PAGES);
    return result;
}
