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
#include <zocle_config.h>

/* avoid infinite recursive memory alloction calls */
#undef MEMORY_DEBUGGER

#include <osal/inc/osal.h>
#include <container/inc/clist.h>

#include <stdlib.h>

struct _memory_logger_element {
  char const *filename;
  int const lineno;
  void * const ptr;
};
typedef struct _memory_logger_element *memory_logger_element;
typedef struct _memory_logger_element _memory_logger_element;

#ifndef DECLARE_CLIST_TYPE_FOR__MEMORY_LOGGER_ELEMENT
#define DECLARE_CLIST_TYPE_FOR__MEMORY_LOGGER_ELEMENT
DECLARE_CLIST_TYPE(_memory_logger_element)
#endif

#ifndef DEFINE_CLIST_TYPE_FOR__MEMORY_LOGGER_ELEMENT
#define DEFINE_CLIST_TYPE_FOR__MEMORY_LOGGER_ELEMENT
DEFINE_CLIST_TYPE(_memory_logger_element)
#endif

CLIST_TYPE(_memory_logger_element) g_memory_logger = NULL;

void *
clOsalMallocDbg(size_t const size, char const *filename, int const lineno) {
  void *ptr;
  if (NULL == g_memory_logger) {
    g_memory_logger = CLIST_NEW(_memory_logger_element)();
  }
  ptr = malloc(size);
  {
    struct _memory_logger_element e = {filename, lineno, ptr};
    CLIST_PUSH_BACK(_memory_logger_element)(g_memory_logger, &e);
  }
  return ptr;
}

void *
clOsalCallocDbg(size_t const size, char const *filename, int const lineno) {
  void *ptr;
  if (NULL == g_memory_logger) {
    g_memory_logger = CLIST_NEW(_memory_logger_element)();
  }
  ptr = calloc(size, 1);
  {
    struct _memory_logger_element e = {filename, lineno, ptr};
    CLIST_PUSH_BACK(_memory_logger_element)(g_memory_logger, &e);
  }
  return ptr;
}

static cl_bool
compare_func(memory_logger_element e, void *ptr) {
  if (e->ptr == ptr) {
    return CL_TRUE;
  } else {
    return CL_FALSE;
  }
}

void *
clOsalReallocDbg(void *ptr, size_t const size, char const *filename, int const lineno) {
  CLIST_ITER_TYPE(_memory_logger_element) iter;
  
  ASSERT(ptr != NULL);
  
  iter = CLIST_SEARCH(_memory_logger_element)(g_memory_logger, ptr, compare_func);
  ASSERT(iter != CLIST_END(_memory_logger_element)(g_memory_logger));
  
  CLIST_ERASE(_memory_logger_element)(g_memory_logger, iter);
  
  if (NULL == g_memory_logger) {
    g_memory_logger = CLIST_NEW(_memory_logger_element)();
  }
  ptr = realloc(ptr, size);
  {
    struct _memory_logger_element e = {filename, lineno, ptr};
    CLIST_PUSH_BACK(_memory_logger_element)(g_memory_logger, &e);
  }
  return ptr;
}

void
clOsalFreeDbg(void *ptr) {
  if (ptr != NULL) {
    CLIST_ITER_TYPE(_memory_logger_element) iter;
    
    iter = CLIST_SEARCH(_memory_logger_element)(g_memory_logger, ptr, compare_func);
    ASSERT(iter != CLIST_END(_memory_logger_element)(g_memory_logger));
    
    CLIST_ERASE(_memory_logger_element)(g_memory_logger, iter);
    
    free(ptr);
  }
}

void
clOsalMemoryChecker(void) {
  if (CLIST_SIZE(_memory_logger_element)(g_memory_logger) != 0) {
    CLIST_ITER_TYPE(_memory_logger_element) iter =
      CLIST_BEGIN(_memory_logger_element)(g_memory_logger);
    while (iter != CLIST_END(_memory_logger_element)(g_memory_logger)) {
      _memory_logger_element *e = CLIST_ITER_GET_DATA(_memory_logger_element)(iter);
      clOsalPrintf("0x%p, at \"%s\":%d\n", e->ptr, e->filename, e->lineno);
      iter = CLIST_ITER_INCREMENT(_memory_logger_element)(iter);
    }
  }
}
