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

#include <compiler/inc/compiler.h>
#include <compiler_frontend/inc/frontend.h>

cl_compiler
clCompilerNew(void) {
  cl_compiler compiler = CL_OSAL_CALLOC(sizeof(struct _cl_compiler));
  ASSERT(compiler != NULL);
  
  compiler->cmd_line_option.files_to_compile = NULL;
  
  return compiler;
}

void
clCompilerDelete(cl_compiler compiler) {
  ASSERT(compiler != NULL);
  
  if (compiler->cmd_line_option.files_to_compile != NULL) {
    CLIST_DELETE(char_ptr)(compiler->cmd_line_option.files_to_compile);
  }
  if (compiler->cmd_line_option.include_paths != NULL) {
    CLIST_DELETE(char_ptr)(compiler->cmd_line_option.include_paths);
  }
  
  CL_OSAL_FREE(compiler);
}

void
clCompilerInit(void) {
  clCompilerFrontendInit();
}
