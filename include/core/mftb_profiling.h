/*
 * Copyright (C) an0nym0us
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 */

#include <stdio.h>
#ifdef __powerpc64__
#include <ppu_intrinsics.h>


unsigned long long start, stop;


#define argprintf(stream,format,args...)fprintf ( stream, "\033[0;33m%s:%s():#%d :: "format"\033[0m\n", __FILE__, __func__, __LINE__, args ) ;
#define mftbStart(start) start=__mftb();
#define mftbStop(start,stop) stop=__mftb();argprintf("TB ticks: %11d\n", stop - start);
#endif
