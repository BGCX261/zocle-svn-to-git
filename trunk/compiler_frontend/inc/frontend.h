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
#ifndef ZOCLE_COMPILER_FRONTEND_FRONTEND_H_
#define ZOCLE_COMPILER_FRONTEND_FRONTEND_H_

#include <compiler/inc/error_code.h>
#include <compiler/inc/compiler.h>

#include <compiler_frontend/inc/token.h>
#include <compiler_frontend/inc/readbuffer.h>

#ifndef DECLARE_CVECTOR_TYPE_FOR_CL_COMPILER_TOKEN
#define DECLARE_CVECTOR_TYPE_FOR_CL_COMPILER_TOKEN
DECLARE_CVECTOR_TYPE(cl_compiler_token)
#endif
#ifndef DEFINE_CVECTOR_TYPE_FOR_CL_COMPILER_TOKEN
#define DEFINE_CVECTOR_TYPE_FOR_CL_COMPILER_TOKEN
DEFINE_CVECTOR_TYPE(cl_compiler_token)
#endif

#ifndef DECLARE_CVECTOR_TYPE_FOR_CL_COMPILER_READ_BUFFER
#define DECLARE_CVECTOR_TYPE_FOR_CL_COMPILER_READ_BUFFER
DECLARE_CVECTOR_TYPE(cl_compiler_read_buffer)
#endif
#ifndef DEFINE_CVECTOR_TYPE_FOR_CL_COMPILER_READ_BUFFER
#define DEFINE_CVECTOR_TYPE_FOR_CL_COMPILER_READ_BUFFER
DEFINE_CVECTOR_TYPE(cl_compiler_read_buffer)
#endif

#ifndef DECLARE_CVECTOR_TYPE_FOR_CL_INT
#define DECLARE_CVECTOR_TYPE_FOR_CL_INT
DECLARE_CVECTOR_TYPE(cl_int)
#endif
#ifndef DEFINE_CVECTOR_TYPE_FOR_CL_INT
#define DEFINE_CVECTOR_TYPE_FOR_CL_INT
DEFINE_CVECTOR_TYPE(cl_int)
#endif

#ifndef DECLARE_CVECTOR_TYPE_FOR_CLIST_TYPE_FOR__CL_COMPILER_TOKEN_TYPE_PAIR
#define DECLARE_CVECTOR_TYPE_FOR_CLIST_TYPE_FOR__CL_COMPILER_TOKEN_TYPE_PAIR
DECLARE_CVECTOR_TYPE(CLIST_TYPE(_cl_compiler_token_type_pair))
#endif
#ifndef DEFINE_CVECTOR_TYPE_FOR_CLIST_TYPE_FOR__CL_COMPILER_TOKEN_TYPE_PAIR
#define DEFINE_CVECTOR_TYPE_FOR_CLIST_TYPE_FOR__CL_COMPILER_TOKEN_TYPE_PAIR
DEFINE_CVECTOR_TYPE(CLIST_TYPE(_cl_compiler_token_type_pair))
#endif

typedef enum cl_compiler_can_return_space_token {
  CAN_RETURN_SPACE_TOKEN,
  CAN_NOT_RETURN_SPACE_TOKEN
} cl_compiler_can_return_space_token;

typedef enum cl_compiler_parse_flag {
  /* return numbers instead of TOK_PPNUM */
  PARSE_FLAG_CONVERT_PREPROCESSOR_NUMBER_TOKEN_TO_REAL_NUMBER = (1 << 0),
  /* clCompilerGetNextTokenWithMacroSubst() can return space tokens (for -E) */
  PARSE_FLAG_CAN_RETURN_SPACE_TOKEN = (1 << 1),
  /* clCompilerGetNextTokenWithMacroSubst() can return linefeed tokens (for -E) */
  PARSE_FLAG_CAN_RETURN_LINEFEED_TOKEN = (1 << 2)
} cl_compiler_parse_flag;

struct _cl_compiler_frontend {
  cl_int                  parse_flags;
  
  cl_int                  token_flags;
  cl_compiler_token_type  token_type;
  cl_compiler_token       token;
  
  HASHTABLE_TYPE(cl_compiler_token)        token_table;
  CLIST_ONE_LEVEL_TYPE(cl_compiler_token, all_tokens)  all_tokens;
  
  CLIST_TYPE(char_ptr)                     include_paths;
  CLIST_TYPE(char_ptr)                     system_include_paths;
  CSTACK_TYPE(cl_compiler_read_buffer)     include_file_stack;
  
  cl_compiler_read_buffer read_buffer;
  
  /* First, token is read from the read buffer, and put into the 'unpreprocessed_token_list',
   * after preprocessed, the token(s) will be removed from the 'unpreprocessed_token_list' and
   * put into the 'preprocessed_token_list'.
   *
   * when parsing, the token will be removed from the preprocessed_token_list, and put into
   * unparsed_token_list. */
  CLIST_TYPE(_cl_compiler_token_type_pair) unpreprocessed_token_list;
  CLIST_TYPE(_cl_compiler_token_type_pair) preprocessed_token_list;
  CLIST_TYPE(_cl_compiler_token_type_pair) unparsed_token_list;
  
  CSTACK_TYPE(cl_compiler_token)           defined_token_stack;
  
  /* bit meaning:
   * 0bxx
   * bit 0: '1' if last #if/#elif expression is true, '0' otherwise.
   * bit 1: '1' if meet an #else directive, '0' otherwise. */
  CSTACK_TYPE(cl_int)                      ifdef_stack;
  
  CVECTOR_TYPE(CLIST_TYPE(_cl_compiler_token_type_pair)) choice_point_token_list;
};
typedef struct _cl_compiler_frontend *cl_compiler_frontend;

extern cl_compiler_frontend clCompilerFrontendNew(void);
extern void clCompilerFrontendDelete(cl_compiler_frontend frontend);

static inline void
clCompilerFrontendSetTokenFlags(cl_compiler_frontend frontend, cl_int flags) {
  ASSERT(frontend != NULL);
  frontend->token_flags = flags;
}

static inline void
clCompilerFrontendAddTokenFlags(cl_compiler_frontend frontend, cl_int flags) {
  ASSERT(frontend != NULL);
  frontend->token_flags |= flags;
}

static inline void
clCompilerFrontendSetParseFlags(cl_compiler_frontend frontend, cl_int flags) {
  ASSERT(frontend != NULL);
  frontend->parse_flags = flags;
}

static inline void
clCompilerFrontendAddParseFlags(cl_compiler_frontend frontend, cl_int flags) {
  ASSERT(frontend != NULL);
  frontend->parse_flags |= flags;
}

static inline cl_compiler_read_buffer
clCompilerFrontendGetCurrReadBuffer(cl_compiler_frontend frontend) {
  ASSERT(frontend != NULL);
  return frontend->read_buffer;
}

static inline void
clCompilerFrontendSetCurrReadBuffer(cl_compiler_frontend frontend,
                                    cl_compiler_read_buffer read_buffer) {
  ASSERT(frontend != NULL); 
  frontend->read_buffer = read_buffer;
}

static inline CSTACK_TYPE(cl_compiler_read_buffer)
clCompilerFrontendGetIncludeFileStack(cl_compiler_frontend frontend) {
  ASSERT(frontend != NULL);
  return frontend->include_file_stack;
}

extern void clCompilerFrontendDumpIncludeFileStack(cl_compiler_frontend frontend, cstring cstr);

static inline void
clCompilerFrontendSetIncludePath(cl_compiler_frontend frontend,
                                 CLIST_TYPE(char_ptr) include_paths) {
  ASSERT(frontend != NULL);
  ASSERT(include_paths != NULL);
  frontend->include_paths = include_paths;
}

static inline void
clCompilerFrontendSetSystemIncludePath(cl_compiler_frontend frontend,
                                       CLIST_TYPE(char_ptr) system_include_paths) {
  ASSERT(frontend != NULL);
  ASSERT(system_include_paths != NULL);
  frontend->system_include_paths = system_include_paths;
}

static inline size_t
clCompilerFrontendCurrDefinedTokenStackSize(cl_compiler_frontend frontend) {
  ASSERT(frontend != NULL);
  return CSTACK_SIZE(cl_compiler_token)(frontend->defined_token_stack);
}

static inline CSTACK_TYPE(cl_compiler_token)
clCompilerFrontendDefinedTokenStack(cl_compiler_frontend frontend) {
  ASSERT(frontend != NULL);
  return frontend->defined_token_stack;
}

extern cl_compiler_error_code clCompilerFrontendGetNextTokenWithoutMacroSubstImpl(
    cl_compiler_frontend frontend,
    cl_compiler_can_return_space_token const can_return_space_token,
    cl_bool const is_get,
    size_t const lookahead_idx,
    cl_bool * const is_token_preprocessed);

static inline cl_compiler_error_code
clCompilerFrontendGetNextTokenWithoutMacroSubst(
    cl_compiler_frontend frontend,
    cl_compiler_can_return_space_token const can_return_space_token,
    cl_bool * const is_token_preprocessed) {
  return clCompilerFrontendGetNextTokenWithoutMacroSubstImpl(
      frontend,
      can_return_space_token,
      CL_TRUE,
      0,
      is_token_preprocessed);
}

extern void clCompilerFrontendEnterBackTracking(cl_compiler_frontend frontend);
extern void clCompilerFrontendLeaveBackTracking(cl_compiler_frontend frontend);

static inline cl_bool
clCompilerFrontendIsInBackTracking(cl_compiler_frontend frontend) {
  ASSERT(frontend != NULL);
  return (0 == CVECTOR_SIZE(CLIST_TYPE(_cl_compiler_token_type_pair))(
              frontend->choice_point_token_list))
    ? CL_FALSE
    : CL_TRUE;
}

extern cl_bool isReadBufferCharEasciiIdOrDecimalDigit(cl_read_buffer_char const ch);

extern cl_compiler_error_code clCompilerFrontendGetNextUnparsedToken(
    cl_compiler_frontend frontend,
    _cl_compiler_token_type_pair * const token_type_pair);

extern cl_compiler_error_code clCompilerFrontendPeekNextUnparsedToken(
    cl_compiler_frontend frontend,
    size_t const lookahead,
    _cl_compiler_token_type_pair * const token_type_pair);

extern cl_compiler_error_code clCompilerFrontendConsumeNextUnparsedToken(cl_compiler_frontend frontend);

extern cl_compiler_error_code clCompilerFrontendEnsureNextUnparsedToken(
    cl_compiler_frontend frontend,
    _cl_compiler_token_type_pair * const token_type_pair);

extern cl_compiler_error_code clCompilerFrontendPreprocessorDefineSymbol(
    cl_compiler_frontend frontend, char const *symbol, char const *value);
extern void clCompilerFrontendPreprocessorUndefineSymbol(
    cl_compiler_frontend frontend, char const *symbol);

extern void clCompilerFrontendInit(void);

extern cl_compiler_error_code clCompilerFrontendSetInputFile(cl_compiler_frontend frontend, char const *filename);

struct _cl_compiler_cmd_line_option;

extern cl_compiler_error_code clCompilerFrontendInitFromCmdLine(
    cl_compiler_frontend frontend, struct _cl_compiler_cmd_line_option * const cmd_line_option);

extern cl_compiler_token clCompilerFrontendTokenFindAndAllocate(
    cl_compiler_frontend frontend, cl_compiler_token_type const token_type, char const *str);

extern cl_compiler_error_code clCompilerFrontendCompileFile(cl_compiler_frontend frontend);

#endif
