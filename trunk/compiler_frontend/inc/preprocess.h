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
#ifndef ZOCLE_COMPILER_FRONTEND_PREPROCESS_H_
#define ZOCLE_COMPILER_FRONTEND_PREPROCESS_H_

#include <compiler_frontend/inc/frontend.h>

extern cl_bool clCompilerFrontendPreprocessFile(cl_compiler_frontend frontend);

extern void clCompilerFrontendParseString(cl_compiler_frontend frontend,
                                          int const seperator,
                                          cl_bool const is_long);

extern void clCompilerFrontendParsePreprocessorNumber(cl_compiler_frontend frontend,
                                                      cl_bool const start_by_dot);

extern void clCompilerFrontendParseComment(cl_compiler_frontend frontend);

extern void clCompilerFrontendParseLineComment(cl_compiler_frontend frontend);

extern void clCompilerFrontendConvertPreprocessorNumberTokenToRealNumber(
    char const *str,
    cl_compiler_token_type * const token_type,
    cl_compiler_token_value token_value);

extern cl_compiler_error_code clCompilerFrontendPreprocess(cl_compiler_frontend frontend);

extern cl_compiler_error_code clCompilerFrontendGetNextTokenWithMacroSubstImpl(
    cl_compiler_frontend frontend, cl_bool const is_get);

/* return next token with macro substitution */
static inline cl_compiler_error_code
clCompilerFrontendGetNextTokenWithMacroSubst(
    cl_compiler_frontend frontend) {
  return clCompilerFrontendGetNextTokenWithMacroSubstImpl(frontend, CL_TRUE);
}

static inline cl_compiler_error_code
clCompilerFrontendConsumeNextTokenWithMacroSubst(
    cl_compiler_frontend frontend) {
  return clCompilerFrontendGetNextTokenWithMacroSubstImpl(frontend, CL_TRUE);
}

/* return next token with macro substitution */
static inline cl_compiler_error_code
clCompilerFrontendPeekNextTokenWithMacroSubst(
    cl_compiler_frontend frontend) {
  return clCompilerFrontendGetNextTokenWithMacroSubstImpl(frontend, CL_FALSE);
}

#endif
