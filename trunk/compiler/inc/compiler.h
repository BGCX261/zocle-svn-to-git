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
#ifndef ZOCLE_COMPILER_COMPILER_H_
#define ZOCLE_COMPILER_COMPILER_H_

#include <container/inc/clist.h>

#define ZCC_VERSION  "0.1"

typedef enum _cl_compiler_output_type {
  OUTPUT_TYPE_PREPROCESSOR_ONLY = (1 << 0)
} cl_compiler_output_type;

typedef char *char_ptr;
#ifndef DECLARE_CLIST_TYPE_FOR_CHAR_PTR
#define DECLARE_CLIST_TYPE_FOR_CHAR_PTR
DECLARE_CLIST_TYPE(char_ptr)
#endif
#ifndef DEFINE_CLIST_TYPE_FOR_CHAR_PTR
#define DEFINE_CLIST_TYPE_FOR_CHAR_PTR
DEFINE_CLIST_TYPE(char_ptr)
#endif

typedef struct _cl_compiler_cmd_line_define_undefine_symbol {
  cl_bool  is_define;
  char    *name;
  char    *value;
} _cl_compiler_cmd_line_define_undefine_symbol;

#ifndef DECLARE_CLIST_TYPE_FOR__CL_COMPILER_CMD_LINE_DEFINE_UNDEFINE_SYMBOL
#define DECLARE_CLIST_TYPE_FOR__CL_COMPILER_CMD_LINE_DEFINE_UNDEFINE_SYMBOL
DECLARE_CLIST_TYPE(_cl_compiler_cmd_line_define_undefine_symbol)
#endif
#ifndef DEFINE_CLIST_TYPE_FOR__CL_COMPILER_CMD_LINE_DEFINE_UNDEFINE_SYMBOL
#define DEFINE_CLIST_TYPE_FOR__CL_COMPILER_CMD_LINE_DEFINE_UNDEFINE_SYMBOL
DEFINE_CLIST_TYPE(_cl_compiler_cmd_line_define_undefine_symbol)
#endif

typedef struct _cl_compiler_cmd_line_option {
  CLIST_TYPE(char_ptr)     files_to_compile;
  cl_compiler_output_type  output_type;
  
  CLIST_TYPE(char_ptr)     include_paths;
  CLIST_TYPE(_cl_compiler_cmd_line_define_undefine_symbol)  define_undefine_symbol;
} _cl_compiler_cmd_line_option;

struct _cl_compiler_frontend;
struct _cl_compiler_middleend;

struct _cl_compiler {
  _cl_compiler_cmd_line_option cmd_line_option;
  
  struct _cl_compiler_frontend  *frontend;
  struct _cl_compiler_middleend *middleend;
};
typedef struct _cl_compiler *cl_compiler;

static inline void
clCompilerSetFrontend(cl_compiler compiler, struct _cl_compiler_frontend *frontend) {
  ASSERT(compiler != NULL);
  compiler->frontend = frontend;
}

static inline cl_compiler_output_type
clCompilerGetOutputType(cl_compiler compiler) {
  ASSERT(compiler != NULL);
  return compiler->cmd_line_option.output_type;
}

static inline void
clCompilerSetOutputType(cl_compiler compiler,
                        cl_compiler_output_type const output_type) {
  ASSERT(compiler != NULL);
  compiler->cmd_line_option.output_type = output_type;
}

static inline void
clCompilerAddFileToCompile(cl_compiler compiler, char *filename) {
  ASSERT(compiler != NULL);
  ASSERT(filename != NULL);
  if (NULL == compiler->cmd_line_option.files_to_compile) {
    compiler->cmd_line_option.files_to_compile = CLIST_NEW(char_ptr)();
  }
  CLIST_PUSH_BACK(char_ptr)(compiler->cmd_line_option.files_to_compile, &filename);
}

static inline CLIST_TYPE(char_ptr)
clCompilerGetFileToCompile(cl_compiler compiler) {
  ASSERT(compiler != NULL);
  return compiler->cmd_line_option.files_to_compile;
}

extern void clCompilerInit(void);

extern cl_compiler clCompilerNew(void);
extern void clCompilerDelete(cl_compiler compiler);

extern cl_bool clCompilerParseArgs(cl_compiler compiler, int argc, char **argv);

#endif
