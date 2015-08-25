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

/* The following algorithm is from
 * 'An Efficient Method of Computing Static Single Assignment Form', figure 2,
 * by Ron Cytron. */
void
clCompilerMiddleendComputeDominanceFrontier(cl_compiler_basicblock X) {
  /* This step must do first,
   * Because this algorithm operates each basic block from the bottom up in
   * the dominator tree, we have to first call this function itself recursively.
   */
  {
    CLIST_ITER_TYPE(cl_compiler_basicblock) iter;
    
    for (iter = CLIST_BEGIN(cl_compiler_basicblock)(X->immediate_dominanced_by_me);
         iter != CLIST_END(cl_compiler_basicblock)(X->immediate_dominanced_by_me);
         iter = CLIST_ITER_INCREMENT(cl_compiler_basicblock)(iter)) {
      cl_compiler_basicblock child = *CLIST_ITER_GET_DATA(cl_compiler_basicblock)(iter);
      clCompilerMiddleendComputeDominanceFrontier(child);
    }
  }
  
  {
    /* for each Y belongs to Succ(X) do */
    CLIST_ITER_TYPE(cl_compiler_basicblock) iter_y;
    
    for (iter_y = CLIST_BEGIN(cl_compiler_basicblock)(X->cfg_successors);
         iter_y != CLIST_END(cl_compiler_basicblock)(X->cfg_successors);
         iter_y = CLIST_ITER_INCREMENT(cl_compiler_basicblock)(iter_y)) {
      cl_compiler_basicblock Y = *CLIST_ITER_GET_DATA(cl_compiler_basicblock)(iter_y);
      
      /* if idom(Y) != X then */
      if (Y->immediate_dominator != X) {
        /* DF(X) = DF(X) U {Y} */
        CLIST_PUSH_BACK_UNIQUE(cl_compiler_basicblock)(X->dominance_frontier, &Y);
      }
    }
  }
  
  {
    /* for each Z belongs to Children(X) do */
    CLIST_ITER_TYPE(cl_compiler_basicblock) iter_z;
    
    for (iter_z = CLIST_BEGIN(cl_compiler_basicblock)(X->immediate_dominanced_by_me);
         iter_z != CLIST_END(cl_compiler_basicblock)(X->immediate_dominanced_by_me);
         iter_z = CLIST_ITER_INCREMENT(cl_compiler_basicblock)(iter_z)) {
      cl_compiler_basicblock Z = *CLIST_ITER_GET_DATA(cl_compiler_basicblock)(iter_z);
      
      {
        /* for each Y belongs to DF(Z) do */
        CLIST_ITER_TYPE(cl_compiler_basicblock) iter_y;
        
        for (iter_y = CLIST_BEGIN(cl_compiler_basicblock)(Z->dominance_frontier);
             iter_y != CLIST_END(cl_compiler_basicblock)(Z->dominance_frontier);
             iter_y = CLIST_ITER_INCREMENT(cl_compiler_basicblock)(iter_y)) {
          cl_compiler_basicblock Y = *CLIST_ITER_GET_DATA(cl_compiler_basicblock)(iter_y);
          
          /* if idom(Y) != X then */
          if (Y->immediate_dominator != X) {
            /* DF(X) = DF(X) U {Y} */
            CLIST_PUSH_BACK_UNIQUE(cl_compiler_basicblock)(X->dominance_frontier, &Y);
          }
        }
      }
    }
  }
}
