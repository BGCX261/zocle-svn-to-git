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
#ifndef ZOCLE_CONTAINER_CSTRING_H_
#define ZOCLE_CONTAINER_CSTRING_H_

#include <cl.h>

#include <osal/inc/osal.h>

#define CHAR_TYPE char

typedef struct _cstring *cstring;
struct _cstring {
  CHAR_TYPE *begin;
  CHAR_TYPE *end;  /**< 'end' points to the location just after the NULL terminator. */
  CHAR_TYPE *capacity;
};

/**
 * Like strlen, this function doesn't count the NULL terminator.
 */
static inline size_t
zocleContainerCStringSize(cstring string) {
  ASSERT(string != NULL);
  if (NULL == string->begin) {
    ASSERT(NULL == string->end);
    ASSERT(NULL == string->capacity);
    return 0;
  } else {
    return strlen(string->begin);
  }
}

static inline size_t
zocleContainerCStringLength(cstring string) {
  return zocleContainerCStringSize(string);
}

static inline void
zocleContainerCStringClear(cstring string) {
  ASSERT(string != NULL);
  ASSERT(string->begin != NULL);
  string->begin[0] = 0;
}

static inline cl_bool
zocleContainerCStringEmpty(cstring string) {
  ASSERT(string != NULL);
  return (0 == zocleContainerCStringSize(string)) ? CL_TRUE : CL_FALSE;
}

static inline CHAR_TYPE *
zocleContainerCStringRawData(cstring string) {
  ASSERT(string != NULL);
  return string->begin;
}

static inline cl_bool
zocleContainerCStringCompare(cstring cstr, char const *comparator) {
  ASSERT(cstr != NULL);
  ASSERT(comparator != NULL);
  return (0 == strcmp(cstr->begin, comparator)) ? CL_TRUE : CL_FALSE;
}

static inline CHAR_TYPE
zocleContainerCStringAt(cstring cstr, size_t const index) {
  ASSERT(cstr != NULL);
  ASSERT(index < zocleContainerCStringSize(cstr));
  return cstr->begin[index];
}

static inline CHAR_TYPE
convertLowercaseCharToUppercaseChar(CHAR_TYPE const ch) {
  if ((ch >= 'a') && (ch <= 'z')) {
    return ch - 'a' + 'A';
  } else {
    return ch;
  }
}

static inline void
zocleContainerCStringShiftFront(cstring cstr, size_t const count) {
  size_t left_size;
  
  ASSERT(cstr != NULL);
  ASSERT(zocleContainerCStringSize(cstr) >= count);
  
  left_size = zocleContainerCStringSize(cstr) - count;
  memmove(cstr->begin, cstr->begin + count, left_size);
  cstr->begin[left_size] = '\0';
}

static inline void
zocleContainerCStringErase(
    cstring cstr, size_t const start_idx, size_t const count) {
  ASSERT(cstr != NULL);
  ASSERT(zocleContainerCStringSize(cstr) > start_idx);
  ASSERT((zocleContainerCStringSize(cstr) + 1) > (start_idx + count));
  
  memmove(cstr->begin + start_idx,
          cstr->begin + start_idx + count,
          zocleContainerCStringSize(cstr) - start_idx - count);
  
  cstr->begin[zocleContainerCStringSize(cstr) - count] = '\0';
}

extern cstring zocleContainerCStringNew(void);
extern void zocleContainerCStringDelete(cstring cstr);
extern cl_int zocleContainerCStringAppendChar(cstring cstr, char const c);
extern cl_int zocleContainerCStringAppendString(cstring cstr, char const * const str);
extern cl_int zocleContainerCStringAppendPartialString(
    cstring cstr, char const * const str, size_t const start_idx, size_t const size);
extern void zocleContainerCStringResize(cstring string, size_t const size, char const ch);
extern void zocleContainerCStringAssign(cstring cstr, char const *s);

extern void zocleContainerCStringVprintf(cstring cstr, char const *fmt, va_list ap);

static inline void
zocleContainerCStringPrintf(cstring cstr, char const *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  zocleContainerCStringVprintf(cstr, fmt, ap);
  va_end(ap);  
}

#define CSTRING_NEW                    zocleContainerCStringNew
#define CSTRING_SIZE                   zocleContainerCStringSize
#define CSTRING_LENGTH                 zocleContainerCStringLength
#define CSTRING_CLEAR                  zocleContainerCStringClear
#define CSTRING_EMPTY                  zocleContainerCStringEmpty
#define CSTRING_RAW_DATA               zocleContainerCStringRawData
#define CSTRING_COMPARE                zocleContainerCStringCompare
#define CSTRING_AT                     zocleContainerCStringAt
#define CSTRING_DELETE                 zocleContainerCStringDelete
#define CSTRING_APPEND_CHAR            zocleContainerCStringAppendChar
#define CSTRING_APPEND_STRING          zocleContainerCStringAppendString
#define CSTRING_APPEND_PARTIAL_STRING  zocleContainerCStringAppendPartialString
#define CSTRING_RESIZE                 zocleContainerCStringResize
#define CSTRING_ASSIGN                 zocleContainerCStringAssign
#define CSTRING_PRINTF                 zocleContainerCStringPrintf
#define CSTRING_VPRINTF                zocleContainerCStringVprintf
#define CSTRING_ERASE                  zocleContainerCStringErase
#define CSTRING_SHIFT_FRONT            zocleContainerCStringShiftFront

#endif
