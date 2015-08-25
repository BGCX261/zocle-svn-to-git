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

#ifndef ZOCLE_COMPILER_MIDDLEEND_IL_INST_H_
#define ZOCLE_COMPILER_MIDDLEEND_IL_INST_H_

#include <compiler_middleend/inc/il_var.h>
#include <compiler_middleend/inc/basicblock.h>

#include <container/inc/clist.h>

#ifndef DECLARE_CLIST_TYPE_FOR_CL_COMPILER_IL_VAR
#define DECLARE_CLIST_TYPE_FOR_CL_COMPILER_IL_VAR
DECLARE_CLIST_TYPE(cl_compiler_il_var)
#endif
#ifndef DEFINE_CLIST_TYPE_FOR_CL_COMPILER_IL_VAR
#define DEFINE_CLIST_TYPE_FOR_CL_COMPILER_IL_VAR
DEFINE_CLIST_TYPE(cl_compiler_il_var)
#endif

enum _cl_compiler_il_inst_type {
  IL_INST_TYPE_ADD,
  IL_INST_TYPE_ADC,
  IL_INST_TYPE_MOV,
  IL_INST_TYPE_AND,
  IL_INST_TYPE_OR,
  IL_INST_TYPE_EOR,
  IL_INST_TYPE_SUB,
  IL_INST_TYPE_SBC,
  IL_INST_TYPE_TST,
  IL_INST_TYPE_CMP,
  IL_INST_TYPE_NOT
};
typedef enum _cl_compiler_il_inst_type cl_compiler_il_inst_type;

struct _cl_compiler_il_inst {
  cl_compiler_il_inst_type  type;
  /* This IL instruction belongs to which basic block. */
  cl_compiler_basicblock     basicblock;
  
  CLIST_TYPE(cl_compiler_il_var)  operand;
  CLIST_TYPE(cl_compiler_il_var)  result;
};
typedef struct _cl_compiler_il_inst *cl_compiler_il_inst;

extern void clCompilerMiddleendCalculateDefUseForInst(cl_compiler_il_inst inst);

#endif
