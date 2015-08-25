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

#include <compiler_middleend/inc/middleend.h>

void
clCompilerMiddleendConvertToSSA(cl_compiler_function function) {
  /* Stage 1: build dominator tree */
  clCompilerMiddleendDominatorTreeLengauerTarjanBuildDominatorTree(function);
  
  /* Stage 2: compute dominance frontier */
  clCompilerMiddleendComputeDominanceFrontier(*CLIST_FRONT(cl_compiler_basicblock)(function->basicblocks));
  
  /* Stage 3: Live Variable Analysis (worklist algorithm) */
  clCompilerMiddleendLivenessAnalysis(function);
  
#if 0
  /* Stage 4: Determine which variable are live global. */
  
  /* Stage 5: insert Phi-function (pruned SSA) */
  
  /* Stage 6: Rename variable */
#endif
}
