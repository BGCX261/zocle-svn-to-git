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
#ifndef ZOCLE_CONTAINER_HASHTABLE_H_
#define ZOCLE_CONTAINER_HASHTABLE_H_

#include <cl.h>

#define DECLARE_HASHTABLE_TYPE(element_type)    \
  DECLARE_HASHTABLE_TYPE_X(element_type)

#define DECLARE_HASHTABLE_TYPE_X(element_type)                          \
  typedef struct _hashtable_for_##element_type *hashtable_for_##element_type; \
  typedef struct _hashtable_element_for_##element_type *hashtable_element_for_##element_type; \
  typedef struct _hashtable_element_for_##element_type *hashtable_iter_for_##element_type; \
                                                                        \
  typedef cl_int  (*hashtable_hash_func_for_##element_type)(element_type); \
  typedef cl_bool (*hashtable_equal_func_for_##element_type)(element_type, void *);

#define DEFINE_HASHTABLE_TYPE(element_type)     \
  DEFINE_HASHTABLE_TYPE_X(element_type)

#define DEFINE_HASHTABLE_TYPE_X(element_type)                           \
  struct _hashtable_element_for_##element_type {                        \
    element_type data;                                                  \
    cl_bool head;                                                       \
    size_t  idx;                                                        \
    struct _hashtable_element_for_##element_type *hash_prev;            \
    struct _hashtable_element_for_##element_type *hash_next;            \
  };                                                                    \
                                                                        \
  struct _hashtable_for_##element_type {                                \
    size_t  size;                                                       \
    struct _hashtable_element_for_##element_type **begin;               \
                                                                        \
    hashtable_hash_func_for_##element_type   hash_func;                 \
    hashtable_equal_func_for_##element_type  equal_func;                \
  };                                                                    \
                                                                        \
  static hashtable_for_##element_type                                   \
  zocleContainerHashTableFor##element_type##New(size_t const table_size) { \
    hashtable_for_##element_type table =                                \
      CL_OSAL_CALLOC(sizeof(struct _hashtable_for_##element_type));     \
    if (NULL == table) {                                                \
      return NULL;                                                      \
    }                                                                   \
    table->size = table_size;                                           \
    table->begin = (struct _hashtable_element_for_##element_type **)CL_OSAL_CALLOC( \
        sizeof(struct _hashtable_element_for_##element_type *) * table_size); \
    table->hash_func = NULL;                                            \
    table->hash_func = NULL;                                            \
    return table;                                                       \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  zocleContainerHashTableFor##element_type##SetHashFunc(                \
      hashtable_for_##element_type table,                               \
      hashtable_hash_func_for_##element_type hash_func) {               \
    ASSERT(table != NULL);                                              \
    table->hash_func = hash_func;                                       \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  zocleContainerHashTableFor##element_type##SetEqualFunc(               \
      hashtable_for_##element_type table,                               \
      hashtable_equal_func_for_##element_type equal_func) {             \
    ASSERT(table != NULL);                                              \
    table->equal_func = equal_func;                                     \
  }                                                                     \
                                                                        \
  static cl_bool                                                        \
  zocleContainerHashTableFor##element_type##FindNextFilledSlot(         \
    hashtable_for_##element_type table,                                 \
    size_t const pivot,                                                 \
    size_t * const result) {                                            \
    size_t i;                                                           \
                                                                        \
    ASSERT(table != NULL);                                              \
    ASSERT((pivot >= 0) && (pivot < table->size));                      \
                                                                        \
    if (pivot == (table->size - 1)) {                                   \
      return CL_FALSE;                                                  \
    }                                                                   \
    for (i = pivot + 1; i < table->size; ++i) {                         \
      if (table->begin[i] != NULL) {                                    \
        *result = i;                                                    \
        return CL_TRUE;                                                 \
      }                                                                 \
    }                                                                   \
    return CL_FALSE;                                                    \
  }                                                                     \
                                                                        \
  static cl_bool                                                        \
  zocleContainerHashTableFor##element_type##FindPrevFilledSlot(         \
    hashtable_for_##element_type table,                                 \
    size_t const pivot,                                                 \
    size_t * const result) {                                            \
    size_t i;                                                           \
                                                                        \
    ASSERT(table != NULL);                                              \
    ASSERT((pivot >= 0) && (pivot < table->size));                      \
                                                                        \
    if (0 == pivot) {                                                   \
      return CL_FALSE;                                                  \
    }                                                                   \
    for (i = pivot - 1; i > 0; --i) {                                   \
      if (table->begin[i] != NULL) {                                    \
        *result = i;                                                    \
        return CL_TRUE;                                                 \
      }                                                                 \
    }                                                                   \
    ASSERT(0 == i);                                                     \
    if (table->begin[0] != NULL) {                                      \
      *result = 0;                                                      \
      return CL_TRUE;                                                   \
    }                                                                   \
    return CL_FALSE;                                                    \
  }                                                                     \
                                                                        \
  static inline void                                                    \
  zocleContainerHashTableFor##element_type##Delete(                     \
      hashtable_for_##element_type table) {                             \
    size_t i;                                                           \
    cl_bool find;                                                       \
    hashtable_element_for_##element_type e, next_e;                     \
                                                                        \
    ASSERT(table != NULL);                                              \
                                                                        \
    if (table->begin[0] != NULL) {                                      \
      i = 0;                                                            \
      find = CL_TRUE;                                                   \
    } else {                                                            \
      find = zocleContainerHashTableFor##element_type##FindNextFilledSlot( \
          table, 0, &i);                                                \
    }                                                                   \
    if (CL_TRUE == find) {                                              \
      e = table->begin[i];                                              \
      while (e != NULL) {                                               \
        next_e = e->hash_next;                                          \
        CL_OSAL_FREE(e);                                                \
        e = next_e;                                                     \
      }                                                                 \
    }                                                                   \
    if (table->begin != NULL) {                                         \
      CL_OSAL_FREE(table->begin);                                       \
    }                                                                   \
    CL_OSAL_FREE(table);                                                \
  }                                                                     \
                                                                        \
  static struct _hashtable_element_for_##element_type *                 \
  zocleContainerHashTableFor##element_type##Search(                     \
      hashtable_for_##element_type table,                               \
      void *searched_item) {                                            \
    size_t hash_value;                                                  \
    cl_bool search_all = CL_FALSE;                                      \
    hashtable_element_for_##element_type current = NULL;                \
                                                                        \
    ASSERT(table != NULL);                                              \
                                                                        \
    if (table->hash_func != NULL) {                                     \
      hash_value = (*table->hash_func)(searched_item);                  \
      hash_value %= table->size;                                        \
      search_all = CL_FALSE;                                            \
    } else {                                                            \
      hash_value = 0;                                                   \
      search_all = CL_TRUE;                                             \
    }                                                                   \
    current = table->begin[hash_value];                                 \
    if (NULL == current) {                                              \
      return NULL;                                                      \
    }                                                                   \
    for (;;) {                                                          \
      if (CL_TRUE == (*table->equal_func)(current->data, searched_item)) { \
        return current;                                                 \
      }                                                                 \
      current = current->hash_next;                                     \
      if (NULL == current) {                                            \
        return NULL;                                                    \
      } else {                                                          \
        if (CL_TRUE == current->head) {                                 \
          if (CL_FALSE == search_all) {                                 \
            return NULL;                                                \
          }                                                             \
        }                                                               \
      }                                                                 \
    }                                                                   \
    ASSERT(0);                                                          \
    return NULL;                                                        \
  }                                                                     \
                                                                        \
  static cl_bool                                                        \
  zocleContainerHashTableFor##element_type##Add(                        \
      hashtable_for_##element_type table,                               \
      element_type * const item) {                                      \
    hashtable_element_for_##element_type e;                             \
    cl_bool existed;                                                    \
                                                                        \
    ASSERT(table != NULL);                                              \
    ASSERT(table->hash_func != NULL);                                   \
                                                                        \
    existed = (zocleContainerHashTableFor##element_type##Search(table, *item) != NULL); \
    ASSERT(CL_FALSE == existed);                                        \
                                                                        \
    {                                                                   \
      e = (hashtable_element_for_##element_type)CL_OSAL_CALLOC(         \
          sizeof(struct _hashtable_element_for_##element_type));        \
      ASSERT(e != NULL);                                                \
      e->head = CL_TRUE;                                                \
      e->hash_prev = NULL;                                              \
      e->hash_next = NULL;                                              \
      memcpy(&(e->data), item, sizeof(element_type));                   \
    }                                                                   \
    {                                                                   \
      hashtable_element_for_##element_type current;                     \
                                                                        \
      size_t hash_result = (*table->hash_func)(*item);                  \
      hash_result %= table->size;                                       \
      e->head = CL_TRUE;                                                \
      e->idx = hash_result;                                             \
                                                                        \
      current = table->begin[hash_result];                              \
      if (current != NULL) {                                            \
        current->head = CL_FALSE;                                       \
        e->hash_prev = current->hash_prev;                              \
        if (current->hash_prev != NULL) {                               \
          current->hash_prev->hash_next = e;                            \
        }                                                               \
        current->hash_prev = e;                                         \
        e->hash_next = current;                                         \
      } else {                                                          \
        size_t neighbor_slot;                                           \
        cl_bool find =                                                  \
          zocleContainerHashTableFor##element_type##FindNextFilledSlot( \
              table, hash_result, &neighbor_slot);                      \
        if (CL_TRUE == find) {                                          \
          hashtable_element_for_##element_type neighbor =               \
            table->begin[neighbor_slot];                                \
          if (neighbor->hash_prev != NULL) {                            \
            neighbor->hash_prev->hash_next = e;                         \
          }                                                             \
          e->hash_prev = neighbor->hash_prev;                           \
          neighbor->hash_prev = e;                                      \
          e->hash_next = neighbor;                                      \
        } else {                                                        \
          find =                                                        \
            zocleContainerHashTableFor##element_type##FindPrevFilledSlot( \
                table, hash_result, &neighbor_slot);                    \
          if (CL_TRUE == find) {                                        \
            hashtable_element_for_##element_type neighbor =             \
              table->begin[neighbor_slot];                              \
            if (neighbor->hash_next != NULL) {                          \
              neighbor->hash_next->hash_prev = e;                       \
            }                                                           \
            e->hash_next = neighbor->hash_next;                         \
            neighbor->hash_next = e;                                    \
            e->hash_prev = neighbor;                                    \
          }                                                             \
        }                                                               \
      }                                                                 \
      table->begin[hash_result] = e;                                    \
    }                                                                   \
    return CL_TRUE;                                                     \
  }                                                                     \
                                                                        \
  static void                                                           \
  zocleContainerHashTableFor##element_type##Remove(                     \
      hashtable_for_##element_type table,                               \
      element_type item) {                                              \
    hashtable_element_for_##element_type e;                             \
                                                                        \
    ASSERT(table != NULL);                                              \
    ASSERT(table->hash_func != NULL);                                   \
                                                                        \
    e = zocleContainerHashTableFor##element_type##Search(table, item);  \
    ASSERT(e != NULL);                                                  \
                                                                        \
    if (e->hash_prev != NULL) {                                         \
      e->hash_prev->hash_next = e->hash_next;                           \
    }                                                                   \
    if (e->hash_next != NULL) {                                         \
      e->hash_next->hash_prev = e->hash_prev;                           \
    }                                                                   \
    {                                                                   \
      size_t hash_result = (*table->hash_func)(item);                   \
      hash_result %= table->size;                                       \
      if (e == table->begin[hash_result]) {                             \
        table->begin[hash_result] = e->hash_next;                       \
        table->begin[hash_result]->head = CL_TRUE;                      \
      }                                                                 \
    }                                                                   \
  }                                                                     \
                                                                        \
  static inline hashtable_iter_for_##element_type                       \
  zocleContainerHashTableFor##element_type##Begin(                      \
      hashtable_for_##element_type table) {                             \
    size_t i;                                                           \
    ASSERT(table != NULL);                                              \
    for (i = 0; i < table->size; ++i) {                                 \
      if (table->begin[i] != NULL) {                                    \
        return table->begin[i];                                         \
      }                                                                 \
    }                                                                   \
    return NULL;                                                        \
  }                                                                     \
                                                                        \
  static inline hashtable_iter_for_##element_type                       \
  zocleContainerHashTableFor##element_type##End(                        \
      hashtable_for_##element_type table) {                             \
    ASSERT(table != NULL);                                              \
    return NULL;                                                        \
  }                                                                     \
                                                                        \
  static inline element_type *                                          \
  zocleContainerHashTableIterFor##element_type##GetData(                \
      hashtable_iter_for_##element_type iter) {                         \
    ASSERT(iter != NULL);                                               \
    return &(iter->data);                                               \
  }                                                                     \
                                                                        \
  static inline hashtable_iter_for_##element_type                       \
  zocleContainerHashTableIterFor##element_type##Increment(              \
      hashtable_iter_for_##element_type iter) {                         \
    ASSERT(iter != NULL);                                               \
    return iter->hash_next;                                             \
  }

#define HASHTABLE_TYPE(element_type)            HASHTABLE_TYPE_X(element_type)
#define HASHTABLE_ITER_TYPE(element_type)       HASHTABLE_ITER_TYPE_X(element_type)

#define HASHTABLE_HASH_FUNC_TYPE(element_type)  HASHTABLE_HASH_FUNC_TYPE_X(element_type)
#define HASHTABLE_EQUAL_FUNC_TYPE(element_type) HASHTABLE_EQUAL_FUNC_TYPE_X(element_type)

#define HASHTABLE_NEW(element_type)             HASHTABLE_NEW_X(element_type)
#define HASHTABLE_SET_HASH_FUNC(element_type)   HASHTABLE_SET_HASH_FUNC_X(element_type)
#define HASHTABLE_SET_EQUAL_FUNC(element_type)  HASHTABLE_SET_EQUAL_FUNC_X(element_type)
#define HASHTABLE_DELETE(element_type)          HASHTABLE_DELETE_X(element_type)
#define HASHTABLE_ADD(element_type)             HASHTABLE_ADD_X(element_type)
#define HASHTABLE_REMOVE(element_type)          HASHTABLE_REMOVE_X(element_type)
#define HASHTABLE_SEARCH(element_type)          HASHTABLE_SEARCH_X(element_type)
#define HASHTABLE_BEGIN(element_type)           HASHTABLE_BEGIN_X(element_type)
#define HASHTABLE_END(element_type)             HASHTABLE_END_X(element_type)

#define HASHTABLE_ITER_GET_DATA(element_type)   HASHTABLE_ITER_GET_DATA_X(element_type)
#define HASHTABLE_ITER_INCREMENT(element_type)  HASHTABLE_ITER_INCREMENT_X(element_type)

/* ==============================================
 *   X macro
 * ============================================== */
#define HASHTABLE_TYPE_X(element_type)            hashtable_for_##element_type
#define HASHTABLE_ITER_TYPE_X(element_type)       hashtable_iter_for_##element_type

#define HASHTABLE_HASH_FUNC_TYPE_X(element_type)  hashtable_hash_func_for_##element_type
#define HASHTABLE_EQUAL_FUNC_TYPE_X(element_type) hashtable_equal_func_for_##element_type

#define HASHTABLE_NEW_X(element_type)             zocleContainerHashTableFor##element_type##New
#define HASHTABLE_SET_HASH_FUNC_X(element_type)   zocleContainerHashTableFor##element_type##SetHashFunc
#define HASHTABLE_SET_EQUAL_FUNC_X(element_type)  zocleContainerHashTableFor##element_type##SetEqualFunc
#define HASHTABLE_DELETE_X(element_type)          zocleContainerHashTableFor##element_type##Delete
#define HASHTABLE_ADD_X(element_type)             zocleContainerHashTableFor##element_type##Add
#define HASHTABLE_REMOVE_X(element_type)          zocleContainerHashTableFor##element_type##Remove
#define HASHTABLE_SEARCH_X(element_type)          zocleContainerHashTableFor##element_type##Search
#define HASHTABLE_BEGIN_X(element_type)           zocleContainerHashTableFor##element_type##Begin
#define HASHTABLE_END_X(element_type)             zocleContainerHashTableFor##element_type##End

#define HASHTABLE_ITER_GET_DATA_X(element_type)   zocleContainerHashTableIterFor##element_type##GetData
#define HASHTABLE_ITER_INCREMENT_X(element_type)  zocleContainerHashTableIterFor##element_type##Increment

#endif
