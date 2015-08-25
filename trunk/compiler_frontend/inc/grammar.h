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
#ifndef ZOCLE_COMPILER_FRONTEND_GRAMMAR_H_
#define ZOCLE_COMPILER_FRONTEND_GRAMMAR_H_

#include <compiler/inc/error_code.h>
#include <compiler_frontend/inc/ast_node.h>

extern cl_compiler_error_code clCompilerFrontendParseConstantExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container);

extern cl_compiler_error_code clCompilerFrontendParseTranslationUnit(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container);

#endif
