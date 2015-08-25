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
#ifndef ZOCLE_UTIL_FILE_H_
#define ZOCLE_UTIL_FILE_H_

#include <cl.h>

#include <string.h>

/* extract the basename of a file */
extern char *clUtilFileBasename(char const *name);

static inline char *
clUtilFileExtension(char const *name) {
  char *b = clUtilFileBasename(name);
  char *e = strrchr(b, '.');
  return e ? e : strchr(b, 0);
}

#endif
