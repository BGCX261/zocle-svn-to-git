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
#include <zocle_config.h>

#include <container/inc/cstring.h>

#include <cl.h>
#include <osal/inc/osal.h>

#include <string.h>
#include <stdarg.h>

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

typedef enum fmt_stage_enum {
  FMT_STAGE_NONE,
  FMT_STAGE_FLAG,
  FMT_STAGE_WIDTH,
  FMT_STAGE_PRECISION,
  FMT_STAGE_LENGTH
} fmt_stage_enum;

enum {
  FMT_FLAG_MINUS = (1 << 0), /**< - */
  FMT_FLAG_PLUS  = (1 << 1), /**< + */
  FMT_FLAG_SPACE = (1 << 2), /**<   */
  FMT_FLAG_SHARP = (1 << 3), /**< # */
  FMT_FLAG_ZERO  = (1 << 4)  /**< 0 */
};

cstring
zocleContainerCStringNew(void) {
  cstring string = CL_OSAL_CALLOC(sizeof(struct _cstring));
  if (NULL == string) {
    return NULL;
  }
  string->begin = NULL;
  string->end = NULL;
  string->capacity = NULL;
  
  /* initial to an empty string */
  {
    CHAR_TYPE * const new_space = (CHAR_TYPE *)CL_OSAL_CALLOC(10 * sizeof(CHAR_TYPE));
    if (NULL == new_space) {
      CL_OSAL_FREE(string);
      return NULL;
    }
    
    string->begin = new_space;
    string->begin[0] = 0;
    /* '+1' is because the NULL terminator */
    string->end = (new_space + 1);
    string->capacity = (new_space + 10);
  }
  
  return string;
}

void
zocleContainerCStringDelete(cstring string) {
  ASSERT(string != NULL);
  if (string->begin != NULL) {
    CL_OSAL_FREE(string->begin);
  }
  CL_OSAL_FREE(string);
}

static inline size_t
zocleContainerCStringCapacity(cstring string) {
  ASSERT(string != NULL);
  return (string->capacity - string->begin);
}

static size_t
zocleContainerCStringUnusedCapacity(cstring string) {
  size_t const size = zocleContainerCStringSize(string);
  size_t const capacity = zocleContainerCStringCapacity(string);
  return capacity - size;
}

static cl_int
zocleContainerCStringReserve(cstring string, size_t const capacity) {
  ASSERT(string != NULL);
  {
    size_t const old_capacity = zocleContainerCStringCapacity(string);
    if (old_capacity >= capacity) {
      return CL_SUCCESS;
    }
  }
  {
    CHAR_TYPE * const new_space = (CHAR_TYPE *)CL_OSAL_CALLOC(capacity * sizeof(CHAR_TYPE));
    if (NULL == new_space) {
      return CL_OUT_OF_HOST_MEMORY;
    }
    {
      size_t const old_size = zocleContainerCStringSize(string);
      if (old_size != 0) {
        /* strcpy will copy the NULL terminator as well. */
        strcpy(new_space, string->begin);
      }
      if (string->begin != NULL) {
        CL_OSAL_FREE(string->begin);
      }
      string->begin = new_space;
      /* '+1' is because the NULL terminator */
      string->end = (new_space + old_size + 1);
      string->capacity = (new_space + capacity);
    }
  }
  return CL_SUCCESS;
}

cl_int
zocleContainerCStringAppendChar(cstring string, char const c) {
  ASSERT(string != NULL);
  if (string->end == string->capacity) {
    cl_int const result =
      zocleContainerCStringReserve(string,
                                   (0 == zocleContainerCStringCapacity(string))
                                   ? 2
                                   : zocleContainerCStringCapacity(string) + 10);
    if (result != CL_SUCCESS) {
      return result;
    }
  }
  (*(string->end - 1)) = c;
  (*string->end) = 0;
  ++(string->end);
  return CL_SUCCESS;
}

cl_int
zocleContainerCStringAppendString(cstring string, char const * const str) {
  ASSERT(string != NULL);
  {
    size_t const unused = zocleContainerCStringUnusedCapacity(string);
    if (unused < (strlen(str) + 1)) {
      cl_int const result = zocleContainerCStringReserve(
          string, zocleContainerCStringCapacity(string) + strlen(str) + 1);
      if (result != CL_SUCCESS) {
        return result;
      }
    }
    strcpy(string->end - 1, str);
    string->end += strlen(str);
    return CL_SUCCESS;
  }
}

cl_int
zocleContainerCStringAppendPartialString(
    cstring string, char const * const str,
    size_t const start_idx, size_t const count) {
  ASSERT(string != NULL);
  ASSERT(strlen(str) >= start_idx);
  ASSERT(strlen(str) >= (start_idx + count));
  
  {
    size_t const unused = zocleContainerCStringUnusedCapacity(string);
    if (unused < count) {
      cl_int const result = zocleContainerCStringReserve(
          string, zocleContainerCStringCapacity(string) + count);
      if (result != CL_SUCCESS) {
        return result;
      }
    }
    strncpy(string->end - 1, str + start_idx, count);
    string->end += (count - 1);
    *(string->end) = 0;
    string->end += 1;
    return CL_SUCCESS;
  }
}

void
zocleContainerCStringResize(cstring string, size_t const size, char const ch) {
  ASSERT(string != NULL);
  {
    size_t const curr_size = zocleContainerCStringSize(string);
    if (size <= curr_size) {
      string->begin[size] = 0;
    } else {
      size_t i;
      for (i = 0; i < (size - curr_size); ++i) {
        zocleContainerCStringAppendChar(string, ch);
      }
    }
  }
}

void
zocleContainerCStringAssign(cstring cstr, char const *s) {
  ASSERT(cstr != NULL);
  ASSERT(s != NULL);
  zocleContainerCStringClear(cstr);
  {
    size_t const capacity = zocleContainerCStringCapacity(cstr);
    if (strlen(s) <= capacity) {
      strcpy(cstr->begin, s);
    } else {
      strncpy(cstr->begin, s, capacity);
      {
        char const *ptr = s + capacity;
        size_t i;
        for (i = 0; i < strlen(s) - capacity; ++i) {
          zocleContainerCStringAppendChar(cstr, *ptr);
          ++ptr;
        }
      }
    }
  }
}

static cstring
convertUnsignedIntToString(unsigned int value, int const base) {
  cstring tmp_cstr = zocleContainerCStringNew();
  cstring cstr = zocleContainerCStringNew();
  unsigned int modulo;
  
  for (;;) {
    if (0 == value) {
      break;
    }
    switch (base) {
    case 8 : modulo = value % 8 ; break;
    case 10: modulo = value % 10; break;
    case 16: modulo = value % 16; break;
      
    default:
      ASSERT(0);
      break;
    }
    switch (modulo) {
    case 0 : zocleContainerCStringAppendChar(tmp_cstr, '0'); break;
    case 1 : zocleContainerCStringAppendChar(tmp_cstr, '1'); break;
    case 2 : zocleContainerCStringAppendChar(tmp_cstr, '2'); break;
    case 3 : zocleContainerCStringAppendChar(tmp_cstr, '3'); break;
    case 4 : zocleContainerCStringAppendChar(tmp_cstr, '4'); break;
    case 5 : zocleContainerCStringAppendChar(tmp_cstr, '5'); break;
    case 6 : zocleContainerCStringAppendChar(tmp_cstr, '6'); break;
    case 7 : zocleContainerCStringAppendChar(tmp_cstr, '7'); break;
    case 8 : zocleContainerCStringAppendChar(tmp_cstr, '8'); break;
    case 9 : zocleContainerCStringAppendChar(tmp_cstr, '9'); break;
    case 10: zocleContainerCStringAppendChar(tmp_cstr, 'A'); break;
    case 11: zocleContainerCStringAppendChar(tmp_cstr, 'B'); break;
    case 12: zocleContainerCStringAppendChar(tmp_cstr, 'C'); break;
    case 13: zocleContainerCStringAppendChar(tmp_cstr, 'D'); break;
    case 14: zocleContainerCStringAppendChar(tmp_cstr, 'E'); break;
    case 15: zocleContainerCStringAppendChar(tmp_cstr, 'F'); break;
    }
    value /= base;
  }
  {
    size_t const size = zocleContainerCStringSize(tmp_cstr);
    if (size > 0) {
      CHAR_TYPE *ptr = tmp_cstr->end - 2;
      size_t i;
      
      for (i = 0; i < size; ++i, --ptr) {
        zocleContainerCStringAppendChar(cstr, *ptr);
      }
    }
  }
  zocleContainerCStringDelete(tmp_cstr);
  return cstr;
}

void
zocleContainerCStringVprintf(cstring cstr, char const *fmt, va_list ap) {
  fmt_stage_enum fmt_stage = FMT_STAGE_NONE;
  size_t fmt_width = 0;
  cl_int fmt_flag = 0;
  
#define LEAVE_PROCESSING_FMT_STAGE              \
  do {                                          \
    fmt_stage = FMT_STAGE_NONE;                 \
    fmt_width = 0;                              \
    fmt_flag = 0;                               \
  } while(0)
  
  for (;;) {
    int iarg;
    unsigned int uarg;
    
    if (FMT_STAGE_NONE == fmt_stage) {
      for (; (*fmt != 0) && (*fmt != '%'); ++fmt) {
        /* A normal character, add to the final string directly. */
        zocleContainerCStringAppendChar(cstr, *fmt);
      }
      if (0 == *fmt) {
        /* NULL terminator */
        break;
      } else {
        /* encounter '%' */
        fmt_stage = FMT_STAGE_FLAG;
        ++fmt; /* bypass '%' */
      }
    } else {
      if (0 == *fmt) {
        break;
      } else {
        int c;
        
        c = *fmt;
        ++fmt;
        
        switch (c) {
        case '0': 
          switch (fmt_stage) {
          case FMT_STAGE_FLAG:
            /* If the 0 and - flags both appear, the 0 flag is ignored. */
            if (0 == (fmt_flag & FMT_FLAG_MINUS)) {
              fmt_flag |= FMT_FLAG_ZERO;
            }
            break;
            
          case FMT_STAGE_WIDTH:
            fmt_width <<= 1;
            break;
            
          default:
            ASSERT(0);
            break;
          }
          break;
          
        case ' ':
          switch (fmt_stage) {
          case FMT_STAGE_FLAG:
            fmt_flag |= FMT_FLAG_SPACE;
            break;
            
          default:
            ASSERT(0);
            break;
          }
          break;
          
        case '#':
          switch (fmt_stage) {
          case FMT_STAGE_FLAG:
            fmt_flag |= FMT_FLAG_SHARP;
            break;
            
          default:
            ASSERT(0);
            break;
          }
          break;
          
        case '+':
          switch (fmt_stage) {
          case FMT_STAGE_FLAG:
            fmt_flag |= FMT_FLAG_PLUS;
            break;
            
          default:
            ASSERT(0);
            break;
          }
          break;
          
        case '-':
          switch (fmt_stage) {
          case FMT_STAGE_FLAG:
            fmt_flag |= FMT_FLAG_MINUS;
            /* If the 0 and - flags both appear, the 0 flag is ignored. */
            if (fmt_flag & FMT_FLAG_ZERO) {
              fmt_flag &= ~FMT_FLAG_ZERO;
            }
            break;
            
          default:
            ASSERT(0);
            break;
          }
          break;
          
        case '1': case '2': case '3':
        case '4': case '5': case '6':
        case '7': case '8': case '9':
          switch (fmt_stage) {
          case FMT_STAGE_FLAG:
            fmt_stage = FMT_STAGE_WIDTH;
            break;
            
          case FMT_STAGE_WIDTH:
            break;
            
          case FMT_STAGE_PRECISION:
            ASSERT(0 && "TODO");
            break;
            
          case FMT_STAGE_LENGTH:
          default:
            ASSERT(0);
            break;
          }
          fmt_width <<= 1;
          fmt_width += (c - '0');
          break;
          
        case '.':
          switch (fmt_stage) {
          case FMT_STAGE_FLAG:
          case FMT_STAGE_WIDTH:
            fmt_stage = FMT_STAGE_PRECISION;
            break;
            
          case FMT_STAGE_PRECISION:
          case FMT_STAGE_LENGTH:
          default:
            ASSERT(0);
            break;
          }
          break;
          
        case 'c':
        {
          int const ch = va_arg(ap, int);
          ASSERT((ch >= 0) && (ch <= 255));
          {
            size_t i;
            if (fmt_width > 1) {
              for (i = 0; i < (fmt_width - 1); ++i) {
                if (fmt_flag & FMT_FLAG_ZERO) {
                  zocleContainerCStringAppendChar(cstr, '0');
                } else {
                  zocleContainerCStringAppendChar(cstr, ' ');
                }
              }
            }
          }
          zocleContainerCStringAppendChar(cstr, ch);
          LEAVE_PROCESSING_FMT_STAGE;
          break;
        }
        
        case 'd': case 'i':
        {
          iarg = va_arg(ap, int);
          if (iarg < 0) {
            zocleContainerCStringAppendChar(cstr, '-');
            uarg = -iarg;
          } else {
            uarg = iarg;
          }
          goto cvt;
        }
          
        case 'x':
        case 'p':
          zocleContainerCStringAppendString(cstr, "0x");
        case 'o': case 'u': 
        {
          uarg = va_arg(ap, unsigned int);
        cvt:
          /* Why needs the following ';'?
           * Due to fix the error message:
           * "a label can only be part of a statement and a
           * declaration is not a statement" */
          ;
          {
            cstring str = convertUnsignedIntToString(
                uarg, ('o' == c) ? 8 : (('x' == c) ? 16 : 10));
            size_t i;
            if (fmt_width > zocleContainerCStringLength(str)) {
              for (i = 0; i < (fmt_width - zocleContainerCStringLength(str)); ++i) {
                if (fmt_flag & FMT_FLAG_ZERO) {
                  zocleContainerCStringAppendChar(cstr, '0');
                } else {
                  zocleContainerCStringAppendChar(cstr, ' ');
                }
              }
            }
            zocleContainerCStringAppendString(cstr, zocleContainerCStringRawData(str));
            zocleContainerCStringDelete(str);
          }
          LEAVE_PROCESSING_FMT_STAGE;
          break;
        }
          
        case 'h':
        case 'l':
        case 'L':
          {
            switch (fmt_stage) {
            case FMT_STAGE_FLAG:
            case FMT_STAGE_WIDTH:
            case FMT_STAGE_PRECISION:
              fmt_stage = FMT_STAGE_LENGTH;
              break;
              
            case FMT_STAGE_LENGTH:
            default:
              ASSERT(0);
              break;
            }
            break;
          }
        
        case 's':
        {
          char * const str = va_arg(ap, char *);
          size_t i;
          if (fmt_width > strlen(str)) {
            for (i = 0; i < (fmt_width - strlen(str)); ++i) {
              if (fmt_flag & FMT_FLAG_ZERO) {
                zocleContainerCStringAppendChar(cstr, '0');
              } else {
                zocleContainerCStringAppendChar(cstr, ' ');
              }
            }
          }
          zocleContainerCStringAppendString(cstr, str);
          LEAVE_PROCESSING_FMT_STAGE;
          break;
        }
        
        case '%':
          zocleContainerCStringAppendChar(cstr, '%');
          LEAVE_PROCESSING_FMT_STAGE;
          break;
          
        default:
          ASSERT(0);
          break;
        }
      }
    }
  }
#undef LEAVE_PROCESSING_FMT_STAGE
}
