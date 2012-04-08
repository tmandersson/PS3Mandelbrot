#include <stdio.h>
#include <ppu_intrinsics.h>
#include <lv2/syscalls.h>

uint64_t start, stop;
uint64_t freq;

#define argprintf(format,args...) printf("\033[0;33m%s:%s():#%d :: "format"\033[0m\n", __FILE__, __func__, __LINE__, args);
#define mftbStart(start) start=__mftb();
#define mftbStop(start,stop) stop=__mftb(); lv2syscall0 (SYSCALL_TIME_GET_TIMEBASE_FREQUENCY); freq=(uint64_t)(p1); argprintf("\nTB ticks: %llu\nTime: %llu usecs\n", stop - start, (stop - start) / (freq / 1000000) );
