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
#ifndef ZOCLE_CONTAINER_CVECTOR_H_
#define ZOCLE_CONTAINER_CVECTOR_H_

#include <string.h>

#include <cl.h>
#include <osal/inc/osal.h>

#define DECLARE_CVECTOR_TYPE(element_type)      \
  DECLARE_CVECTOR_TYPE_X(element_type)

#define DECLARE_CVECTOR_TYPE_X(element_type)                            \
  typedef struct _cvector_for_##element_type *cvector_for_##element_type; \
  typedef element_type *cvector_iter_for_##element_type;

#define DEFINE_CVECTOR_TYPE(element_type)       \
  DEFINE_CVECTOR_TYPE_X(element_type)

#define DEFINE_CVECTOR_TYPE_X(element_type)                             \
  struct _cvector_for_##element_type {                                  \
    element_type *begin;                                                \
    element_type *end;                                                  \
    element_type *capacity;                                             \
  };                                                                    \
                                                                        \
  static cl_int                                                         \
  zocleContainerCVectorFor##element_type##Reserve(cvector_for_##element_type cvector, \
                                                  size_t const capacity); \
                                                                        \
  static cvector_for_##element_type                                     \
  zocleContainerCVectorFor##element_type##New(size_t const init_capacity) { \
    cvector_for_##element_type cvector =                                \
      CL_OSAL_CALLOC(sizeof(struct _cvector_for_##element_type));       \
    if (NULL == cvector) {                                              \
      return NULL;                                                      \
    }                                                                   \
    if (0 == init_capacity) {                                           \
      cvector->begin = NULL;                                            \
      cvector->end = NULL;                                              \
      cvector->capacity = NULL;                                         \
    } else {                                                            \
      cl_int const result =                                             \
        zocleContainerCVectorFor##element_type##Reserve(                \
            cvector, init_capacity);                                    \
      if (result != CL_SUCCESS) {                                       \
        CL_OSAL_FREE(cvector);                                          \
        return NULL;                                                    \
      }                                                                 \
    }                                                                   \
    return cvector;                                                     \
  }                                                                     \
                                                                        \
  static void                                                           \
  zocleContainerCVectorFor##element_type##Delete(cvector_for_##element_type cvector) { \
    ASSERT(cvector != NULL);                                            \
    if (cvector->begin != NULL) {                                       \
      CL_OSAL_FREE(cvector->begin);                                     \
    }                                                                   \
    CL_OSAL_FREE(cvector);                                              \
  }                                                                     \
                                                                        \
  static inline size_t                                                  \
  zocleContainerCVectorFor##element_type##Size(cvector_for_##element_type cvector) { \
    ASSERT(cvector != NULL);                                            \
    return (cvector->end - cvector->begin);                             \
  }                                                                     \
                                                                        \
  static inline size_t                                                  \
  zocleContainerCVectorFor##element_type##Capacity(                     \
      cvector_for_##element_type cvector) {                             \
    ASSERT(cvector != NULL);                                            \
    return (cvector->capacity - cvector->begin);                        \
  }                                                                     \
                                                                        \
  static cl_int                                                         \
  zocleContainerCVectorFor##element_type##Reserve(cvector_for_##element_type cvector, \
                                                  size_t const capacity) { \
    ASSERT(cvector != NULL);                                            \
                                                                        \
    {                                                                   \
      size_t const old_capacity = zocleContainerCVectorFor##element_type##Capacity(cvector); \
      if (old_capacity >= capacity) {                                   \
        return CL_SUCCESS;                                              \
      }                                                                 \
    }                                                                   \
    {                                                                   \
      element_type * const new_space =                                  \
        (element_type *)CL_OSAL_CALLOC(capacity * sizeof(element_type)); \
      if (NULL == new_space) {                                          \
        return CL_OUT_OF_HOST_MEMORY;                                   \
      }                                                                 \
      {                                                                 \
        size_t const old_size = zocleContainerCVectorFor##element_type##Size(cvector); \
        if (old_size != 0) {                                            \
          ASSERT(cvector->begin != NULL);                               \
          memcpy(new_space, cvector->begin, capacity * sizeof(element_type)); \
          CL_OSAL_FREE(cvector->begin);                                 \
        }                                                               \
        cvector->begin = new_space;                                     \
        cvector->end = (new_space + old_size);                          \
        cvector->capacity = (new_space + capacity);                     \
      }                                                                 \
    }                                                                   \
    return CL_SUCCESS;                                                  \
  }                                                                     \
                                                                        \
  static cl_int                                                         \
  zocleContainerCVectorFor##element_type##Resize(cvector_for_##element_type cvector, \
                                                 size_t const size,     \
                                                 element_type const c) { \
    cl_int result;                                                      \
    ASSERT(cvector != NULL);                                            \
    result = zocleContainerCVectorFor##element_type##Reserve(cvector, size); \
    if (result != CL_SUCCESS) {                                         \
      return result;                                                    \
    }                                                                   \
    {                                                                   \
      size_t const old_size = zocleContainerCVectorFor##element_type##Size(cvector); \
      if (old_size >= size) {                                           \
        cvector->end = cvector->begin + size;                           \
      } else {                                                          \
        size_t i;                                                       \
        size_t diff = (size - old_size);                                \
        for (i = 0; i < diff; ++i) {                                    \
          (*cvector->end) = c;                                          \
          ++cvector->end;                                               \
        }                                                               \
      }                                                                 \
      return CL_SUCCESS;                                                \
    }                                                                   \
  }                                                                     \
                                                                        \
  static cl_int                                                         \
  zocleContainerCVectorFor##element_type##Clear(cvector_for_##element_type cvector) { \
    ASSERT(cvector != NULL);                                            \
    cvector->end = cvector->begin;                                      \
    return CL_SUCCESS;                                                  \
  }                                                                     \
                                                                        \
  static cl_int                                                         \
  zocleContainerCVectorFor##element_type##PushBack(                     \
      cvector_for_##element_type cvector, element_type * const data) {  \
    ASSERT(cvector != NULL);                                            \
    if (cvector->end == cvector->capacity) {                            \
      cl_int const result =                                             \
        zocleContainerCVectorFor##element_type##Reserve(                \
            cvector,                                                    \
            (0 == zocleContainerCVectorFor##element_type##Capacity(cvector)) \
            ? 1                                                         \
            : zocleContainerCVectorFor##element_type##Capacity(cvector) + 10); \
      if (result != CL_SUCCESS) {                                       \
        return result;                                                  \
      }                                                                 \
    }                                                                   \
    memcpy(cvector->end, data, sizeof(element_type));                   \
    cvector->end += 1;                                                  \
    return CL_SUCCESS;                                                  \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  zocleContainerCVectorFor##element_type##PopBack(                      \
      cvector_for_##element_type cvector) {                             \
    ASSERT(cvector != NULL);                                            \
    ASSERT(zocleContainerCVectorFor##element_type##Size(cvector) != 0); \
    cvector->end -= 1;                                                  \
  }                                                                     \
                                                                        \
  static inline element_type *                                          \
  zocleContainerCVectorFor##element_type##Front(cvector_for_##element_type cvector) { \
    ASSERT(cvector != NULL);                                            \
    ASSERT(zocleContainerCVectorFor##element_type##Size(cvector) != 0); \
    return cvector->begin;                                              \
  }                                                                     \
                                                                        \
  static inline element_type *                                          \
  zocleContainerCVectorFor##element_type##Back(cvector_for_##element_type cvector) { \
    ASSERT(cvector != NULL);                                            \
    ASSERT(zocleContainerCVectorFor##element_type##Size(cvector) != 0); \
    {                                                                   \
      element_type *p = cvector->end;                                   \
      p -= 1;                                                           \
      return p;                                                         \
    }                                                                   \
  }                                                                     \
                                                                        \
  static inline element_type *                                          \
  zocleContainerCVectorFor##element_type##Pop(cvector_for_##element_type cvector) { \
    element_type *top = zocleContainerCVectorFor##element_type##Back(cvector); \
    zocleContainerCVectorFor##element_type##PopBack(cvector);           \
    return top;                                                         \
  }                                                                     \
                                                                        \
  static inline cvector_iter_for_##element_type                         \
  zocleContainerCVectorFor##element_type##Begin(cvector_for_##element_type cvector) { \
    ASSERT(cvector != NULL);                                            \
    return cvector->begin;                                              \
  }                                                                     \
                                                                        \
  static inline element_type*                                           \
  zocleContainerCVectorFor##element_type##At(cvector_for_##element_type cvector, \
                                             size_t const index) {      \
    ASSERT(cvector != NULL);                                            \
    return &(cvector->begin[index]);                                    \
  }                                                                     \
                                                                        \
  static inline cvector_iter_for_##element_type                         \
  zocleContainerCVectorFor##element_type##End(cvector_for_##element_type cvector) { \
    ASSERT(cvector != NULL);                                            \
    return cvector->end;                                                \
  }                                                                     \
                                                                        \
  static inline cvector_iter_for_##element_type                         \
  zocleContainerCVectorIterFor##element_type##Increment(                \
      cvector_iter_for_##element_type iter) {                           \
    ASSERT(iter != NULL);                                               \
    return iter + 1;                                                    \
  }                                                                     \
                                                                        \
  static inline cvector_iter_for_##element_type                         \
  zocleContainerCVectorIterFor##element_type##Decrement(                \
      cvector_iter_for_##element_type iter) {                           \
    ASSERT(iter != NULL);                                               \
    return iter - 1;                                                    \
  }                                                                     \
                                                                        \
  static inline element_type *                                          \
  zocleContainerCVectorIterFor##element_type##GetData(                  \
      cvector_iter_for_##element_type iter) {                           \
    return iter;                                                        \
  }                                                                     \
                                                                        \
  static inline size_t                                                  \
  zocleContainerCVectorIterFor##element_type##GetIndex(                 \
      cvector_for_##element_type cvector,                               \
      cvector_iter_for_##element_type iter) {                           \
    ASSERT(iter != NULL);                                               \
    ASSERT(cvector != NULL);                                            \
    {                                                                   \
      element_type * const e = zocleContainerCVectorIterFor##element_type##GetData(iter); \
      return e - cvector->begin;                                        \
    }                                                                   \
  }                                                                     \
                                                                        \
  typedef cl_bool (*cvector_for_##element_type##_search_compare_func_t)(element_type *, void *); \
  static cvector_iter_for_##element_type                                \
  zocleContainerCVectorFor##element_type##Search(                       \
      cvector_for_##element_type cvector,                               \
      void *data,                                                       \
      cvector_for_##element_type##_search_compare_func_t compare_func) { \
    ASSERT(cvector != NULL);                                            \
    ASSERT(data != NULL);                                               \
    {                                                                   \
      cvector_iter_for_##element_type e;                                \
      for (e = zocleContainerCVectorFor##element_type##Begin(cvector);  \
           e != zocleContainerCVectorFor##element_type##End(cvector);   \
           e = zocleContainerCVectorIterFor##element_type##Increment(e)) { \
        element_type *element = zocleContainerCVectorIterFor##element_type##GetData(e); \
        if (CL_TRUE == compare_func(element, data)) {                   \
          return e;                                                     \
        }                                                               \
      }                                                                 \
      return NULL;                                                      \
    }                                                                   \
  }                                                                     \
                                                                        \
  static void                                                           \
  zocleContainerCVectorFor##element_type##Copy(                         \
      cvector_for_##element_type target,                                \
      cvector_for_##element_type src) {                                 \
    ASSERT(target != NULL);                                             \
    ASSERT(src != NULL);                                                \
    ASSERT(zocleContainerCVectorFor##element_type##Size(target) ==      \
           zocleContainerCVectorFor##element_type##Size(src));          \
    memcpy(target->begin, src->begin,                                   \
           zocleContainerCVectorFor##element_type##Size(src) * sizeof(element_type)); \
  }                                                                     \
                                                                        \
  static void                                                           \
  zocleContainerCVectorFor##element_type##SetAllToZero(                 \
      cvector_for_##element_type cvector) {                             \
    ASSERT(cvector != NULL);                                            \
    memset(cvector->begin, 0,                                           \
           zocleContainerCVectorFor##element_type##Size(cvector) * sizeof(element_type)); \
  }                                                                     \
                                                                        \
  static int                                                            \
  zocleContainerCVectorFor##element_type##Compare(                      \
      cvector_for_##element_type target,                                \
      cvector_for_##element_type src) {                                 \
    ASSERT(target != NULL);                                             \
    ASSERT(src != NULL);                                                \
    ASSERT(zocleContainerCVectorFor##element_type##Size(target) ==      \
           zocleContainerCVectorFor##element_type##Size(src));          \
    return memcmp(target->begin, src->begin,                            \
                  zocleContainerCVectorFor##element_type##Size(src) * sizeof(element_type)); \
  }

/* ======================================================
 * vector
 * ====================================================== */
#define CVECTOR_TYPE(element_type)      CVECTOR_TYPE_X(element_type)
#define CVECTOR_ITER_TYPE(element_type) CVECTOR_ITER_TYPE_X(element_type)

#define CVECTOR_NEW(element_type)       CVECTOR_NEW_X(element_type)
#define CVECTOR_DELETE(element_type)    CVECTOR_DELETE_X(element_type)
#define CVECTOR_PUSH_BACK(element_type) CVECTOR_PUSH_BACK_X(element_type)
#define CVECTOR_POP_BACK(element_type)  CVECTOR_POP_BACK_X(element_type)
#define CVECTOR_SEARCH(element_type)    CVECTOR_SEARCH_X(element_type)
#define CVECTOR_SIZE(element_type)      CVECTOR_SIZE_X(element_type)
#define CVECTOR_FRONT(element_type)     CVECTOR_FRONT_X(element_type)
#define CVECTOR_BACK(element_type)      CVECTOR_BACK_X(element_type)
#define CVECTOR_BEGIN(element_type)     CVECTOR_BEGIN_X(element_type)
#define CVECTOR_END(element_type)       CVECTOR_END_X(element_type)
#define CVECTOR_AT(element_type)        CVECTOR_AT_X(element_type)
#define CVECTOR_RESIZE(element_type)    CVECTOR_RESIZE_X(element_type)
#define CVECTOR_CLEAR(element_type)     CVECTOR_CLEAR_X(element_type)
#define CVECTOR_COPY(element_type)      CVECTOR_COPY_X(element_type)
#define CVECTOR_COMPARE(element_type)   CVECTOR_COMPARE_X(element_type)
#define CVECTOR_SET_ALL_TO_ZERO(element_type)      CVECTOR_SET_ALL_TO_ZERO_X(element_type)

#define CVECTOR_ITER_INCREMENT(element_type)    \
  CVECTOR_ITER_INCREMENT_X(element_type)
#define CVECTOR_ITER_DECREMENT(element_type)    \
  CVECTOR_ITER_DECREMENT_X(element_type)
#define CVECTOR_ITER_GET_DATA(element_type)     \
  CVECTOR_ITER_GET_DATA_X(element_type)
#define CVECTOR_ITER_GET_INDEX(element_type)    \
  CVECTOR_ITER_GET_INDEX_X(element_type)

#define CVECTOR_TYPE_X(element_type)      cvector_for_##element_type
#define CVECTOR_ITER_TYPE_X(element_type) cvector_iter_for_##element_type

#define CVECTOR_NEW_X(element_type)       zocleContainerCVectorFor##element_type##New
#define CVECTOR_DELETE_X(element_type)    zocleContainerCVectorFor##element_type##Delete
#define CVECTOR_PUSH_BACK_X(element_type) zocleContainerCVectorFor##element_type##PushBack
#define CVECTOR_POP_BACK_X(element_type)  zocleContainerCVectorFor##element_type##PopBack
#define CVECTOR_SEARCH_X(element_type)    zocleContainerCVectorFor##element_type##Search
#define CVECTOR_SIZE_X(element_type)      zocleContainerCVectorFor##element_type##Size
#define CVECTOR_FRONT_X(element_type)     zocleContainerCVectorFor##element_type##Front
#define CVECTOR_BACK_X(element_type)      zocleContainerCVectorFor##element_type##Back
#define CVECTOR_BEGIN_X(element_type)     zocleContainerCVectorFor##element_type##Begin
#define CVECTOR_END_X(element_type)       zocleContainerCVectorFor##element_type##End
#define CVECTOR_AT_X(element_type)        zocleContainerCVectorFor##element_type##At
#define CVECTOR_RESIZE_X(element_type)    zocleContainerCVectorFor##element_type##Resize
#define CVECTOR_CLEAR_X(element_type)     zocleContainerCVectorFor##element_type##Clear
#define CVECTOR_COPY_X(element_type)      zocleContainerCVectorFor##element_type##Copy
#define CVECTOR_COMPARE_X(element_type)   zocleContainerCVectorFor##element_type##Compare
#define CVECTOR_SET_ALL_TO_ZERO_X(element_type)      zocleContainerCVectorFor##element_type##SetAllToZero

#define CVECTOR_ITER_INCREMENT_X(element_type)          \
  zocleContainerCVectorIterFor##element_type##Increment
#define CVECTOR_ITER_DECREMENT_X(element_type)          \
  zocleContainerCVectorIterFor##element_type##Decrement
#define CVECTOR_ITER_GET_DATA_X(element_type)         \
  zocleContainerCVectorIterFor##element_type##GetData
#define CVECTOR_ITER_GET_INDEX_X(element_type)          \
  zocleContainerCVectorIterFor##element_type##GetIndex

/* ======================================================
 * stack
 * ====================================================== */
#define CSTACK_TYPE(element_type)       CSTACK_TYPE_X(element_type)
#define CSTACK_ITER_TYPE(element_type)  CSTACK_ITER_TYPE_X(element_type)

#define CSTACK_NEW(element_type)        CSTACK_NEW_X(element_type)
#define CSTACK_DELETE(element_type)     CSTACK_DELETE_X(element_type)
#define CSTACK_SIZE(element_type)       CSTACK_SIZE_X(element_type)
#define CSTACK_TOP(element_type)        CSTACK_TOP_X(element_type)
#define CSTACK_PUSH(element_type)       CSTACK_PUSH_X(element_type)
#define CSTACK_POP(element_type)        CSTACK_POP_X(element_type)
#define CSTACK_BEGIN(element_type)      CSTACK_BEGIN_X(element_type)
#define CSTACK_END(element_type)        CSTACK_END_X(element_type)

#define CSTACK_ITER_INCREMENT(element_type)  CSTACK_ITER_INCREMENT_X(element_type)
#define CSTACK_ITER_GET_DATA(element_type)   CSTACK_ITER_GET_DATA_X(element_type)

#define CSTACK_TYPE_X(element_type)       cvector_for_##element_type
#define CSTACK_ITER_TYPE_X(element_type)  cvector_iter_for_##element_type

#define CSTACK_NEW_X(element_type)        zocleContainerCVectorFor##element_type##New
#define CSTACK_DELETE_X(element_type)     zocleContainerCVectorFor##element_type##Delete
#define CSTACK_SIZE_X(element_type)       zocleContainerCVectorFor##element_type##Size
#define CSTACK_TOP_X(element_type)        zocleContainerCVectorFor##element_type##Back
#define CSTACK_PUSH_X(element_type)       zocleContainerCVectorFor##element_type##PushBack
#define CSTACK_POP_X(element_type)        zocleContainerCVectorFor##element_type##Pop
#define CSTACK_BEGIN_X(element_type)      zocleContainerCVectorFor##element_type##Begin
#define CSTACK_END_X(element_type)        zocleContainerCVectorFor##element_type##End

#define CSTACK_ITER_INCREMENT_X(element_type)  zocleContainerCVectorIterFor##element_type##Increment
#define CSTACK_ITER_GET_DATA_X(element_type)   zocleContainerCVectorIterFor##element_type##GetData

#endif
