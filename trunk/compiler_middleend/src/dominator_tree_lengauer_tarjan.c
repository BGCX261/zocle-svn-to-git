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

#include <container/inc/cvector.h>
#include <container/inc/clist.h>

#include <osal/inc/osal.h>

#ifndef DECLARE_CVECTOR_TYPE_FOR_CL_COMPILER_BASICBLOCK
#define DECLARE_CVECTOR_TYPE_FOR_CL_COMPILER_BASICBLOCK
DECLARE_CVECTOR_TYPE(cl_compiler_basicblock)
#endif
#ifndef DEFINE_CVECTOR_TYPE_FOR_CL_COMPILER_BASICBLOCK
#define DEFINE_CVECTOR_TYPE_FOR_CL_COMPILER_BASICBLOCK
DEFINE_CVECTOR_TYPE(cl_compiler_basicblock)
#endif

#ifndef DECLARE_CVECTOR_TYPE_FOR_SIZE_T
#define DECLARE_CVECTOR_TYPE_FOR_SIZE_T
DECLARE_CVECTOR_TYPE(size_t)
#endif
#ifndef DEFINE_CVECTOR_TYPE_FOR_SIZE_T
#define DEFINE_CVECTOR_TYPE_FOR_SIZE_T
DEFINE_CVECTOR_TYPE(size_t)
#endif

#ifndef DECLARE_CLIST_TYPE_FOR_SIZE_T
#define DECLARE_CLIST_TYPE_FOR_SIZE_T
DECLARE_CLIST_TYPE(size_t)
#endif
#ifndef DEFINE_CLIST_TYPE_FOR_SIZE_T
#define DEFINE_CLIST_TYPE_FOR_SIZE_T
DEFINE_CLIST_TYPE(size_t)
#endif

#ifndef DECLARE_CVECTOR_TYPE_FOR_CLIST_TYPE_FOR_SIZE_T
#define DECLARE_CVECTOR_TYPE_FOR_CLIST_TYPE_FOR_SIZE_T
DECLARE_CVECTOR_TYPE(CLIST_TYPE(size_t))
#endif
#ifndef DEFINE_CVECTOR_TYPE_FOR_CLIST_TYPE_FOR_SIZE_T
#define DEFINE_CVECTOR_TYPE_FOR_CLIST_TYPE_FOR_SIZE_T
DEFINE_CVECTOR_TYPE(CLIST_TYPE(size_t))
#endif

#define PARENT(dfs_idx)   (*(CVECTOR_AT(size_t)(dfs_parent_basicblock_in_dfs_order, (dfs_idx) - 1)))
#define VERTEX(dfs_idx)   (*(CVECTOR_AT(cl_compiler_basicblock)(basicblock_in_dfs_order, (dfs_idx) - 1)))

#define SEMI(dfs_idx)     (*(CVECTOR_AT(size_t)(dfs_idx_of_semidominator_in_dfs_order, (dfs_idx))))
#define SIZE(dfs_idx)     (*(CVECTOR_AT(size_t)(size_in_dfs_order, (dfs_idx))))
#define LABEL(dfs_idx)    (*(CVECTOR_AT(size_t)(label_in_dfs_order, (dfs_idx))))
#define CHILD(dfs_idx)    (*(CVECTOR_AT(size_t)(child_in_dfs_order, (dfs_idx))))
#define ANCESTOR(dfs_idx) (*(CVECTOR_AT(size_t)(ancestor_in_dfs_order, (dfs_idx))))
#define DOM(dfs_idx)      (*(CVECTOR_AT(size_t)(dom_in_dfs_order, (dfs_idx))))

static void
clCompilerMiddleendDominatorTreeLengauerTarjanPerformDFS(
    cl_compiler_basicblock v,
    size_t latest_dfs_idx,
    CVECTOR_TYPE(cl_compiler_basicblock) basicblock_in_dfs_order,
    CVECTOR_TYPE(size_t) dfs_idx_of_semidominator_in_dfs_order,
    CVECTOR_TYPE(size_t) dfs_parent_basicblock_in_dfs_order) {
  ASSERT(v != NULL);
  
  /* The index number for the 'depth first search' counts from 1. */
  /* Lengauer-Tarjan [1979], p.128:
   *
   * semi(v) := n := n + 1; */
  ++latest_dfs_idx;
  v->dfs_tree_idx = latest_dfs_idx;
  SEMI(latest_dfs_idx) = latest_dfs_idx;
  /* Lengauer-Tarjan [1979], p.127:
   *
   * After w is numbered but before its semidominator is computed,
   * semi(w) is the number of w.
   */
  
  /* Lengauer-Tarjan [1979], p.128:
   *
   * vertex(n) := v; */
  CVECTOR_PUSH_BACK(cl_compiler_basicblock)(basicblock_in_dfs_order, &v);
  
  {
    /* Lengauer-Tarjan [1979], p.128:
     *
     * comment initialize variables for steps 2, 3, and 4;
     * for each w belongs to succ(v) do */
    CLIST_ITER_TYPE(cl_compiler_basicblock) iter_w;
    
    for (iter_w = CLIST_BEGIN(cl_compiler_basicblock)(v->cfg_successors);
         iter_w != CLIST_END(cl_compiler_basicblock)(v->cfg_successors);
         iter_w = CLIST_ITER_INCREMENT(cl_compiler_basicblock)(iter_w)) {
      cl_compiler_basicblock w = *CLIST_ITER_GET_DATA(cl_compiler_basicblock)(iter_w);
      ASSERT(w != NULL);
      
      /* Lengauer-Tarjan [1979], p.128:
       *
       * if semi(w) = 0 then */
      if (0 == SEMI(w->dfs_tree_idx)) {
        /* SEMI(w) == 0 means that we did not visst this basicblock before. */
        
        /* Lengauer-Tarjan [1979], p.128:
         *
         * parent(w) := v; */
        PARENT(w->dfs_tree_idx) = v->dfs_tree_idx;
        
        /* Lengauer-Tarjan [1979], p.128:
         *
         * DFS(w); fi
         */
        clCompilerMiddleendDominatorTreeLengauerTarjanPerformDFS(
            w,
            latest_dfs_idx,
            basicblock_in_dfs_order,
            dfs_idx_of_semidominator_in_dfs_order,
            dfs_parent_basicblock_in_dfs_order);
      }
      /* Lengauer-Tarjan [1979], p.128:
       *
       * add v to pred(w); od
       */
      CLIST_PUSH_BACK(cl_compiler_basicblock)(w->cfg_predecessors, &v);
    }
  }
}

static void
clCompilerMiddleendDominatorTreeLengauerTarjanCompress(
    size_t v,
    CVECTOR_TYPE(size_t) dfs_idx_of_semidominator_in_dfs_order,
    CVECTOR_TYPE(size_t) label_in_dfs_order,
    CVECTOR_TYPE(size_t) ancestor_in_dfs_order) {
  /* Lengauer-Tarjan [1979], p.131:
   *
   * comment this procedure assumes ancestor(v) != 0
   */
  ASSERT(ANCESTOR(v) != 0);
  
  /* Lengauer-Tarjan [1979], p.131:
   *
   * if ancestor(ancestor(v)) != 0 then
   */
  if (ANCESTOR(ANCESTOR(v)) != 0) {
    /* Lengauer-Tarjan [1979], p.131:
     *
     * compress(ancestor(v));
     */
    clCompilerMiddleendDominatorTreeLengauerTarjanCompress(
        ANCESTOR(v),
        dfs_idx_of_semidominator_in_dfs_order,
        label_in_dfs_order,
        ancestor_in_dfs_order);
    
    /* Lengauer-Tarjan [1979], p.131:
     *
     * if semi(label(ancestor(v))) < semi(label(v)) then
     */
    if (SEMI(LABEL(ANCESTOR(v))) < SEMI(LABEL(v))) {
      /* Lengauer-Tarjan [1979], p.131:
       *
       * label(v) := label(ancestor(v)) fi;
       */
      LABEL(v) = LABEL(ANCESTOR(v));
    }
    
    /* Lengauer-Tarjan [1979], p.131:
     *
     * ancestor(v) := ancestor(ancestor(v)) fi;
     */
    ANCESTOR(v) = ANCESTOR(ANCESTOR(v));
  }
}

static size_t
clCompilerMiddleendDominatorTreeLengauerTarjanEval(
    size_t v,
    CVECTOR_TYPE(size_t) ancestor_in_dfs_order,
    CVECTOR_TYPE(size_t) label_in_dfs_order,
    CVECTOR_TYPE(size_t) dfs_idx_of_semidominator_in_dfs_order) {
  /* Lengauer-Tarjan [1979], p.131:
   *
   * if ancestor(v) = 0 then
   */
  if (0 == ANCESTOR(v)) {
    /* Lengauer-Tarjan [1979], p.131:
     *
     * EVAL := label(v);
     */
    return LABEL(v);
  } else {
    /* Lengauer-Tarjan [1979], p.131:
     *
     * else
     *   compress(v);
     */
    clCompilerMiddleendDominatorTreeLengauerTarjanCompress(
        v,
        dfs_idx_of_semidominator_in_dfs_order,
        label_in_dfs_order,
        ancestor_in_dfs_order);
    
    /* Lengauer-Tarjan [1979], p.131:
     *
     * EVAL := if semi(label(ancestor(v))) >= semi(label(v)) then
     */
    if (SEMI(LABEL(ANCESTOR(v))) >= SEMI(LABEL(v))) {
      /* Lengauer-Tarjan [1979], p.131:
       *
       * label(v);
       */
      return LABEL(v);
    } else {
      /* Lengauer-Tarjan [1979], p.131:
       *
       * else
       *   label(ancestor(v)) fi fi;
       */
      return LABEL(ANCESTOR(v));
    }
  }
}

static void
clCompilerMiddleendDominatorTreeLengauerTarjanLink(
    size_t v,
    size_t w,
    CVECTOR_TYPE(size_t) dfs_idx_of_semidominator_in_dfs_order,
    CVECTOR_TYPE(size_t) size_in_dfs_order,
    CVECTOR_TYPE(size_t) label_in_dfs_order,
    CVECTOR_TYPE(size_t) child_in_dfs_order,
    CVECTOR_TYPE(size_t) ancestor_in_dfs_order) {
  size_t s;
  
  /* Lengauer-Tarjan [1979], p.132:
   *
   * comment this procedure assumes for convenience that
   * size(0) = label(0) = semi(0) = 0; 
   */
  ASSERT(0 == SIZE(0));
  ASSERT(0 == LABEL(0));
  ASSERT(0 == SEMI(0));
  
  /* Lengauer-Tarjan [1979], p.132:
   *
   * s := w
   */
  s = w;
  
  /* Lengauer-Tarjan [1979], p.132:
   *
   * while semi(label(w)) < semi(label(child(s))) do
   */
  while (SEMI(LABEL(w)) < SEMI(LABEL(CHILD(s)))) {
    /* Lengauer-Tarjan [1979], p.132:
     *
     * if size(s) + size(child(child(s))) >= 2 * size(child(s)) then
     */
    if (SIZE(s) + SIZE(CHILD(CHILD(s))) >= 2 * SIZE(CHILD(s))) {
      /* Lengauer-Tarjan [1979], p.132:
       *
       * ancestor(child(s)) := s;
       * child(s) := child(child(s))
       */
      ANCESTOR(CHILD(s)) = s;
      CHILD(s) = CHILD(CHILD(s));
    }
    else
    {
      /* Lengauer-Tarjan [1979], p.132:
       *
       * else
       *   size(child(s)) := size(s);
       *   s := ancestor(s) := child(s) fi od;
       */
      SIZE(CHILD(s)) = SIZE(s);
      ANCESTOR(s) = CHILD(s);
      s = ANCESTOR(s);
    }
  }
  
  /* Lengauer-Tarjan [1979], p.132:
   *
   * label(s) := label(w);
   */
  LABEL(s) = LABEL(w);
  
  /* Lengauer-Tarjan [1979], p.132:
   *
   * size(v) := size(v) + size(w);
   */
  SIZE(v) += SIZE(w);
  
  /* Lengauer-Tarjan [1979], p.132:
   *
   * if size(v) < 2 * size(w) then
   */
  if (SIZE(v) < 2 * SIZE(w)) {
    /* Lengauer-Tarjan [1979], p.132:
     *
     * s, child(v) := child(v), s fi;
     */
    size_t const tmp = s;
    s = CHILD(v);
    CHILD(v) = tmp;
  }
  
  /* Lengauer-Tarjan [1979], p.132:
   *
   * while s != 0 do
   */
  while (s != 0) {
    /* Lengauer-Tarjan [1979], p.132:
     *
     * ancestor(s) := v;
     * s := child(s) od
     */
    ANCESTOR(s) = v;
    s = CHILD(s);
  }
}

/* There are several algorithms to build dominator tree:
 *
 * (1) Purdom-Moore [1972]:
 *
 * Says a flowgraph G = (V, E, r), V means all vertex, E means all edges,
 * r means the start node.
 *
 * for all v in (V - r) do
 *    remove v from G
 *    R(v) = unreachable vertices
 *    for all u in R(v) do
 *       Dom(u) = Dom(u) | {v}
 *    done
 * done
 *
 * This algorithm is very simple.
 * We remove a node, A, from the graph at a time,
 * and to see whether there are nodes become unreachable from the rood node.
 * If such a node, B, exists, then the dominator of node B will include
 * node A.
 * Because we have to go through node A to reach node B, otherwise,
 * node B will unable to reach (i.e. unreachable).
 *
 * Disadvantages: very slow in pratice.
 *
 * (2) Allen and Cocke [1972]:
 *
 * Disadvantages: very slow in pratice (but better than Purdom-Moore).
 * 
 * (3) Lengauer-Tarjan [1979]:
 *
 * Refer to 'A Fast Algorithm for Finding Dominators in a Flowgraph (1979)'
 * for the details.
 *
 * Tarjan defines a quantity called the semi-dominator and computes
 * these value in a bottom-up walk of the depth-first search tree.
 * Having these values, he can easily compute the actual dominators.
 */  
/*
 * Use Lengauer-Tarjan's sophisticated algorithm to compute the dominator tree.
 */
void
clCompilerMiddleendDominatorTreeLengauerTarjanBuildDominatorTree(
    cl_compiler_function function) {
  CVECTOR_TYPE(cl_compiler_basicblock) basicblock_in_dfs_order;
  CVECTOR_TYPE(size_t) dfs_idx_of_semidominator_in_dfs_order;
  CVECTOR_TYPE(size_t) size_in_dfs_order;
  CVECTOR_TYPE(size_t) dfs_parent_basicblock_in_dfs_order;
  CVECTOR_TYPE(size_t) label_in_dfs_order;
  CVECTOR_TYPE(size_t) child_in_dfs_order;
  CVECTOR_TYPE(size_t) ancestor_in_dfs_order;
  CVECTOR_TYPE(size_t) dom_in_dfs_order;
  size_t w;
  
  basicblock_in_dfs_order =
    CVECTOR_NEW(cl_compiler_basicblock)(CLIST_SIZE(cl_compiler_basicblock)(function->basicblocks));
  
  /* Create an array to store the DFS index value for the semidominator basicblock
   * for each basicblock. The basicblock in this array is in the DFS index order.
   */
  dfs_idx_of_semidominator_in_dfs_order =
    CVECTOR_NEW(size_t)(CLIST_SIZE(cl_compiler_basicblock)(function->basicblocks));
  CVECTOR_RESIZE(size_t)(dfs_idx_of_semidominator_in_dfs_order,
                         CLIST_SIZE(cl_compiler_basicblock)(function->basicblocks) + 1,
                         0);
  
  dfs_parent_basicblock_in_dfs_order =
    CVECTOR_NEW(size_t)(CLIST_SIZE(cl_compiler_basicblock)(function->basicblocks));
  CVECTOR_RESIZE(size_t)(dfs_parent_basicblock_in_dfs_order,
                         CLIST_SIZE(cl_compiler_basicblock)(function->basicblocks),
                         0);
  
  clCompilerMiddleendDominatorTreeLengauerTarjanPerformDFS(
      *CLIST_FRONT(cl_compiler_basicblock)(function->basicblocks),
      /* Lengauer-Tarjan [1979], p.128:
       * step1: n := 0;
       */
      0,
      basicblock_in_dfs_order,
      dfs_idx_of_semidominator_in_dfs_order,
      dfs_parent_basicblock_in_dfs_order);
  
  size_in_dfs_order =
    CVECTOR_NEW(size_t)(CVECTOR_SIZE(cl_compiler_basicblock)(basicblock_in_dfs_order));
  CVECTOR_RESIZE(size_t)(size_in_dfs_order,
                         CVECTOR_SIZE(cl_compiler_basicblock)(basicblock_in_dfs_order) + 1,
                         0);
  
  label_in_dfs_order =
    CVECTOR_NEW(size_t)(CVECTOR_SIZE(cl_compiler_basicblock)(basicblock_in_dfs_order));
  CVECTOR_RESIZE(size_t)(label_in_dfs_order,
                         CVECTOR_SIZE(cl_compiler_basicblock)(basicblock_in_dfs_order) + 1,
                         0);
  
  child_in_dfs_order =
    CVECTOR_NEW(size_t)(CVECTOR_SIZE(cl_compiler_basicblock)(basicblock_in_dfs_order));
  CVECTOR_RESIZE(size_t)(child_in_dfs_order,
                         CVECTOR_SIZE(cl_compiler_basicblock)(basicblock_in_dfs_order) + 1,
                         0);
  
  ancestor_in_dfs_order =
    CVECTOR_NEW(size_t)(CVECTOR_SIZE(cl_compiler_basicblock)(basicblock_in_dfs_order));
  CVECTOR_RESIZE(size_t)(ancestor_in_dfs_order,
                         CVECTOR_SIZE(cl_compiler_basicblock)(basicblock_in_dfs_order) + 1,
                         0);
  
  dom_in_dfs_order =
    CVECTOR_NEW(size_t)(CVECTOR_SIZE(cl_compiler_basicblock)(basicblock_in_dfs_order));
  CVECTOR_RESIZE(size_t)(dom_in_dfs_order,
                         CVECTOR_SIZE(cl_compiler_basicblock)(basicblock_in_dfs_order) + 1,
                         0);
  
  {
    /* Lengauer-Tarjan [1979], p.127:
     *
     * Initially ancestor(v) = 0 and label(v) = v for each vertex v.
     *
     * Lengauer-Tarjan [1979], p.131:
     *
     * Initially size(v) = 1 and child(v) = 0 for all vertices v.
     */
    size_t i;
    for (i = 1; i < CVECTOR_SIZE(cl_compiler_basicblock)(basicblock_in_dfs_order) + 1; ++i) {
      ANCESTOR(i) = 0;
      CHILD(i) = 0;
      SIZE(i) = 1;
      LABEL(i) = i;
    }
  }
  
  {
    size_t i;
    CVECTOR_TYPE(CLIST_TYPE(size_t)) bucket_vector =
      CVECTOR_NEW(CLIST_TYPE(size_t))(CVECTOR_SIZE(cl_compiler_basicblock)(basicblock_in_dfs_order));
    CVECTOR_RESIZE(CLIST_TYPE(size_t))(bucket_vector,
                                       CVECTOR_SIZE(cl_compiler_basicblock)(basicblock_in_dfs_order),
                                       NULL);
    
    /* Lengauer-Tarjan [1979], p.129:
     *
     * comment initialize variables;
     * for i := n by -1 until 2 do
     */
    for (i = CVECTOR_SIZE(cl_compiler_basicblock)(basicblock_in_dfs_order);
         i != 1;
         --i) {
      /* Lengauer-Tarjan [1979], p.129:
       *
       *   w = vertex(i); */
      w = i;
      
      /* Lengauer-Tarjan [1979], p.129:
       *
       * step2: for each v belongs to pred(w) do */
      {
        CLIST_ITER_TYPE(cl_compiler_basicblock) iter_v;
        for (iter_v = CLIST_BEGIN(cl_compiler_basicblock)(VERTEX(w)->cfg_predecessors);
             iter_v != CLIST_END(cl_compiler_basicblock)(VERTEX(w)->cfg_predecessors);
             iter_v = CLIST_ITER_INCREMENT(cl_compiler_basicblock)(iter_v)) {
          /* Lengauer-Tarjan [1979], p.129:
           *
           * u := eval(v) */
          size_t u = clCompilerMiddleendDominatorTreeLengauerTarjanEval(
              (*CLIST_ITER_GET_DATA(cl_compiler_basicblock)(iter_v))->dfs_tree_idx,
              ancestor_in_dfs_order,
              label_in_dfs_order,
              dfs_idx_of_semidominator_in_dfs_order);
          
          /* Lengauer-Tarjan [1979], p.129:
           *
           * if semi(u) < semi(w) then semi(w) := semi(u) fi od; */
          if (SEMI(u) < SEMI(w)) {
            SEMI(w) = SEMI(u);
          }
        }
        
        /* Lengauer-Tarjan [1979], p.129:
         *
         * add w to bucket(vertex(semi(w))); */
        {
          CLIST_TYPE(size_t) *bucket_addr =
            CVECTOR_AT(CLIST_TYPE(size_t))(bucket_vector, SEMI(w) - 1);
          if (NULL == *bucket_addr) {
            *bucket_addr = CLIST_NEW(size_t)();
          }
          CLIST_PUSH_BACK(size_t)(*bucket_addr, &w);
        }
        
        /* Lengauer-Tarjan [1979], p.129:
         *
         * link(parent(w), w); */
        clCompilerMiddleendDominatorTreeLengauerTarjanLink(
            PARENT(w),
            w,
            dfs_idx_of_semidominator_in_dfs_order,
            size_in_dfs_order,
            label_in_dfs_order,
            child_in_dfs_order,
            ancestor_in_dfs_order);
        
        {
          /* Lengauer-Tarjan [1979], p.129:
           *
           * step3:
           *   for each v belongs to bucket(parent(w)) do
           *     delete v from bucket(parent(w)); */
          CLIST_TYPE(size_t) bucket = *CVECTOR_AT(CLIST_TYPE(size_t))(bucket_vector, PARENT(w) - 1);
          size_t v = *CLIST_BACK(size_t)(bucket);
          CLIST_POP_BACK(size_t)(bucket);
          
          {
            /* Lengauer-Tarjan [1979], p.129:
             *
             * u := eval(v); */
            size_t u = clCompilerMiddleendDominatorTreeLengauerTarjanEval(
                v,
                ancestor_in_dfs_order,
                label_in_dfs_order,
                dfs_idx_of_semidominator_in_dfs_order);
            
            /* Lengauer-Tarjan [1979], p.129:
             *
             *   if semi(u) < semi(v) then
             *     dom(v) := u
             *   else
             *     dom(v) := parent(w)
             *   fi
             *  od
             * od; */
            if (SEMI(u) < SEMI(v)) {
              DOM(v) = u;
            } else {
              DOM(v) = PARENT(w);
            }
          }
        }
      }
    }
  }
  
  {
    /* Lengauer-Tarjan [1979], p.129:
     *
     * step4:
     *   for i := 2 until n do
     */
    size_t i;
    for (i = 2; i < CVECTOR_SIZE(cl_compiler_basicblock)(basicblock_in_dfs_order) + 1; ++i) {
      /* Lengauer-Tarjan [1979], p.129:
       *
       *     w := vertex(i); 
       */
      w = i;
      
      /* Lengauer-Tarjan [1979], p.129:
       *
       * if dom(w) != vertex(semi(w)) then dom(w) := dom(dom(w)) fi od;
       */
      if (DOM(w) != SEMI(w)) {
        DOM(w) = DOM(DOM(w));
      }
    }
  }
  
  /* Lengauer-Tarjan [1979], p.129:
   *
   * dom(r) := 0;
   */
  DOM(1) = 0;
  
  /* Link each basic block with its dominator one. */
  if (CVECTOR_SIZE(cl_compiler_basicblock)(basicblock_in_dfs_order) > 1) {
    size_t i;
    for (i = 2; i < CVECTOR_SIZE(cl_compiler_basicblock)(basicblock_in_dfs_order) + 1; ++i) {
      VERTEX(i)->immediate_dominator = VERTEX(DOM(i));
      CLIST_PUSH_BACK(cl_compiler_basicblock)(VERTEX(DOM(i))->immediate_dominanced_by_me, &VERTEX(i));
    }
  }
  VERTEX(1)->immediate_dominator = NULL;
}
