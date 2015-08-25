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
#ifndef ZOCLE_CONTAINER_TREENODE_H_
#define ZOCLE_CONTAINER_TREENODE_H_

#include <container/inc/clist.h>

#include <osal/inc/osal.h>

#define DECLARE_TREENODE_TYPE(element_type)     \
  DECLARE_TREENODE_TYPE_X(element_type)

#define DECLARE_TREENODE_TYPE_X(element_type)                           \
  typedef struct _treenode_for_##element_type *treenode_for_##element_type; \
  typedef struct _treenode_for_##element_type _treenode_for_##element_type;

#define DEFINE_TREENODE_TYPE(element_type)      \
  DEFINE_TREENODE_TYPE_X(element_type)

#define DEFINE_TREENODE_TYPE_X(element_type)                            \
  DECLARE_CLIST_TYPE(_treenode_for_##element_type)                      \
                                                                        \
  struct _treenode_for_##element_type {                                 \
    element_type  data;                                                 \
    treenode_for_##element_type  parent;                                \
    CLIST_TYPE(_treenode_for_##element_type) children;                  \
  };                                                                    \
                                                                        \
  DEFINE_CLIST_TYPE(_treenode_for_##element_type)                       \
                                                                        \
  static treenode_for_##element_type                                    \
  zocleContainerTreenodeFor##element_type##New(void) {                  \
    treenode_for_##element_type treenode =                              \
      CL_OSAL_CALLOC(sizeof(struct _treenode_for_##element_type));      \
    if (NULL == treenode) {                                             \
      return NULL;                                                      \
    }                                                                   \
    treenode->parent = NULL;                                            \
    treenode->children = NULL;                                          \
                                                                        \
    return treenode;                                                    \
  }                                                                     \
                                                                        \
  static void                                                           \
  zocleContainerTreenodeFor##element_type##Delete(                      \
      treenode_for_##element_type treenode) {                           \
    ASSERT(treenode != NULL);                                           \
    if (treenode->children != NULL) {                                   \
      CLIST_DELETE(_treenode_for_##element_type)(treenode->children);   \
    }                                                                   \
    CL_OSAL_FREE(treenode);                                             \
  }                                                                     \
                                                                        \
  static inline element_type *                                          \
  zocleContainerTreenodeFor##element_type##GetData(                     \
      treenode_for_##element_type treenode) {                           \
    ASSERT(treenode != NULL);                                           \
    return &(treenode->data);                                           \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  zocleContainerTreenodeFor##element_type##SetData(                     \
      treenode_for_##element_type treenode,                             \
      element_type * const data) {                                      \
    ASSERT(treenode != NULL);                                           \
    ASSERT(data != NULL);                                               \
                                                                        \
    memcpy(&(treenode->data), data, sizeof(element_type));              \
  }                                                                     \
                                                                        \
  static inline treenode_for_##element_type                             \
  zocleContainerTreenodeFor##element_type##GetParent(                   \
      treenode_for_##element_type treenode) {                           \
    ASSERT(treenode != NULL);                                           \
    return treenode->parent;                                            \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  zocleContainerTreenodeFor##element_type##SetParent(                   \
      treenode_for_##element_type treenode,                             \
      treenode_for_##element_type parent) {                             \
    ASSERT(treenode != NULL);                                           \
    treenode->parent = parent;                                          \
  }                                                                     \
                                                                        \
  static inline CLIST_TYPE(_treenode_for_##element_type)                \
    zocleContainerTreenodeFor##element_type##GetChildren(               \
        treenode_for_##element_type treenode) {                         \
    ASSERT(treenode != NULL);                                           \
    return treenode->children;                                          \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  zocleContainerTreenodeFor##element_type##AddChildren(                 \
      treenode_for_##element_type treenode,                             \
      treenode_for_##element_type child) {                              \
    ASSERT(treenode != NULL);                                           \
    ASSERT(child != NULL);                                              \
    if (NULL == treenode->children) {                                   \
      treenode->children = CLIST_NEW(_treenode_for_##element_type)();   \
    }                                                                   \
    CLIST_PUSH_BACK(_treenode_for_##element_type)(                      \
        treenode->children, child);                                     \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  zocleContainerTreenodeFor##element_type##AddChildrenList(             \
      treenode_for_##element_type treenode,                             \
      CLIST_TYPE(_treenode_for_##element_type) child_list) {            \
    ASSERT(treenode != NULL);                                           \
    ASSERT(child_list != NULL);                                         \
    if (NULL == treenode->children) {                                   \
      treenode->children = CLIST_NEW(_treenode_for_##element_type)();   \
    }                                                                   \
    CLIST_SPLICE(_treenode_for_##element_type)(                         \
        treenode->children, NULL, child_list);                          \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  zocleContainerTreenodeFor##element_type##BuildRelation(               \
      treenode_for_##element_type parent,                               \
      treenode_for_##element_type child) {                              \
    ASSERT(parent != NULL);                                             \
    ASSERT(child != NULL);                                              \
    zocleContainerTreenodeFor##element_type##SetParent(child, parent);  \
    zocleContainerTreenodeFor##element_type##AddChildren(parent, child); \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  zocleContainerTreenodeFor##element_type##BuildRelationList(           \
      treenode_for_##element_type parent,                               \
      CLIST_TYPE(_treenode_for_##element_type) child) {                 \
    ASSERT(parent != NULL);                                             \
    ASSERT(child != NULL);                                              \
    {                                                                   \
      CLIST_ITER_TYPE(_treenode_for_##element_type) iter;               \
      for (iter = CLIST_BEGIN(_treenode_for_##element_type)(child);     \
           iter != CLIST_END(_treenode_for_##element_type)(child);      \
           iter = CLIST_ITER_INCREMENT(_treenode_for_##element_type)(iter)) { \
        zocleContainerTreenodeFor##element_type##SetParent(             \
            CLIST_ITER_GET_DATA(_treenode_for_##element_type)(iter), parent); \
      }                                                                 \
      zocleContainerTreenodeFor##element_type##AddChildrenList(parent, child); \
    }                                                                   \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  zocleContainerTreenodeContainerFor##element_type##AddChildren(        \
      CLIST_TYPE(_treenode_for_##element_type) list,                    \
      treenode_for_##element_type child) {                              \
    ASSERT(list != NULL);                                               \
    ASSERT(child != NULL);                                              \
    CLIST_PUSH_BACK(_treenode_for_##element_type)(list, child);         \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  zocleContainerTreenodeContainerFor##element_type##AddChildrenList(    \
      CLIST_TYPE(_treenode_for_##element_type) list,                    \
      CLIST_TYPE(_treenode_for_##element_type) child) {                 \
    ASSERT(list != NULL);                                               \
    ASSERT(child != NULL);                                              \
    CLIST_SPLICE(_treenode_for_##element_type)(list, NULL, child);      \
  }                                                                     \
                                                                        \
  static inline size_t                                                  \
  zocleContainerTreenodeContainerFor##element_type##Size(               \
      CLIST_TYPE(_treenode_for_##element_type) container) {             \
    ASSERT(container != NULL);                                          \
    return CLIST_SIZE(_treenode_for_##element_type)(container);         \
  }

#define TREENODE_TYPE(element_type)                         TREENODE_TYPE_X(element_type)
#define TREENODE_CONTAINER_TYPE(element_type)               TREENODE_CONTAINER_TYPE_X(element_type)
#define TREENODE_CONTAINER_CONCRETE_TYPE(element_type)      TREENODE_CONTAINER_CONCRETE_TYPE_X(element_type)
#define TREENODE_CONTAINER_ITER_TYPE(element_type)          TREENODE_CONTAINER_ITER_TYPE_X(element_type)

#define TREENODE_NEW(element_type)                          TREENODE_NEW_X(element_type)
#define TREENODE_DELETE(element_type)                       TREENODE_DELETE_X(element_type)
#define TREENODE_GET_DATA(element_type)                     TREENODE_GET_DATA_X(element_type)
#define TREENODE_SET_DATA(element_type)                     TREENODE_SET_DATA_X(element_type)
#define TREENODE_GET_PARENT(element_type)                   TREENODE_GET_PARENT_X(element_type)
#define TREENODE_SET_PARENT(element_type)                   TREENODE_SET_PARENT_X(element_type)
#define TREENODE_GET_CHILDREN(element_type)                 TREENODE_GET_CHILDREN_X(element_type)
#define TREENODE_ADD_CHILDREN(element_type)                 TREENODE_ADD_CHILDREN_X(element_type)
#define TREENODE_ADD_CHILDREN_LIST(element_type)            TREENODE_ADD_CHILDREN_LIST_X(element_type)
#define TREENODE_BUILD_RELATION(element_type)               TREENODE_BUILD_RELATION_X(element_type)
#define TREENODE_BUILD_RELATION_LIST(element_type)          TREENODE_BUILD_RELATION_LIST_X(element_type)

#define TREENODE_CONTAINER_NEW(element_type)                TREENODE_CONTAINER_NEW_X(element_type)
#define TREENODE_CONTAINER_ADD_CHILDREN(element_type)       TREENODE_CONTAINER_ADD_CHILDREN_X(element_type)
#define TREENODE_CONTAINER_ADD_CHILDREN_LIST(element_type)  TREENODE_CONTAINER_ADD_CHILDREN_LIST_X(element_type)
#define TREENODE_CONTAINER_SIZE(element_type)               TREENODE_CONTAINER_SIZE_X(element_type)
#define TREENODE_CONTAINER_FRONT(element_type)              TREENODE_CONTAINER_FRONT_X(element_type)
#define TREENODE_CONTAINER_BEGIN(element_type)              TREENODE_CONTAINER_BEGIN_X(element_type)
#define TREENODE_CONTAINER_END(element_type)                TREENODE_CONTAINER_END_X(element_type)

#define TREENODE_CONTAINER_ITER_INCREMENT(element_type)     TREENODE_CONTAINER_ITER_INCREMENT_X(element_type)
#define TREENODE_CONTAINER_ITER_GET_DATA(element_type)      TREENODE_CONTAINER_ITER_GET_DATA_X(element_type)

/* =========================================
 *   X macro
 * ========================================= */
#define TREENODE_TYPE_X(element_type)                         treenode_for_##element_type
#define TREENODE_CONTAINER_TYPE_X(element_type)               CLIST_TYPE(_treenode_for_##element_type)
#define TREENODE_CONTAINER_CONCRETE_TYPE_X(element_type)      CLIST_CONCRETE_TYPE(_treenode_for_##element_type)
#define TREENODE_CONTAINER_ITER_TYPE_X(element_type)          CLIST_ITER_TYPE(_treenode_for_##element_type)

#define TREENODE_NEW_X(element_type)                          zocleContainerTreenodeFor##element_type##New
#define TREENODE_DELETE_X(element_type)                       zocleContainerTreenodeFor##element_type##Delete
#define TREENODE_GET_DATA_X(element_type)                     zocleContainerTreenodeFor##element_type##GetData
#define TREENODE_SET_DATA_X(element_type)                     zocleContainerTreenodeFor##element_type##SetData
#define TREENODE_GET_PARENT_X(element_type)                   zocleContainerTreenodeFor##element_type##GetParent
#define TREENODE_SET_PARENT_X(element_type)                   zocleContainerTreenodeFor##element_type##SetParent
#define TREENODE_GET_CHILDREN_X(element_type)                 zocleContainerTreenodeFor##element_type##GetChildren
#define TREENODE_ADD_CHILDREN_X(element_type)                 zocleContainerTreenodeFor##element_type##AddChildren
#define TREENODE_ADD_CHILDREN_LIST_X(element_type)            zocleContainerTreenodeFor##element_type##AddChildrenList
#define TREENODE_BUILD_RELATION_X(element_type)               zocleContainerTreenodeFor##element_type##BuildRelation
#define TREENODE_BUILD_RELATION_LIST_X(element_type)          zocleContainerTreenodeFor##element_type##BuildRelationList

#define TREENODE_CONTAINER_NEW_X(element_type)                CLIST_NEW(_treenode_for_##element_type)
#define TREENODE_CONTAINER_ADD_CHILDREN_X(element_type)       zocleContainerTreenodeContainerFor##element_type##AddChildren
#define TREENODE_CONTAINER_ADD_CHILDREN_LIST_X(element_type)  zocleContainerTreenodeContainerFor##element_type##AddChildrenList
#define TREENODE_CONTAINER_SIZE_X(element_type)               zocleContainerTreenodeContainerFor##element_type##Size
#define TREENODE_CONTAINER_FRONT_X(element_type)              CLIST_FRONT(_treenode_for_##element_type)
#define TREENODE_CONTAINER_BEGIN_X(element_type)              CLIST_BEGIN(_treenode_for_##element_type)
#define TREENODE_CONTAINER_END_X(element_type)                CLIST_END(_treenode_for_##element_type)

#define TREENODE_CONTAINER_ITER_INCREMENT_X(element_type)     CLIST_ITER_INCREMENT(_treenode_for_##element_type)
#define TREENODE_CONTAINER_ITER_GET_DATA_X(element_type)      CLIST_ITER_GET_DATA(_treenode_for_##element_type)

#endif
