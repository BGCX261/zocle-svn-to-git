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
#ifndef ZOCLE_COMPILER_FRONTEND_READBUFFER_H_
#define ZOCLE_COMPILER_FRONTEND_READBUFFER_H_

#include <cl.h>

#include <osal/inc/osal.h>
#include <container/inc/cstring.h>

#include <stdio.h>

typedef cl_char cl_read_buffer_char;
typedef struct _cl_compiler_read_buffer *cl_compiler_read_buffer;
struct _cl_compiler_read_buffer {
  size_t size;
  cl_read_buffer_char *buffer_begin;
  cl_read_buffer_char *buffer_end; /**< Points to the location just after
                                    * the last character of the buffer. */
  
  cl_read_buffer_char *get_ptr;
  cl_read_buffer_char *write_ptr; /**< next writable location is write_ptr */
  
  FILE *file;
  cstring filename;
  cl_int  line_num;
};

static inline cl_bool
clCompilerReadBufferSetInputFromFile(cl_compiler_read_buffer const read_buffer,
                                     char const * filename) {
  ASSERT(read_buffer != NULL);
  ASSERT(filename != NULL);
  
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
  read_buffer->file = fopen(filename, "rb");
#ifdef _MSC_VER
#pragma warning(pop)
#endif
  if (NULL == read_buffer->file) {
    return CL_FALSE;
  }
  
  if (NULL == read_buffer->filename) {
    read_buffer->filename = CSTRING_NEW();
  } else {
    CSTRING_CLEAR(read_buffer->filename);
  }
  CSTRING_APPEND_STRING(read_buffer->filename, filename);
  read_buffer->line_num = 1;
  
  return CL_TRUE;
}

static inline cl_int
clCompilerReadBufferGetCurrLineNum(cl_compiler_read_buffer const read_buffer) {
  ASSERT(read_buffer != NULL);
  return read_buffer->line_num;
}

static inline char *
clCompilerReadBufferGetFilename(cl_compiler_read_buffer const read_buffer) {
  ASSERT(read_buffer != NULL);
  return CSTRING_RAW_DATA(read_buffer->filename);
}

static inline void
clCompilerReadBufferSetFilename(
    cl_compiler_read_buffer const read_buffer, char const *filename) {
  ASSERT(read_buffer != NULL);
  ASSERT(filename != NULL);
  CSTRING_ASSIGN(read_buffer->filename, filename);
}

static inline void
clCompilerReadBufferAddContentString(
    cl_compiler_read_buffer const read_buffer, char const *data) {
  ASSERT(read_buffer != NULL);
  ASSERT(data != NULL);
  if (((read_buffer->buffer_end - read_buffer->write_ptr) > 0) &&
      ((size_t)(read_buffer->buffer_end - read_buffer->write_ptr) > strlen(data))) {
    memcpy(read_buffer->write_ptr + 1, data, strlen(data));
    read_buffer->write_ptr += strlen(data);
  } else {
    ASSERT(0);
  }
}

extern cl_bool clCompilerReadBufferCharConvertToEascii(
    cl_read_buffer_char const ch, cl_int * const result);

extern cl_bool clCompilerReadBufferCharIsSpaceChar(cl_read_buffer_char const ch);

extern cl_compiler_read_buffer clCompilerReadBufferNew(size_t const size);
extern void clCompilerReadBufferDelete(cl_compiler_read_buffer const read_buffer);

extern cl_bool clCompilerReadBufferPeekChar(
    cl_compiler_read_buffer const read_buffer,
    size_t const lookahead,
    cl_read_buffer_char * const ch);

extern cl_bool clCompilerReadBufferGetChar(
    cl_compiler_read_buffer const read_buffer,
    cl_read_buffer_char * const ch);

static inline void
clCompilerReadBufferConsumeChar(cl_compiler_read_buffer const read_buffer) {
  cl_bool const result = clCompilerReadBufferGetChar(read_buffer, NULL);
  if (CL_FALSE == result) {
    ASSERT(0);
  }
}

extern cl_bool clCompilerReadBufferHandleNewLineStray(
    cl_compiler_read_buffer const read_buffer);

extern cl_bool clCompilerReadBufferGetCharSkipNewLineStray(
    cl_compiler_read_buffer const read_buffer,
    cl_read_buffer_char * const ch);

extern cl_bool clCompilerReadBufferPeekCharSkipNewLineStray(
    cl_compiler_read_buffer const read_buffer,
    cl_read_buffer_char * const ch);

#endif
