#include <stdio.h>

#include "hwbp.h"

bool install_breakpoint(void *addr, int num_bytes, int bpno, void (*handler)(int)) {
    static bool reported = false;
    if (!reported) {
        fprintf(stderr, "WARNING: Using null implementation of hwbp.\n");
        reported = true;
    }
    return true;
}

bool disable_breakpoint(int bpno)
{
    return true;
}
