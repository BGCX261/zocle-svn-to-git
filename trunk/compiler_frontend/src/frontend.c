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

#include <cl.h>

#include <compiler_frontend/inc/ast_node.h>
#include <compiler_frontend/inc/grammar.h>
#include <compiler_frontend/inc/frontend.h>
#include <compiler_frontend/inc/preprocess.h>

#include <compiler/inc/error_msg.h>

#include <container/inc/hashtable.h>
#include <container/inc/cvector.h>
#include <container/inc/clist.h>
#include <container/inc/cstring.h>
#include <osal/inc/osal.h>
#include <util/inc/math.h>
#include <util/inc/char.h>

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>

#ifndef DECLARE_CVECTOR_TYPE_FOR_CL_COMPILER_TOKEN_TYPE
#define DECLARE_CVECTOR_TYPE_FOR_CL_COMPILER_TOKEN_TYPE
DECLARE_CVECTOR_TYPE(cl_compiler_token_type)
#endif
#ifndef DEFINE_CVECTOR_TYPE_FOR_CL_COMPILER_TOKEN_TYPE
#define DEFINE_CVECTOR_TYPE_FOR_CL_COMPILER_TOKEN_TYPE
DEFINE_CVECTOR_TYPE(cl_compiler_token_type)
#endif

/* true if isId(c) || isDecimalDigit(c) */
static cl_bool easciiIsIdOrDecimalDigitTable[256];

static inline cl_bool
clCompilerFrontendEasciiIsIdOrDecimalDigit(cl_int const ch) {
  ASSERT((ch >= 0) && (ch <= 255));
  return easciiIsIdOrDecimalDigitTable[ch];
}

static inline cl_bool
isId(cl_int const ch) {
  if ((CL_TRUE == clUtilCharIsUpperCaseLetter(ch)) ||
      (CL_TRUE == clUtilCharIsLowerCaseLetter(ch)) ||
      ('_' == ch)) {
    return CL_TRUE;
  }
  return CL_FALSE;
}

/* init isid table */
static void
clCompilerFrontendInitEasciiIsIdOrDecimalDigitTable(void) {
  cl_int i;
  for (i = 0; i < 256; ++i) {
    easciiIsIdOrDecimalDigitTable[i] = isId(i) || clUtilCharIsDecimalDigit(i);
  }
}

cl_bool
isReadBufferCharEasciiIdOrDecimalDigit(cl_read_buffer_char const ch) {
  cl_int result;
  cl_bool const success = clCompilerReadBufferCharConvertToEascii(ch, &result);
  if (CL_FALSE == success) {
    return CL_FALSE;
  }
  
  return clCompilerFrontendEasciiIsIdOrDecimalDigit(result);
}

void
clCompilerFrontendInit(void) {
  clCompilerFrontendInitEasciiIsIdOrDecimalDigitTable();
}

cl_compiler_frontend
clCompilerFrontendNew(void) {
  cl_compiler_frontend frontend = CL_OSAL_CALLOC(sizeof(struct _cl_compiler_frontend));
  ASSERT(frontend != NULL);
  
  frontend->parse_flags = 0;
  frontend->token_flags = 0;
  frontend->token = NULL;
  
  {
    frontend->token_table = HASHTABLE_NEW(cl_compiler_token)(100);
    clCompilerTokenHashTableSetDefaultFunction(frontend->token_table);
  }
  
  frontend->all_tokens = CLIST_ONE_LEVEL_NEW(cl_compiler_token, all_tokens)();
  frontend->include_file_stack = CSTACK_NEW(cl_compiler_read_buffer)(0);
  frontend->read_buffer = NULL;
  frontend->unpreprocessed_token_list = CLIST_NEW(_cl_compiler_token_type_pair)();
  frontend->preprocessed_token_list = CLIST_NEW(_cl_compiler_token_type_pair)();
  frontend->defined_token_stack = CSTACK_NEW(cl_compiler_token)(0);
  frontend->ifdef_stack = CSTACK_NEW(cl_int)(0);
  frontend->unparsed_token_list = CLIST_NEW(_cl_compiler_token_type_pair)();
  frontend->choice_point_token_list = CVECTOR_NEW(CLIST_TYPE(_cl_compiler_token_type_pair))(0);
  
  return frontend;
}

void
clCompilerFrontendDelete(cl_compiler_frontend frontend) {
  ASSERT(frontend != NULL);
  
  HASHTABLE_DELETE(cl_compiler_token)(frontend->token_table);
  
  {
    CVECTOR_ITER_TYPE(cl_compiler_read_buffer) iter;
    for (iter = CVECTOR_BEGIN(cl_compiler_read_buffer)(frontend->include_file_stack);
         iter != CVECTOR_END(cl_compiler_read_buffer)(frontend->include_file_stack);
         iter = CVECTOR_ITER_INCREMENT(cl_compiler_read_buffer)(iter)) {
      clCompilerReadBufferDelete(*CVECTOR_ITER_GET_DATA(cl_compiler_read_buffer)(iter));
    }
    CVECTOR_DELETE(cl_compiler_read_buffer)(frontend->include_file_stack);
    
    if (frontend->read_buffer != NULL) {
      clCompilerReadBufferDelete(frontend->read_buffer);
    }
  }
  
  CLIST_DELETE(_cl_compiler_token_type_pair)(frontend->unpreprocessed_token_list);
  CLIST_DELETE(_cl_compiler_token_type_pair)(frontend->preprocessed_token_list);
  CLIST_DELETE(_cl_compiler_token_type_pair)(frontend->unparsed_token_list);
  
  CSTACK_DELETE(cl_compiler_token)(frontend->defined_token_stack);
  
  CSTACK_DELETE(cl_int)(frontend->ifdef_stack);
  
  {
    CVECTOR_ITER_TYPE(CLIST_TYPE(_cl_compiler_token_type_pair)) outer_iter;
    for (outer_iter = CVECTOR_BEGIN(CLIST_TYPE(_cl_compiler_token_type_pair))(frontend->choice_point_token_list);
         outer_iter != CVECTOR_END(CLIST_TYPE(_cl_compiler_token_type_pair))(frontend->choice_point_token_list);
         outer_iter = CVECTOR_ITER_INCREMENT(CLIST_TYPE(_cl_compiler_token_type_pair))(outer_iter)) {
      CLIST_TYPE(_cl_compiler_token_type_pair) inner =
        *CVECTOR_ITER_GET_DATA(CLIST_TYPE(_cl_compiler_token_type_pair))(outer_iter);
      
      CLIST_DELETE(_cl_compiler_token_type_pair)(inner);
    }
    CVECTOR_DELETE(CLIST_TYPE(_cl_compiler_token_type_pair))(frontend->choice_point_token_list);
  }
  
  {
    CLIST_ONE_LEVEL_ITER_TYPE(cl_compiler_token, all_tokens) iter = 
      CLIST_ONE_LEVEL_BEGIN(cl_compiler_token, all_tokens)(frontend->all_tokens);
    while (iter != CLIST_ONE_LEVEL_END(cl_compiler_token, all_tokens)(frontend->all_tokens)) {
      cl_compiler_token token = *CLIST_ONE_LEVEL_ITER_GET_DATA(cl_compiler_token, all_tokens)(iter);
      ASSERT(1 == clCompilerTokenGetReferenceCount(token));
      
      iter = CLIST_ONE_LEVEL_ITER_INCREMENT(cl_compiler_token, all_tokens)(iter);
      clCompilerTokenDelete(frontend, token);
    }
    CLIST_ONE_LEVEL_DELETE(cl_compiler_token, all_tokens)(frontend->all_tokens);
  }
  
  CL_OSAL_FREE(frontend);
}

/* find a token and add it if not found */
cl_compiler_token
clCompilerFrontendTokenFindAndAllocate(cl_compiler_frontend frontend,
                                       cl_compiler_token_type const token_type,
                                       char const *str) {
  cl_compiler_token result =
    clCompilerTokenHashTableSearchByStr(frontend->token_table, str);
  if (result != NULL) {
    return result;
  } else {
    cl_compiler_token token = clCompilerTokenNew(frontend, token_type, str);
    /* add in hash table */
    HASHTABLE_ADD(cl_compiler_token)(frontend->token_table, &token);
    return token;
  }
}

static void
clCompilerFrontendParseIdentifier(cl_compiler_frontend frontend) {
  cstring cstr = CSTRING_NEW();
  ASSERT(cstr != NULL);
  
  {
    cl_bool found;
    cl_read_buffer_char ch;
    
    cl_compiler_read_buffer const read_buffer = frontend->read_buffer;
    ASSERT(read_buffer != NULL);
    
    found = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
    ASSERT(CL_TRUE == found);
    
    while (CL_TRUE == isReadBufferCharEasciiIdOrDecimalDigit(ch)) {
      CSTRING_APPEND_CHAR(cstr, ch);
      
      clCompilerReadBufferConsumeChar(read_buffer);
      
      found = clCompilerReadBufferPeekCharSkipNewLineStray(read_buffer, &ch);
      ASSERT(CL_TRUE == found);
    }
  }
  
  /* check if it is special identifier token */
  {
    frontend->token_type =
      clCompilerTokenIsSpecialIdentifierToken(CSTRING_RAW_DATA(cstr));
    if (TOKEN_TYPE_IDENTIFIER == frontend->token_type) {
      frontend->token =
        clCompilerFrontendTokenFindAndAllocate(frontend, TOKEN_TYPE_IDENTIFIER, CSTRING_RAW_DATA(cstr));
    }
  }
  
  CSTRING_DELETE(cstr);
}

static cl_compiler_error_code
clCompilerFrontendGetNextTokenWithoutMacroSubstFromReadBuffer(
    cl_compiler_frontend frontend) {
  cl_compiler_read_buffer read_buffer;
  cl_bool keep_token_flags = CL_FALSE;
  
  ASSERT(frontend != NULL);
  
  read_buffer = frontend->read_buffer;
  ASSERT(read_buffer != NULL);
  
  frontend->token_type = TOKEN_TYPE_MEANINGLESS;
  frontend->token = NULL;
  
  while (TOKEN_TYPE_MEANINGLESS == frontend->token_type) {
    cl_read_buffer_char ch;
    cl_bool found_a_char = CL_FALSE;
    
    while (CL_FALSE == found_a_char) {
      found_a_char = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
      if (CL_FALSE == found_a_char) {
        if (0 == CSTACK_SIZE(cl_compiler_read_buffer)(
                frontend->include_file_stack)) {
          /* no include left : end of file. */
          frontend->token_type = TOKEN_TYPE_EOF;
          
          found_a_char = CL_TRUE;
        } else {
          /* There is an included file left, pop include file */
          clCompilerReadBufferDelete(read_buffer);
          frontend->read_buffer = *(CSTACK_POP(cl_compiler_read_buffer)(
                                        frontend->include_file_stack));
          read_buffer = frontend->read_buffer;
        }
      }
    }
    
    if (frontend->token_type != TOKEN_TYPE_MEANINGLESS) {
      break;
    }
    
    switch (ch) {
    case ' ':  /* white space */
    case '\t': /* horizontal tab */
      clCompilerReadBufferConsumeChar(read_buffer);
      frontend->token_type = TOKEN_TYPE_WHITE_SPACE;
      keep_token_flags = CL_TRUE;
      break;
      
    case '\f': /* form feed */
    case '\v': /* vertical tab */
    case '\r': /* carriage return */
      clCompilerReadBufferConsumeChar(read_buffer);
      break;
      
    case '\\': /* \ */
      if (CL_FALSE == clCompilerReadBufferHandleNewLineStray(read_buffer)) {
        ASSERT(0);
      }
      break;
      
    case '\n':
      clCompilerReadBufferConsumeChar(read_buffer);
      ++read_buffer->line_num;
      frontend->token_flags |= TOKEN_FLAG_BEGIN_OF_LINE;
      if (0 == (frontend->parse_flags & PARSE_FLAG_CAN_RETURN_LINEFEED_TOKEN)) {
        frontend->token_type = TOKEN_TYPE_MEANINGLESS;
      } else {
        frontend->token_type = TOKEN_TYPE_LINEFEED;
        keep_token_flags = CL_TRUE;
      }
      break;
      
    case '#':
      clCompilerReadBufferConsumeChar(read_buffer);
      if (frontend->token_flags & TOKEN_FLAG_BEGIN_OF_LINE) {
        cl_compiler_error_code error_code;
        error_code = clCompilerFrontendPreprocess(frontend);
        if (error_code != COMPILER_NO_ERROR) {
          return error_code;
        }
        /* redo the get token process */
        frontend->token_type = TOKEN_TYPE_MEANINGLESS;
        /* because this function will get the next token from the read buffer,
         * there is no reason that there is remainding tokens in the 'preprocessed_token_list'
         * & 'unpreprocessed_token_list' */
        ASSERT(0 == CLIST_SIZE(_cl_compiler_token_type_pair)(frontend->preprocessed_token_list));
        ASSERT(0 == CLIST_SIZE(_cl_compiler_token_type_pair)(frontend->unpreprocessed_token_list));
      } else {
        found_a_char = clCompilerReadBufferPeekCharSkipNewLineStray(read_buffer, &ch);
        ASSERT(CL_TRUE == found_a_char);
        if ('#' == ch) {
          clCompilerReadBufferConsumeChar(read_buffer);
          frontend->token_type = TOKEN_TYPE_TWOSHARPS;
        } else {
          frontend->token_type = TOKEN_TYPE_SHARP;
        }
      }
      break;
      
      /* lowercase */
    case 'a': case 'b': case 'c': case 'd':
    case 'e': case 'f': case 'g': case 'h':
    case 'i': case 'j': case 'k': case 'l':
    case 'm': case 'n': case 'o': case 'p':
    case 'q': case 'r': case 's': case 't':
    case 'u': case 'v': case 'w': case 'x':
    case 'y': case 'z': 
      /* uppercase */
    case 'A': case 'B': case 'C': case 'D':
    case 'E': case 'F': case 'G': case 'H':
    case 'I': case 'J': case 'K': 
    case 'M': case 'N': case 'O': case 'P':
    case 'Q': case 'R': case 'S': case 'T':
    case 'U': case 'V': case 'W': case 'X':
    case 'Y': case 'Z': 
      /* underscore */
    case '_':
      clCompilerFrontendParseIdentifier(frontend);
      break;
      
    case 'L':
    {
      found_a_char = clCompilerReadBufferPeekChar(read_buffer, 1, &ch);
      ASSERT(CL_TRUE == found_a_char);
      if ((ch != '\\') &&
          /* Below means the L is _NOT_ followed by a character or a string. */
          (ch != '\'') &&
          (ch != '\"')) {
        clCompilerFrontendParseIdentifier(frontend);
      } else {
        if (('\'' == ch) || ('\"' == ch)) {
          clCompilerFrontendParseString(frontend, ch, CL_TRUE);
        } else {
          clCompilerFrontendParseIdentifier(frontend);
        }
      }
    }
    break;
    
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
    case '8': case '9':
      clCompilerFrontendParsePreprocessorNumber(frontend, CL_FALSE);
      break;
      
    case '.':
      /* special dot handling because it can also start a number */
      clCompilerReadBufferConsumeChar(read_buffer);
      found_a_char = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
      ASSERT(CL_TRUE == found_a_char);
      if (CL_TRUE == clUtilCharIsDecimalDigit(
              clCompilerReadBufferCharConvertToEascii(ch, NULL))) {
        clCompilerFrontendParsePreprocessorNumber(frontend, CL_TRUE);
      } else if ('.' == ch) {
        clCompilerReadBufferConsumeChar(read_buffer);
        found_a_char = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
        ASSERT(CL_TRUE == found_a_char);
        if (ch != '.') {
          ASSERT(0 && "expect '.'");
        }
        clCompilerReadBufferConsumeChar(read_buffer);
        frontend->token_type = TOKEN_TYPE_DOTS;
      } else {
        frontend->token_type = TOKEN_TYPE_DOT;
      }
      break;
      
    case '<':
      clCompilerReadBufferConsumeChar(read_buffer);
      found_a_char = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
      ASSERT(CL_TRUE == found_a_char);
      if ('=' == ch) {
        clCompilerReadBufferConsumeChar(read_buffer);
        frontend->token_type = TOKEN_TYPE_LESS_EQUAL;
      } else if ('<' == ch) {
        clCompilerReadBufferConsumeChar(read_buffer);
        found_a_char = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
        ASSERT(CL_TRUE == found_a_char);
        if ('=' == ch) {
          clCompilerReadBufferConsumeChar(read_buffer);
          frontend->token_type = TOKEN_TYPE_ASSIGN_LEFT_SHIFT;
        } else {
          frontend->token_type = TOKEN_TYPE_LEFT_SHIFT;
        }
      } else {
        frontend->token_type = TOKEN_TYPE_LESS_THAN;
      }
      break;
      
    case '>':
      clCompilerReadBufferConsumeChar(read_buffer);
      found_a_char = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
      ASSERT(CL_TRUE == found_a_char);
      if ('=' == ch) {
        clCompilerReadBufferConsumeChar(read_buffer);
        frontend->token_type = TOKEN_TYPE_GREATER_EQUAL;
      } else if ('>' == ch) {
        clCompilerReadBufferConsumeChar(read_buffer);
        found_a_char = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
        ASSERT(CL_TRUE == found_a_char);
        if ('=' == ch) {
          clCompilerReadBufferConsumeChar(read_buffer);
          frontend->token_type = TOKEN_TYPE_ASSIGN_RIGHT_SHIFT;
        } else {
          frontend->token_type = TOKEN_TYPE_RIGHT_SHIFT;
        }
      } else {
        frontend->token_type = TOKEN_TYPE_GREATER_THAN;
      }
      break;
      
    case '&':
      clCompilerReadBufferConsumeChar(read_buffer);
      found_a_char = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
      ASSERT(CL_TRUE == found_a_char);
      if ('&' == ch) {
        clCompilerReadBufferConsumeChar(read_buffer);
        frontend->token_type = TOKEN_TYPE_LOGICAL_AND;
      } else if ('=' == ch) {
        clCompilerReadBufferConsumeChar(read_buffer);
        frontend->token_type = TOKEN_TYPE_ASSIGN_AND;
      } else {
        frontend->token_type = TOKEN_TYPE_BITWISE_AND;
      }
      break;
      
    case '|':
      clCompilerReadBufferConsumeChar(read_buffer);
      found_a_char = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
      ASSERT(CL_TRUE == found_a_char);
      if ('|' == ch) {
        clCompilerReadBufferConsumeChar(read_buffer);
        frontend->token_type = TOKEN_TYPE_LOGICAL_OR;
      } else if ('=' == ch) {
        clCompilerReadBufferConsumeChar(read_buffer);
        frontend->token_type = TOKEN_TYPE_ASSIGN_OR;
      } else {
        frontend->token_type = TOKEN_TYPE_BITWISE_OR;
      }
      break;
      
    case '+':
      clCompilerReadBufferConsumeChar(read_buffer);
      found_a_char = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
      ASSERT(CL_TRUE == found_a_char);
      if ('+' == ch) {
        clCompilerReadBufferConsumeChar(read_buffer);
        frontend->token_type = TOKEN_TYPE_INCREMENT;
      } else if ('=' == ch) {
        clCompilerReadBufferConsumeChar(read_buffer);
        frontend->token_type = TOKEN_TYPE_ASSIGN_ADDITION;
      } else {
        frontend->token_type = TOKEN_TYPE_ADDITION;
      }
      break;
      
    case '-':
      clCompilerReadBufferConsumeChar(read_buffer);
      found_a_char = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
      ASSERT(CL_TRUE == found_a_char);
      if ('-' == ch) {
        clCompilerReadBufferConsumeChar(read_buffer);
        frontend->token_type = TOKEN_TYPE_DECREMENT;
      } else if ('=' == ch) {
        clCompilerReadBufferConsumeChar(read_buffer);
        frontend->token_type = TOKEN_TYPE_ASSIGN_SUBTRACTION;
      } else if ('>' == ch) {
        clCompilerReadBufferConsumeChar(read_buffer);
        frontend->token_type = TOKEN_TYPE_ARROW;
      } else {
        frontend->token_type = TOKEN_TYPE_SUBTRACTION;
      }
      break;
      
      /* comments or operator */
    case '/':
      clCompilerReadBufferConsumeChar(read_buffer);
      found_a_char = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
      ASSERT(CL_TRUE == found_a_char);
      if ('*' == ch) {
        clCompilerFrontendParseComment(frontend);
        continue;
      } else if ('/' == ch) {
        clCompilerFrontendParseLineComment(frontend);
        continue;
      } else if ('=' == ch) {
        clCompilerReadBufferConsumeChar(read_buffer);
        frontend->token_type = TOKEN_TYPE_ASSIGN_DIVISION;
      } else {
        frontend->token_type = TOKEN_TYPE_SLASH;
      }
      break;
      
      /* simple tokens */
    case '(':
      frontend->token_type = TOKEN_TYPE_LEFT_PAREN;
      clCompilerReadBufferConsumeChar(read_buffer);
      break;
      
    case ')':
      frontend->token_type = TOKEN_TYPE_RIGHT_PAREN;
      clCompilerReadBufferConsumeChar(read_buffer);
      break;
      
    case '[':
      frontend->token_type = TOKEN_TYPE_LEFT_BRACKET;
      clCompilerReadBufferConsumeChar(read_buffer);
      break;
      
    case ']':
      frontend->token_type = TOKEN_TYPE_RIGHT_BRACKET;
      clCompilerReadBufferConsumeChar(read_buffer);
      break;
      
    case '{':
      frontend->token_type = TOKEN_TYPE_LEFT_BRACE;
      clCompilerReadBufferConsumeChar(read_buffer);
      break;
      
    case '}':
      frontend->token_type = TOKEN_TYPE_RIGHT_BRACE;
      clCompilerReadBufferConsumeChar(read_buffer);
      break;
      
    case ',':
      frontend->token_type = TOKEN_TYPE_COMMA;
      clCompilerReadBufferConsumeChar(read_buffer);
      break;
      
    case ';':
      frontend->token_type = TOKEN_TYPE_SEMICOLON;
      clCompilerReadBufferConsumeChar(read_buffer);
      break;
      
    case ':':
      frontend->token_type = TOKEN_TYPE_COLON;
      clCompilerReadBufferConsumeChar(read_buffer);
      break;
      
    case '?':
      frontend->token_type = TOKEN_TYPE_QUESTION_MARK;
      clCompilerReadBufferConsumeChar(read_buffer);
      break;
      
    case '~':
      frontend->token_type = TOKEN_TYPE_TILDE;
      clCompilerReadBufferConsumeChar(read_buffer);
      break;
      
    case '!':
      clCompilerReadBufferConsumeChar(read_buffer);
      found_a_char = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
      ASSERT(CL_TRUE == found_a_char);
      if ('=' == ch) {
        frontend->token_type = TOKEN_TYPE_NOT_EQUAL;
        clCompilerReadBufferConsumeChar(read_buffer);
      } else {
        frontend->token_type = TOKEN_TYPE_EXCLAMATION_MARK;
      }
      break;
      
    case '=':
      clCompilerReadBufferConsumeChar(read_buffer);
      found_a_char = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
      ASSERT(CL_TRUE == found_a_char);
      if ('=' == ch) {
        frontend->token_type = TOKEN_TYPE_EQUAL;
        clCompilerReadBufferConsumeChar(read_buffer);
      } else {
        frontend->token_type = TOKEN_TYPE_ASSIGN;
      }
      break;
      
    case '*':
      clCompilerReadBufferConsumeChar(read_buffer);
      found_a_char = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
      ASSERT(CL_TRUE == found_a_char);
      if ('=' == ch) {
        frontend->token_type = TOKEN_TYPE_ASSIGN_MULTIPLY;
        clCompilerReadBufferConsumeChar(read_buffer);
      } else {
        frontend->token_type = TOKEN_TYPE_MULTIPLY;
      }
      break;
      
    case '%': 
      clCompilerReadBufferConsumeChar(read_buffer);
      found_a_char = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
      ASSERT(CL_TRUE == found_a_char);
      if ('=' == ch) {
        frontend->token_type = TOKEN_TYPE_ASSIGN_MODULO;
        clCompilerReadBufferConsumeChar(read_buffer);
      } else {
        frontend->token_type = TOKEN_TYPE_MODULO;
      }
      break;
      
    case '^':
      clCompilerReadBufferConsumeChar(read_buffer);
      found_a_char = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
      ASSERT(CL_TRUE == found_a_char);
      if ('=' == ch) {
        frontend->token_type = TOKEN_TYPE_ASSIGN_XOR;
        clCompilerReadBufferConsumeChar(read_buffer);
      } else {
        frontend->token_type = TOKEN_TYPE_BITWISE_XOR;
      }
      break;
      
    case '\'':
    case '\"':
      {
        clCompilerFrontendParseString(frontend, ch, CL_FALSE);
        break;
      }
    
    default:
      ASSERT(0 && "unrecognized character");
      break;
    }
  }
  
  if (CL_FALSE == keep_token_flags) {
    frontend->token_flags = 0;
  }
  return COMPILER_NO_ERROR;
}

/* This function takes care of all issues corresponding to
 * getting next token with macro substitution. */
cl_compiler_error_code
clCompilerFrontendGetNextTokenWithoutMacroSubstImpl(
    cl_compiler_frontend frontend,
    cl_compiler_can_return_space_token const can_return_space_token,
    cl_bool const is_get,
    size_t const lookahead_idx,
    cl_bool * const is_token_preprocessed) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair *token_type_pair = NULL;
  
  ASSERT(frontend != NULL);
  
  if ((CL_TRUE == is_get) ||
      /* In the peeking mode, if 'can_not_return_space_token',
       * then this function will actually remove the space tokens if it peeks them,
       * and because I only allow to remove tokens from the frontest position,
       * hence we have the following assertion condition. */
      (CAN_NOT_RETURN_SPACE_TOKEN == can_return_space_token)) {
    /* we can only remove the frontest one. */
    ASSERT(0 == lookahead_idx);
  }
  
  /* set to the initial value. */
  {
    frontend->token_type = TOKEN_TYPE_MEANINGLESS;
    frontend->token = NULL;
  }
  
  /* check if we have already found one. */
  while (TOKEN_TYPE_MEANINGLESS == frontend->token_type) {
    /* check if we can get/peek from the preprocessed_token_list */
    if (CLIST_SIZE(_cl_compiler_token_type_pair)(frontend->preprocessed_token_list) > lookahead_idx) {
      if (0 == lookahead_idx) {
        token_type_pair = CLIST_FRONT(_cl_compiler_token_type_pair)(frontend->preprocessed_token_list);
      } else {
        token_type_pair = CLIST_AT(_cl_compiler_token_type_pair)(frontend->preprocessed_token_list, lookahead_idx);
      }
      
      frontend->token_type = token_type_pair->token_type;
      frontend->token = token_type_pair->token;
      
      if (CL_TRUE == is_get) {
        /* we can only remove the frontest one. */
        CLIST_POP_FRONT(_cl_compiler_token_type_pair)(frontend->preprocessed_token_list);
      } else {
        /* In the peeking mode, remove the white space if encounter one */
        if ((CAN_NOT_RETURN_SPACE_TOKEN == can_return_space_token) &&
            (TOKEN_TYPE_WHITE_SPACE == frontend->token_type)) {
          /* we can only remove the frontest one. */
          CLIST_POP_FRONT(_cl_compiler_token_type_pair)(frontend->preprocessed_token_list);
          
          /* re-get/peek the next token */
          frontend->token_type = TOKEN_TYPE_MEANINGLESS;
          frontend->token = NULL;
        }
      }
      
      if (is_token_preprocessed != NULL) {
        (*is_token_preprocessed) = CL_TRUE;
      }
    } else {
      /* find an unpreprocessed token */
      /* check the unpreprocessed_token_list first */
      if (CLIST_SIZE(_cl_compiler_token_type_pair)(frontend->unpreprocessed_token_list) > lookahead_idx) {
        if (0 == lookahead_idx) {
          token_type_pair = CLIST_FRONT(_cl_compiler_token_type_pair)(frontend->unpreprocessed_token_list);
        } else {
          token_type_pair = CLIST_AT(_cl_compiler_token_type_pair)(frontend->unpreprocessed_token_list, lookahead_idx);
        }
        
        frontend->token_type = token_type_pair->token_type;
        frontend->token = token_type_pair->token;
        
        if ((CL_TRUE == is_get) ||
            /* In the peeking mode, remove the white space if encounter one */
            ((CAN_NOT_RETURN_SPACE_TOKEN == can_return_space_token) &&
             (TOKEN_TYPE_WHITE_SPACE == frontend->token_type))) {
          /* we can only remove the frontest one. */
          CLIST_POP_FRONT(_cl_compiler_token_type_pair)(frontend->unpreprocessed_token_list);
          
          /* re-get/peek the next token */
          frontend->token_type = TOKEN_TYPE_MEANINGLESS;
          frontend->token = NULL;
        }
      } else {
        size_t i;
        /* calculate the tokens this function needs to read from the read buffer */
        size_t const diff =
          (lookahead_idx - CLIST_SIZE(_cl_compiler_token_type_pair)(frontend->unpreprocessed_token_list) + 1);
        
        ASSERT(frontend->read_buffer != NULL);
        
        if (CL_TRUE == is_get) {
          error_code = clCompilerFrontendGetNextTokenWithoutMacroSubstFromReadBuffer(frontend);
          if (error_code != COMPILER_NO_ERROR) {
            goto finish;
          }
          if ((CAN_NOT_RETURN_SPACE_TOKEN == can_return_space_token) &&
              (TOKEN_TYPE_WHITE_SPACE == frontend->token_type)) {
            /* re-get/peek the next token */
            frontend->token_type = TOKEN_TYPE_MEANINGLESS;
            frontend->token = NULL;
          }
        } else {
          for (i = 0; i < diff;) {
            error_code = clCompilerFrontendGetNextTokenWithoutMacroSubstFromReadBuffer(frontend);
            if (error_code != COMPILER_NO_ERROR) {
              goto finish;
            }
            /* In the peeking mode, doesn't need to add the white space token if we don't
             * want it */
            if ((CAN_RETURN_SPACE_TOKEN == can_return_space_token) ||
                (frontend->token_type != TOKEN_TYPE_WHITE_SPACE)) {
              /* add this new token into the 'unpreprocessed_token_list' */
              _cl_compiler_token_type_pair e = { frontend->token_type, frontend->token };
              CLIST_PUSH_BACK(_cl_compiler_token_type_pair)(frontend->unpreprocessed_token_list, &e);
              ++i;
            }
          }
        }
      }
      
      if (is_token_preprocessed != NULL) {
        (*is_token_preprocessed) = CL_FALSE;
      }
    }
  }
  
 finish:
  return error_code;
}

static cl_compiler_error_code
clCompilerFrontendPeekNextTokenWithoutMacroSubst(
    cl_compiler_frontend frontend,
    cl_compiler_can_return_space_token const can_return_space_token,
    size_t const lookahead_idx,
    cl_bool * const is_token_preprocessed) {
  return clCompilerFrontendGetNextTokenWithoutMacroSubstImpl(
      frontend,
      can_return_space_token,
      CL_FALSE,
      lookahead_idx,
      is_token_preprocessed);
}

static cl_compiler_error_code
clCompilerFrontendConsumeNextTokenWithoutMacroSubst(
    cl_compiler_frontend frontend) {
  return clCompilerFrontendGetNextTokenWithoutMacroSubstImpl(
      frontend,
      CL_TRUE,
      CL_TRUE,
      0,
      NULL);
}

void
clCompilerFrontendEnterBackTracking(cl_compiler_frontend frontend) {
  CLIST_TYPE(_cl_compiler_token_type_pair) list =
    CLIST_NEW(_cl_compiler_token_type_pair)();
  
  ASSERT(frontend != NULL);
  
  CVECTOR_PUSH_BACK(CLIST_TYPE(_cl_compiler_token_type_pair))(
      frontend->choice_point_token_list, &list);
}

void
clCompilerFrontendLeaveBackTracking(cl_compiler_frontend frontend) {
  ASSERT(frontend != NULL);
  
  CLIST_SPLICE(_cl_compiler_token_type_pair)(
      frontend->unparsed_token_list,
      CLIST_BEGIN(_cl_compiler_token_type_pair)(frontend->unparsed_token_list),
      *(CVECTOR_BACK(CLIST_TYPE(_cl_compiler_token_type_pair))(
            frontend->choice_point_token_list)));
  
  {
    CLIST_TYPE(_cl_compiler_token_type_pair) bt_list =
      *(CVECTOR_BACK(CLIST_TYPE(_cl_compiler_token_type_pair))(
            frontend->choice_point_token_list));
    
    CLIST_DELETE(_cl_compiler_token_type_pair)(bt_list);
  }
  
  CVECTOR_POP_BACK(CLIST_TYPE(_cl_compiler_token_type_pair))(
      frontend->choice_point_token_list);
}

cl_compiler_error_code
clCompilerFrontendGetNextUnparsedToken(
    cl_compiler_frontend frontend,
    _cl_compiler_token_type_pair * const token_type_pair) {
  _cl_compiler_token_type_pair local_token_type_pair;
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  
  ASSERT(frontend != NULL);
  
  if (CLIST_SIZE(_cl_compiler_token_type_pair)(frontend->unparsed_token_list) != 0) {
    _cl_compiler_token_type_pair * const e =
      CLIST_FRONT(_cl_compiler_token_type_pair)(frontend->unparsed_token_list);
    ASSERT(e != NULL);
    
    local_token_type_pair = *e;
    
    CLIST_POP_FRONT(_cl_compiler_token_type_pair)(frontend->unparsed_token_list);
  } else {
    error_code = clCompilerFrontendGetNextTokenWithMacroSubst(frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    
    local_token_type_pair.token_type = frontend->token_type;
    local_token_type_pair.token = frontend->token;
  }
  
  if (CVECTOR_SIZE(CLIST_TYPE(_cl_compiler_token_type_pair))(
          frontend->choice_point_token_list) != 0) {
    /* If the size of 'choice_point_token_list' is not 0, it means
     * we are enabling the backtracking mechanism.
     * We have to record/remember the tokens we have processed. */
    CLIST_PUSH_BACK(_cl_compiler_token_type_pair)(
        *(CVECTOR_BACK(CLIST_TYPE(_cl_compiler_token_type_pair))(
              frontend->choice_point_token_list)),
        &local_token_type_pair);
  }
  
  if (token_type_pair != NULL) {
    *token_type_pair = local_token_type_pair;
  }
  
 finish:
  return error_code;
}

cl_compiler_error_code
clCompilerFrontendPeekNextUnparsedToken(
    cl_compiler_frontend frontend,
    size_t const lookahead,
    _cl_compiler_token_type_pair * const token_type_pair) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  size_t unparsed_token_size;
  
  ASSERT(frontend != NULL);
  ASSERT(token_type_pair != NULL);
  
  unparsed_token_size = CLIST_SIZE(_cl_compiler_token_type_pair)(frontend->unparsed_token_list);
  
  if (unparsed_token_size > lookahead) {
    _cl_compiler_token_type_pair * const e =
      CLIST_AT(_cl_compiler_token_type_pair)(frontend->unparsed_token_list, lookahead);
    ASSERT(e != NULL);
    
    *token_type_pair = *e;
  } else {
    size_t tokens_need_to_fetch = lookahead - unparsed_token_size + 1;
    size_t i;
    
    for (i = 0; i < tokens_need_to_fetch; ++i) {
      error_code = clCompilerFrontendGetNextTokenWithMacroSubst(frontend);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      
      {
        _cl_compiler_token_type_pair e = { frontend->token_type, frontend->token };
        CLIST_PUSH_BACK(_cl_compiler_token_type_pair)(frontend->unparsed_token_list, &e);
      }
    }
    
    token_type_pair->token_type = CLIST_BACK(_cl_compiler_token_type_pair)(
        frontend->unparsed_token_list)->token_type;
    
    token_type_pair->token = CLIST_BACK(_cl_compiler_token_type_pair)(
        frontend->unparsed_token_list)->token;
  }
  
 finish:
  return error_code;
}

cl_compiler_error_code
clCompilerFrontendConsumeNextUnparsedToken(cl_compiler_frontend frontend) {
  ASSERT(frontend != NULL);
  return clCompilerFrontendGetNextUnparsedToken(frontend, NULL);
}

cl_compiler_error_code
clCompilerFrontendEnsureNextUnparsedToken(
    cl_compiler_frontend frontend,
    _cl_compiler_token_type_pair * const token_type_pair) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair local_token_type_pair;
  
  ASSERT(frontend != NULL);
  ASSERT(token_type_pair != NULL);
  
  error_code = clCompilerFrontendGetNextUnparsedToken(frontend, &local_token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  switch (token_type_pair->token_type) {
  case TOKEN_TYPE_IDENTIFIER:
  case TOKEN_TYPE_STRING:
  case TOKEN_TYPE_LONG_STRING:
  case TOKEN_TYPE_CHAR:
  case TOKEN_TYPE_LONG_CHAR:
  {
    char const * const string_1 =
      clCompilerTokenGetString(token_type_pair->token_type,
                               token_type_pair->token);
    char const * const string_2 =
      clCompilerTokenGetString(local_token_type_pair.token_type,
                               local_token_type_pair.token);
    
    if (strcmp(string_1, string_2) != 0) {
      ASSERT(0);
    }
  }
  break;
  
  default:
    ASSERT(token_type_pair->token_type == local_token_type_pair.token_type);
    break;
  }
  
 finish:
  return error_code;
}

void
clCompilerFrontendDumpIncludeFileStack(cl_compiler_frontend frontend,
                                       cstring cstr) {
  if (frontend->read_buffer != NULL) {
    cl_compiler_read_buffer read_buffer = frontend->read_buffer;
    
    CSTACK_TYPE(cl_compiler_read_buffer) include_file_stack =
      clCompilerFrontendGetIncludeFileStack(frontend);
    
    CSTACK_ITER_TYPE(cl_compiler_read_buffer) iter;
    for (iter = CSTACK_BEGIN(cl_compiler_read_buffer)(include_file_stack);
         iter != CSTACK_END(cl_compiler_read_buffer)(include_file_stack);
         iter = CSTACK_ITER_INCREMENT(cl_compiler_read_buffer)(iter)) {
      cl_compiler_read_buffer old_read_buffer =
        *CSTACK_ITER_GET_DATA(cl_compiler_read_buffer)(iter);
      CSTRING_PRINTF(cstr, "In file included from %s:%d:\n", 
                     clCompilerReadBufferGetFilename(old_read_buffer),
                     clCompilerReadBufferGetCurrLineNum(old_read_buffer));
    }
    
    if (clCompilerReadBufferGetCurrLineNum(read_buffer) > 0) {
      CSTRING_PRINTF(cstr, "%s:%d: ",
                     clCompilerReadBufferGetFilename(read_buffer),
                     clCompilerReadBufferGetCurrLineNum(read_buffer));
    } else {
      CSTRING_PRINTF(cstr, "%s: ",
                     clCompilerReadBufferGetFilename(read_buffer));
    }
  }
}

cl_compiler_error_code
clCompilerFrontendSetInputFile(cl_compiler_frontend frontend, char const *filename) {
  cl_bool result;
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  cl_compiler_read_buffer read_buffer = NULL;
  
  ASSERT(frontend != NULL);
  ASSERT(filename != NULL);
  
  /* open the file */
  read_buffer = clCompilerReadBufferNew(1000);
  ASSERT(read_buffer != NULL);
  
  result = clCompilerReadBufferSetInputFromFile(read_buffer, filename);
  if (CL_FALSE == result) {
    clOsalPrintf("%s\n does not exist.", filename);
    error_code = COMPILER_ERROR__COMMAND_LINE__FILE_DOES_NOT_EXIST;
    goto finish;
  }
  
  frontend->read_buffer = read_buffer;
  read_buffer = NULL;
  
 finish:
  if (read_buffer != NULL) {
    clCompilerReadBufferDelete(read_buffer);
  }
  return error_code;
}

cl_compiler_error_code
clCompilerFrontendInitFromCmdLine(
    cl_compiler_frontend frontend,
    _cl_compiler_cmd_line_option * const cmd_line_option) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  
  ASSERT(frontend != NULL);
  ASSERT(cmd_line_option != NULL);
  
  if (cmd_line_option->include_paths != NULL) {
    clCompilerFrontendSetIncludePath(frontend, cmd_line_option->include_paths);
  }
  
  if (cmd_line_option->define_undefine_symbol != NULL) {
    CLIST_ITER_TYPE(_cl_compiler_cmd_line_define_undefine_symbol) iter =
      CLIST_BEGIN(_cl_compiler_cmd_line_define_undefine_symbol)(cmd_line_option->define_undefine_symbol);
    for (;
         iter != CLIST_END(_cl_compiler_cmd_line_define_undefine_symbol)(cmd_line_option->define_undefine_symbol);
         iter = CLIST_ITER_INCREMENT(_cl_compiler_cmd_line_define_undefine_symbol)(iter)) {
      _cl_compiler_cmd_line_define_undefine_symbol symbol = 
        *CLIST_ITER_GET_DATA(_cl_compiler_cmd_line_define_undefine_symbol)(iter);
      
      if (CL_TRUE == symbol.is_define) {
        error_code = clCompilerFrontendPreprocessorDefineSymbol(frontend, symbol.name, symbol.value);
        if (error_code != COMPILER_NO_ERROR) {
          goto finish;
        }
      } else {
        clCompilerFrontendPreprocessorUndefineSymbol(frontend, symbol.name);
      }
    }
  }
  
 finish:
  return error_code;
}

cl_compiler_error_code
clCompilerFrontendCompileFile(cl_compiler_frontend frontend) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) container = {0};
  
  clCompilerFrontendSetTokenFlags(frontend, TOKEN_FLAG_BEGIN_OF_LINE);
  clCompilerFrontendSetParseFlags(frontend, PARSE_FLAG_CONVERT_PREPROCESSOR_NUMBER_TOKEN_TO_REAL_NUMBER);
  
  ASSERT(frontend != NULL);
  
  //............................................
  error_code = clCompilerFrontendParseTranslationUnit(frontend, &container);
#if defined(DUMP_INTERMEDIATE_DATA)
  {
    CLIST_ITER_TYPE(cl_compiler_ast_node) iter = CLIST_BEGIN(cl_compiler_ast_node)(&container);
    for (;
         iter != CLIST_END(cl_compiler_ast_node)(&container);
         iter = CLIST_ITER_INCREMENT(cl_compiler_ast_node)(iter)) {
      clCompilerFrontendAstNodeDumpInfo(*CLIST_ITER_GET_DATA(cl_compiler_ast_node)(iter), 0);
    }
  }
#endif
  
  clCompilerFrontendAstNodeFreeChildren(&container);  
  
  return error_code;
}
