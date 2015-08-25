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
#ifndef ZOCLE_PLATFORM_OSAL_H_
#define ZOCLE_PLATFORM_OSAL_H_

#include <cl.h>

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define ASSERT(x) assert(x)
#define STATIC_ASSERT(x) typedef char __STATIC_ASSERT__[(x) ? 1 : -1]

static inline int
clOsalPrintf(char const *format, ...) {
  va_list args;
  int return_value;
  
  va_start(args, format);
  return_value = vprintf(format, args);
  va_end(args);
  
  return return_value;
}

#include <osal/inc/osal_mem.h>
#include <osal/inc/osal_file.h>

#endif
