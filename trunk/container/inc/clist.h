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
#ifndef ZOCLE_CONTAINER_CLIST_H_
#define ZOCLE_CONTAINER_CLIST_H_

#include <string.h>

#include <cl.h>
#include <osal/inc/osal.h>

#define DECLARE_CLIST_TYPE(element_type)        \
  DECLARE_CLIST_TYPE_X(element_type)

#define DECLARE_CLIST_TYPE_X(element_type)          \
  typedef struct _clist_element_for_##element_type  \
  *clist_element_for_##element_type;                \
  typedef struct _clist_element_for_##element_type  \
  *clist_iter_for_##element_type;                   \
  typedef struct _clist_for_##element_type          \
  *clist_for_##element_type;

#define DEFINE_CLIST_TYPE(element_type)         \
  DEFINE_CLIST_TYPE_X(element_type)

#define DEFINE_CLIST_TYPE_X(element_type)                               \
  struct _clist_element_for_##element_type {                            \
    element_type  data;                                                 \
    clist_element_for_##element_type next;                              \
    clist_element_for_##element_type prev;                              \
  };                                                                    \
                                                                        \
  struct _clist_for_##element_type {                                    \
    size_t size_of_element;                                             \
    size_t element_count;                                               \
    clist_element_for_##element_type begin;                             \
    clist_element_for_##element_type end;                               \
  };                                                                    \
                                                                        \
  static inline size_t                                                  \
  zocleContainerCListFor##element_type##Size(clist_for_##element_type clist) { \
    ASSERT(clist != NULL);                                              \
    return clist->element_count;                                        \
  }                                                                     \
                                                                        \
  static inline element_type *                                          \
  zocleContainerCListFor##element_type##Front(clist_for_##element_type clist) { \
    ASSERT(clist != NULL);                                              \
    ASSERT(zocleContainerCListFor##element_type##Size(clist) != 0);     \
    return &(clist->begin->data);                                       \
  }                                                                     \
                                                                        \
  static inline element_type *                                          \
  zocleContainerCListFor##element_type##Back(clist_for_##element_type clist) { \
    ASSERT(clist != NULL);                                              \
    ASSERT(zocleContainerCListFor##element_type##Size(clist) != 0);     \
    return &(clist->end->data);                                         \
  }                                                                     \
                                                                        \
  static inline clist_iter_for_##element_type                           \
  zocleContainerCListFor##element_type##Begin(clist_for_##element_type clist) { \
    ASSERT(clist != NULL);                                              \
    if (0 == zocleContainerCListFor##element_type##Size(clist)) {       \
      ASSERT(NULL == clist->begin);                                     \
      ASSERT(NULL == clist->end);                                       \
    }                                                                   \
    return clist->begin;                                                \
  }                                                                     \
                                                                        \
  static inline clist_iter_for_##element_type                           \
  zocleContainerCListFor##element_type##End(clist_for_##element_type clist) { \
    return NULL;                                                        \
  }                                                                     \
                                                                        \
  static inline clist_iter_for_##element_type                           \
  zocleContainerCListFor##element_type##RBegin(clist_for_##element_type clist) { \
    ASSERT(clist != NULL);                                              \
    if (0 == zocleContainerCListFor##element_type##Size(clist)) {       \
      ASSERT(NULL == clist->begin);                                     \
      ASSERT(NULL == clist->end);                                       \
    }                                                                   \
    return clist->end;                                                  \
  }                                                                     \
                                                                        \
  static inline clist_iter_for_##element_type                           \
  zocleContainerCListFor##element_type##REnd(clist_for_##element_type clist) { \
    return NULL;                                                        \
  }                                                                     \
                                                                        \
  static clist_iter_for_##element_type                                  \
  zocleContainerCListFor##element_type##Insert(                         \
      clist_for_##element_type clist,                                   \
      clist_iter_for_##element_type iter,                               \
      element_type * const data) {                                      \
    ASSERT(clist != NULL);                                              \
    ASSERT(data != NULL);                                               \
    if (0 == zocleContainerCListFor##element_type##Size(clist)) {       \
      ASSERT(NULL == iter);                                             \
    } else {                                                            \
      ASSERT(iter != NULL);                                             \
    }                                                                   \
    {                                                                   \
      clist_element_for_##element_type e =                              \
        CL_OSAL_CALLOC(sizeof(struct _clist_element_for_##element_type)); \
      ASSERT(e != NULL);                                                \
      memcpy(&(e->data), data, sizeof(element_type));                   \
      if (NULL == iter) {                                               \
        e->prev = NULL;                                                 \
        e->next = NULL;                                                 \
        clist->begin = e;                                               \
        clist->end = e;                                                 \
      } else {                                                          \
        e->next = iter;                                                 \
        e->prev = iter->prev;                                           \
        iter->prev = e;                                                 \
        if (NULL == e->prev) {                                          \
          clist->begin = e;                                             \
        }                                                               \
      }                                                                 \
      ++clist->element_count;                                           \
      return e;                                                         \
    }                                                                   \
  }                                                                     \
                                                                        \
  static clist_iter_for_##element_type                                  \
  zocleContainerCListFor##element_type##Erase(                          \
      clist_for_##element_type clist,                                   \
      clist_iter_for_##element_type iter) {                             \
    ASSERT(clist != NULL);                                              \
    ASSERT(iter != NULL);                                               \
                                                                        \
    {                                                                   \
      clist_iter_for_##element_type follower = iter->next;              \
      if (iter->prev != NULL) {                                         \
        iter->prev->next = iter->next;                                  \
        if (clist->end == iter) {                                       \
          clist->end = iter->prev;                                      \
        }                                                               \
      } else {                                                          \
        if (clist->begin == clist->end) {                               \
          clist->begin = NULL;                                          \
          clist->end = NULL;                                            \
        } else {                                                        \
          clist->begin = iter->next;                                    \
        }                                                               \
      }                                                                 \
      if (iter->next != NULL) {                                         \
        iter->next->prev = iter->prev;                                  \
      }                                                                 \
      --clist->element_count;                                           \
      if (clist->element_count != 0) {                                  \
        ASSERT((clist->begin != NULL) && (clist->end != NULL));         \
      }                                                                 \
      CL_OSAL_FREE(iter);                                               \
      return follower;                                                  \
    }                                                                   \
  }                                                                     \
                                                                        \
  static element_type *                                                 \
  zocleContainerCListFor##element_type##At(clist_for_##element_type clist, \
                                           size_t const idx) {          \
    ASSERT(clist != NULL);                                              \
                                                                        \
    {                                                                   \
      size_t const size = zocleContainerCListFor##element_type##Size(clist); \
      size_t const half_size = size / 2;                                \
      ASSERT(size > idx);                                               \
      if (idx < half_size) {                                            \
        if (0 == idx) {                                                 \
          return &(clist->begin->data);                                 \
        } else {                                                        \
          size_t i;                                                     \
          clist_element_for_##element_type e = clist->begin;            \
          for (i = 0; i < idx; ++i) {                                   \
            e = e->next;                                                \
          }                                                             \
          return &(e->data);                                            \
        }                                                               \
      } else {                                                          \
        if (zocleContainerCListFor##element_type##Size(clist) == (idx + 1)) { \
          return &(clist->end->data);                                   \
        } else {                                                        \
          size_t distance_to_the_end = (size - 1 - idx);                \
          size_t i;                                                     \
          clist_element_for_##element_type e = clist->end;              \
          for (i = 0; i < distance_to_the_end; ++i) {                   \
            e = e->prev;                                                \
          }                                                             \
          return &(e->data);                                            \
        }                                                               \
      }                                                                 \
    }                                                                   \
  }                                                                     \
                                                                        \
  static void                                                           \
  zocleContainerCListFor##element_type##Splice(clist_for_##element_type target_clist, \
                                               clist_iter_for_##element_type iter, \
                                               clist_for_##element_type source_clist) { \
    ASSERT(target_clist != NULL);                                       \
    ASSERT(source_clist != NULL);                                       \
                                                                        \
    if (target_clist->element_count != 0) {                             \
      ASSERT(target_clist->begin != NULL);                              \
      ASSERT(target_clist->end != NULL);                                \
    }                                                                   \
    if (source_clist->element_count != 0) {                             \
      ASSERT(source_clist->begin != NULL);                              \
      ASSERT(source_clist->end != NULL);                                \
    }                                                                   \
                                                                        \
    if (NULL == iter) {                                                 \
      if (0 == zocleContainerCListFor##element_type##Size(target_clist)) { \
        target_clist->begin = source_clist->begin;                      \
        target_clist->end = source_clist->end;                          \
        target_clist->element_count = source_clist->element_count;      \
      } else {                                                          \
        if (source_clist->element_count != 0) {                         \
          target_clist->end->next = source_clist->begin;                \
          if (source_clist->begin != NULL) {                            \
            source_clist->begin->prev = target_clist->end;              \
          }                                                             \
          target_clist->end = source_clist->end;                        \
          target_clist->element_count += source_clist->element_count;   \
        }                                                               \
      }                                                                 \
    } else {                                                            \
      clist_element_for_##element_type e = (clist_element_for_##element_type)iter; \
      clist_element_for_##element_type e_prev = e->prev;                \
      ASSERT(target_clist->element_count != 0);                         \
                                                                        \
      if (source_clist->element_count != 0) {                           \
        e->prev = source_clist->end;                                    \
        if (source_clist->end != NULL) {                                \
          source_clist->end->next = e;                                  \
        }                                                               \
        if (e_prev != NULL) {                                           \
          e_prev->next = source_clist->begin;                           \
        }                                                               \
        if (source_clist->begin != NULL) {                              \
          source_clist->begin->prev = e_prev;                           \
        }                                                               \
        if (NULL == e_prev) {                                           \
          target_clist->begin = source_clist->begin;                    \
        }                                                               \
                                                                        \
        target_clist->element_count += source_clist->element_count;     \
      }                                                                 \
    }                                                                   \
                                                                        \
    if (target_clist->element_count != 0) {                             \
      ASSERT(target_clist->begin != NULL);                              \
      ASSERT(target_clist->end != NULL);                                \
    }                                                                   \
    source_clist->element_count = 0;                                    \
    source_clist->begin = NULL;                                         \
    source_clist->end = NULL;                                           \
  }                                                                     \
                                                                        \
  static inline clist_iter_for_##element_type                           \
  zocleContainerCListIterFor##element_type##Increment(                  \
      clist_iter_for_##element_type iter) {                             \
    ASSERT(iter != NULL);                                               \
    return iter->next;                                                  \
  }                                                                     \
                                                                        \
  /* Pass an extra clist parameter to avoid to create an extra end_of_list \
   * element.                                                           \
   * the "End" function will return that end_of_list element and        \
   * the "Decrement" function will use that end_of_list function.       \
   */                                                                   \
  static inline clist_iter_for_##element_type                           \
  zocleContainerCListIterFor##element_type##Decrement(                  \
      clist_iter_for_##element_type iter,                               \
      clist_for_##element_type clist) {                                 \
    ASSERT(clist != NULL);                                              \
    if (iter != NULL) {                                                 \
      return iter->prev;                                                \
    } else {                                                            \
      return clist->end;                                                \
    }                                                                   \
  }                                                                     \
                                                                        \
  static inline element_type *                                          \
  zocleContainerCListIterFor##element_type##GetData(                    \
      clist_iter_for_##element_type iter) {                             \
    return &(iter->data);                                               \
  }                                                                     \
                                                                        \
  static clist_for_##element_type                                       \
  zocleContainerCListFor##element_type##New(void) {                     \
    clist_for_##element_type clist =                                    \
      CL_OSAL_CALLOC(sizeof(struct _clist_for_##element_type));         \
    if (NULL == clist) {                                                \
      return NULL;                                                      \
    }                                                                   \
    clist->element_count = 0;                                           \
    clist->begin = NULL;                                                \
    clist->end = NULL;                                                  \
    return clist;                                                       \
  }                                                                     \
                                                                        \
  static void                                                           \
  zocleContainerCListFor##element_type##Clear(                          \
      clist_for_##element_type clist) {                                 \
    ASSERT(clist != NULL);                                              \
    {                                                                   \
      clist_element_for_##element_type e = clist->begin;                \
      clist_element_for_##element_type next_e;                          \
      while (e != NULL) {                                               \
        next_e = e->next;                                               \
        CL_OSAL_FREE(e);                                                \
        e = next_e;                                                     \
      }                                                                 \
      clist->element_count = 0;                                         \
    }                                                                   \
  }                                                                     \
                                                                        \
  static void                                                           \
  zocleContainerCListFor##element_type##Delete(                         \
      clist_for_##element_type clist) {                                 \
    zocleContainerCListFor##element_type##Clear(clist);                 \
    CL_OSAL_FREE(clist);                                                \
  }                                                                     \
                                                                        \
  static cl_int                                                         \
  zocleContainerCListFor##element_type##PushFront(clist_for_##element_type clist, \
                                                  element_type * const data) { \
    ASSERT(clist != NULL);                                              \
    ASSERT(data != NULL);                                               \
    {                                                                   \
      clist_element_for_##element_type e =                              \
        CL_OSAL_CALLOC(sizeof(struct _clist_element_for_##element_type)); \
      ASSERT(e != NULL);                                                \
      memcpy(&(e->data), data, sizeof(element_type));                   \
      if (NULL == clist->begin) {                                       \
        clist->begin = e;                                               \
        clist->end = e;                                                 \
      } else {                                                          \
        e->next = clist->begin;                                         \
        clist->begin->prev = e;                                         \
        clist->begin = e;                                               \
      }                                                                 \
      ++clist->element_count;                                           \
      return CL_SUCCESS;                                                \
    }                                                                   \
  }                                                                     \
                                                                        \
  static cl_int                                                         \
  zocleContainerCListFor##element_type##PushBack(clist_for_##element_type clist, \
                                                 element_type * const data) { \
    ASSERT(clist != NULL);                                              \
    ASSERT(data != NULL);                                               \
    {                                                                   \
      clist_element_for_##element_type e =                              \
        CL_OSAL_CALLOC(sizeof(struct _clist_element_for_##element_type)); \
      ASSERT(e != NULL);                                                \
      memcpy(&(e->data), data, sizeof(element_type));                   \
      if (NULL == clist->begin) {                                       \
        clist->begin = e;                                               \
        clist->end = e;                                                 \
      } else {                                                          \
        e->prev = clist->end;                                           \
        clist->end->next = e;                                           \
        clist->end = e;                                                 \
      }                                                                 \
      ++clist->element_count;                                           \
      return CL_SUCCESS;                                                \
    }                                                                   \
  }                                                                     \
                                                                        \
  static void                                                           \
  zocleContainerCListFor##element_type##Copy(                           \
      clist_for_##element_type dest,                                    \
      clist_for_##element_type src) {                                   \
    ASSERT(dest != NULL);                                               \
    ASSERT(src != NULL);                                                \
    if (0 == zocleContainerCListFor##element_type##Size(src)) {         \
      return;                                                           \
    }                                                                   \
    {                                                                   \
      clist_iter_for_##element_type iter;                               \
      for (iter = zocleContainerCListFor##element_type##Begin(src);     \
           iter != zocleContainerCListFor##element_type##End(src);      \
           iter = zocleContainerCListIterFor##element_type##Increment(iter)) { \
        element_type e;                                                 \
                                                                        \
        memcpy(&e,                                                       \
               zocleContainerCListIterFor##element_type##GetData(iter), \
               sizeof(element_type));                                   \
                                                                        \
        (void)zocleContainerCListFor##element_type##PushBack(dest, &e); \
      }                                                                 \
    }                                                                   \
  }                                                                     \
                                                                        \
  static void                                                           \
  zocleContainerCListFor##element_type##PopFront(clist_for_##element_type clist) { \
    ASSERT(clist != NULL);                                              \
    ASSERT(zocleContainerCListFor##element_type##Size(clist) != 0);     \
    {                                                                   \
      clist_element_for_##element_type e = clist->begin;                \
      clist->begin = e->next;                                           \
      if (clist->begin != NULL) {                                       \
        clist->begin->prev = NULL;                                      \
      } else {                                                          \
        clist->end = NULL;                                              \
      }                                                                 \
      CL_OSAL_FREE(e);                                                  \
      --clist->element_count;                                           \
    }                                                                   \
  }                                                                     \
                                                                        \
  static void                                                           \
  zocleContainerCListFor##element_type##PopBack(                        \
      clist_for_##element_type clist) {                                 \
    ASSERT(clist != NULL);                                              \
    ASSERT(zocleContainerCListFor##element_type##Size(clist) != 0);     \
    {                                                                   \
      clist_element_for_##element_type e = clist->end;                  \
      clist->end = e->prev;                                             \
      clist->end->next = NULL;                                          \
      CL_OSAL_FREE(e);                                                  \
      --clist->element_count;                                           \
    }                                                                   \
  }                                                                     \
                                                                        \
  typedef cl_bool (*clist_for_##element_type##_search_compare_func_t)(element_type *, void *); \
  static clist_iter_for_##element_type                                  \
  zocleContainerCListFor##element_type##Search(                         \
      clist_for_##element_type clist,                                   \
      void *data,                                                       \
      clist_for_##element_type##_search_compare_func_t compare_func) {  \
    ASSERT(clist != NULL);                                              \
    ASSERT(data != NULL);                                               \
    {                                                                   \
      clist_iter_for_##element_type e;                                  \
      for (e = zocleContainerCListFor##element_type##Begin(clist);      \
           e != zocleContainerCListFor##element_type##End(clist);       \
           e = zocleContainerCListIterFor##element_type##Increment(e)) { \
        element_type *element = zocleContainerCListIterFor##element_type##GetData(e); \
        if (CL_TRUE == compare_func(element, data)) {                   \
          return e;                                                     \
        }                                                               \
      }                                                                 \
      return NULL;                                                      \
    }                                                                   \
  }                                                                     \
                                                                        \
  static cl_bool                                                        \
  zocleContainerCListFor##element_type##PushBackUniqueCompareFunc(      \
      element_type *element_in_list,                                    \
      element_type *data) {                                             \
    if (0 == memcmp(element_in_list, data, sizeof(*element_in_list))) { \
      return CL_TRUE;                                                   \
    } else {                                                            \
      return CL_FALSE;                                                  \
    }                                                                   \
  }                                                                     \
                                                                        \
  static cl_int                                                         \
  zocleContainerCListFor##element_type##PushBackUnique(clist_for_##element_type clist, \
                                                       element_type * const data) { \
    clist_iter_for_##element_type iter =                                \
      zocleContainerCListFor##element_type##Search(                     \
          clist, data,                                                  \
          zocleContainerCListFor##element_type##PushBackUniqueCompareFunc); \
    if (iter == zocleContainerCListFor##element_type##End(clist)) {     \
      return zocleContainerCListFor##element_type##PushBack(clist, data); \
    }                                                                   \
    return CL_SUCCESS;                                                  \
  }

/* ======================================================
 * list
 * ====================================================== */
#define CLIST_TYPE(element_type)           CLIST_TYPE_X(element_type)
#define CLIST_ITER_TYPE(element_type)      CLIST_ITER_TYPE_X(element_type)

#define CLIST_NEW(element_type)        CLIST_NEW_X(element_type)
#define CLIST_DELETE(element_type)     CLIST_DELETE_X(element_type)
#define CLIST_PUSH_BACK(element_type)  CLIST_PUSH_BACK_X(element_type)
#define CLIST_PUSH_BACK_UNIQUE(element_type)  CLIST_PUSH_BACK_UNIQUE_X(element_type)
#define CLIST_PUSH_FRONT(element_type) CLIST_PUSH_FRONT_X(element_type)
#define CLIST_POP_FRONT(element_type)  CLIST_POP_FRONT_X(element_type)
#define CLIST_POP_BACK(element_type)   CLIST_POP_BACK_X(element_type)
#define CLIST_SIZE(element_type)       CLIST_SIZE_X(element_type)
#define CLIST_AT(element_type)         CLIST_AT_X(element_type)
#define CLIST_FRONT(element_type)      CLIST_FRONT_X(element_type)
#define CLIST_BACK(element_type)       CLIST_BACK_X(element_type)
#define CLIST_BEGIN(element_type)      CLIST_BEGIN_X(element_type)
#define CLIST_END(element_type)        CLIST_END_X(element_type)
#define CLIST_RBEGIN(element_type)     CLIST_RBEGIN_X(element_type)
#define CLIST_REND(element_type)       CLIST_REND_X(element_type)
#define CLIST_ERASE(element_type)      CLIST_ERASE_X(element_type)
#define CLIST_CLEAR(element_type)      CLIST_CLEAR_X(element_type)
#define CLIST_INSERT(element_type)     CLIST_INSERT_X(element_type)
#define CLIST_COPY(element_type)       CLIST_COPY_X(element_type)
#define CLIST_SEARCH(element_type)     CLIST_SEARCH_X(element_type)
#define CLIST_SPLICE(element_type)     CLIST_SPLICE_X(element_type)

#define CLIST_ITER_INCREMENT(element_type)      \
  CLIST_ITER_INCREMENT_X(element_type)
#define CLIST_ITER_DECREMENT(element_type)      \
  CLIST_ITER_DECREMENT_X(element_type)
#define CLIST_ITER_GET_DATA(element_type)       \
  CLIST_ITER_GET_DATA_X(element_type)

#define CLIST_CONCRETE_TYPE(element_type)    CLIST_CONCRETE_TYPE_X(element_type)

/* The following macro is for internal use, it should not be called by the
 * user. The purpose of this macro is to enable the preprocessor prescan
 * mechanism in token concatenation (##).
 *
 * So that we can use CLIST_TYPE(CLIST_TYPE(x)) */
#define CLIST_TYPE_X(element_type)           clist_for_##element_type
#define CLIST_ITER_TYPE_X(element_type)      clist_iter_for_##element_type

#define CLIST_NEW_X(element_type)        zocleContainerCListFor##element_type##New
#define CLIST_DELETE_X(element_type)     zocleContainerCListFor##element_type##Delete
#define CLIST_PUSH_BACK_X(element_type)  zocleContainerCListFor##element_type##PushBack
#define CLIST_PUSH_BACK_UNIQUE_X(element_type)  zocleContainerCListFor##element_type##PushBackUnique
#define CLIST_PUSH_FRONT_X(element_type) zocleContainerCListFor##element_type##PushFront
#define CLIST_POP_FRONT_X(element_type)  zocleContainerCListFor##element_type##PopFront
#define CLIST_POP_BACK_X(element_type)   zocleContainerCListFor##element_type##PopBack
#define CLIST_SIZE_X(element_type)       zocleContainerCListFor##element_type##Size
#define CLIST_AT_X(element_type)         zocleContainerCListFor##element_type##At
#define CLIST_FRONT_X(element_type)      zocleContainerCListFor##element_type##Front
#define CLIST_BACK_X(element_type)       zocleContainerCListFor##element_type##Back
#define CLIST_BEGIN_X(element_type)      zocleContainerCListFor##element_type##Begin
#define CLIST_END_X(element_type)        zocleContainerCListFor##element_type##End
#define CLIST_RBEGIN_X(element_type)     zocleContainerCListFor##element_type##RBegin
#define CLIST_REND_X(element_type)       zocleContainerCListFor##element_type##REnd
#define CLIST_ERASE_X(element_type)      zocleContainerCListFor##element_type##Erase
#define CLIST_CLEAR_X(element_type)      zocleContainerCListFor##element_type##Clear
#define CLIST_INSERT_X(element_type)     zocleContainerCListFor##element_type##Insert
#define CLIST_COPY_X(element_type)       zocleContainerCListFor##element_type##Copy
#define CLIST_SEARCH_X(element_type)     zocleContainerCListFor##element_type##Search
#define CLIST_SPLICE_X(element_type)     zocleContainerCListFor##element_type##Splice

#define CLIST_ITER_INCREMENT_X(element_type)          \
  zocleContainerCListIterFor##element_type##Increment
#define CLIST_ITER_DECREMENT_X(element_type)          \
  zocleContainerCListIterFor##element_type##Decrement
#define CLIST_ITER_GET_DATA_X(element_type)         \
  zocleContainerCListIterFor##element_type##GetData

#define CLIST_CONCRETE_TYPE_X(element_type)    struct _clist_for_##element_type

#endif
