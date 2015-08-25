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

#include <compiler_middleend/inc/basicblock.h>
#include <compiler_middleend/inc/function.h>
#include <compiler_middleend/inc/il_var.h>
#include <compiler_middleend/inc/il_inst.h>

#include <container/inc/cvector.h>

static void
indexingVariable(
    cl_compiler_function function,
    CVECTOR_TYPE(cl_compiler_il_var) used_variables_array) {
  ASSERT(0 == CVECTOR_SIZE(cl_compiler_il_var)(used_variables_array));
  
  {
    CLIST_ITER_TYPE(cl_compiler_il_var) iter_var;
    
    /* Number the variables (including global & temporary ones),
     * so that we can use the integer type to represent 'use', 'def', 'in' and 'out'. */
    for (iter_var = CLIST_BEGIN(cl_compiler_il_var)(function->used_variables);
         iter_var != CLIST_END(cl_compiler_il_var)(function->used_variables);
         iter_var = CLIST_ITER_INCREMENT(cl_compiler_il_var)(iter_var)) {
      cl_compiler_il_var var = *CLIST_ITER_GET_DATA(cl_compiler_il_var)(iter_var);
      CVECTOR_PUSH_BACK(cl_compiler_il_var)(used_variables_array, &var);
    }
  }
}

static void
calculateDefUseForBasicBlock(cl_compiler_basicblock bb,
                             size_t const array_size_needed_for_used_variables) {
  CVECTOR_CLEAR(type_for_varlist)(bb->def_varlist);
  CVECTOR_CLEAR(type_for_varlist)(bb->use_varlist);
  CVECTOR_CLEAR(type_for_varlist)(bb->live_in_varlist);
  CVECTOR_CLEAR(type_for_varlist)(bb->live_out_varlist);
  
  CVECTOR_RESIZE(type_for_varlist)(bb->def_varlist, array_size_needed_for_used_variables, 0);
  CVECTOR_RESIZE(type_for_varlist)(bb->use_varlist, array_size_needed_for_used_variables, 0);
  /* initialize IN(X) to 0 for all basic blocks */
  CVECTOR_RESIZE(type_for_varlist)(bb->live_in_varlist, array_size_needed_for_used_variables, 0);
  CVECTOR_RESIZE(type_for_varlist)(bb->live_out_varlist, array_size_needed_for_used_variables, 0);
  
  /* According to the 'Advanced Compiler Techniques',
   * the calculation of USE is from the last LIR to the first LIR.
   *
   * Ex: USE[x=z; x=x+1; y=1;] = {z} (x not in USE)
   *     DEF[x=z; x=x+1; y=1;] = {x, y}
   */
  {
    CLIST_ITER_TYPE(cl_compiler_il_inst) iter_inst;
    
    for (iter_inst = CLIST_RBEGIN(cl_compiler_il_inst)(bb->il_instructions);
         iter_inst != CLIST_REND(cl_compiler_il_inst)(bb->il_instructions);
         iter_inst = CLIST_ITER_DECREMENT(cl_compiler_il_inst)(iter_inst, bb->il_instructions)) {
      cl_compiler_il_inst inst = *CLIST_ITER_GET_DATA(cl_compiler_il_inst)(iter_inst);
      clCompilerMiddleendCalculateDefUseForInst(inst);
    }
  }
}

static void
buildWorklist(
    cl_compiler_basicblock bb,
    uint32_t const array_size_needed_for_used_variables,
    CLIST_TYPE(cl_compiler_basicblock) worklist) {
  if (CL_FALSE == bb->is_in_worklist) {
    /* Compute def-use for each basic block. */
    calculateDefUseForBasicBlock(bb, array_size_needed_for_used_variables);
    
    CLIST_PUSH_BACK(cl_compiler_basicblock)(worklist, &bb);
    bb->is_in_worklist = CL_TRUE;
    
    {
      CLIST_ITER_TYPE(cl_compiler_basicblock) iter_successor_bb;
      
      for (iter_successor_bb = CLIST_BEGIN(cl_compiler_basicblock)(bb->cfg_successors);
           iter_successor_bb != CLIST_END(cl_compiler_basicblock)(bb->cfg_successors);
           iter_successor_bb = CLIST_ITER_INCREMENT(cl_compiler_basicblock)(iter_successor_bb)) {
        cl_compiler_basicblock successor_bb =
          *CLIST_ITER_GET_DATA(cl_compiler_basicblock)(iter_successor_bb);
        buildWorklist(successor_bb, array_size_needed_for_used_variables, worklist);
      }
    }
  }
}

enum LiveInOutInfoChanged {
  LIVE_IN_OUT_INFO_HAS_CHANGED,
  LIVE_IN_OUT_INFO_DID_NOT_CHANGE
};

static enum LiveInOutInfoChanged
calculateLiveInOut(cl_compiler_basicblock X) {
  /* old_IN = IN(X) */
  CVECTOR_TYPE(type_for_varlist) old_live_in =
    CVECTOR_NEW(type_for_varlist)(CVECTOR_SIZE(type_for_varlist)(X->live_in_varlist));
  CVECTOR_COPY(type_for_varlist)(old_live_in, X->live_in_varlist);
  
  /* Reset OUT(X) to 0. */
  CVECTOR_SET_ALL_TO_ZERO(type_for_varlist)(X->live_out_varlist);
  
  /* OUT(X) = Union(IN(Y)) for all successors Y of X. */
  {
    CLIST_ITER_TYPE(cl_compiler_basicblock) iter_Y;
    
    for (iter_Y = CLIST_BEGIN(cl_compiler_basicblock)(X->cfg_successors);
         iter_Y != CLIST_END(cl_compiler_basicblock)(X->cfg_successors);
         iter_Y = CLIST_ITER_INCREMENT(cl_compiler_basicblock)(iter_Y)) {
      cl_compiler_basicblock Y =
        *CLIST_ITER_GET_DATA(cl_compiler_basicblock)(iter_Y);
      
      CVECTOR_ITER_TYPE(type_for_varlist) in_iter = CVECTOR_BEGIN(type_for_varlist)(Y->live_in_varlist);
      CVECTOR_ITER_TYPE(type_for_varlist) out_iter = CVECTOR_BEGIN(type_for_varlist)(X->live_out_varlist);
      
      while (in_iter != CVECTOR_END(type_for_varlist)(Y->live_in_varlist)) {
        ASSERT(out_iter != CVECTOR_END(type_for_varlist)(X->live_out_varlist));
        
        {
          type_for_varlist *in = CVECTOR_ITER_GET_DATA(type_for_varlist)(in_iter);
          type_for_varlist *out = CVECTOR_ITER_GET_DATA(type_for_varlist)(out_iter);
          
          (*out) |= (*in);
          
          in_iter = CVECTOR_ITER_INCREMENT(type_for_varlist)(in_iter);
          out_iter = CVECTOR_ITER_INCREMENT(type_for_varlist)(out_iter);
        }
      }
    }
  }
  
  {
    /* IN(X) = USE(X) Union (OUT(X) - DEF(X)); */
    CVECTOR_ITER_TYPE(type_for_varlist) in_iter = CVECTOR_BEGIN(type_for_varlist)(X->live_in_varlist);
    CVECTOR_ITER_TYPE(type_for_varlist) out_iter = CVECTOR_BEGIN(type_for_varlist)(X->live_out_varlist);
    CVECTOR_ITER_TYPE(type_for_varlist) use_iter = CVECTOR_BEGIN(type_for_varlist)(X->use_varlist);
    CVECTOR_ITER_TYPE(type_for_varlist) def_iter = CVECTOR_BEGIN(type_for_varlist)(X->def_varlist);
    
    while (in_iter != CVECTOR_END(type_for_varlist)(X->live_in_varlist)) {
      ASSERT(out_iter != CVECTOR_END(type_for_varlist)(X->live_out_varlist));
      ASSERT(def_iter != CVECTOR_END(type_for_varlist)(X->def_varlist));
      ASSERT(use_iter != CVECTOR_END(type_for_varlist)(X->use_varlist));
      {
        type_for_varlist *in = CVECTOR_ITER_GET_DATA(type_for_varlist)(in_iter);
        type_for_varlist *out = CVECTOR_ITER_GET_DATA(type_for_varlist)(out_iter);
        type_for_varlist *use = CVECTOR_ITER_GET_DATA(type_for_varlist)(use_iter);
        type_for_varlist *def = CVECTOR_ITER_GET_DATA(type_for_varlist)(def_iter);
        
        (*in) = ((*use) | ((*out) & (~(*def))));
        
        in_iter = CVECTOR_ITER_INCREMENT(type_for_varlist)(in_iter);
        out_iter = CVECTOR_ITER_INCREMENT(type_for_varlist)(out_iter);
        def_iter = CVECTOR_ITER_INCREMENT(type_for_varlist)(def_iter);
        use_iter = CVECTOR_ITER_INCREMENT(type_for_varlist)(use_iter);
      }
    }
    ASSERT(out_iter == CVECTOR_END(type_for_varlist)(X->live_out_varlist));
    ASSERT(def_iter == CVECTOR_END(type_for_varlist)(X->def_varlist));
    ASSERT(use_iter == CVECTOR_END(type_for_varlist)(X->use_varlist));
  }
  
  return ((CVECTOR_COMPARE(type_for_varlist)(old_live_in, X->live_in_varlist) != 0)
          ? LIVE_IN_OUT_INFO_HAS_CHANGED
          : LIVE_IN_OUT_INFO_DID_NOT_CHANGE);
}

void
clCompilerMiddleendLivenessAnalysis(cl_compiler_function function) {
  CLIST_TYPE(cl_compiler_basicblock) worklist;
  size_t array_size_needed_for_used_variables;
  
  worklist = CLIST_NEW(cl_compiler_basicblock)();
  
  if (NULL == function->used_variables_array) {
    function->used_variables_array =
      CVECTOR_NEW(cl_compiler_il_var)(CLIST_SIZE(cl_compiler_il_var)(function->used_variables));
  } else {
    CVECTOR_CLEAR(cl_compiler_il_var)(function->used_variables_array);
  }
  
  /* indexing the variables for the following stage. */
  indexingVariable(function, function->used_variables_array);
  
  array_size_needed_for_used_variables =
    ARRAY_SIZE_FOR_VARLIST(CVECTOR_SIZE(cl_compiler_il_var)(function->used_variables_array));
  
  /* Build work list first. */
  buildWorklist(*CLIST_FRONT(cl_compiler_basicblock)(
                    function->basicblocks),
                array_size_needed_for_used_variables,
                worklist);
  
  {
    CLIST_ITER_TYPE(cl_compiler_basicblock) iter_bb;
    for (iter_bb = CLIST_BEGIN(cl_compiler_basicblock)(worklist);
         iter_bb != CLIST_END(cl_compiler_basicblock)(worklist);
         iter_bb = CLIST_ITER_INCREMENT(cl_compiler_basicblock)(iter_bb)) {
      cl_compiler_basicblock bb = *CLIST_ITER_GET_DATA(cl_compiler_basicblock)(iter_bb);
      /* Indicate that this basic block has already been removed from the
       * work list. */
      bb->is_in_worklist = CL_FALSE;
      
      if (LIVE_IN_OUT_INFO_HAS_CHANGED == calculateLiveInOut(bb)) {
        /* add the predecessor bb of this bb back to the worklist */
        CLIST_ITER_TYPE(cl_compiler_basicblock) iter_predecessor_bb;
        
        for (iter_predecessor_bb = CLIST_BEGIN(cl_compiler_basicblock)(bb->cfg_predecessors);
             iter_predecessor_bb != CLIST_END(cl_compiler_basicblock)(bb->cfg_predecessors);
             iter_predecessor_bb = CLIST_ITER_INCREMENT(cl_compiler_basicblock)(iter_predecessor_bb)) {
          cl_compiler_basicblock predecessor_bb =
            *CLIST_ITER_GET_DATA(cl_compiler_basicblock)(iter_predecessor_bb);
          if (CL_FALSE == predecessor_bb->is_in_worklist) {
            CLIST_PUSH_BACK(cl_compiler_basicblock)(worklist, &predecessor_bb);
            predecessor_bb->is_in_worklist = CL_TRUE;
          }
        }
      }
    }
  }
  
  CLIST_DELETE(cl_compiler_basicblock)(worklist);
}
