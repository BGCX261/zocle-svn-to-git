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
#include <compiler/inc/error_code.h>
#include <compiler_frontend/inc/frontend.h>
#include <compiler_frontend/inc/preprocess.h>
#include <util/inc/file.h>
#include <osal/inc/osal.h>

#include <stdio.h>

static void
help(void) {
  printf("zcc version " ZCC_VERSION " - ZoCle Compiler - Copyright (C) 2009 Wei Hu\n"
         "usage: zcc [-v] [-c] [-o outfile] [-Idir] [-Dsym[=val]] [-Usym]\n"
         "           [infile1 infile2...]\n"
         "\n"
         "General options:\n"
         "  -v          display current version\n"
         "Preprocessor options:\n"
         "  -E          preprocess only\n"
         "  -Idir       add include path 'dir'\n"
         "  -Dsym[=val] define 'sym' with value 'val'\n"
         "  -Usym       undefine 'sym'\n"
         );
}

static cl_bool
clCompilerIsSupportedFileType(char const * const ext) {
  if (0 == strcmp(ext, ".c")) {
    return CL_TRUE;
  }
  return CL_FALSE;
}

static cl_compiler_error_code
clCompilerProcessFile(cl_compiler compiler, char const *filename) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  cl_compiler_frontend frontend = NULL;
  
  /* find source file type with extension */
  char *ext = clUtilFileExtension(filename);
  
  if (CL_FALSE == clCompilerIsSupportedFileType(ext)) {
    error_code = COMPILER_ERROR__COMMAND_LINE__UNSUPPORTED_SOURCE_FILE_TYPE;
    goto finish;
  }
  
  {
    frontend = clCompilerFrontendNew();
    ASSERT(frontend != NULL);
    
    error_code = clCompilerFrontendInitFromCmdLine(frontend, &compiler->cmd_line_option);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    
    error_code = clCompilerFrontendSetInputFile(frontend, filename);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    
    clCompilerSetFrontend(compiler, frontend);
    frontend = NULL;
  }
  
  if (OUTPUT_TYPE_PREPROCESSOR_ONLY == clCompilerGetOutputType(compiler)) {
    error_code = clCompilerFrontendPreprocessFile(compiler->frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
  } else if (0 == PATHCMP(ext, ".c")) {
#ifdef _MSC_VER
#pragma warning(pop)
#endif
    /* C file assumed */
    error_code = clCompilerFrontendCompileFile(compiler->frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
  }
  
 finish:
  if (compiler->frontend != NULL) {
    clCompilerFrontendDelete(compiler->frontend);
    compiler->frontend = NULL;
  }
  return error_code;
}

int
main(int argc, char **argv) {
  cl_compiler compiler = NULL;
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  
  clCompilerInit();
  
  compiler = clCompilerNew();
  
  error_code = clCompilerParseArgs(compiler, argc, argv);
  if ((error_code != COMPILER_NO_ERROR) ||
      (0 == CLIST_SIZE(char_ptr)(clCompilerGetFileToCompile(compiler)))) {
    help();
    goto finish;
  }
  
  {
    CLIST_TYPE(char_ptr) files_to_compile = clCompilerGetFileToCompile(compiler);
    CLIST_ITER_TYPE(char_ptr) iter;
    
    if (NULL == files_to_compile) {
      error_code = COMPILER_ERROR__COMMAND_LINE__DOES_NOT_SPECIFY_SOURCE_FILES_TO_COMPILE;
      goto finish;
    }
    for (iter = CLIST_BEGIN(char_ptr)(files_to_compile);
         iter != CLIST_END(char_ptr)(files_to_compile);
         iter = CLIST_ITER_INCREMENT(char_ptr)(iter)) {
      error_code = clCompilerProcessFile(compiler, *CLIST_ITER_GET_DATA(char_ptr)(iter));
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
    }
  }
  
  clCompilerDelete(compiler);
  compiler = NULL;
  
#if defined(MEMORY_DEBUGGER)
  clOsalMemoryChecker();
#endif
  
 finish:
  if (compiler != NULL) {
    clCompilerDelete(compiler);
  }
  return error_code;
}
