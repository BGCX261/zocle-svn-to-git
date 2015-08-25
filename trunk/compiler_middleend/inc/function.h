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
#ifndef CL_COMPILER_MIDDLEEND_FUNCTION_H_
#define CL_COMPILER_MIDDLEEND_FUNCTION_H_

#include <compiler_middleend/inc/basicblock.h>
#include <compiler_middleend/inc/il_var.h>

#include <cl.h>

#include <container/inc/clist.h>

#ifndef DECLARE_CLIST_TYPE_FOR_CL_COMPILER_IL_VAR
#define DECLARE_CLIST_TYPE_FOR_CL_COMPILER_IL_VAR
DECLARE_CLIST_TYPE(cl_compiler_il_var);
#endif
#ifndef DEFINE_CLIST_TYPE_FOR_CL_COMPILER_IL_VAR
#define DEFINE_CLIST_TYPE_FOR_CL_COMPILER_IL_VAR
DEFINE_CLIST_TYPE(cl_compiler_il_var);
#endif

#ifndef DECLARE_CVECTOR_TYPE_FOR_CL_COMPILER_IL_VAR
#define DECLARE_CVECTOR_TYPE_FOR_CL_COMPILER_IL_VAR
DECLARE_CVECTOR_TYPE(cl_compiler_il_var);
#endif
#ifndef DEFINE_CVECTOR_TYPE_FOR_CL_COMPILER_IL_VAR
#define DEFINE_CVECTOR_TYPE_FOR_CL_COMPILER_IL_VAR
DEFINE_CVECTOR_TYPE(cl_compiler_il_var);
#endif

typedef struct _cl_compiler_function *cl_compiler_function;
struct _cl_compiler_function {
  CLIST_TYPE(cl_compiler_basicblock) basicblocks;
  
  CLIST_TYPE(cl_compiler_il_var) used_variables;
  CVECTOR_TYPE(cl_compiler_il_var) used_variables_array;
};

extern void clCompilerMiddleendConvertToSSA(cl_compiler_function function);
extern void clCompilerMiddleendDominatorTreeLengauerTarjanBuildDominatorTree(
    cl_compiler_function function);
extern void clCompilerMiddleendLivenessAnalysis(cl_compiler_function function);
extern void clCompilerMiddleendDetermineLiveGlobalVar(cl_compiler_function function);

#endif
