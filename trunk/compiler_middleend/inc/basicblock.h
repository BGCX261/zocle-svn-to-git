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
#ifndef CL_COMPILER_MIDDLEEND_BASICBLOCK_H_
#define CL_COMPILER_MIDDLEEND_BASICBLOCK_H_

#include <cl.h>

#include <container/inc/clist.h>
#include <container/inc/cvector.h>

typedef struct _cl_compiler_basicblock *cl_compiler_basicblock;
typedef struct _cl_compiler_il_inst *cl_compiler_il_inst;

#define type_for_varlist cl_uint
#define ARRAY_SIZE_FOR_VARLIST(x) (((x) + 31) >> 5)

#ifndef DECLARE_CLIST_TYPE_FOR_CL_COMPILER_BASICBLOCK
#define DECLARE_CLIST_TYPE_FOR_CL_COMPILER_BASICBLOCK
DECLARE_CLIST_TYPE(cl_compiler_basicblock);
#endif
#ifndef DEFINE_CLIST_TYPE_FOR_CL_COMPILER_BASICBLOCK
#define DEFINE_CLIST_TYPE_FOR_CL_COMPILER_BASICBLOCK
DEFINE_CLIST_TYPE(cl_compiler_basicblock);
#endif

#ifndef DECLARE_CLIST_TYPE_FOR_CL_COMPILER_IL_INST
#define DECLARE_CLIST_TYPE_FOR_CL_COMPILER_IL_INST
DECLARE_CLIST_TYPE(cl_compiler_il_inst);
#endif
#ifndef DEFINE_CLIST_TYPE_FOR_CL_COMPILER_IL_INST
#define DEFINE_CLIST_TYPE_FOR_CL_COMPILER_IL_INST
DEFINE_CLIST_TYPE(cl_compiler_il_inst);
#endif

#ifndef DECLARE_CVECTOR_TYPE_FOR_CL_UINT
#define DECLARE_CVECTOR_TYPE_FOR_CL_UINT
DECLARE_CVECTOR_TYPE(cl_uint);
#endif
#ifndef DEFINE_CVECTOR_TYPE_FOR_CL_UINT
#define DEFINE_CVECTOR_TYPE_FOR_CL_UINT
DEFINE_CVECTOR_TYPE(cl_uint);
#endif

struct _cl_compiler_basicblock {
  cl_int dfs_tree_idx;
  
  cl_compiler_basicblock immediate_dominator;
  CLIST_TYPE(cl_compiler_basicblock) immediate_dominanced_by_me;
  CLIST_TYPE(cl_compiler_basicblock) dominance_frontier;
  
  CLIST_TYPE(cl_compiler_basicblock) cfg_successors;
  CLIST_TYPE(cl_compiler_basicblock) cfg_predecessors;
  
  cl_bool is_in_worklist;
  CVECTOR_TYPE(type_for_varlist) def_varlist;
  CVECTOR_TYPE(type_for_varlist) use_varlist;
  CVECTOR_TYPE(type_for_varlist) live_in_varlist;
  CVECTOR_TYPE(type_for_varlist) live_out_varlist;
  
  CLIST_TYPE(cl_compiler_il_inst) il_instructions;
};

extern void clCompilerMiddleendComputeDominanceFrontier(cl_compiler_basicblock X);

#endif
