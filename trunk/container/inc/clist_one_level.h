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
#ifndef ZOCLE_CONTAINER_CLIST_ONE_LEVEL_H_
#define ZOCLE_CONTAINER_CLIST_ONE_LEVEL_H_

#include <string.h>

#include <cl.h>

/**
 * clist_one_level element needs to be of pointer type, and points to
 * a structure type containing the following fields.
 */
#define CLIST_ONE_LEVEL_ELEMENT_NEEDED_FIELDS(element_type, list_name)  \
  CLIST_ONE_LEVEL_ELEMENT_NEEDED_FIELDS_X(element_type, list_name)

#define CLIST_ONE_LEVEL_ELEMENT_NEEDED_FIELDS_X(element_type, list_name) \
  element_type point_to_current_##list_name##_element;                  \
  element_type list_name##_prev;                                        \
  element_type list_name##_next;

#define DECLARE_CLIST_ONE_LEVEL_TYPE(element_type, list_name) \
  DECLARE_CLIST_ONE_LEVEL_TYPE_X(element_type, list_name)

#define DECLARE_CLIST_ONE_LEVEL_TYPE_X(element_type, list_name) \
  typedef element_type clist_one_level_iter_for_##element_type; \
  typedef struct _clist_one_level_for_##element_type            \
  *clist_one_level_for_##element_type;

#define DEFINE_CLIST_ONE_LEVEL_TYPE(element_type, list_name)  \
  DEFINE_CLIST_ONE_LEVEL_TYPE_X(element_type, list_name)

#define DEFINE_CLIST_ONE_LEVEL_TYPE_X(element_type, list_name)          \
  struct _clist_one_level_for_##element_type {                          \
    size_t size_of_element;                                             \
    size_t element_count;                                               \
    element_type begin;                                                 \
    element_type end;                                                   \
  };                                                                    \
                                                                        \
  static inline void                                                    \
  zocleContainerCListOneLevelElementFor##element_type##InitNeededFields( \
      element_type element) {                                           \
    ASSERT(element != NULL);                                            \
    element->point_to_current_##list_name##_element = element;          \
    element->list_name##_prev = NULL;                                   \
    element->list_name##_next = NULL;                                   \
  }                                                                     \
                                                                        \
  static inline size_t                                                  \
  zocleContainerCListOneLevelFor##element_type##Size(clist_one_level_for_##element_type clist) { \
    ASSERT(clist != NULL);                                              \
    return clist->element_count;                                        \
  }                                                                     \
                                                                        \
  static inline element_type *                                          \
  zocleContainerCListOneLevelFor##element_type##Front(clist_one_level_for_##element_type clist) { \
    ASSERT(clist != NULL);                                              \
    ASSERT(zocleContainerCListOneLevelFor##element_type##Size(clist) != 0); \
    return &(clist->begin);                                             \
  }                                                                     \
                                                                        \
  static inline element_type *                                          \
  zocleContainerCListOneLevelFor##element_type##Back(clist_one_level_for_##element_type clist) { \
    ASSERT(clist != NULL);                                              \
    ASSERT(zocleContainerCListOneLevelFor##element_type##Size(clist) != 0); \
    return &(clist->end);                                               \
  }                                                                     \
                                                                        \
  static inline clist_one_level_iter_for_##element_type                 \
  zocleContainerCListOneLevelFor##element_type##Begin(clist_one_level_for_##element_type clist) { \
    ASSERT(clist != NULL);                                              \
    if (0 == zocleContainerCListOneLevelFor##element_type##Size(clist)) { \
      ASSERT(NULL == clist->begin);                                     \
      ASSERT(NULL == clist->end);                                       \
    }                                                                   \
    return clist->begin;                                                \
  }                                                                     \
                                                                        \
  static inline clist_one_level_iter_for_##element_type                 \
  zocleContainerCListOneLevelFor##element_type##End(clist_one_level_for_##element_type clist) { \
    return NULL;                                                        \
  }                                                                     \
                                                                        \
  static clist_one_level_iter_for_##element_type                        \
  zocleContainerCListOneLevelFor##element_type##Insert(                 \
      clist_one_level_for_##element_type clist,                         \
      clist_one_level_iter_for_##element_type iter,                     \
      element_type * const data) {                                      \
    ASSERT(clist != NULL);                                              \
    ASSERT(data != NULL);                                               \
                                                                        \
    zocleContainerCListOneLevelElementFor##element_type##InitNeededFields(*data); \
                                                                        \
    if (0 == zocleContainerCListOneLevelFor##element_type##Size(clist)) { \
      ASSERT(NULL == iter);                                             \
    } else {                                                            \
      ASSERT(iter != NULL);                                             \
    }                                                                   \
    {                                                                   \
      if (NULL == iter) {                                               \
        (*data)->list_name##_prev = NULL;                               \
        (*data)->list_name##_next = NULL;                               \
        clist->begin = (*data);                                         \
        clist->end = (*data);                                           \
      } else {                                                          \
        (*data)->list_name##_next = iter;                               \
        (*data)->list_name##_prev = iter->list_name##_prev;             \
        iter->list_name##_prev = (*data);                               \
        if (NULL == (*data)->list_name##_prev) {                        \
          clist->begin = (*data);                                       \
        }                                                               \
      }                                                                 \
      ++clist->element_count;                                           \
      return (*data);                                                   \
    }                                                                   \
  }                                                                     \
                                                                        \
  static clist_one_level_iter_for_##element_type                        \
  zocleContainerCListOneLevelFor##element_type##Erase(                  \
      clist_one_level_for_##element_type clist,                         \
      clist_one_level_iter_for_##element_type iter) {                   \
    ASSERT(clist != NULL);                                              \
    ASSERT(iter != NULL);                                               \
                                                                        \
    {                                                                   \
      clist_one_level_iter_for_##element_type follower = iter->list_name##_next; \
      if (iter->list_name##_prev != NULL) {                             \
        iter->list_name##_prev->list_name##_next = iter->list_name##_next; \
        if (clist->end == iter) {                                       \
          clist->end = iter->list_name##_prev;                          \
        }                                                               \
      } else {                                                          \
        if (clist->begin == clist->end) {                               \
          clist->begin = NULL;                                          \
          clist->end = NULL;                                            \
        } else {                                                        \
          clist->begin = iter->list_name##_next;                        \
        }                                                               \
      }                                                                 \
      if (iter->list_name##_next != NULL) {                             \
        iter->list_name##_next->list_name##_prev = iter->list_name##_prev; \
      }                                                                 \
      --clist->element_count;                                           \
      return follower;                                                  \
    }                                                                   \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  zocleContainerCListOneLevelFor##element_type##Remove(                 \
      clist_one_level_for_##element_type clist,                         \
      element_type * const e) {                                         \
    (void)zocleContainerCListOneLevelFor##element_type##Erase(clist, *e); \
  }                                                                     \
                                                                        \
  static element_type *                                                 \
  zocleContainerCListOneLevelFor##element_type##At(clist_one_level_for_##element_type clist, \
                                                   size_t const idx) {  \
    ASSERT(clist != NULL);                                              \
                                                                        \
    {                                                                   \
      size_t const size = zocleContainerCListOneLevelFor##element_type##Size(clist); \
      size_t const half_size = size / 2;                                \
      ASSERT(size > idx);                                               \
      if (idx < half_size) {                                            \
        if (0 == idx) {                                                 \
          return &(clist->begin);                                       \
        } else {                                                        \
          size_t i;                                                     \
          element_type e = clist->begin;                                \
          for (i = 0; i < idx; ++i) {                                   \
            e = e->list_name##_next;                                    \
          }                                                             \
          return &(e->point_to_current_##list_name##_element);          \
        }                                                               \
      } else {                                                          \
        if (zocleContainerCListOneLevelFor##element_type##Size(clist) == (idx + 1)) { \
          return &(clist->end);                                         \
        } else {                                                        \
          size_t distance_to_the_end = (size - 1 - idx);                \
          size_t i;                                                     \
          element_type e = clist->end;                                  \
          for (i = 0; i < distance_to_the_end; ++i) {                   \
            e = e->list_name##_prev;                                    \
          }                                                             \
          return &(e->point_to_current_##list_name##_element);          \
        }                                                               \
      }                                                                 \
    }                                                                   \
  }                                                                     \
                                                                        \
  static void                                                           \
  zocleContainerCListOneLevelFor##element_type##Splice(clist_one_level_for_##element_type target_clist, \
                                                       clist_one_level_iter_for_##element_type iter, \
                                                       clist_one_level_for_##element_type source_clist) { \
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
      if (0 == zocleContainerCListOneLevelFor##element_type##Size(target_clist)) { \
        target_clist->begin = source_clist->begin;                      \
        target_clist->end = source_clist->end;                          \
        target_clist->element_count = source_clist->element_count;      \
      } else {                                                          \
        if (source_clist->element_count != 0) {                         \
          target_clist->end->list_name##_next = source_clist->begin;    \
          if (source_clist->begin != NULL) {                            \
            source_clist->begin->list_name##_prev = target_clist->end;  \
          }                                                             \
          target_clist->end = source_clist->end;                        \
          target_clist->element_count += source_clist->element_count;   \
        }                                                               \
      }                                                                 \
    } else {                                                            \
      element_type e = iter;                                            \
      element_type e_prev = e->list_name##_prev;                        \
      ASSERT(target_clist->element_count != 0);                         \
                                                                        \
      if (source_clist->element_count != 0) {                           \
        e->list_name##_prev = source_clist->end;                        \
        if (source_clist->end != NULL) {                                \
          source_clist->end->list_name##_next = e;                      \
        }                                                               \
        if (e_prev != NULL) {                                           \
          e_prev->list_name##_next = source_clist->begin;               \
        }                                                               \
        if (source_clist->begin != NULL) {                              \
          source_clist->begin->list_name##_prev = e_prev;               \
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
  static inline clist_one_level_iter_for_##element_type                 \
  zocleContainerCListOneLevelIterFor##element_type##Increment(          \
      clist_one_level_iter_for_##element_type iter) {                   \
    ASSERT(iter != NULL);                                               \
    return iter->list_name##_next;                                      \
  }                                                                     \
                                                                        \
  static inline clist_one_level_iter_for_##element_type                 \
  zocleContainerCListOneLevelIterFor##element_type##Decrement(          \
      clist_one_level_iter_for_##element_type iter,                     \
      clist_one_level_for_##element_type clist) {                       \
    ASSERT(clist != NULL);                                              \
    if (iter != NULL) {                                                 \
      return iter->list_name##_prev;                                    \
    } else {                                                            \
      return clist->end;                                                \
    }                                                                   \
  }                                                                     \
                                                                        \
  static inline element_type *                                          \
  zocleContainerCListOneLevelIterFor##element_type##GetData(            \
      clist_one_level_iter_for_##element_type iter) {                   \
    return &(iter->point_to_current_##list_name##_element);             \
  }                                                                     \
                                                                        \
  static clist_one_level_for_##element_type                             \
  zocleContainerCListOneLevelFor##element_type##New(void) {             \
    clist_one_level_for_##element_type clist =                          \
      CL_OSAL_CALLOC(sizeof(struct _clist_one_level_for_##element_type)); \
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
  zocleContainerCListOneLevelFor##element_type##Clear(                  \
      clist_one_level_for_##element_type clist) {                       \
    /* do not need to do anything */                                    \
  }                                                                     \
                                                                        \
  static void                                                           \
  zocleContainerCListOneLevelFor##element_type##Delete(                 \
      clist_one_level_for_##element_type clist) {                       \
    zocleContainerCListOneLevelFor##element_type##Clear(clist);         \
    CL_OSAL_FREE(clist);                                                \
  }                                                                     \
                                                                        \
  static cl_int                                                         \
  zocleContainerCListOneLevelFor##element_type##PushBack(clist_one_level_for_##element_type clist, \
                                                         element_type * const data) { \
    ASSERT(clist != NULL);                                              \
    ASSERT(data != NULL);                                               \
                                                                        \
    zocleContainerCListOneLevelElementFor##element_type##InitNeededFields(*data); \
                                                                        \
    if (NULL == clist->begin) {                                         \
      clist->begin = (*data);                                           \
      clist->end = (*data);                                             \
    } else {                                                            \
      (*data)->list_name##_prev = clist->end;                           \
      clist->end->list_name##_next = (*data);                           \
      clist->end = (*data);                                             \
    }                                                                   \
    ++clist->element_count;                                             \
    return CL_SUCCESS;                                                  \
  }                                                                     \
                                                                        \
  static void                                                           \
  zocleContainerCListOneLevelFor##element_type##PopFront(clist_one_level_for_##element_type clist) { \
    ASSERT(clist != NULL);                                              \
    ASSERT(zocleContainerCListOneLevelFor##element_type##Size(clist) != 0); \
    {                                                                   \
      element_type e = clist->begin;                                    \
      clist->begin = e->list_name##_next;                               \
      if (clist->begin != NULL) {                                       \
        clist->begin->list_name##_prev = NULL;                          \
      } else {                                                          \
        clist->end = NULL;                                              \
      }                                                                 \
      --clist->element_count;                                           \
    }                                                                   \
  }                                                                     \
                                                                        \
  static void                                                           \
  zocleContainerCListOneLevelFor##element_type##PopBack(                \
      clist_one_level_for_##element_type clist) {                       \
    ASSERT(clist != NULL);                                              \
    ASSERT(zocleContainerCListOneLevelFor##element_type##Size(clist) != 0); \
    {                                                                   \
      element_type e = clist->end;                                      \
      clist->end = e->list_name##_prev;                                 \
      clist->end->list_name##_next = NULL;                              \
      --clist->element_count;                                           \
    }                                                                   \
  }                                                                     \
                                                                        \
  typedef cl_bool (*clist_one_level_for_##element_type##_search_compare_func_t)(element_type *, void *); \
  static clist_one_level_iter_for_##element_type                        \
  zocleContainerCListOneLevelFor##element_type##Search(                 \
      clist_one_level_for_##element_type clist,                         \
      void *data,                                                       \
      clist_one_level_for_##element_type##_search_compare_func_t compare_func) { \
    ASSERT(clist != NULL);                                              \
    ASSERT(data != NULL);                                               \
    {                                                                   \
      clist_one_level_iter_for_##element_type e;                        \
      for (e = zocleContainerCListOneLevelFor##element_type##Begin(clist); \
           e != zocleContainerCListOneLevelFor##element_type##End(clist); \
           e = zocleContainerCListOneLevelIterFor##element_type##Increment(e)) { \
        element_type *element = zocleContainerCListOneLevelIterFor##element_type##GetData(e); \
        if (CL_TRUE == compare_func(element, data)) {                   \
          return e;                                                     \
        }                                                               \
      }                                                                 \
      return NULL;                                                      \
    }                                                                   \
  }

/* ======================================================
 * list
 * ====================================================== */
#define CLIST_ONE_LEVEL_TYPE(element_type, list_name)           CLIST_ONE_LEVEL_TYPE_X(element_type, list_name)
#define CLIST_ONE_LEVEL_ITER_TYPE(element_type, list_name)      CLIST_ONE_LEVEL_ITER_TYPE_X(element_type, list_name)

#define CLIST_ONE_LEVEL_NEW(element_type, list_name)        CLIST_ONE_LEVEL_NEW_X(element_type, list_name)
#define CLIST_ONE_LEVEL_DELETE(element_type, list_name)     CLIST_ONE_LEVEL_DELETE_X(element_type, list_name)
#define CLIST_ONE_LEVEL_PUSH_BACK(element_type, list_name)  CLIST_ONE_LEVEL_PUSH_BACK_X(element_type, list_name)
#define CLIST_ONE_LEVEL_POP_FRONT(element_type, list_name)  CLIST_ONE_LEVEL_POP_FRONT_X(element_type, list_name)
#define CLIST_ONE_LEVEL_POP_BACK(element_type, list_name)   CLIST_ONE_LEVEL_POP_BACK_X(element_type, list_name)
#define CLIST_ONE_LEVEL_SIZE(element_type, list_name)       CLIST_ONE_LEVEL_SIZE_X(element_type, list_name)
#define CLIST_ONE_LEVEL_AT(element_type, list_name)         CLIST_ONE_LEVEL_AT_X(element_type, list_name)
#define CLIST_ONE_LEVEL_FRONT(element_type, list_name)      CLIST_ONE_LEVEL_FRONT_X(element_type, list_name)
#define CLIST_ONE_LEVEL_BACK(element_type, list_name)       CLIST_ONE_LEVEL_BACK_X(element_type, list_name)
#define CLIST_ONE_LEVEL_BEGIN(element_type, list_name)      CLIST_ONE_LEVEL_BEGIN_X(element_type, list_name)
#define CLIST_ONE_LEVEL_END(element_type, list_name)        CLIST_ONE_LEVEL_END_X(element_type, list_name)
#define CLIST_ONE_LEVEL_ERASE(element_type, list_name)      CLIST_ONE_LEVEL_ERASE_X(element_type, list_name)
#define CLIST_ONE_LEVEL_CLEAR(element_type, list_name)      CLIST_ONE_LEVEL_CLEAR_X(element_type, list_name)
#define CLIST_ONE_LEVEL_REMOVE(element_type, list_name)     CLIST_ONE_LEVEL_REMOVE_X(element_type, list_name)
#define CLIST_ONE_LEVEL_INSERT(element_type, list_name)     CLIST_ONE_LEVEL_INSERT_X(element_type, list_name)
#define CLIST_ONE_LEVEL_SEARCH(element_type, list_name)     CLIST_ONE_LEVEL_SEARCH_X(element_type, list_name)
#define CLIST_ONE_LEVEL_SPLICE(element_type, list_name)     CLIST_ONE_LEVEL_SPLICE_X(element_type, list_name)

#define CLIST_ONE_LEVEL_ITER_INCREMENT(element_type, list_name) \
  CLIST_ONE_LEVEL_ITER_INCREMENT_X(element_type, list_name)
#define CLIST_ONE_LEVEL_ITER_DECREMENT(element_type, list_name) \
  CLIST_ONE_LEVEL_ITER_DECREMENT_X(element_type, list_name)
#define CLIST_ONE_LEVEL_ITER_GET_DATA(element_type, list_name)  \
  CLIST_ONE_LEVEL_ITER_GET_DATA_X(element_type, list_name)

#define CLIST_ONE_LEVEL_CONCRETE_TYPE(element_type, list_name)    CLIST_ONE_LEVEL_CONCRETE_TYPE_X(element_type, list_name)

/* The following macro is for internal use, it should not be called by the
 * user. The purpose of this macro is to enable the preprocessor prescan
 * mechanism in token concatenation (##).
 *
 * So that we can use CLIST_ONE_LEVEL_TYPE(CLIST_ONE_LEVEL_TYPE(x)) */
#define CLIST_ONE_LEVEL_TYPE_X(element_type, list_name)           clist_one_level_for_##element_type
#define CLIST_ONE_LEVEL_ITER_TYPE_X(element_type, list_name)      clist_one_level_iter_for_##element_type

#define CLIST_ONE_LEVEL_NEW_X(element_type, list_name)        zocleContainerCListOneLevelFor##element_type##New
#define CLIST_ONE_LEVEL_DELETE_X(element_type, list_name)     zocleContainerCListOneLevelFor##element_type##Delete
#define CLIST_ONE_LEVEL_PUSH_BACK_X(element_type, list_name)  zocleContainerCListOneLevelFor##element_type##PushBack
#define CLIST_ONE_LEVEL_POP_FRONT_X(element_type, list_name)  zocleContainerCListOneLevelFor##element_type##PopFront
#define CLIST_ONE_LEVEL_POP_BACK_X(element_type, list_name)   zocleContainerCListOneLevelFor##element_type##PopBack
#define CLIST_ONE_LEVEL_SIZE_X(element_type, list_name)       zocleContainerCListOneLevelFor##element_type##Size
#define CLIST_ONE_LEVEL_AT_X(element_type, list_name)         zocleContainerCListOneLevelFor##element_type##At
#define CLIST_ONE_LEVEL_FRONT_X(element_type, list_name)      zocleContainerCListOneLevelFor##element_type##Front
#define CLIST_ONE_LEVEL_BACK_X(element_type, list_name)       zocleContainerCListOneLevelFor##element_type##Back
#define CLIST_ONE_LEVEL_BEGIN_X(element_type, list_name)      zocleContainerCListOneLevelFor##element_type##Begin
#define CLIST_ONE_LEVEL_END_X(element_type, list_name)        zocleContainerCListOneLevelFor##element_type##End
#define CLIST_ONE_LEVEL_ERASE_X(element_type, list_name)      zocleContainerCListOneLevelFor##element_type##Erase
#define CLIST_ONE_LEVEL_CLEAR_X(element_type, list_name)      zocleContainerCListOneLevelFor##element_type##Clear
#define CLIST_ONE_LEVEL_REMOVE_X(element_type, list_name)     zocleContainerCListOneLevelFor##element_type##Remove
#define CLIST_ONE_LEVEL_INSERT_X(element_type, list_name)     zocleContainerCListOneLevelFor##element_type##Insert
#define CLIST_ONE_LEVEL_SEARCH_X(element_type, list_name)     zocleContainerCListOneLevelFor##element_type##Search
#define CLIST_ONE_LEVEL_SPLICE_X(element_type, list_name)     zocleContainerCListOneLevelFor##element_type##Splice

#define CLIST_ONE_LEVEL_ITER_INCREMENT_X(element_type, list_name) \
  zocleContainerCListOneLevelIterFor##element_type##Increment
#define CLIST_ONE_LEVEL_ITER_DECREMENT_X(element_type, list_name) \
  zocleContainerCListOneLevelIterFor##element_type##Decrement
#define CLIST_ONE_LEVEL_ITER_GET_DATA_X(element_type, list_name)  \
  zocleContainerCListOneLevelIterFor##element_type##GetData

#define CLIST_ONE_LEVEL_CONCRETE_TYPE_X(element_type, list_name)    struct _clist_one_level_for_##element_type

#endif
