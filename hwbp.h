#ifndef HWBP_INCLUDED
#define HWBP_INCLUDED

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// NOTE: If 'handler' is not null, it will be invoked as a signal handler for SIGTRAP.
// Keep in mind the limitations on what signal handlers can/should do.
bool install_breakpoint(void *addr, int num_bytes, int bpno, void (*handler)(int));

bool disable_breakpoint(int bpno);

#ifdef __cplusplus
}
#endif


#endif
