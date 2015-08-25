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

#include <compiler/inc/error_msg.h>
#include <compiler_frontend/inc/frontend.h>

static void
dump_error(cl_compiler_frontend frontend, cl_bool const is_warning,
           char const *fmt, va_list ap) {
  cstring cstr = CSTRING_NEW();
  
  if (frontend != NULL) {
    clCompilerFrontendDumpIncludeFileStack(frontend, cstr);
  } else {
    CSTRING_PRINTF(cstr, "zcc: ");
  }
  
  if (CL_TRUE == is_warning) {
    CSTRING_PRINTF(cstr, "warning: ");
  }
  CSTRING_VPRINTF(cstr, fmt, ap);
  
  fprintf(stderr, "%s\n", CSTRING_RAW_DATA(cstr));
  
  CSTRING_DELETE(cstr);
}

void
error(cl_compiler_frontend frontend, char const *fmt, ...) {
  va_list ap;
  
  va_start(ap, fmt);
  dump_error(frontend, CL_FALSE, fmt, ap);
  va_end(ap);
}

void
expect(cl_compiler_frontend frontend, char const *msg) {
  error(frontend, "%s expected", msg);
}

void
warning(cl_compiler_frontend frontend, char const *fmt, ...) {
  va_list ap;
  
  va_start(ap, fmt);
  dump_error(frontend, CL_TRUE, fmt, ap);
  va_end(ap);
}
