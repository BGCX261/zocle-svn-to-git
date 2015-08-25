/* zocle - Z OpenCL Environment
 * Copyright (C) 2009 Wei Hu <wei.hu.tw@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef ZOCLE_PLATFORM_OSAL_MEM_H_
#define ZOCLE_PLATFORM_OSAL_MEM_H_

#include <cl.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef MEMORY_DEBUGGER
#define CL_OSAL_MALLOC(size)        clOsalMallocDbg(size, __FILE__, __LINE__)
#define CL_OSAL_CALLOC(size)        clOsalCallocDbg(size, __FILE__, __LINE__)
#define CL_OSAL_REALLOC(ptr, size)  clOsalReallocDbg(ptr, size, __FILE__, __LINE__)
#define CL_OSAL_FREE(ptr)           clOsalFreeDbg(ptr)
#else
#define CL_OSAL_MALLOC(size)        clOsalMalloc(size)
#define CL_OSAL_CALLOC(size)        clOsalCalloc(size)
#define CL_OSAL_REALLOC(ptr, size)  clOsalRealloc(ptr, size)
#define CL_OSAL_FREE(ptr)           clOsalFree(ptr)
#endif

extern void *clOsalMallocDbg(size_t const size, char const *filename, int const lineno);
extern void *clOsalCallocDbg(size_t const size, char const *filename, int const lineno);
extern void *clOsalReallocDbg(void *ptr, size_t const size, char const *filename, int const lineno);
extern void  clOsalFreeDbg(void *ptr);

extern void  clOsalMemoryChecker(void);

static inline void *
clOsalMalloc(size_t const size) {
  return malloc(size);
}

static inline void *
clOsalRealloc(void *ptr, size_t const size) {
  return realloc(ptr, size);
}

static inline void *
clOsalCalloc(size_t const size) {
  return calloc(size, 1);
}

static inline void
clOsalFree(void *ptr) {
  if (ptr != NULL) {
    free(ptr);
  }
}

#endif
