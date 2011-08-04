#include <stdio.h>
#ifdef __powerpc64__
#include <ppu_intrinsics.h>

unsigned long long start, stop;

#define argprintf(format,args...) printf("\033[0;33m%s:%s():#%d :: "format"\033[0m\n", __FILE__, __func__, __LINE__, args);
#define mftbStart(start) start=__mftb();
#define mftbStop(start,stop) stop=__mftb(); argprintf("TB ticks: %11llu\n", stop - start);

#else

#include <time.h>

unsigned long long start, stop;

#define argprintf(format,args...) printf("\033[0;33m%s:%s():#%d :: "format"\033[0m\n", __FILE__, __func__, __LINE__, args);
#define mftbStart(start) start=clock();
#define mftbStop(start,stop) stop=clock(); argprintf("Clock cycles: %11llu\n", stop - start);

#endif
