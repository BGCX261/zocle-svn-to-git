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

#include <compiler_frontend/inc/readbuffer.h>

#include <cl.h>
#include <osal/inc/osal.h>

cl_bool
clCompilerReadBufferCharConvertToEascii(
    cl_read_buffer_char const ch,
    cl_int * const result) {
  if ((ch < 0) || (ch > 255)) {
    ASSERT(0);
    return CL_FALSE;
  } else {
    if (result != NULL) {
      *result = ch;
    }
    return CL_TRUE;
  }
}

cl_bool
clCompilerReadBufferCharIsSpaceChar(
    cl_read_buffer_char const ch) {
  return ((' ' == ch) || ('\t' == ch) || ('\v' == ch) || ('\f' == ch) || ('\r' == ch));
}

cl_compiler_read_buffer
clCompilerReadBufferNew(size_t const size) {
  cl_compiler_read_buffer read_buffer = NULL;
  
  ASSERT(size >= 1);
  
  read_buffer = CL_OSAL_CALLOC(sizeof(struct _cl_compiler_read_buffer));
  if (NULL == read_buffer) {
    return NULL;
  }
  read_buffer->buffer_begin =
    CL_OSAL_CALLOC(sizeof(cl_read_buffer_char) * (size + 1));
  if (NULL == read_buffer->buffer_begin) {
    goto fail;
  }
  read_buffer->buffer_end = read_buffer->buffer_begin + size + 1;
  read_buffer->size = size;
  read_buffer->get_ptr = read_buffer->buffer_begin;
  read_buffer->write_ptr = read_buffer->buffer_begin;
  
  read_buffer->file = NULL;
  read_buffer->filename = NULL;
  
  return read_buffer;
  
 fail:
  if (read_buffer != NULL) {
    if (read_buffer->buffer_begin != NULL) {
      CL_OSAL_FREE(read_buffer->buffer_begin);
    }
    CL_OSAL_FREE(read_buffer);
  }
  return NULL;
}

void
clCompilerReadBufferDelete(cl_compiler_read_buffer const read_buffer) {
  ASSERT(read_buffer != NULL);
  
  if (read_buffer->buffer_begin != NULL) {
    CL_OSAL_FREE(read_buffer->buffer_begin);
  }
  if (read_buffer->file != NULL) {
    fclose(read_buffer->file);
  }
  if (read_buffer->filename != NULL) {
    CSTRING_DELETE(read_buffer->filename);
  }
  CL_OSAL_FREE(read_buffer);
}

static size_t
clCompilerReadBufferGetableSize(cl_compiler_read_buffer const read_buffer) {
  ASSERT(read_buffer != NULL);                                        
  if (read_buffer->get_ptr == read_buffer->write_ptr) {            
    return 0;                                                         
  }                                                                   
  if (read_buffer->write_ptr >= read_buffer->buffer_end) {            
    read_buffer->write_ptr = read_buffer->buffer_begin;               
  }                                                                   
  if (read_buffer->write_ptr == read_buffer->get_ptr) {            
    /* empty */                                                       
    return read_buffer->size;                                         
  } else if (read_buffer->get_ptr < read_buffer->write_ptr) {      
    return read_buffer->write_ptr - read_buffer->get_ptr;          
  } else if (read_buffer->get_ptr > read_buffer->write_ptr) {      
    size_t size = read_buffer->buffer_end - read_buffer->get_ptr;  
    /* the "-1" is because "if write_ptr is equal to the peek         
     * or get_ptr, it means empty" */                                 
    size += read_buffer->write_ptr - read_buffer->buffer_begin - 1;   
    return size;                                                      
  } else {                                                            
    ASSERT(0);                                                        
    return 0;                                                         
  }                                                                   
}

static size_t
clCompilerReadBufferWritableSize(cl_compiler_read_buffer const read_buffer,
                                 size_t writable_size[2]) {
  ASSERT(read_buffer != NULL);
  
  if (read_buffer->get_ptr == read_buffer->write_ptr) {            
    /* empty */                                                       
    writable_size[0] = read_buffer->size;                             
    writable_size[1] = 0;                                             
  } else if (read_buffer->get_ptr > read_buffer->write_ptr) {      
    /* the "-1" is because "if write_ptr is eqaul to the peek         
     * or get_ptr, it means empty" */                                 
    writable_size[0] = read_buffer->get_ptr - read_buffer->write_ptr - 1; 
    writable_size[1] = 0;                                             
  } else {                                                            
    writable_size[0] = read_buffer->buffer_end - read_buffer->write_ptr; 
    /* the "-1" is because "if write_ptr is eqaul to the peek         
     * or get_ptr, it means empty" */                                 
    writable_size[1] = read_buffer->get_ptr - read_buffer->buffer_begin - 1; 
  }                                                                   
  return writable_size[0] + writable_size[1];                         
}

static void
readMoreChar(cl_compiler_read_buffer const read_buffer) {
  if (read_buffer->file != NULL) {                                     
    size_t writable_size[2];
    size_t const total_writable_size =                            
      clCompilerReadBufferWritableSize(read_buffer, writable_size);    
    int i;                                                        
    
    if (0 == total_writable_size) {
      ASSERT(0);
    }
    ASSERT(1 == sizeof(cl_read_buffer_char));                     
    
    /* TODO: handle reading unicode case */                       
    for (i = 0; i < 2; ++i) {                                     
      if (writable_size[i] != 0) {
        size_t const len = fread(read_buffer->write_ptr,               
                                 1,                               
                                 writable_size[i],                
                                 read_buffer->file);                   
        if (len < 0) {                                            
          break;                                                  
        }                                                         
        read_buffer->write_ptr += len;
        ASSERT(read_buffer->write_ptr <= read_buffer->buffer_end);
        if (read_buffer->write_ptr == read_buffer->buffer_end) {
          read_buffer->write_ptr = read_buffer->buffer_begin;
        }                                                         
      }                                                           
    }                                                             
  }                                                               
}

/**
 * @param lookahead The index of the peeked character.
 *        0 means the current character.
 */
cl_bool
clCompilerReadBufferPeekChar(cl_compiler_read_buffer const read_buffer,
                             size_t const lookahead,
                             cl_read_buffer_char * const ch) {
  ASSERT(read_buffer != NULL);
  if (clCompilerReadBufferGetableSize(read_buffer) < (lookahead + 1)) {
    readMoreChar(read_buffer);
  }
  if (clCompilerReadBufferGetableSize(read_buffer) < (lookahead + 1)) {
    return CL_FALSE;
  } else {
    if (ch != NULL) {
      cl_read_buffer_char * const peek_ptr = read_buffer->get_ptr;
      if (peek_ptr + lookahead >= read_buffer->buffer_end) {
        size_t const count_at_the_end = read_buffer->buffer_end - peek_ptr;
        *ch = read_buffer->buffer_begin[lookahead - count_at_the_end];
      } else {
        *ch = peek_ptr[lookahead];
      }
    }
    return CL_TRUE;
  }
}

cl_bool
clCompilerReadBufferGetChar(cl_compiler_read_buffer const read_buffer,
                            cl_read_buffer_char * const ch) {
  ASSERT(read_buffer != NULL);
  if (0 == clCompilerReadBufferGetableSize(read_buffer)) {
    readMoreChar(read_buffer);
  }
  if (0 == clCompilerReadBufferGetableSize(read_buffer)) {
    return CL_FALSE;
  } else {
    if (ch != NULL) {
      *ch = read_buffer->get_ptr[0];
    }
    ++read_buffer->get_ptr;
    if (read_buffer->get_ptr == read_buffer->buffer_end) {
      read_buffer->get_ptr = read_buffer->buffer_begin;
    }
    return CL_TRUE;
  }
}

static inline cl_bool
clCompilerReadBufferIsPeekCharStray(cl_compiler_read_buffer const read_buffer) {
  cl_read_buffer_char ch;
  cl_bool result;
  
  ASSERT(read_buffer != NULL);
  
  result = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
  if (CL_FALSE == result) {
    return CL_FALSE;
  } else {
    if ('\\' == ch) {
      return CL_TRUE;
    } else {
      return CL_FALSE;
    }
  }
}

/* handle '\[\r]\n' */
cl_bool
clCompilerReadBufferHandleNewLineStray(
    cl_compiler_read_buffer const read_buffer) {
  cl_read_buffer_char ch;
  cl_bool result;
  
  ASSERT(read_buffer != NULL);
  ASSERT(CL_TRUE == clCompilerReadBufferIsPeekCharStray(read_buffer));
  
  while (CL_TRUE == clCompilerReadBufferIsPeekCharStray(read_buffer)) {
    result = clCompilerReadBufferGetChar(read_buffer, &ch);
    if (CL_FALSE == result) {
      ASSERT(0);
      return CL_FALSE;
    }
    if ('\n' == ch) {
      /* UNIX variants use \n */
      ++read_buffer->line_num;
      clCompilerReadBufferConsumeChar(read_buffer);
    } else if ('\r' == ch) {
      /* Windows use \r\n */
      result = clCompilerReadBufferGetChar(read_buffer, NULL);
      if (CL_FALSE == result) {
        ASSERT(0);
        return CL_FALSE;
      }
      if (ch != '\n') {
        return CL_FALSE;
      }
      ++read_buffer->line_num;
      clCompilerReadBufferConsumeChar(read_buffer);
    } else {
      return CL_FALSE;
    }
  }
  return CL_TRUE;
}

cl_bool
clCompilerReadBufferGetCharSkipNewLineStray(
    cl_compiler_read_buffer const read_buffer,
    cl_read_buffer_char * const ch) {
  cl_read_buffer_char tmp_ch;
  cl_bool found;
  
  ASSERT(read_buffer != NULL);
  
  found = clCompilerReadBufferGetChar(read_buffer, &tmp_ch);
  ASSERT(CL_TRUE == found);
  if ('\\' == tmp_ch) {
    found = clCompilerReadBufferHandleNewLineStray(read_buffer);
    ASSERT(CL_TRUE == found);
    
    found = clCompilerReadBufferGetChar(read_buffer, &tmp_ch);
    ASSERT(CL_TRUE == found);
  }
  
  if (ch != NULL) {
    *ch = tmp_ch;
  }
  
  return found;
}

cl_bool
clCompilerReadBufferPeekCharSkipNewLineStray(
    cl_compiler_read_buffer const read_buffer,
    cl_read_buffer_char * const ch) {
  cl_read_buffer_char tmp_ch;
  cl_bool found;
  
  ASSERT(read_buffer != NULL);
  
  found = clCompilerReadBufferPeekChar(read_buffer, 0, &tmp_ch);
  ASSERT(CL_TRUE == found);
  if ('\\' == tmp_ch) {
    found = clCompilerReadBufferHandleNewLineStray(read_buffer);
    ASSERT(CL_TRUE == found);
    
    found = clCompilerReadBufferPeekChar(read_buffer, 0, &tmp_ch);
    ASSERT(CL_TRUE == found);
  }
  
  if (ch != NULL) {
    *ch = tmp_ch;
  }
  
  return found;
}
