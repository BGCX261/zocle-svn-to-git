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

#include <compiler_frontend/inc/preprocess.h>

#include <compiler_frontend/inc/grammar.h>
#include <compiler_frontend/inc/readbuffer.h>
#include <compiler_frontend/inc/ast_node.h>

#include <compiler/inc/error_msg.h>
#include <container/inc/clist.h>

#include <util/inc/file.h>
#include <util/inc/char.h>

#include <stdint.h>
#include <time.h>

typedef enum token_number_catagory_enum {
  TOKEN_NUMBER_CAT_UNSUFFIXED_DECIMAL,
  TOKEN_NUMBER_CAT_UNSUFFIXED_OCTAL_HEX,
  TOKEN_NUMBER_CAT_SUFFIXED_BY_U,
  TOKEN_NUMBER_CAT_SUFFIXED_BY_L_DECIMAL,
  TOKEN_NUMBER_CAT_SUFFIXED_BY_L_OCTAL_HEX,
  TOKEN_NUMBER_CAT_SUFFIXED_BY_U_L,
  TOKEN_NUMBER_CAT_SUFFIXED_BY_LL_DECIMAL,
  TOKEN_NUMBER_CAT_SUFFIXED_BY_LL_OCTAL_HEX,
  TOKEN_NUMBER_CAT_SUFFIXED_BY_U_LL
} token_number_catagory_enum;

typedef enum cl_compiler_space_handling_state {
  SPACE_HANDLING_STATE_CURR_TOKEN_ISNOT_SPACE_AND_SKIP_FOLLOWING_SPACE,
  SPACE_HANDLING_STATE_CURR_TOKEN_ISNOT_SPACE_AND_ADD_FOLLOWING_SPACE,
  SPACE_HANDLING_STATE_CURR_TOKEN_IS_SPACE
} cl_compiler_space_handling_state;

typedef struct _token_list_and_its_iter {
  CLIST_TYPE(_cl_compiler_token_type_pair) list;
  CLIST_ITER_TYPE(_cl_compiler_token_type_pair) *iter;
} _token_list_and_its_iter;

#ifndef DECLARE_CLIST_TYPE_FOR_TOKEN_LIST_AND_ITS_ITER
#define DECLARE_CLIST_TYPE_FOR_TOKEN_LIST_AND_ITS_ITER
DECLARE_CLIST_TYPE(_token_list_and_its_iter)
#endif
#ifndef DEFINE_CLIST_TYPE_FOR_TOKEN_LIST_AND_ITS_ITER
#define DEFINE_CLIST_TYPE_FOR_TOKEN_LIST_AND_ITS_ITER
DEFINE_CLIST_TYPE(_token_list_and_its_iter)
#endif

static char const monthName[12][4] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

/*                            digit
 *  .-- . --.             .-- letter --.
 *  |       v             |     .      v
 * -+-------+-- digit ----+---------------+--
 *                     ^  |            ^  |
 *                     |  '-- E -- + --'  |
 *                     |      e    -      |
 *                     '------------------'
 */
void
clCompilerFrontendParsePreprocessorNumber(
    cl_compiler_frontend frontend,
    cl_bool const start_by_dot) {
  cl_read_buffer_char ch;
  cl_bool found;
  cstring cstr;
  
  cl_compiler_read_buffer const read_buffer = frontend->read_buffer;
  ASSERT(read_buffer != NULL);
  
  found = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
  ASSERT(CL_TRUE == found);
  
  cstr = CSTRING_NEW();
  ASSERT(cstr != NULL);
  
  if (CL_TRUE == start_by_dot) {
    CSTRING_APPEND_CHAR(cstr, '.');
  }
  
  /* after the first digit, accept digits, alphabet, '.' or sign if
   * prefixed by 'eEpP' */
  for (;;) {
    cl_read_buffer_char old_ch;
    
    CSTRING_APPEND_CHAR(cstr, ch);
    clCompilerReadBufferConsumeChar(read_buffer);
    
    old_ch = ch;
    found = clCompilerReadBufferPeekCharSkipNewLineStray(read_buffer, &ch);
    ASSERT(CL_TRUE == found);
    
    if (!((CL_TRUE == isReadBufferCharEasciiIdOrDecimalDigit(ch)) ||
          ('.' == ch) ||
          ((('+' == ch) || ('-' == ch)) && 
           (('e' == old_ch) || ('E' == old_ch) ||
            ('p' == old_ch) || ('P' == old_ch))))) {
      break;
    }
  }
  
  CSTRING_APPEND_CHAR(cstr, '\0');
  {
    cl_compiler_token token =
      clCompilerFrontendTokenFindAndAllocate(
          frontend, TOKEN_TYPE_PREPROCESSOR_NUMBER, CSTRING_RAW_DATA(cstr));
    frontend->token_type = TOKEN_TYPE_PREPROCESSOR_NUMBER;
    frontend->token = token;
  }
  CSTRING_DELETE(cstr);
}

/* C comments */
void
clCompilerFrontendParseComment(cl_compiler_frontend frontend) {
  cl_read_buffer_char ch;
  cl_bool found;
  cl_compiler_read_buffer read_buffer = frontend->read_buffer;
  ASSERT(read_buffer != NULL);
  clCompilerReadBufferConsumeChar(read_buffer);
  for (;;) {
    /* fast skip loop */
    for (;;) {
      found = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
      ASSERT((CL_TRUE == found) && "unexpected end of file in comment");
      if (('\n' == ch) || ('\r' == ch) || ('*' == ch)) {
        break;
      }
      found = clCompilerReadBufferGetChar(read_buffer, &ch);
      ASSERT((CL_TRUE == found) && "unexpected end of file in comment");
      if (('\n' == ch) || ('\r' == ch) || ('*' == ch)) {
        break;
      }
      clCompilerReadBufferConsumeChar(read_buffer);
    }
    /* now we can handle all the cases */
    if ('\n' == ch) {
      ++read_buffer->line_num;
      clCompilerReadBufferConsumeChar(read_buffer);
    } else if ('\r' == ch) {
      found = clCompilerReadBufferGetChar(read_buffer, &ch);
      ASSERT(CL_TRUE == found);
      if ('\n' == ch) {
        ++read_buffer->line_num;
        clCompilerReadBufferConsumeChar(read_buffer);
      }
    } else if ('*' == ch) {
      clCompilerReadBufferConsumeChar(read_buffer);
      for (;;) {
        found = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
        ASSERT((CL_TRUE == found) && "unexpected end of file in comment");
        if ('*' == ch) {
          clCompilerReadBufferConsumeChar(read_buffer);
        } else if ('/' == ch) {
          goto end_of_comment;
        } else {
          break;
        }
      }
    }
  }
 end_of_comment:
  clCompilerReadBufferConsumeChar(read_buffer);
}

/* single line C++ comments */
void
clCompilerFrontendParseLineComment(cl_compiler_frontend frontend) {
  cl_read_buffer_char ch;
  cl_bool found;
  
  cl_compiler_read_buffer const read_buffer = frontend->read_buffer;
  ASSERT(read_buffer != NULL);
  
  clCompilerReadBufferConsumeChar(read_buffer);
  
  for (;;) {
    found = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
    if (CL_FALSE == found) {
      break;
    }
    if ('\n' == ch) {
      break;
    } else if ('\\' == ch) {
      found = clCompilerReadBufferGetChar(read_buffer, &ch);
      ASSERT(CL_TRUE == found);
      if ('\n' == ch) {
        ++read_buffer->line_num;
        clCompilerReadBufferConsumeChar(read_buffer);
      } else if ('\r' == ch) {
        found = clCompilerReadBufferGetChar(read_buffer, &ch);
        ASSERT(CL_TRUE == found);
        if ('\n' == ch) {
          ++read_buffer->line_num;
          clCompilerReadBufferConsumeChar(read_buffer);
        }
      }
    } else { 
      clCompilerReadBufferConsumeChar(read_buffer);
    }
  }
}

/* parse a string without interpreting escapes */
static void
clCompilerFrontendParsePreprocessorString(cl_compiler_frontend frontend,
                                          int const seperator,
                                          cstring cstr) {
  cl_read_buffer_char ch;
  cl_bool found;
  cl_compiler_read_buffer read_buffer = frontend->read_buffer;
  ASSERT(read_buffer != NULL);
  clCompilerReadBufferConsumeChar(read_buffer);
  for (;;) {
    found = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
    if (CL_FALSE == found) {
      ASSERT(0 && "missing terminating character");
    }
    if (ch == seperator) {
      break;
    } else if ('\\' == ch) {
      /* escape : just skip \[\r]\n */
      found = clCompilerReadBufferGetChar(read_buffer, &ch);
      ASSERT(CL_TRUE == found);
      if ('\n' == ch) {
        ++read_buffer->line_num;
        clCompilerReadBufferConsumeChar(read_buffer);
      } else if ('\r' == ch) {
        found = clCompilerReadBufferGetChar(read_buffer, &ch);
        ASSERT(CL_TRUE == found);
        if ('\n' == ch) {
          ++read_buffer->line_num;
          clCompilerReadBufferConsumeChar(read_buffer);
        }
      } else {
        if (cstr != NULL) {
          CSTRING_APPEND_CHAR(cstr, '\\');
          CSTRING_APPEND_CHAR(cstr, ch);
        }
        clCompilerReadBufferConsumeChar(read_buffer);
      }
    } else if ('\n' == ch) {
      ++read_buffer->line_num;
      if (cstr != NULL) {
        CSTRING_APPEND_CHAR(cstr, ch);
      }
      clCompilerReadBufferConsumeChar(read_buffer);
    } else if ('\r' == ch) {
      found = clCompilerReadBufferGetChar(read_buffer, &ch);
      ASSERT(CL_TRUE == found);
      if (ch != '\n') {
        if (cstr != NULL) {
          CSTRING_APPEND_CHAR(cstr, '\r');
        }
      } else {
        ++read_buffer->line_num;
        if (cstr != NULL) {
          CSTRING_APPEND_CHAR(cstr, ch);
        }
        clCompilerReadBufferConsumeChar(read_buffer);
      }
    } else {
      if (cstr != NULL) {
        CSTRING_APPEND_CHAR(cstr, ch);
      }
      clCompilerReadBufferConsumeChar(read_buffer);
    }
  }
  clCompilerReadBufferConsumeChar(read_buffer);
}

/* evaluate escape codes in a string. */
static void
clCompilerFrontendParseEscapeString(cl_compiler_frontend frontend,
                                    cstring out_cstr,
                                    cstring src_cstr,
                                    cl_bool const is_long) {
  char *p = CSTRING_RAW_DATA(src_cstr);
  for (;;) {
    int c = *p;
    if ('\0' == c) {
      break;
    }
    if ('\\' == c) {
      ++p;
      /* escape */
      c = *p;
      switch (c) {
      case '0': case '1': case '2': case '3':
      case '4': case '5': case '6': case '7':
      {
        /* at most three octal digits */
        int n = c - '0';
        ++p;
        c = *p;
        if (clUtilCharIsOctalDigit(c)) {
          n = (n * 8) + (c - '0');
          ++p;
          c = *p;
          if (clUtilCharIsOctalDigit(c)) {
            n = (n * 8) + (c - '0');
            ++p;
          }
        }
        c = n;
        goto add_char_nonext;
      }
        
      case 'x':
      case 'u':
      case 'U':
        {
          int n = 0;
          ++p;
          for (;;) {
            c = *p;
            if ((c >= 'a') && (c <= 'f')) {
              c = (c - 'a') + 10;
            } else if ((c >= 'A') && (c <= 'F')) {
              c = (c - 'A') + 10;
            } else if (clUtilCharIsDecimalDigit(c)) {
              c = c - '0';
            } else {
              break;
            }
            n = (n * 16) + c;
            ++p;
          }
          c = n;
          goto add_char_nonext;
        }
      
      case 'a': c = '\a'; break;
      case 'b': c = '\b'; break;
      case 'f': c = '\f'; break;
      case 'n': c = '\n'; break;
      case 'r': c = '\r'; break;
      case 't': c = '\t'; break;
      case 'v': c = '\v'; break;
      case '\'':
      case '\"':
      case '\\': 
      case '?':
        break;
      
      default:
        ASSERT(0 && "unknown escape sequence");
        break;
      }
    }
    p++;
  add_char_nonext:
    if (CL_FALSE == is_long) {
      CSTRING_APPEND_CHAR(out_cstr, c);
    } else {
      ASSERT(0 && "TODO: Handle this situation");
    }
  }
  /* add a trailing '\0' */
  if (CL_FALSE == is_long) {
    CSTRING_APPEND_CHAR(out_cstr, '\0');
  } else {
    ASSERT(0 && "TODO: Handle this situation");
  }
}

void
clCompilerFrontendParseString(cl_compiler_frontend frontend,
                              int const seperator,
                              cl_bool const is_long) {
  cl_compiler_token token;
  cstring tmp_cstr = CSTRING_NEW();
  cstring cstr = CSTRING_NEW();
  
  /* parse the string */
  clCompilerFrontendParsePreprocessorString(frontend, seperator, tmp_cstr);
  CSTRING_APPEND_CHAR(tmp_cstr, '\0');
  
  /* eval the escape (should be done as TOK_PPNUM) */
  clCompilerFrontendParseEscapeString(frontend, cstr, tmp_cstr, is_long);
  CSTRING_DELETE(tmp_cstr);
  
  if ('\'' == seperator) {
    size_t char_size;
    if (CL_FALSE == is_long) {
      char_size = 1;
    } else {
      ASSERT(0 && "TODO: Handle this situation");
      char_size = 2;
    }
    if (CSTRING_SIZE(cstr) < char_size) {
      ASSERT(0 && "empty character constant");
    }
    if (CSTRING_SIZE(cstr) >= 2 * char_size) {
      ASSERT(0 && "multi-character character constant");
    }
    if (CL_FALSE == is_long) {
      frontend->token_type = TOKEN_TYPE_CHAR;
    } else {
      frontend->token_type = TOKEN_TYPE_LONG_CHAR;
    }
  } else {
    if (CL_FALSE == is_long) {
      frontend->token_type = TOKEN_TYPE_STRING;
    } else {
      frontend->token_type = TOKEN_TYPE_LONG_STRING;
    }
  }
  token = clCompilerFrontendTokenFindAndAllocate(
      frontend, frontend->token_type, CSTRING_RAW_DATA(cstr));
  frontend->token = token;
  CSTRING_DELETE(cstr);
}

static inline cl_bool
canSkipSpace(cl_compiler_token_type const token_type,
             cl_compiler_space_handling_state * const space_handling_state) {
  if (TOKEN_TYPE_WHITE_SPACE == token_type) {
    switch (*space_handling_state) {
    case SPACE_HANDLING_STATE_CURR_TOKEN_ISNOT_SPACE_AND_ADD_FOLLOWING_SPACE:
      *space_handling_state = SPACE_HANDLING_STATE_CURR_TOKEN_IS_SPACE;
      return CL_FALSE;
      
    case SPACE_HANDLING_STATE_CURR_TOKEN_ISNOT_SPACE_AND_SKIP_FOLLOWING_SPACE:
    case SPACE_HANDLING_STATE_CURR_TOKEN_IS_SPACE:
      return CL_TRUE;
      
    default:
      ASSERT(0);
      return CL_TRUE;
    }
  } else {
    *space_handling_state = SPACE_HANDLING_STATE_CURR_TOKEN_ISNOT_SPACE_AND_ADD_FOLLOWING_SPACE;
    return CL_FALSE;
  }
}

static void
clCompilerFrontendDefinedTokenStackPush(cl_compiler_frontend frontend,
                                        cl_compiler_token token) {
  CSTACK_TYPE(cl_compiler_token) defined_token_stack;
  
  ASSERT(frontend != NULL);
  ASSERT(token != NULL);
  
  defined_token_stack = frontend->defined_token_stack;
  CSTACK_PUSH(cl_compiler_token)(defined_token_stack, &token);
}

/* parse after #define */
static cl_compiler_error_code
clCompilerFrontendPreprocessParseDefine(cl_compiler_frontend frontend) {
  cl_compiler_identifier_token the_token_this_time;
  cl_compiler_macro_type macro_type;
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  
  ASSERT(frontend != NULL);
  
  /* get the macro name */
  clCompilerFrontendGetNextTokenWithoutMacroSubst(frontend, CAN_NOT_RETURN_SPACE_TOKEN, NULL);
  
  if (frontend->token_type != TOKEN_TYPE_IDENTIFIER) {
    ASSERT(0 && "invalid macro name");
  }
  
  the_token_this_time = (cl_compiler_identifier_token)(frontend->token);
  ASSERT(the_token_this_time != NULL);
  
  error_code = clCompilerFrontendGetNextTokenWithoutMacroSubst(frontend, CAN_RETURN_SPACE_TOKEN, NULL);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  macro_type = MACRO_TYPE_OBJECT_LIKE;
  
  /* '(' must be just after macro definition for function-like macro */
  if (TOKEN_TYPE_LEFT_PAREN == frontend->token_type) { /* '(' */
    /* This is a function-like macro, get the argument part */
    CVECTOR_TYPE(_cl_compiler_token_type_pair) defined_arguments = NULL;
    
    /* get the next token just after the '(' */
    error_code = clCompilerFrontendGetNextTokenWithoutMacroSubst(
        frontend, CAN_NOT_RETURN_SPACE_TOKEN, NULL);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    
    defined_arguments = CVECTOR_NEW(_cl_compiler_token_type_pair)(0);
    
    while (frontend->token_type != TOKEN_TYPE_RIGHT_PAREN) { /* ')' */
      cl_bool is_variadic = CL_FALSE;
      
      /* find an argument */
      if (TOKEN_TYPE_DOTS == frontend->token_type) {
        //..................... handle variadic
        is_variadic = CL_TRUE;
      }
      
      if (frontend->token_type != TOKEN_TYPE_IDENTIFIER) {
        ASSERT(0 && "badly punctuated parameter list");
      }
      
      {
        _cl_compiler_token_type_pair pair = { frontend->token_type,
                                              frontend->token };
        CVECTOR_PUSH_BACK(_cl_compiler_token_type_pair)(defined_arguments, &pair);
      }
      
      /* get the next token after this argument */
      error_code = clCompilerFrontendGetNextTokenWithoutMacroSubst(frontend, CAN_NOT_RETURN_SPACE_TOKEN, NULL);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      
      if (frontend->token_type != TOKEN_TYPE_COMMA) { /* ',' */
        break;
      } else {
        /* get the next argument/token just after the ',' */
        error_code = clCompilerFrontendGetNextTokenWithoutMacroSubst(
            frontend, CAN_NOT_RETURN_SPACE_TOKEN, NULL);
        if (error_code != COMPILER_NO_ERROR) {
          goto finish;
        }
      }
    }
    
    if (TOKEN_TYPE_RIGHT_PAREN == frontend->token_type) { /* ')' */
      /* get the next token just after the ')' */
      error_code = clCompilerFrontendGetNextTokenWithoutMacroSubst(
          frontend, CAN_NOT_RETURN_SPACE_TOKEN, NULL);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
    }
    clCompilerTokenSetDefinedArguments(the_token_this_time, defined_arguments);
    macro_type = MACRO_TYPE_FUNCTION_LIKE;
  }
  
  clCompilerTokenSetMacroType(the_token_this_time, macro_type);
  
  /* get the defined tokens part for this '#define'd token */
  {
    CLIST_TYPE(_cl_compiler_token_type_pair) defined_tokens =
      CLIST_NEW(_cl_compiler_token_type_pair)();
    cl_compiler_space_handling_state space_handling_state =
      SPACE_HANDLING_STATE_CURR_TOKEN_ISNOT_SPACE_AND_SKIP_FOLLOWING_SPACE;
    
    while ((frontend->token_type != TOKEN_TYPE_LINEFEED) &&
           (frontend->token_type != TOKEN_TYPE_EOF)) {
      /* remove spaces around ## and after '#' */
      if (TOKEN_TYPE_TWOSHARPS == frontend->token_type) {
        if (SPACE_HANDLING_STATE_CURR_TOKEN_IS_SPACE == space_handling_state) {
          clCompilerTokenDelete(frontend,
                                CLIST_BACK(_cl_compiler_token_type_pair)(defined_tokens)->token);
          
          CLIST_POP_BACK(_cl_compiler_token_type_pair)(defined_tokens);
        }
        space_handling_state = SPACE_HANDLING_STATE_CURR_TOKEN_ISNOT_SPACE_AND_SKIP_FOLLOWING_SPACE;
      } else if (TOKEN_TYPE_SHARP == frontend->token_type) {
        space_handling_state = SPACE_HANDLING_STATE_CURR_TOKEN_ISNOT_SPACE_AND_SKIP_FOLLOWING_SPACE;
      } else if (CL_TRUE == canSkipSpace(frontend->token_type, &space_handling_state)) {
        goto skip;
      }
      
      {
        _cl_compiler_token_type_pair pair = { frontend->token_type, frontend->token };
        CLIST_PUSH_BACK(_cl_compiler_token_type_pair)(defined_tokens, &pair);
      }
      
    skip:
      error_code = clCompilerFrontendGetNextTokenWithoutMacroSubst(frontend, CAN_RETURN_SPACE_TOKEN, NULL);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
    }
    
    clCompilerTokenSetDefinedTokens(the_token_this_time, defined_tokens);
  }
  
  clCompilerFrontendDefinedTokenStackPush(frontend, (cl_compiler_token)the_token_this_time);
  
 finish:
  return error_code;
}

/* define a preprocessor symbol */
cl_compiler_error_code
clCompilerFrontendPreprocessorDefineSymbol(
    cl_compiler_frontend frontend, char const *symbol, char const *value) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  cl_compiler_read_buffer read_buffer = NULL;
  
  if (NULL == value) {
    value = "1";
  }
  read_buffer = clCompilerReadBufferNew(strlen(symbol) + strlen(value) + 1 + 1);
  
  clCompilerReadBufferAddContentString(read_buffer, symbol);
  clCompilerReadBufferAddContentString(read_buffer, " ");
  clCompilerReadBufferAddContentString(read_buffer, value);
  
  frontend->read_buffer = read_buffer;
  ASSERT(0 == CSTACK_SIZE(cl_compiler_read_buffer)(frontend->include_file_stack));
  
  /* parse with define parser */
  error_code = clCompilerFrontendPreprocessParseDefine(frontend);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
 finish:
  frontend->read_buffer = NULL;
  if (read_buffer != NULL) {
    clCompilerReadBufferDelete(read_buffer);
  }
  return error_code;
}

void
clCompilerFrontendPreprocessorUndefineSymbol(
    cl_compiler_frontend frontend, char const *symbol) {
  cl_compiler_token result;
  
  ASSERT(frontend != NULL);
  ASSERT(symbol != NULL);
  
  result = clCompilerTokenHashTableSearchByStr(frontend->token_table, symbol);
  ASSERT(result != NULL);
  ASSERT(TOKEN_TYPE_IDENTIFIER == result->type);
  
  CLIST_DELETE(_cl_compiler_token_type_pair)(((cl_compiler_identifier_token)result)->defined_tokens);
  
  clCompilerTokenSetDefinedTokens((cl_compiler_identifier_token)result, NULL);
}

/* skip block of text until #else, #elif or #endif. skip also pairs of
 * #if/#endif */
static cl_compiler_error_code
clCompilerFrontendPreprocessSkipThisIfdefBlock(cl_compiler_frontend frontend) {
  cl_compiler_read_buffer const read_buffer = frontend->read_buffer;
  cl_bool start_of_line = CL_FALSE;
  cl_bool in_warning_or_error_directive = CL_FALSE;
  cl_uint nested_ifdef_level = 0;
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  
  ASSERT(read_buffer != NULL);
  
  for (;;) {
    cl_read_buffer_char ch;
    cl_bool found = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
    if (CL_FALSE == found) {
      ASSERT(0 && "expect #endif");
    }
    switch (ch) {
    case ' ': case '\t': case '\f': case '\v': case '\r':
      clCompilerReadBufferConsumeChar(read_buffer);
      break;
      
    case '\n':
      ++read_buffer->line_num;
      
      in_warning_or_error_directive = CL_FALSE;
      start_of_line = CL_TRUE;
      
      clCompilerReadBufferConsumeChar(read_buffer);
      continue;
      break;
      
    case '\\':
      if (CL_FALSE == clCompilerReadBufferHandleNewLineStray(read_buffer)) {
        ASSERT(0);
      }
      break;
      
      /* skip strings */
    case '\"': case '\'':
      if (CL_TRUE == in_warning_or_error_directive) {
        /* because #warning & #error must in a single line,
         * so that we don't need to handle stray, we can just skip it. */
        clCompilerReadBufferConsumeChar(read_buffer);
      } else {
        clCompilerFrontendParsePreprocessorString(frontend, ch, NULL);
      }
      break;
      
      /* skip comments */
    case '/':
      if (CL_TRUE == in_warning_or_error_directive) {
        clCompilerReadBufferConsumeChar(read_buffer);
      } else {
        found = clCompilerReadBufferGetChar(read_buffer, &ch);
        ASSERT(CL_TRUE == found);
        if ('*' == ch) {
          clCompilerFrontendParseComment(frontend);
          break;
        } else if ('/' == ch) {
          clCompilerFrontendParseLineComment(frontend);
          break;
        }
      }
      break;
      
    case '#':
      clCompilerReadBufferConsumeChar(read_buffer);
      if (CL_TRUE == start_of_line) {
        error_code = clCompilerFrontendGetNextTokenWithoutMacroSubst(
            frontend, CAN_NOT_RETURN_SPACE_TOKEN, NULL);
        if (error_code != COMPILER_NO_ERROR) {
          goto finish;
        }
        
        if ((0 == nested_ifdef_level) &&
            ((TOKEN_TYPE_ELSE == frontend->token_type) ||
             (TOKEN_TYPE_ELIF == frontend->token_type) ||
             (TOKEN_TYPE_ENDIF == frontend->token_type))) {
          /* find the end of this #ifdef block */
          goto finish;
        }
        if ((TOKEN_TYPE_IF == frontend->token_type) ||
            (TOKEN_TYPE_IFDEF == frontend->token_type) ||
            (TOKEN_TYPE_IFNDEF == frontend->token_type)) {
          ++nested_ifdef_level;
        } else if (TOKEN_TYPE_ENDIF == frontend->token_type) {
          --nested_ifdef_level;
        } else if ((TOKEN_TYPE_ERROR == frontend->token_type) ||
                   (TOKEN_TYPE_WARNING == frontend->token_type)) {
          in_warning_or_error_directive = CL_TRUE;
        }
      }
      break;
      
    default:
      clCompilerReadBufferConsumeChar(read_buffer);
      break;
    }
    start_of_line = CL_FALSE;
  }
  
 finish:
  return error_code;
}

static cl_compiler_error_code
clCompilerFrontendPreprocessParseIfdef(cl_compiler_frontend frontend,
                                       cl_bool const is_ifdef,
                                       cl_bool * const success) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  CLIST_TYPE(_cl_compiler_token_type_pair) defined_tokens = NULL;
  
  ASSERT(frontend != NULL);
  ASSERT(success != NULL);
  
  error_code = clCompilerFrontendGetNextTokenWithoutMacroSubst(
      frontend, CAN_NOT_RETURN_SPACE_TOKEN, NULL);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  if (frontend->token_type != TOKEN_TYPE_IDENTIFIER) {
    ASSERT(0 && "invalid argument");
  }
  
  ASSERT(TOKEN_TYPE_IDENTIFIER == frontend->token->type);
  defined_tokens = clCompilerTokenGetDefinedTokens((cl_compiler_identifier_token)(frontend->token));
  
  {
    cl_int ifdef_stack_element = (CL_TRUE == is_ifdef) ? 0 : 1;
    
    ifdef_stack_element ^= (defined_tokens != NULL);
    CSTACK_PUSH(cl_int)(frontend->ifdef_stack, &ifdef_stack_element);
    
    if (!(ifdef_stack_element & 1)) {
      error_code = clCompilerFrontendPreprocessSkipThisIfdefBlock(frontend);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      *success = CL_FALSE;
    } else {
      *success = CL_TRUE;
    }
  }
  
 finish:
  return error_code;
}

static void
clCompilerFrontendPreprocessParsePragma(cl_compiler_frontend frontend) {
  ASSERT(frontend != NULL);
  /* Currently, zocle compiler doesn't support any #pragma */
}

static void
clCompilerFrontendPreprocessParseErrorAndWarning(cl_compiler_frontend frontend,
                                                 cl_bool const is_error) {
  cl_read_buffer_char ch;
  cl_compiler_read_buffer read_buffer;
  cl_bool found;
  cstring cstr;
  
  ASSERT(frontend != NULL);
  
  read_buffer = frontend->read_buffer;
  found = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
  
  /* skip space characters */
  while (CL_TRUE == clCompilerReadBufferCharIsSpaceChar(ch)) {
    clCompilerReadBufferConsumeChar(read_buffer);
    found = clCompilerReadBufferPeekCharSkipNewLineStray(read_buffer, &ch);
    ASSERT(CL_TRUE == found);
  }
  
  cstr = CSTRING_NEW();
  
  while (ch != '\n') {
    if ('\\' == ch) {
      if (CL_FALSE == clCompilerReadBufferHandleNewLineStray(read_buffer)) {
        CSTRING_APPEND_CHAR(cstr, ch);
      }
    } else {
      CSTRING_APPEND_CHAR(cstr, ch);
      clCompilerReadBufferConsumeChar(read_buffer);
      found = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
      if (CL_FALSE == found) {
        break;
      }
    }
  }
  
  if (CL_TRUE == is_error) {
    clOsalPrintf("#error %s\n", CSTRING_RAW_DATA(cstr));
  } else {
    clOsalPrintf("#warning %s\n", CSTRING_RAW_DATA(cstr));
  }
  
  CSTRING_DELETE(cstr);
}

static cl_bool
clCompilerFrontendPreprocessExpressionConstantFolding(
    cl_compiler_frontend frontend,
    cl_compiler_ast_node ast_node,
    cl_compiler_ast_node *new_ast_node_place) {
  ASSERT(frontend != NULL);
  ASSERT(ast_node != NULL);
  
  switch (ast_node->type) {
  case AST_NODE_TYPE_PREFIX_INCREMENT_OPERATOR:    /* ++ */
  case AST_NODE_TYPE_PREFIX_DECREMENT_OPERATOR:    /* -- */
  case AST_NODE_TYPE_POSTFIX_INCREMENT_OPERATOR:   /* ++ */
  case AST_NODE_TYPE_POSTFIX_DECREMENT_OPERATOR:   /* -- */
  
  case AST_NODE_TYPE_ADDRESS_OPERATOR:             /* &  */
  case AST_NODE_TYPE_INDIRECTION_OPERATOR:         /* *  */
  
  case AST_NODE_TYPE_POSITIVE_OPERATOR:            /* +  */
  case AST_NODE_TYPE_NEGATIVE_OPERATOR:            /* -  */
  case AST_NODE_TYPE_LOGICAL_NEGATION_OPERATOR:    /* !  */
  case AST_NODE_TYPE_SIZEOF_OPERATOR:              /* sizeof  */
  
  case AST_NODE_TYPE_SUBTRACTION_OPERATOR:         /* -  */
  case AST_NODE_TYPE_MULTIPLY_OPERATOR:            /* *  */
  case AST_NODE_TYPE_DIVISION_OPERATOR:            /* /  */
  case AST_NODE_TYPE_MODULO_OPERATOR:              /* %  */
  
  case AST_NODE_TYPE_LESS_THAN_OPERATOR:           /* << */
  case AST_NODE_TYPE_LESS_EQUAL_OPERATOR:          /* <= */
  case AST_NODE_TYPE_GREATER_THAN_OPERATOR:        /* >> */
  case AST_NODE_TYPE_GREATER_EQUAL_OPERATOR:       /* >= */
  case AST_NODE_TYPE_EQUAL_OPERATOR:               /* == */
  case AST_NODE_TYPE_NOT_EQUAL_OPERATOR:           /* != */
  
  case AST_NODE_TYPE_BITWISE_LEFT_SHIFT_OPERATOR:  /* << */
  case AST_NODE_TYPE_BITWISE_RIGHT_SHIFT_OPERATOR: /* >> */
  case AST_NODE_TYPE_BITWISE_COMPLEMENT_OPERATOR:  /* ~  */
  case AST_NODE_TYPE_BITWISE_AND_OPERATOR:         /* &  */
  case AST_NODE_TYPE_BITWISE_XOR_OPERATOR:         /* ^  */
  case AST_NODE_TYPE_BITWISE_OR_OPERATOR:          /* |  */
  
  case AST_NODE_TYPE_LOGICAL_AND_OPERATOR:         /* && */
  case AST_NODE_TYPE_LOGICAL_OR_OPERATOR:          /* || */
  
  case AST_NODE_TYPE_IF:                           /* if-statement or ?: */
  
  case AST_NODE_TYPE_ARRAY_SUBSCRIPTING:           /* [] */
  case AST_NODE_TYPE_COMPOUND_LITERAL:
  case AST_NODE_TYPE_TYPENAME:
  case AST_NODE_TYPE_INIT_LIST:
  case AST_NODE_TYPE_CONCRETE_TYPE_MEMBER:
  case AST_NODE_TYPE_PTR_TYPE_MEMBER:
  case AST_NODE_TYPE_FUNCTION_CALL:
  case AST_NODE_TYPE_CAST_TO_TYPE:
    
  case AST_NODE_TYPE_IDENTIFIER:
    
  case AST_NODE_TYPE_STRING:
  case AST_NODE_TYPE_LONG_STRING:
    
  case AST_NODE_TYPE_CHAR:
  case AST_NODE_TYPE_LONG_CHAR:
    ASSERT(0 && "TODO");
    return CL_FALSE;
    
  case AST_NODE_TYPE_ADDITION_OPERATOR:            /* +  */
  {
    int64_t result = 0;
    CLIST_ITER_TYPE(cl_compiler_ast_node) iter;
    for (iter = CLIST_BEGIN(cl_compiler_ast_node)(&(ast_node->children));
         iter != CLIST_END(cl_compiler_ast_node)(&(ast_node->children));
         ) {
      cl_compiler_ast_node child = *CLIST_ITER_GET_DATA(cl_compiler_ast_node)(iter);
      if (CL_TRUE == clCompilerFrontendAstNodeIsIntegerConstant(child)) {
        result += clCompilerFrontendIntegerContantAstNodeGetValue(child);
        clCompilerFrontendAstNodeDelete(child);
        iter = CLIST_ERASE(cl_compiler_ast_node)(&(ast_node->children), iter);
      } else {
        /* it is not an integer constant node, calling this function recursively
         * to try to make it become an integer constant node. */
        cl_compiler_ast_node new_ast_node;
        cl_bool const success = 
          clCompilerFrontendPreprocessExpressionConstantFolding(frontend, child, &new_ast_node);
        if (CL_TRUE == success) {
          ASSERT(CL_TRUE == clCompilerFrontendAstNodeIsIntegerConstant(new_ast_node));
          result += clCompilerFrontendIntegerContantAstNodeGetValue(new_ast_node);
        } else {
          /* although maybe there are still children nodes behind this one, 
           * however, because we are fail in this child, we don't need to
           * process the remainding children anymore. */
          return CL_FALSE;
        }
      }
    }
    
    /* delete the original ast_node, and create a new one with constant integer type. */
    {
      cl_compiler_ast_node new_ast_node =
        clCompilerFrontendAstNodeNew(AST_NODE_TYPE_CONSTANT_LONGLONG_INT);
      cl_compiler_number_token token =
        (cl_compiler_number_token)clCompilerTokenNew(frontend, TOKEN_TYPE_CONSTANT_LONGLONG_INT, NULL);
      
      token->token_value = CL_OSAL_CALLOC(sizeof(_cl_compiler_token_value));
      ASSERT(token->token_value != NULL);
      token->token_value->longlong_value = result;
      
      ((cl_compiler_ast_node_with_token)new_ast_node)->token = (cl_compiler_token)token;
      
      {
        /* add this newly created one to the parent ast_node */
        cl_compiler_ast_node parent = clCompilerFrontendAstNodeGetParent(ast_node);
        
        /* delete the original ast_node */
        clCompilerFrontendAstNodeDelete(ast_node);
        
        if (parent != NULL) {
          clCompilerFrontendAstNodeAddChildren(parent, new_ast_node);
          
          /* remove it from its parent */
          {
            CLIST_ITER_TYPE(cl_compiler_ast_node) iter;
            
            iter = clCompilerFrontendAstNodeSearch(&(parent->children), ast_node);
            CLIST_ERASE(cl_compiler_ast_node)(&(parent->children), iter);
          }
        }
        
        if (new_ast_node != NULL) {
          *new_ast_node_place = new_ast_node;
        }
        return CL_TRUE;
      }
    }
  }
  break;
  
  case AST_NODE_TYPE_CONSTANT_INT:
  case AST_NODE_TYPE_CONSTANT_UNSIGNED_INT:
  case AST_NODE_TYPE_CONSTANT_LONG_INT:
  case AST_NODE_TYPE_CONSTANT_UNSIGNED_LONG_INT:
  case AST_NODE_TYPE_CONSTANT_LONGLONG_INT:
  case AST_NODE_TYPE_CONSTANT_UNSIGNED_LONGLONG_INT:
  case AST_NODE_TYPE_CONSTANT_FLOAT:
  case AST_NODE_TYPE_CONSTANT_DOUBLE:
  case AST_NODE_TYPE_CONSTANT_LONG_DOUBLE:
    /* it has already been an integer constant node, just return. */
    return CL_TRUE;
    
  default:
    ASSERT(0 && "should not reach here");
    return CL_TRUE;
  }
}

/* eval an expression for #if/#elif
 *
 * when calling this function, the parse_flags will NOT contain
 * PARSE_FLAG_CAN_RETURN_WHITE_SPACE, so that we will NOT get
 * WHITE_SPAEC token in this function. */
static cl_compiler_error_code
clCompilerFrontendEvalPreprocessExpression(
    cl_compiler_frontend frontend, cl_int * const result) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_TYPE(_cl_compiler_token_type_pair) expr_tokens = NULL;
  
  ASSERT(frontend != NULL);
  
  /* I have to create a new list for the new 'preprocessed token list' for later parsing,
   * because in the later while loop, I will get preprocesed token from the preprocessed_token_list,
   * and if I put them into its original token list, it is WRONG. */
  expr_tokens = CLIST_NEW(_cl_compiler_token_type_pair)();
  
  error_code = clCompilerFrontendGetNextTokenWithMacroSubst(frontend); /* do macro subst */
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  /* collect tokens for the parsing */
  while ((frontend->token_type != TOKEN_TYPE_LINEFEED) &&
         (frontend->token_type != TOKEN_TYPE_EOF)) {
    if (TOKEN_TYPE_DEFINED == frontend->token_type) {
      /* #if defined(... */
      cl_int value;
      
      {
        /* eat '(' */
        error_code = clCompilerFrontendGetNextTokenWithoutMacroSubst(
            frontend, CAN_NOT_RETURN_SPACE_TOKEN, NULL);
        if (error_code != COMPILER_NO_ERROR) {
          goto finish;
        }
        if (frontend->token_type != TOKEN_TYPE_LEFT_PAREN) { /* '(' */
          goto finish;
        }
      }
      
      error_code = clCompilerFrontendGetNextTokenWithoutMacroSubst(
          frontend, CAN_NOT_RETURN_SPACE_TOKEN, NULL);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      ASSERT(TOKEN_TYPE_IDENTIFIER == frontend->token->type);
      value = (clCompilerTokenGetDefinedTokens((cl_compiler_identifier_token)(frontend->token)) != NULL);
      
      {
        /* eat ')' */
        error_code = clCompilerFrontendGetNextTokenWithoutMacroSubst(
            frontend, CAN_NOT_RETURN_SPACE_TOKEN, NULL);
        if (error_code != COMPILER_NO_ERROR) {
          goto finish;
        }
        if (frontend->token_type != TOKEN_TYPE_RIGHT_PAREN) { /* ')' */
          goto finish;
        }
      }
      
      {
        cl_compiler_number_token token =
          (cl_compiler_number_token)clCompilerTokenNew(frontend, TOKEN_TYPE_CONSTANT_INT, NULL);
        token->token_value = CL_OSAL_CALLOC(sizeof(_cl_compiler_token_value));
        ASSERT(token->token_value != NULL);
        
        token->token_value->int_value = value;
        
        token_type_pair.token_type = TOKEN_TYPE_CONSTANT_INT;
        token_type_pair.token = (cl_compiler_token)token;
      }
    } else if (TOKEN_TYPE_IDENTIFIER == frontend->token_type) {
      /* From C99 spec, p148:
       *
       * "After all replacements due to macro expansion and the defined unary
       * operator have been performed, all remaining identifiers are replaced with the pp-number
       * 0, and then each preprocessing token is converted into a token."
       *
       * That means: codes like '#if ddd' is equvilent to '#if 0' */
      cl_compiler_number_token token =
        (cl_compiler_number_token)clCompilerTokenNew(frontend, TOKEN_TYPE_CONSTANT_INT, NULL);
      token->token_value = CL_OSAL_CALLOC(sizeof(_cl_compiler_token_value));
      ASSERT(token->token_value != NULL);
      
      token->token_value->int_value = 0;
      
      token_type_pair.token_type = TOKEN_TYPE_CONSTANT_INT;
      token_type_pair.token = (cl_compiler_token)token;
    } else {
      token_type_pair.token_type = frontend->token_type;
      token_type_pair.token = frontend->token;
    }
    
    switch (token_type_pair.token_type) {
    case TOKEN_TYPE_INCREMENT:
    case TOKEN_TYPE_DECREMENT:
    case TOKEN_TYPE_ASSIGN:
    case TOKEN_TYPE_COMMA:
      ASSERT(0 && "TODO");
      goto finish;
      
    case TOKEN_TYPE_CONSTANT_FLOAT:
    case TOKEN_TYPE_CONSTANT_DOUBLE:
    case TOKEN_TYPE_CONSTANT_LONG_DOUBLE:
      error(frontend, "expression must have integral type");
      error_code = COMPILER_ERROR__PREPROCESSOR__IF_DIRECTIVE_PARAMETER_IS_NOT_INTEGRAL_CONSTANT_TYPE;
      goto finish;
      
    default:
      break;
    }
    
    CLIST_PUSH_BACK(_cl_compiler_token_type_pair)(expr_tokens, &token_type_pair);
    
    error_code = clCompilerFrontendGetNextTokenWithMacroSubst(frontend); /* do macro subst */
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
  }
  
  /* simulate EOF */
  {
    token_type_pair.token_type = TOKEN_TYPE_EOF;
    token_type_pair.token = NULL;
    CLIST_PUSH_BACK(_cl_compiler_token_type_pair)(expr_tokens, &token_type_pair);
  }
  
  {
    int const_expr_value;
    CLIST_TYPE(_cl_compiler_token_type_pair) old_preprocessed_token_list =
      frontend->preprocessed_token_list;
    frontend->preprocessed_token_list = expr_tokens;
    
    /* now evaluate C constant expression */
    /* The #if directive parameter must be in the integral constant type */
    /* From BOOST:
     *
     * "Don't declare integral constant expressions whose type is wider than int.
     *
     * Rationale: while in theory all integral types are usable in integral
     * constant expressions, in practice many compilers limit integral constant
     * expressions to types no wider than int.
     */
    {
      CLIST_CONCRETE_TYPE(cl_compiler_ast_node) container = {0};
      cl_compiler_ast_node ast_node = NULL;
      cl_bool constant_folding_success;
      
      error_code = clCompilerFrontendParseConstantExpression(frontend, &container);
      if (error_code != COMPILER_NO_ERROR) {
        ASSERT(0);
        goto finish;
      }
      /* if success, there should be only one remainding tokens in the 'unparsed_token_list',
       * which is TOKEN_TYPE_EOF */
      ASSERT(1 == CLIST_SIZE(_cl_compiler_token_type_pair)(frontend->unparsed_token_list));
      ASSERT(TOKEN_TYPE_EOF == CLIST_FRONT(_cl_compiler_token_type_pair)(frontend->unparsed_token_list)->token_type);
      ASSERT(1 == CLIST_SIZE(cl_compiler_ast_node)(&container));
      
      /* remove this EOF token because it is added for the constant expression evaluation purpose. */
      CLIST_POP_FRONT(_cl_compiler_token_type_pair)(frontend->unparsed_token_list);
      
      ast_node = *CLIST_FRONT(cl_compiler_ast_node)(&container);
#if defined(DUMP_INTERMEDIATE_DATA)
      clCompilerFrontendAstNodeDumpInfo(ast_node, 0);
#endif
      constant_folding_success =
        clCompilerFrontendPreprocessExpressionConstantFolding(frontend, ast_node, &ast_node);
      /* after constant folding on a constant expression, there should be only one
       * ast_node left. */
      if (CL_TRUE == constant_folding_success) {
        ASSERT(0 == CLIST_SIZE(cl_compiler_ast_node)(&(ast_node->children)));
        ASSERT(CL_TRUE == clCompilerFrontendAstNodeIsIntegerConstant(ast_node));
        ASSERT(((cl_compiler_ast_node_with_token)ast_node)->token != NULL);
        
        {
          cl_compiler_number_token token =
            (cl_compiler_number_token)(((cl_compiler_ast_node_with_token)ast_node)->token);
          
          switch (ast_node->type) {
          case AST_NODE_TYPE_CONSTANT_INT:
            const_expr_value = token->token_value->int_value;
            break;
          case AST_NODE_TYPE_CONSTANT_UNSIGNED_INT:
            const_expr_value = token->token_value->uint_value;
            break;
          case AST_NODE_TYPE_CONSTANT_LONG_INT:
            const_expr_value = token->token_value->long_value;
            break;
          case AST_NODE_TYPE_CONSTANT_UNSIGNED_LONG_INT:
            const_expr_value = token->token_value->ulong_value;
            break;
          case AST_NODE_TYPE_CONSTANT_LONGLONG_INT:
            const_expr_value = (cl_int)token->token_value->longlong_value;
            break;
          case AST_NODE_TYPE_CONSTANT_UNSIGNED_LONGLONG_INT:
            const_expr_value = (cl_int)token->token_value->ulonglong_value;
            break;
          default:
            ASSERT(0);
            break;
          }
        }
      } else {
        ASSERT(0 && "TODO: error_code & clean ast_node, container");
      }
      
      clCompilerFrontendAstNodeDelete(ast_node);
      CLIST_CLEAR(cl_compiler_ast_node)(&container);
    }
    
    *result = (const_expr_value != 0);
    
    frontend->preprocessed_token_list = old_preprocessed_token_list;
    CLIST_DELETE(_cl_compiler_token_type_pair)(expr_tokens);
    expr_tokens = NULL;
  }
  
 finish:
  if (expr_tokens != NULL) {
    CLIST_DELETE(_cl_compiler_token_type_pair)(expr_tokens);
  }
  /* because the caller function, clCompilerFrontendPreprocess() searches
   * TOKEN_TYPE_LINEFEED to find the end of the directive line, and from the upper
   * 'while' loop (it ends by finding a LINEFEED and eating it), this function
   * have to set the current token_type to TOKEN_TYPE_LINEFEED */
  frontend->token_type = TOKEN_TYPE_LINEFEED;
  frontend->token = NULL;
  
  return error_code;
}

static void
clCompilerFrontendReadHeaderFileName(
    cl_compiler_frontend frontend,
    cl_read_buffer_char const delimiter,
    cstring cstr) {
  cl_compiler_read_buffer read_buffer;
  cl_read_buffer_char ch;
  cl_bool result;
  
  ASSERT(frontend != NULL);
  ASSERT(('>' == delimiter) || ('"' == delimiter));
  ASSERT(cstr != NULL);
  
  read_buffer = frontend->read_buffer;
  ASSERT(read_buffer != NULL);
  
  result = clCompilerReadBufferGetChar(read_buffer, &ch);
  
  if (CL_FALSE == result) {
    return;
  }
  
  while ((ch != delimiter) && (ch != '\n')) {
    if ('\\' == ch) {
      if (CL_FALSE == clCompilerReadBufferHandleNewLineStray(read_buffer)) {
        /* '\' is not belongs to \\r\n, hence adding to the filename */
        CSTRING_APPEND_CHAR(cstr, ch);
      }
    } else {
      CSTRING_APPEND_CHAR(cstr, ch);
      result = clCompilerReadBufferGetChar(read_buffer, &ch);
      if (CL_FALSE == result) {
        return;
      }
    }
  }
}

static cl_bool
clCompilerFrontendOpenIncludeFile(cl_compiler_frontend frontend, char const *filename) {
  cl_compiler_read_buffer read_buffer;
  cl_bool result;
  
  ASSERT(frontend != NULL);
  ASSERT(filename != NULL);
  
  read_buffer = clCompilerReadBufferNew(1000);
  ASSERT(read_buffer != NULL);
  
  result = clCompilerReadBufferSetInputFromFile(read_buffer, filename);
  ASSERT(CL_TRUE == result);
  
  /* push current file in stack */
  CSTACK_PUSH(cl_compiler_read_buffer)(frontend->include_file_stack,
                                       &frontend->read_buffer);
  frontend->read_buffer = read_buffer;
  
  /* add include file debug info */
  frontend->token_flags |= TOKEN_FLAG_BEGIN_OF_LINE;
  
  return CL_TRUE;
}

static cl_bool
findIncludeFile(cl_compiler_frontend frontend,
                CLIST_TYPE(char_ptr) include_path,
                cstring pathname,
                cstring filename) {
  cl_bool find_the_include_file = CL_FALSE;
  CLIST_ITER_TYPE(char_ptr) iter;
  
  ASSERT(frontend != NULL);
  
  for (iter = CLIST_BEGIN(char_ptr)(include_path);
       iter != CLIST_END(char_ptr)(include_path);
       iter = CLIST_ITER_INCREMENT(char_ptr)(iter)) {
    char const *path = *CLIST_ITER_GET_DATA(char_ptr)(iter);
    CSTRING_CLEAR(pathname);
    CSTRING_APPEND_STRING(pathname, path);
    CSTRING_APPEND_CHAR(pathname, '/');
    CSTRING_APPEND_STRING(pathname, CSTRING_RAW_DATA(filename));
    
    find_the_include_file = clCompilerFrontendOpenIncludeFile(
        frontend, CSTRING_RAW_DATA(pathname));
    if (CL_TRUE == find_the_include_file) {
      break;
    }
  }
  
  return find_the_include_file;
}

/* return CL_FALSE means we encounter some situation which means
 * an error and should not continue to compile. */
cl_compiler_error_code
clCompilerFrontendPreprocess(cl_compiler_frontend frontend) {
  cl_bool skip_remaining_char_in_the_directive_line;
  cl_int saved_parse_flags;
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  
  ASSERT(frontend != NULL);
  
  skip_remaining_char_in_the_directive_line = CL_TRUE;
  
  saved_parse_flags = frontend->parse_flags;
  /* Because this function doesn't care the WHITE_SPACE,
   * remove the PARSE_FLAG_CAN_RETURN_SPCAE_TOKEN from the parse_flags.
   *
   * Because we indeed need to see the LINEFEED token to indicate the end of the line where the
   * preprocessor directive resides, we enable 'PARSE_FLAG_CAN_RETURN_LINEFEED_TOKEN' flag
   * in the 'frontend->parse_flags'. */
  frontend->parse_flags =
    (PARSE_FLAG_CONVERT_PREPROCESSOR_NUMBER_TOKEN_TO_REAL_NUMBER |
     PARSE_FLAG_CAN_RETURN_LINEFEED_TOKEN);
  
  error_code = clCompilerFrontendGetNextTokenWithoutMacroSubst(frontend, CAN_NOT_RETURN_SPACE_TOKEN, NULL);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
 reprocess_if_directive:
  switch (frontend->token_type) {
  case TOKEN_TYPE_DEFINE:
    error_code = clCompilerFrontendPreprocessParseDefine(frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    break;
    
  case TOKEN_TYPE_UNDEF:
    error_code = clCompilerFrontendGetNextTokenWithoutMacroSubst(
        frontend, CAN_NOT_RETURN_SPACE_TOKEN, NULL);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    {
      CLIST_TYPE(_cl_compiler_token_type_pair) defined_tokens;
      
      ASSERT(TOKEN_TYPE_IDENTIFIER == frontend->token->type);
      
      defined_tokens = clCompilerTokenGetDefinedTokens((cl_compiler_identifier_token)(frontend->token));
      if (defined_tokens != NULL) {
        CLIST_DELETE(_cl_compiler_token_type_pair)(defined_tokens);
        clCompilerTokenSetDefinedTokens((cl_compiler_identifier_token)(frontend->token), NULL);
      }
    }
    break;
    
  case TOKEN_TYPE_IFDEF:
  {
    cl_bool success;
    error_code = clCompilerFrontendPreprocessParseIfdef(frontend, CL_TRUE, &success);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    if (CL_FALSE == success) {
      goto reprocess_if_directive;
    }
  }
  break;
    
  case TOKEN_TYPE_IFNDEF:
  {
    cl_bool success;
    error_code = clCompilerFrontendPreprocessParseIfdef(frontend, CL_FALSE, &success);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    if (CL_FALSE == success) {
      goto reprocess_if_directive;
    }
  }
  break;
    
  case TOKEN_TYPE_ELSE:
  {
    if (0 == CSTACK_SIZE(cl_int)(frontend->ifdef_stack)) {
      ASSERT(0 && "#else without matching #if");
    }
    {
      cl_int *token_type = CSTACK_TOP(cl_int)(frontend->ifdef_stack);
      if ((*token_type & 2) != 0) {
        ASSERT(0 && "#else after #else");
      }
      (*token_type) ^= 3;
      if (!(*token_type) & 1) {
        error_code = clCompilerFrontendPreprocessSkipThisIfdefBlock(frontend);
        if (error_code != COMPILER_NO_ERROR) {
          goto finish;
        }
        goto reprocess_if_directive;
      }
    }
  }
  break;
  
  case TOKEN_TYPE_ENDIF:
    if (0 == CSTACK_SIZE(cl_int)(frontend->ifdef_stack)) {
      ASSERT(0 && "#endif without matching #if");
    }
    CSTACK_POP(cl_int)(frontend->ifdef_stack);
    break;
    
  case TOKEN_TYPE_IF:
  {
    cl_int value;
    
    error_code = clCompilerFrontendEvalPreprocessExpression(frontend, &value);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    CSTACK_PUSH(cl_int)(frontend->ifdef_stack, &value);
    
    if (0 == (value & 1)) {
      error_code = clCompilerFrontendPreprocessSkipThisIfdefBlock(frontend);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      goto reprocess_if_directive;
    }
  }
  break;
  
  case TOKEN_TYPE_ELIF:
  {
    cl_int *last_value;
    
    if (0 == CSTACK_SIZE(cl_int)(frontend->ifdef_stack)) {
      ASSERT(0 && "#elif without matching #if");
    }
    last_value = CSTACK_TOP(cl_int)(frontend->ifdef_stack);
    if (*last_value > 1) {
      ASSERT(0 && "#elif after #else");
    }
    /* last #if/#elif expression was true: we skip */
    if (1 == *last_value) {
      error_code = clCompilerFrontendPreprocessSkipThisIfdefBlock(frontend);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      goto reprocess_if_directive;
    } else {
      cl_int new_value;
      
      error_code = clCompilerFrontendEvalPreprocessExpression(frontend, &new_value);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      *(CSTACK_TOP(cl_int)(frontend->ifdef_stack)) = new_value;
      
      if (0 == (new_value & 1)) {
        error_code = clCompilerFrontendPreprocessSkipThisIfdefBlock(frontend);
        if (error_code != COMPILER_NO_ERROR) {
          goto finish;
        }
        goto reprocess_if_directive;
      }
    }
  }
  break;
  
  case TOKEN_TYPE_PRAGMA:
    clCompilerFrontendPreprocessParsePragma(frontend);
    break;
    
  case TOKEN_TYPE_ERROR:
    clCompilerFrontendPreprocessParseErrorAndWarning(frontend, CL_TRUE);
    return COMPILER_ERROR__PREPROCESSOR__ENCOUNTER_ERROR_DIRECTIVE;
    
  case TOKEN_TYPE_WARNING:
    clCompilerFrontendPreprocessParseErrorAndWarning(frontend, CL_FALSE);
    break;
    
  case TOKEN_TYPE_LINE:
    /* get the new line number */
    error_code = clCompilerFrontendGetNextTokenWithMacroSubst(frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    if (frontend->token_type != TOKEN_TYPE_CONSTANT_INT) {
      ASSERT(0 && "#line");
    }
    
    {
      cl_compiler_number_token number_token = (cl_compiler_number_token)(frontend->token);
      
      /* the line number will be incremented after due to encountering
       * the linefeed character in the end of this line. */
      frontend->read_buffer->line_num =
        number_token->token_value->int_value - 1;
    }
    
    /* delete the token because it is useless. */
    clCompilerTokenDelete(frontend, frontend->token);
    frontend->token = NULL;
    
    /* get the '\n' or the new filename */
    error_code = clCompilerFrontendGetNextTokenWithMacroSubst(frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    
    if (frontend->token_type != TOKEN_TYPE_LINEFEED) {
      if (frontend->token_type != TOKEN_TYPE_STRING) {
        ASSERT(0 && "#line");
      }
      clCompilerReadBufferSetFilename(
          frontend->read_buffer,
          clCompilerTokenGetString(frontend->token_type, frontend->token));
      
      /* delete the token because it is useless. */
      clCompilerTokenDelete(frontend, frontend->token);
      frontend->token = NULL;
    }
    break;
    
  case TOKEN_TYPE_INCLUDE:
  {
    cl_read_buffer_char ch;
    cl_bool include_type_search_curr_dir_first;
    cstring filename = NULL, pathname = NULL;
    cl_bool find_the_include_file;
    
    cl_compiler_read_buffer const read_buffer = frontend->read_buffer;
    cl_bool found = clCompilerReadBufferPeekChar(read_buffer, 0, &ch);
    
    /* skip space characters */
    while (CL_TRUE == clCompilerReadBufferCharIsSpaceChar(ch)) {
      found = clCompilerReadBufferGetCharSkipNewLineStray(read_buffer, &ch);
      ASSERT(CL_TRUE == found);
    }
    
    filename = CSTRING_NEW();
    
    if ('<' == ch) {
      clCompilerReadBufferConsumeChar(read_buffer);
      clCompilerFrontendReadHeaderFileName(frontend, '>', filename);
      include_type_search_curr_dir_first = CL_FALSE;
    } else if ('"' == ch) {
      clCompilerReadBufferConsumeChar(read_buffer);
      clCompilerFrontendReadHeaderFileName(frontend, '"', filename);      
      include_type_search_curr_dir_first = CL_TRUE;
    } else {
      /* computed #include : either we have only strings or
       * we have anything enclosed in '<>' */
      error_code = clCompilerFrontendGetNextTokenWithMacroSubst(frontend);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      if (TOKEN_TYPE_STRING == frontend->token_type) {
        CSTRING_APPEND_STRING(filename, clCompilerTokenGetString(
                                  frontend->token_type, frontend->token));
        error_code = clCompilerFrontendGetNextTokenWithMacroSubst(frontend);
        if (error_code != COMPILER_NO_ERROR) {
          goto finish;
        }
        include_type_search_curr_dir_first = CL_TRUE;
      } else {
        while (frontend->token_type != TOKEN_TYPE_LINEFEED) {
          CSTRING_APPEND_STRING(filename, clCompilerTokenGetString(
                                    frontend->token_type, frontend->token));
          error_code = clCompilerFrontendGetNextTokenWithMacroSubst(frontend);
          if (error_code != COMPILER_NO_ERROR) {
            goto finish;
          }
        }
        if ((CSTRING_SIZE(filename) < 2) ||
            (CSTRING_AT(filename, 0) != '<') ||
            (CSTRING_AT(filename, CSTRING_SIZE(filename) - 1) != '>')) {
          ASSERT(0 && "'#include' expects \"FILENAME\" or <FILENAME>");
        }
        CSTRING_SHIFT_FRONT(filename, 1);
        CSTRING_ERASE(filename, CSTRING_SIZE(filename) - 1, 1);
        include_type_search_curr_dir_first = CL_FALSE;
      }
    }
    
    find_the_include_file = CL_FALSE;
    
    /* check absolute include path */
    if (IS_ABSOLUTE_PATH(CSTRING_RAW_DATA(filename))) {
      find_the_include_file = clCompilerFrontendOpenIncludeFile(
          frontend, CSTRING_RAW_DATA(filename));
      
      if (CL_TRUE == find_the_include_file) {
        skip_remaining_char_in_the_directive_line = CL_FALSE;
      }
    } else {
      pathname = CSTRING_NEW();
      ASSERT(pathname != NULL);
      
      /* search in current dir if "header.h" */
      if (CL_TRUE == include_type_search_curr_dir_first) {
        size_t const size = clUtilFileBasename(
            clCompilerReadBufferGetFilename(read_buffer)) -
          clCompilerReadBufferGetFilename(read_buffer);
        
        CSTRING_APPEND_PARTIAL_STRING(
            pathname,
            clCompilerReadBufferGetFilename(read_buffer),
            0,
            size);
        
        CSTRING_APPEND_STRING(pathname, CSTRING_RAW_DATA(filename));
        
        find_the_include_file = clCompilerFrontendOpenIncludeFile(
            frontend, CSTRING_RAW_DATA(pathname));
        if (CL_TRUE == find_the_include_file) {
          skip_remaining_char_in_the_directive_line = CL_FALSE;
        }
      }
      
      if (CL_FALSE == find_the_include_file) {
        find_the_include_file = findIncludeFile(frontend,
                                                frontend->include_paths,
                                                pathname,
                                                filename);
        skip_remaining_char_in_the_directive_line = CL_FALSE;
      }
      
      if (CL_FALSE == find_the_include_file) {
        find_the_include_file = findIncludeFile(frontend,
                                                frontend->system_include_paths,
                                                pathname,
                                                filename);
        skip_remaining_char_in_the_directive_line = CL_FALSE;
      }
    }
    
    CSTRING_DELETE(pathname);
    CSTRING_DELETE(filename);
  }
  break;
  
  default:
    ASSERT(0 && "unknown preprocessor directive");
    break;
  }
  
  /* In almost all cases, we want to skip remainding tokens in the same line,
   * including the last LINEFEED token, so that we can re-get the real next token
   * in the beginning of the next line in the caller function. */
  if (CL_TRUE == skip_remaining_char_in_the_directive_line) {
    /* get tokens until the LINEFEED */
    while (frontend->token_type != TOKEN_TYPE_LINEFEED) {
      error_code = clCompilerFrontendGetNextTokenWithoutMacroSubst(
          frontend, CAN_NOT_RETURN_SPACE_TOKEN, NULL);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
    }
    ASSERT(TOKEN_TYPE_LINEFEED == frontend->token_type);
  }
  
  frontend->parse_flags = saved_parse_flags;
  
 finish:
  return error_code;
}

static token_number_catagory_enum
determineTokenNumberCatagory(int const suffix_l_count,
                             int const suffix_u_count,
                             int const base) {
  if (10 == base) {
    switch (suffix_u_count) {
    case 0:
      switch (suffix_l_count) {
      case 0: return TOKEN_NUMBER_CAT_UNSUFFIXED_DECIMAL; break;
      case 1: return TOKEN_NUMBER_CAT_SUFFIXED_BY_L_DECIMAL; break;
      case 2: return TOKEN_NUMBER_CAT_SUFFIXED_BY_LL_DECIMAL; break;
      default: ASSERT(0); return TOKEN_NUMBER_CAT_UNSUFFIXED_DECIMAL; break;
      }
      break;
    case 1:
      switch (suffix_l_count) {
      case 0: return TOKEN_NUMBER_CAT_SUFFIXED_BY_U; break;
      case 1: return TOKEN_NUMBER_CAT_SUFFIXED_BY_U_L; break;
      case 2: return TOKEN_NUMBER_CAT_SUFFIXED_BY_U_LL; break;
      default: ASSERT(0); return TOKEN_NUMBER_CAT_UNSUFFIXED_DECIMAL; break;
      }
      break;
    default:
      ASSERT(0);
      return TOKEN_NUMBER_CAT_UNSUFFIXED_DECIMAL;
      break;
    }
  } else {
    ASSERT((8 == base) || (16 == base));
    switch (suffix_u_count) {
    case 0:
      switch (suffix_l_count) {
      case 0: return TOKEN_NUMBER_CAT_UNSUFFIXED_OCTAL_HEX; break;
      case 1: return TOKEN_NUMBER_CAT_SUFFIXED_BY_L_OCTAL_HEX; break;
      case 2: return TOKEN_NUMBER_CAT_SUFFIXED_BY_LL_OCTAL_HEX; break;
      default: ASSERT(0); return TOKEN_NUMBER_CAT_UNSUFFIXED_DECIMAL; break;
      }
      break;
    case 1:
      switch (suffix_l_count) {
      case 0: return TOKEN_NUMBER_CAT_SUFFIXED_BY_U; break;
      case 1: return TOKEN_NUMBER_CAT_SUFFIXED_BY_U_L; break;
      case 2: return TOKEN_NUMBER_CAT_SUFFIXED_BY_U_LL; break;
      default: ASSERT(0); return TOKEN_NUMBER_CAT_UNSUFFIXED_DECIMAL; break;
      }
      break;
    default:
      ASSERT(0);
      return TOKEN_NUMBER_CAT_UNSUFFIXED_DECIMAL;
      break;
    }
  }
}

/* parse number in null terminated string 'p' and return it in the
 * current token */
static void
clCompilerFrontendConvertPreprocessorNumberTokenToRealNumber(
    char const *str,
    cl_compiler_token_type * const token_type,
    cl_compiler_token_value token_value) {
  int ch1, ch2, base;
  cstring cstr;
  
  ASSERT(token_type != NULL);
  ASSERT(token_value != NULL);
  
  /* number */
  ch1 = *str;
  ++str;
  ch2 = *str;
  ++str;
  base = 10;
  cstr = CSTRING_NEW();
  ASSERT(cstr != NULL);
  
  CSTRING_APPEND_CHAR(cstr, ch1);
  
  if ('.' == ch1) {
    ch1 = ch2;
    goto begin_with_decimal_fractional_part;
  } else if ('0' == ch1) {
    if (('x' == ch2) || ('X' == ch2)) {
      CSTRING_APPEND_CHAR(cstr, ch2);
      
      ch1 = *str;
      ++str;
      
      base = 16;
    } else {
      ch1 = ch2;
    }
  } else {
    ch1 = ch2;
  }
  
  /* parse all digits before the suffix L or U or LL or E or P.
   * cannot check octal numbers at this stage
   * because of floating point constants */
  for (;;) {
    int value;
    if ((ch1 >= 'a') && (ch1 <= 'f')) {
      value = ch1 - 'a' + 10;
    } else if ((ch1 >= 'A') && (ch1 <= 'F')) {
      value = ch1 - 'A' + 10;
    } else if (CL_TRUE == clUtilCharIsDecimalDigit(ch1)) {
      value = ch1 - '0';
    } else {
      break;
    }
    if (value >= base) {
      break;
    }
    CSTRING_APPEND_CHAR(cstr, ch1);
    ch1 = *str;
    ++str;
  }
  
  if (('.' == ch1) ||
      ((('e' == ch1) || ('E' == ch1)) && (10 == base)) ||
      ((('p' == ch1) || ('P' == ch1)) && (16 == base))) {
    /* floating number */
    if (base != 10) {
      /* hexadecimal floats */
      cl_bool has_fractional_part = CL_FALSE;
      cl_bool has_integral_part = CL_FALSE;
      
      if (CSTRING_LENGTH(cstr) != 0) {
        has_integral_part = CL_TRUE;
      }
      
      if ('.' == ch1) {
        /* get the fractional part */
        CSTRING_APPEND_CHAR(cstr, ch1);
        ch1 = *str;
        ++str;
        
        while (((ch1 >= '0') && (ch1 <= '9')) ||
               ((ch1 >= 'A') && (ch1 <= 'F')) ||
               ((ch1 >= 'a') && (ch1 <= 'f'))) {
          has_fractional_part = CL_TRUE;
          CSTRING_APPEND_CHAR(cstr, ch1);
          ch1 = *str;
          ++str;
        }
      }
      
      if ((CL_FALSE == has_integral_part) &&
          (CL_FALSE == has_fractional_part)) {
        ASSERT(0 && "Can not omit both the integral part and the fractional part");
      }
      
      if (('p' == ch1) || ('P' == ch1)) {
        /* get the exponent part */
        CSTRING_APPEND_CHAR(cstr, ch1);
        ch1 = *str;
        ++str;
        if (('-' == ch1) || ('+' == ch1)) {
          CSTRING_APPEND_CHAR(cstr, ch1);
          ch1 = *str;
          ++str;
        }
        if ((ch1 < '0') || (ch1 > '9')) {
          ASSERT(0 && "exponent digits");
        }
        while ((ch1 >= '0') && (ch1 <= '9')) {
          CSTRING_APPEND_CHAR(cstr, ch1);
          ch1 = *str;
          ++str;
        }
      } else {
        ASSERT(0 && "The binary exponent part is required");
      }
      
      {
        int const suffix = convertLowercaseCharToUppercaseChar(ch1);
        errno = 0;
        if ('F' == suffix) {
          ch1 = *str;
          ++str;
          *token_type = TOKEN_TYPE_CONSTANT_FLOAT;
          (*token_value).float_value =
            strtof(CSTRING_RAW_DATA(cstr), NULL);
        } else if ('L' == suffix) {
          ch1 = *str;
          ++str;
          *token_type = TOKEN_TYPE_CONSTANT_LONG_DOUBLE;
          (*token_value).longdouble_value =
            strtold(CSTRING_RAW_DATA(cstr), NULL);
        } else {
          *token_type = TOKEN_TYPE_CONSTANT_DOUBLE;
          (*token_value).double_value =
            strtod(CSTRING_RAW_DATA(cstr), NULL);
        }
        if (errno != 0) {
          ASSERT(0 && "decimal floating point constant overflow");
        }
      }
    } else {
      /* decimal floats */
      cl_bool has_decimal_point = CL_FALSE;
      cl_bool has_exponent_part = CL_FALSE;
      cl_bool has_fractional_part = CL_FALSE;
      cl_bool has_integral_part = CL_FALSE;
      
      if (CSTRING_LENGTH(cstr) != 0) {
        has_integral_part = CL_TRUE;
      }
      
      if ('.' == ch1) {
        has_decimal_point = CL_TRUE;
        /* get the fractional part */
        CSTRING_APPEND_CHAR(cstr, ch1);
        ch1 = *str;
        ++str;
      begin_with_decimal_fractional_part:
        while ((ch1 >= '0') && (ch1 <= '9')) {
          has_fractional_part = CL_TRUE;
          CSTRING_APPEND_CHAR(cstr, ch1);
          ch1 = *str;
          ++str;
        }
      }
      
      if ((CL_FALSE == has_integral_part) &&
          (CL_FALSE == has_fractional_part)) {
        ASSERT(0 && "Can not omit both the integral part and the fractional part");
      }
      
      if (('e' == ch1) || ('E' == ch1)) {
        has_exponent_part = CL_TRUE;
        /* get the exponent part */
        CSTRING_APPEND_CHAR(cstr, ch1);
        ch1 = *str;
        ++str;
        if (('-' == ch1) || ('+' == ch1)) {
          CSTRING_APPEND_CHAR(cstr, ch1);
          ch1 = *str;
          ++str;
        }
        if ((ch1 < '0') || (ch1 > '9')) {
          ASSERT(0 && "exponent digits");
        }
        while ((ch1 >= '0') && (ch1 <= '9')) {
          CSTRING_APPEND_CHAR(cstr, ch1);
          ch1 = *str;
          ++str;
        }
      }
      
      if ((CL_FALSE == has_decimal_point) &&
          (CL_FALSE == has_exponent_part)) {
        ASSERT(0 && "Can not omit both the decimal point and the exponent part");
      }
      
      {
        int const suffix = convertLowercaseCharToUppercaseChar(ch1);
        errno = 0;
        if ('F' == suffix) {
          ch1 = *str;
          ++str;
          *token_type = TOKEN_TYPE_CONSTANT_FLOAT;
          (*token_value).float_value =
            strtof(CSTRING_RAW_DATA(cstr), NULL);
        } else if ('L' == suffix) {
          ch1 = *str;
          ++str;
          *token_type = TOKEN_TYPE_CONSTANT_LONG_DOUBLE;
          (*token_value).longdouble_value =
            strtold(CSTRING_RAW_DATA(cstr), NULL);
        } else {
          *token_type = TOKEN_TYPE_CONSTANT_DOUBLE;
          (*token_value).double_value =
            strtod(CSTRING_RAW_DATA(cstr), NULL);
        }
        if (errno != 0) {
          ASSERT(0 && "decimal floating point constant overflow");
        }
      }
    }
  } else {
    /* integer number */
    
    /* Integer literal 	       Possible data types
       -------------------------------------------------------------
       unsuffixed decimal 	   int, long int, C only long long int
       -------------------------------------------------------------
       unsuffixed octal or
       hexadecimal 	           int, unsigned int, long int, unsigned long int,
       long long int1, unsigned long long int
       -------------------------------------------------------------
       decimal, octal, or
       hexadecimal suffixed
       by u or U 	             unsigned int, unsigned long int,
       unsigned long long int
       -------------------------------------------------------------
       decimal suffixed by
       l or L                  long int, long long int
       -------------------------------------------------------------
       octal or hexadecimal
       suffixed by l or L      long int, unsigned long int, long long int,
       unsigned long long int
       -------------------------------------------------------------
       decimal, octal, or
       hexadecimal suffixed
       by both u or U,
       and l or L              unsigned long int, unsigned long long int
       -------------------------------------------------------------
       decimal suffixed by
       ll or LL                long long int
       -------------------------------------------------------------
       octal or hexadecimal
       suffixed by ll or LL    long long int, unsigned long long int
       -------------------------------------------------------------
       decimal, octal, or
       hexadecimal suffixed
       by both u or U, and
       ll or LL                unsigned long long int
       ------------------------------------------------------------- */
    int suffix_l_count = 0;
    int suffix_u_count = 0;
    cl_bool just_meet_an_l = CL_FALSE;
    char *cstr_raw_data_ptr;
    token_number_catagory_enum token_number_cat;
    cl_compiler_token_type tmp_token_type;
    _cl_compiler_token_value tmp_token_value;
    
    /* get 'L', 'l', 'U', 'u' suffix if any first, so that we can base on
     * this info to determine the integer type. */
    for (;;) {
      int const suffix = convertLowercaseCharToUppercaseChar(ch1);
      if ('L' == suffix) {
        switch (suffix_l_count) {
        case 0:
          ++suffix_l_count;
          just_meet_an_l = CL_TRUE;
          break;
        case 1:
          if (CL_TRUE == just_meet_an_l) {
            ++suffix_l_count;
          } else {
            ASSERT(0 && "Discontinuous 'l's in integer constant");
          }
          break;
        default:
          ASSERT(0 && "three 'l's in integer constant");
          break;
        }
        ch1 = *str;
        ++str;
      } else if ('U' == suffix) {
        just_meet_an_l = CL_FALSE;
        if (suffix_u_count >= 1) {
          ASSERT(0 && "two 'u's in integer constant");
        }
        ++suffix_u_count;
        ch1 = *str;
        ++str;
      } else {
        break;
      }
    }
    
    cstr_raw_data_ptr = CSTRING_RAW_DATA(cstr);
    
    /* check if the integer is octal integer or not. */
    if ((10 == base) && ('0' == *cstr_raw_data_ptr)) {
      base = 8;
      ++cstr_raw_data_ptr;
    }
    
    token_number_cat = determineTokenNumberCatagory(suffix_l_count,
                                                    suffix_u_count,
                                                    base);
    
    tmp_token_type = TOKEN_TYPE_CONSTANT_INT;
    
    /* determine the value and type of this integer constant */
    tmp_token_value.int_value = 0;
    tmp_token_value.uint_value = 0;
    tmp_token_value.long_value = 0;
    tmp_token_value.ulong_value = 0;
    tmp_token_value.longlong_value = 0;
    tmp_token_value.ulonglong_value = 0;
    
    for (;;) {
      cl_int t = *cstr_raw_data_ptr;
      ++cstr_raw_data_ptr;
      /* no need for checks except for base 10 / 8 errors */
      if ('\0' == t) {
        break;
      } else if ((t >= 'a') && (t <= 'f')) {
        t = t - 'a' + 10;
      } else if ((t >= 'A') && (t <= 'F')) {
        t = t - 'A' + 10;
      } else {
        t = t - '0';
        if (t >= base) {
          ASSERT(0 && "invalid digit");
        }
      }
      
#define CHECK_AND_UPGRADE(curr_range_max, curr_range_field,             \
                          next_range, next_range_field)                 \
      do {                                                              \
        if ((((curr_range_max) - t) / base) >= tmp_token_value.curr_range_field) { \
          /* still in this integer type */                              \
          tmp_token_value.curr_range_field = tmp_token_value.curr_range_field * base + t; \
          finish_finding_the_right_type = CL_TRUE;                      \
        } else {                                                        \
          tmp_token_type = TOKEN_TYPE_CONSTANT_##next_range;            \
          tmp_token_value.next_range_field = tmp_token_value.curr_range_field; \
        }                                                               \
      } while(0)
      
#define CHECK_AND_FAIL(curr_range_max, curr_range_field)                \
      do {                                                              \
        if ((((curr_range_max) - t) / base) >= tmp_token_value.curr_range_field) { \
          /* still in this integer type */                              \
          tmp_token_value.curr_range_field = tmp_token_value.curr_range_field * base + t; \
          finish_finding_the_right_type = CL_TRUE;                      \
        } else {                                                        \
          ASSERT(0 && "integer constant overflow");                     \
        }                                                               \
      } while(0)
      
#define CHECK_AND_FAIL_SIMPLE(curr_range_max, curr_range_field)         \
      do {                                                              \
        if ((((curr_range_max) - t) / base) >= tmp_token_value.curr_range_field) { \
          /* still in this integer type */                              \
          tmp_token_value.curr_range_field = tmp_token_value.curr_range_field * base + t; \
        } else {                                                        \
          ASSERT(0 && "integer constant overflow");                     \
        }                                                               \
      } while(0)
      
      switch (token_number_cat) {
      case TOKEN_NUMBER_CAT_UNSUFFIXED_DECIMAL:
      {
        cl_bool finish_finding_the_right_type = CL_FALSE;
        tmp_token_type = TOKEN_TYPE_CONSTANT_INT;
        
        while (CL_FALSE == finish_finding_the_right_type) {
          switch (tmp_token_type) {
          case TOKEN_TYPE_CONSTANT_INT:
            CHECK_AND_UPGRADE(INT_MAX, int_value, LONG_INT, long_value);
            break;
          case TOKEN_TYPE_CONSTANT_LONG_INT:
            CHECK_AND_UPGRADE(LONG_MAX, long_value, LONGLONG_INT, longlong_value);
            break;
          case TOKEN_TYPE_CONSTANT_LONGLONG_INT:
            CHECK_AND_FAIL(LLONG_MAX, longlong_value);
            break;
          default:
            ASSERT(0);
            break;
          }
        }
        break;
      }
      
      case TOKEN_NUMBER_CAT_UNSUFFIXED_OCTAL_HEX:
      {
        cl_bool finish_finding_the_right_type = CL_FALSE;
        tmp_token_type = TOKEN_TYPE_CONSTANT_INT;
        
        while (CL_FALSE == finish_finding_the_right_type) {
          switch (tmp_token_type) {
          case TOKEN_TYPE_CONSTANT_INT:
            CHECK_AND_UPGRADE(INT_MAX, int_value, UNSIGNED_INT, uint_value);
            break;
          case TOKEN_TYPE_CONSTANT_UNSIGNED_INT:
            CHECK_AND_UPGRADE(UINT_MAX, uint_value, LONG_INT, long_value);
            break;
          case TOKEN_TYPE_CONSTANT_LONG_INT:
            CHECK_AND_UPGRADE(LONG_MAX, long_value, UNSIGNED_LONG_INT, ulong_value);
            break;
          case TOKEN_TYPE_CONSTANT_UNSIGNED_LONG_INT:
            CHECK_AND_UPGRADE(ULONG_MAX, ulong_value, LONGLONG_INT, longlong_value);
            break;
          case TOKEN_TYPE_CONSTANT_LONGLONG_INT:
            CHECK_AND_UPGRADE(LLONG_MAX, longlong_value, UNSIGNED_LONGLONG_INT, ulonglong_value);
            break;
          case TOKEN_TYPE_CONSTANT_UNSIGNED_LONGLONG_INT:
            CHECK_AND_FAIL(ULLONG_MAX, ulonglong_value);
            break;
          default:
            ASSERT(0);
            break;
          }
        }
        break;
      }
      
      case TOKEN_NUMBER_CAT_SUFFIXED_BY_U:
      {
        cl_bool finish_finding_the_right_type = CL_FALSE;
        tmp_token_type = TOKEN_TYPE_CONSTANT_UNSIGNED_INT;
        
        while (CL_FALSE == finish_finding_the_right_type) {
          switch (tmp_token_type) {
          case TOKEN_TYPE_CONSTANT_UNSIGNED_INT:
            CHECK_AND_UPGRADE(UINT_MAX, uint_value, UNSIGNED_LONG_INT, ulong_value);
            break;
          case TOKEN_TYPE_CONSTANT_UNSIGNED_LONG_INT:
            CHECK_AND_UPGRADE(ULONG_MAX, ulong_value, UNSIGNED_LONGLONG_INT, ulonglong_value);
            break;
          case TOKEN_TYPE_CONSTANT_UNSIGNED_LONGLONG_INT:
            CHECK_AND_FAIL(ULLONG_MAX, ulonglong_value);
            break;
          default:
            ASSERT(0);
            break;
          }
        }
        break;
      }
      
      case TOKEN_NUMBER_CAT_SUFFIXED_BY_L_DECIMAL:
      {
        cl_bool finish_finding_the_right_type = CL_FALSE;
        tmp_token_type = TOKEN_TYPE_CONSTANT_LONG_INT;
        
        while (CL_FALSE == finish_finding_the_right_type) {
          switch (tmp_token_type) {
          case TOKEN_TYPE_CONSTANT_LONG_INT:
            CHECK_AND_UPGRADE(LONG_MAX, long_value, LONGLONG_INT, longlong_value);
            break;
          case TOKEN_TYPE_CONSTANT_LONGLONG_INT:
            CHECK_AND_FAIL(LLONG_MAX, longlong_value);
            break;
          default:
            ASSERT(0);
            break;
          }
        }
        break;
      }
      
      case TOKEN_NUMBER_CAT_SUFFIXED_BY_L_OCTAL_HEX:
      {
        cl_bool finish_finding_the_right_type = CL_FALSE;
        tmp_token_type = TOKEN_TYPE_CONSTANT_LONG_INT;
        
        while (CL_FALSE == finish_finding_the_right_type) {
          switch (tmp_token_type) {
          case TOKEN_TYPE_CONSTANT_LONG_INT:
            CHECK_AND_UPGRADE(LONG_MAX, long_value, UNSIGNED_LONG_INT, ulong_value);
            break;
          case TOKEN_TYPE_CONSTANT_UNSIGNED_LONG_INT:
            CHECK_AND_UPGRADE(ULONG_MAX, ulong_value, LONGLONG_INT, longlong_value);
            break;
          case TOKEN_TYPE_CONSTANT_LONGLONG_INT:
            CHECK_AND_UPGRADE(LLONG_MAX, longlong_value, UNSIGNED_LONGLONG_INT, ulonglong_value);
            break;
          case TOKEN_TYPE_CONSTANT_UNSIGNED_LONGLONG_INT:
            CHECK_AND_FAIL(ULLONG_MAX, ulonglong_value);
            break;
          default:
            ASSERT(0);
            break;
          }
        }
        break;
      }
      
      case TOKEN_NUMBER_CAT_SUFFIXED_BY_U_L:
      {
        cl_bool finish_finding_the_right_type = CL_FALSE;
        tmp_token_type = TOKEN_TYPE_CONSTANT_UNSIGNED_LONG_INT;
        
        while (CL_FALSE == finish_finding_the_right_type) {
          switch (tmp_token_type) {
          case TOKEN_TYPE_CONSTANT_UNSIGNED_LONG_INT:
            CHECK_AND_UPGRADE(ULONG_MAX, ulong_value, UNSIGNED_LONGLONG_INT, ulonglong_value);
            break;
          case TOKEN_TYPE_CONSTANT_UNSIGNED_LONGLONG_INT:
            CHECK_AND_FAIL(ULLONG_MAX, ulonglong_value);
            break;
          default:
            ASSERT(0);
            break;
          }
        }
        break;
      }
      
      case TOKEN_NUMBER_CAT_SUFFIXED_BY_LL_DECIMAL:
      {
        CHECK_AND_FAIL_SIMPLE(LLONG_MAX, longlong_value);
        break;
      }
      
      case TOKEN_NUMBER_CAT_SUFFIXED_BY_LL_OCTAL_HEX:
      {
        cl_bool finish_finding_the_right_type = CL_FALSE;
        tmp_token_type = TOKEN_TYPE_CONSTANT_LONGLONG_INT;
        
        while (CL_FALSE == finish_finding_the_right_type) {
          switch (tmp_token_type) {
          case TOKEN_TYPE_CONSTANT_LONGLONG_INT:
            CHECK_AND_UPGRADE(LLONG_MAX, longlong_value, UNSIGNED_LONGLONG_INT, ulonglong_value);
            break;
          case TOKEN_TYPE_CONSTANT_UNSIGNED_LONGLONG_INT:
            CHECK_AND_FAIL(ULLONG_MAX, ulonglong_value);
            break;
          default:
            ASSERT(0);
            break;
          }
        }
        break;
      }
      
      case TOKEN_NUMBER_CAT_SUFFIXED_BY_U_LL:
      {
        CHECK_AND_FAIL_SIMPLE(ULLONG_MAX, ulonglong_value);
        break;
      }
      
      default:
        ASSERT(0);
        break;
      }
      
#undef CHECK_AND_UPGRADE
#undef CHECK_AND_FAIL
#undef CHECK_AND_FAIL_SIMPLE
    }
    *token_type = tmp_token_type;
    *token_value = tmp_token_value;
  }
  if (ch1 != '\0') {
    ASSERT(0 && "invalid number");
  }
  CSTRING_DELETE(cstr);
}

static void
clCompilerFrontendMacroSubstHandleTwoSharps(
    cl_compiler_frontend frontend,
    CLIST_TYPE(_cl_compiler_token_type_pair) token_list) {
  cstring cstr;
  CLIST_ITER_TYPE(_cl_compiler_token_type_pair) prev_iter = NULL;
  _cl_compiler_token_type_pair *prev_element = NULL, *curr_element = NULL;
  cl_bool meet_twosharps;
  
  /* we search if there is '##' */
  CLIST_ITER_TYPE(_cl_compiler_token_type_pair) e;
  for (e = CLIST_BEGIN(_cl_compiler_token_type_pair)(token_list);
       e != CLIST_END(_cl_compiler_token_type_pair)(token_list);
       e = CLIST_ITER_INCREMENT(_cl_compiler_token_type_pair)(e)) {
    if (TOKEN_TYPE_TWOSHARPS ==
        CLIST_ITER_GET_DATA(_cl_compiler_token_type_pair)(e)->token_type) {
      /* find a '##' */
      break;
    }
  }
  
  /* nothing more to do if we don't find a '##' */
  if (e == CLIST_END(_cl_compiler_token_type_pair)(token_list)) {
    return;
  }
  
  /* we saw '##', so we need more processing to handle it */
  cstr = CSTRING_NEW();
  ASSERT(cstr != NULL);
  
  meet_twosharps = CL_FALSE;
  for (e = CLIST_BEGIN(_cl_compiler_token_type_pair)(token_list);
       e != CLIST_END(_cl_compiler_token_type_pair)(token_list);
       ) {
    curr_element = CLIST_ITER_GET_DATA(_cl_compiler_token_type_pair)(e);
    
    if (TOKEN_TYPE_TWOSHARPS == curr_element->token_type) {
      /* If the current token is "##", then don't update 'prev_iter' */
      meet_twosharps = CL_TRUE;
      /* remove this "##" token right now */
      e = CLIST_ERASE(_cl_compiler_token_type_pair)(token_list, e);
      continue;
    } else {
      if (CL_TRUE == meet_twosharps) {
        /* just encounter a '##', concatenate the previous & the current token string
         * and check if it can form a new token string. */
        char const *prev_string;
        char const *curr_string;
        struct _cl_compiler_token_type_pair new_token_element;
        cl_compiler_token token;
        cl_bool do_not_merge_just_pasting_directly = CL_FALSE;
        
        prev_element = CLIST_ITER_GET_DATA(_cl_compiler_token_type_pair)(prev_iter);
        curr_element = CLIST_ITER_GET_DATA(_cl_compiler_token_type_pair)(e);
        
        if (NULL == prev_element) {
          ASSERT(0 && "'##' cannot appear at either end of a macro expension.");
        }
        prev_string = clCompilerTokenGetString(prev_element->token_type,
                                               prev_element->token);
        curr_string = clCompilerTokenGetString(curr_element->token_type,
                                               curr_element->token);
        
        /* concatenate the two tokens' string */
        CSTRING_APPEND_STRING(cstr, prev_string);
        CSTRING_APPEND_STRING(cstr, curr_string);
        
        token = NULL;
        /* determine if the two tokens can form a new token */
        if (((TOKEN_TYPE_PREPROCESSOR_NUMBER == prev_element->token_type) ||
             (TOKEN_TYPE_IDENTIFIER == prev_element->token_type)) &&
            ((TOKEN_TYPE_PREPROCESSOR_NUMBER == curr_element->token_type) ||
             (TOKEN_TYPE_IDENTIFIER == curr_element->token_type))) {
          if (TOKEN_TYPE_PREPROCESSOR_NUMBER == prev_element->token_type) {
            /* if previous token is a number, then whether if the next token
             * is a number or an identifier, we should treat it as a number,
             * hence to create a new number token.
             *
             * If the new number token is actually not a valid number token,
             * ex: 3x, then the parser will find it out and fire an error. */
            new_token_element.token_type = TOKEN_TYPE_PREPROCESSOR_NUMBER;
          } else {
            /* if identifier, before we create a new identifier token,
             * we must do a test to validate if we have a correct identifier.
             *
             * If both tokens are identifiers, we do not need to do any checing,
             * but if the 2nd token is a number, we need to do the checking. */
            if (TOKEN_TYPE_PREPROCESSOR_NUMBER == curr_element->token_type) {
              char const *p;
              for (p = curr_string; p != '\0'; ++p) {
                if (CL_FALSE == isReadBufferCharEasciiIdOrDecimalDigit(*p)) {
                  /* the current token string contains invalide identifier
                   * type character, hence still treating these 2 tokens as
                   * two separate ones. */
                  goto find_an_error_and_pasting_together_directly;
                }
              }
            }
            new_token_element.token_type = TOKEN_TYPE_IDENTIFIER;
          }
          token = clCompilerTokenNew(frontend, new_token_element.token_type, CSTRING_RAW_DATA(cstr));
        } else {
          if (CL_TRUE == CSTRING_COMPARE(cstr, ">>=")) {
            new_token_element.token_type = TOKEN_TYPE_ASSIGN_RIGHT_SHIFT;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, "<<=")) {
            new_token_element.token_type = TOKEN_TYPE_ASSIGN_LEFT_SHIFT;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, "##")) {
            new_token_element.token_type = TOKEN_TYPE_TWOSHARPS;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, "<=")) {
            new_token_element.token_type = TOKEN_TYPE_LESS_EQUAL;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, ">=")) {
            new_token_element.token_type = TOKEN_TYPE_GREATER_EQUAL;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, "<<")) {
            new_token_element.token_type = TOKEN_TYPE_LEFT_SHIFT;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, ">>")) {
            new_token_element.token_type = TOKEN_TYPE_RIGHT_SHIFT;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, "++")) {
            new_token_element.token_type = TOKEN_TYPE_INCREMENT;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, "--")) {
            new_token_element.token_type = TOKEN_TYPE_DECREMENT;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, "&&")) {
            new_token_element.token_type = TOKEN_TYPE_LOGICAL_AND;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, "||")) {
            new_token_element.token_type = TOKEN_TYPE_LOGICAL_OR;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, "==")) {
            new_token_element.token_type = TOKEN_TYPE_EQUAL;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, "!=")) {
            new_token_element.token_type = TOKEN_TYPE_NOT_EQUAL;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, "&=")) {
            new_token_element.token_type = TOKEN_TYPE_ASSIGN_AND;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, "|=")) {
            new_token_element.token_type = TOKEN_TYPE_ASSIGN_OR;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, "^=")) {
            new_token_element.token_type = TOKEN_TYPE_ASSIGN_XOR;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, "+=")) {
            new_token_element.token_type = TOKEN_TYPE_ASSIGN_ADDITION;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, "-=")) {
            new_token_element.token_type = TOKEN_TYPE_ASSIGN_SUBTRACTION;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, "*=")) {
            new_token_element.token_type = TOKEN_TYPE_ASSIGN_MULTIPLY;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, "/=")) {
            new_token_element.token_type = TOKEN_TYPE_ASSIGN_DIVISION;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, "%=")) {
            new_token_element.token_type = TOKEN_TYPE_ASSIGN_MODULO;
          } else if (CL_TRUE == CSTRING_COMPARE(cstr, "->")) {
            new_token_element.token_type = TOKEN_TYPE_ARROW;
          } else {
          find_an_error_and_pasting_together_directly:
            do_not_merge_just_pasting_directly = CL_TRUE;
          }
        }
        
        if (CL_FALSE == do_not_merge_just_pasting_directly) {
          CLIST_ITER_TYPE(_cl_compiler_token_type_pair) loc;
          
          new_token_element.token = token;
          
          /* remove prev token */
          CLIST_ERASE(_cl_compiler_token_type_pair)(token_list, prev_iter);
          
          /* remove curr token */
          loc = CLIST_ERASE(_cl_compiler_token_type_pair)(token_list, e);
          
          /* inser the new token */
          e = CLIST_INSERT(_cl_compiler_token_type_pair)(token_list, loc, &new_token_element);
        }
        
        meet_twosharps = CL_FALSE;
        CSTRING_CLEAR(cstr);
      } else {
        prev_iter = e;
      }
      
      e = CLIST_ITER_INCREMENT(_cl_compiler_token_type_pair)(e);
    }
  }
  CSTRING_DELETE(cstr);
}

static cl_bool
HandledDefinedSearchCallback(cl_compiler_token * token, void *data) {
  ASSERT(token != NULL);
  ASSERT(data != NULL);
  if (token == data) {
    return CL_TRUE;
  } else {
    return CL_FALSE;
  }
}

static cl_bool
ArgumentVectorSearchCallback(_cl_compiler_token_type_pair *e1,
                             void *e2_) {
  ASSERT(e1 != NULL);
  ASSERT(e2_ != NULL);
  
  {
    _cl_compiler_token_type_pair *e2 = (_cl_compiler_token_type_pair *)e2_;
    return (e1->token == e2->token) ? CL_TRUE : CL_FALSE;
  }
}

static cl_bool
ArgumentVectorSearch(
    CVECTOR_TYPE(_cl_compiler_token_type_pair) argument_vector,
    _cl_compiler_token_type_pair * const element,
    CVECTOR_ITER_TYPE(_cl_compiler_token_type_pair) * const argument_vector_iter) {
  ASSERT(argument_vector != NULL);
  ASSERT(element != NULL);
  ASSERT(argument_vector_iter != NULL);
  
  *argument_vector_iter = 
    CVECTOR_SEARCH(_cl_compiler_token_type_pair)(
        argument_vector, element, ArgumentVectorSearchCallback);
  
  if (*argument_vector_iter != CVECTOR_END(_cl_compiler_token_type_pair)(
          argument_vector)) {
    return CL_TRUE;
  } else {
    return CL_FALSE;
  }
}

static cl_compiler_error_code
clCompilerFrontendMacroSubstOfToken(
    cl_compiler_frontend frontend,
    CLIST_TYPE(_cl_compiler_token_type_pair) final_preprocessed_token_list,
    cl_compiler_token_type const token_type,
    cl_compiler_token token,
    CVECTOR_TYPE(cl_compiler_token) handled_defined,
    CLIST_TYPE(_token_list_and_its_iter) macro_subst_input_source,
    cl_bool * const substitution_happened);

static cl_compiler_error_code
clCompilerFrontendMacroSubstOfTokenArray(
    cl_compiler_frontend frontend,
    CLIST_TYPE(_cl_compiler_token_type_pair) final_preprocessed_token_list,
    CLIST_TYPE(_cl_compiler_token_type_pair) source_token_list,
    CVECTOR_TYPE(cl_compiler_token) handled_defined,
    CLIST_TYPE(_token_list_and_its_iter) macro_subst_input_source) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  CLIST_ITER_TYPE(_cl_compiler_token_type_pair) e;
  
  /* first scan for '##' operator handling */
  clCompilerFrontendMacroSubstHandleTwoSharps(frontend, source_token_list);
  
  /* scan all the tokens and do the concatenation */
  for (e = CLIST_BEGIN(_cl_compiler_token_type_pair)(source_token_list);
       e != CLIST_END(_cl_compiler_token_type_pair)(source_token_list);
       e = CLIST_ITER_INCREMENT(_cl_compiler_token_type_pair)(e)) {
    _cl_compiler_token_type_pair *element = 
      CLIST_ITER_GET_DATA(_cl_compiler_token_type_pair)(e);
    
    CLIST_TYPE(_cl_compiler_token_type_pair) defined_tokens = NULL;
    
    if (TOKEN_TYPE_IDENTIFIER == element->token->type) {
      defined_tokens = clCompilerTokenGetDefinedTokens((cl_compiler_identifier_token)(element->token));
    }
    
    if (defined_tokens != NULL) {
      if (CVECTOR_SEARCH(cl_compiler_token)(
              handled_defined, element->token,
              HandledDefinedSearchCallback) != NULL) {
        /* if nested substitution, do not replace it with its defined_tokens,
         * add this token into the final_preprocessed_token_list directly. */
        if (element->token_type != TOKEN_TYPE_WHITE_SPACE) {
          _cl_compiler_token_type_pair new_element = { element->token_type, element->token };
          CLIST_PUSH_BACK(_cl_compiler_token_type_pair)(final_preprocessed_token_list, &new_element);
        }
      } else {
        if (macro_subst_input_source != NULL) {
          /* Handle the following situation:
           * 
           * 1 #define aaa bbb
           * 2 #define bbb(x,y) x##y
           * 3 #define ttt(x,y) aaa(x,y)
           * 4
           * 5 int
           * 6 main() {
           * 7   ttt(a,b);
           * 8   return 0;
           * 9 }
           * 
           * 1) ttt -> aaa
           * 2) aaa -> bbb
           * 3) because bbb is function-like macro, search '('
           * 4) but due to line 1 doesn't have '(', search back to line 3,
           *    find the '('.
           *
           * This code snippet can be compiled by gcc and Comeau EDG frontend.
           */
          _token_list_and_its_iter list_and_iter = { source_token_list, &e };
          CLIST_PUSH_BACK(_token_list_and_its_iter)(macro_subst_input_source, &list_and_iter);
        }
        
        {
          cl_bool subst;
          
          error_code = clCompilerFrontendMacroSubstOfToken(
              frontend,
              final_preprocessed_token_list,
              element->token_type,
              element->token,
              handled_defined,
              macro_subst_input_source,
              &subst);
          if (error_code != COMPILER_NO_ERROR) {
            goto finish;
          }
          
          if (CL_FALSE == subst) {
            if (element->token_type != TOKEN_TYPE_WHITE_SPACE) {
              _cl_compiler_token_type_pair new_element = {
                element->token_type, element->token };
              
              CLIST_PUSH_BACK(_cl_compiler_token_type_pair)(
                  final_preprocessed_token_list, &new_element);
            }
          }
        }
      }
    } else {
      if (element->token_type != TOKEN_TYPE_WHITE_SPACE) {
        _cl_compiler_token_type_pair new_element = {
          element->token_type, element->token };
        
        CLIST_PUSH_BACK(_cl_compiler_token_type_pair)(
            final_preprocessed_token_list, &new_element);
      }
    }
  }
  
 finish:
  return error_code;
}

static cl_compiler_error_code
clCompilerFrontendMacroSubstOfArgument(
    cl_compiler_frontend frontend,
    CLIST_TYPE(_cl_compiler_token_type_pair) token_list_after_replacing_argument,
    CLIST_TYPE(_cl_compiler_token_type_pair) source_token_list,
    CVECTOR_TYPE(_cl_compiler_token_type_pair) argument_vector,
    CVECTOR_TYPE(CLIST_TYPE(_cl_compiler_token_type_pair)) parameter_list_vector,
    CVECTOR_TYPE(cl_compiler_token) handled_defined) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  CLIST_ITER_TYPE(_cl_compiler_token_type_pair) e = NULL, prev_e = NULL;
  e = CLIST_BEGIN(_cl_compiler_token_type_pair)(source_token_list);
  
  for (;;) {
    if (e == CLIST_END(_cl_compiler_token_type_pair)(source_token_list)) {
      break;
    }
    if (TOKEN_TYPE_SHARP == /* '#' */
        CLIST_ITER_GET_DATA(_cl_compiler_token_type_pair)(e)->token_type) {
      /* stringize */
      /* remove '#' */
      
      CVECTOR_ITER_TYPE(_cl_compiler_token_type_pair) argument_vector_iter = NULL;
      
      /* advance to the next source token just after the '#' */
      e = CLIST_ITER_INCREMENT(_cl_compiler_token_type_pair)(e);
      if (e == CLIST_END(_cl_compiler_token_type_pair)(source_token_list)) {
        /* see '#' but without any furthur tokens */
        ASSERT(0 && "error: expected a macro parameter name");
      }
      
      /* check to see if the token matchs an argument */
      if (CL_TRUE == ArgumentVectorSearch(
              argument_vector, 
              CLIST_ITER_GET_DATA(_cl_compiler_token_type_pair)(e),
              &argument_vector_iter)) {
        size_t const argument_vector_index =
          CVECTOR_ITER_GET_INDEX(_cl_compiler_token_type_pair)(
              argument_vector, argument_vector_iter);
        
        CVECTOR_ITER_TYPE(CLIST_TYPE(_cl_compiler_token_type_pair)) parameter_list_vector_iter = 
          CVECTOR_AT(CLIST_TYPE(_cl_compiler_token_type_pair))(
              parameter_list_vector, argument_vector_index);
        
        CLIST_TYPE(_cl_compiler_token_type_pair) *parameter_list_ptr = 
          CVECTOR_ITER_GET_DATA(CLIST_TYPE(_cl_compiler_token_type_pair))(
              parameter_list_vector_iter);
        
        CLIST_TYPE(_cl_compiler_token_type_pair) parameter_list = *parameter_list_ptr;
        
        cstring cstr = CSTRING_NEW();
        
        CLIST_ITER_TYPE(_cl_compiler_token_type_pair) parameter_list_iter = 
          CLIST_BEGIN(_cl_compiler_token_type_pair)(parameter_list);
        for (;
             parameter_list_iter != CLIST_END(_cl_compiler_token_type_pair)(
                 parameter_list);
             parameter_list_iter = CLIST_ITER_INCREMENT(_cl_compiler_token_type_pair)(
                 parameter_list_iter)) {
          _cl_compiler_token_type_pair *parameter =
            CLIST_ITER_GET_DATA(_cl_compiler_token_type_pair)(parameter_list_iter);
          
          CSTRING_APPEND_STRING(
              cstr, clCompilerTokenGetString(parameter->token_type,
                                             parameter->token));
        }
        
        {
          struct _cl_compiler_token_type_pair element;
          element.token_type = TOKEN_TYPE_STRING;
          element.token = clCompilerTokenNew(frontend, TOKEN_TYPE_STRING, CSTRING_RAW_DATA(cstr));
          
          CLIST_PUSH_BACK(_cl_compiler_token_type_pair)(
              token_list_after_replacing_argument, &element);
        }
        
        CSTRING_DELETE(cstr);
      } else {
        /* this token doesn't match any argument, put it into the
         * token_list_after_replacing_argument directly */
        struct _cl_compiler_token_type_pair element = {
          CLIST_ITER_GET_DATA(_cl_compiler_token_type_pair)(e)->token_type,
          CLIST_ITER_GET_DATA(_cl_compiler_token_type_pair)(e)->token };      
        
        CLIST_PUSH_BACK(_cl_compiler_token_type_pair)(token_list_after_replacing_argument, &element);
      }
    } else if (TOKEN_TYPE_IDENTIFIER ==
               CLIST_ITER_GET_DATA(_cl_compiler_token_type_pair)(e)->token_type) {
      CVECTOR_ITER_TYPE(_cl_compiler_token_type_pair) argument_vector_iter;
      
      /* Check if this token matchs an argument */
      if (CL_TRUE == ArgumentVectorSearch(
              argument_vector, 
              CLIST_ITER_GET_DATA(_cl_compiler_token_type_pair)(e),
              &argument_vector_iter)) {
        /* This token is corresponding to an argument, hence find the corresponding
         * parameter tokens */
        size_t const argument_vector_index =
          CVECTOR_ITER_GET_INDEX(_cl_compiler_token_type_pair)(
              argument_vector, argument_vector_iter);
        
        CVECTOR_ITER_TYPE(CLIST_TYPE(_cl_compiler_token_type_pair))
          parameter_list_vector_iter = 
          CVECTOR_AT(CLIST_TYPE(_cl_compiler_token_type_pair))(
              parameter_list_vector, argument_vector_index);
        
        CLIST_TYPE(_cl_compiler_token_type_pair) *parameter_list_ptr = 
          CVECTOR_ITER_GET_DATA(CLIST_TYPE(_cl_compiler_token_type_pair))(
              parameter_list_vector_iter);
        
        CLIST_TYPE(_cl_compiler_token_type_pair) parameter_list = *parameter_list_ptr;
        
        CLIST_ITER_TYPE(_cl_compiler_token_type_pair) next_e =
          CLIST_ITER_INCREMENT(_cl_compiler_token_type_pair)(e);
        
        /* if '##' is present before or after, no arg substitution */
        if (/* check if the previous token is '##' */
            ((prev_e != NULL) &&
             (TOKEN_TYPE_TWOSHARPS == CLIST_ITER_GET_DATA(
                 _cl_compiler_token_type_pair)(prev_e)->token_type)) ||
            /* check if the next token is '##' */
            ((next_e != CLIST_END(_cl_compiler_token_type_pair)(source_token_list)) &&
             (TOKEN_TYPE_TWOSHARPS == CLIST_ITER_GET_DATA(
                 _cl_compiler_token_type_pair)(next_e)->token_type))) {
          /* The argument corresponding to the parameters is next to or previous to
           * a '##', hence, put the parameter tokens into the
           * token_list_after_replacing_argument directly without argument replacing. */
          CLIST_ITER_TYPE(_cl_compiler_token_type_pair) parameter_list_iter = 
            CLIST_BEGIN(_cl_compiler_token_type_pair)(parameter_list);
          for (;
               parameter_list_iter != CLIST_END(_cl_compiler_token_type_pair)(
                   parameter_list);
               parameter_list_iter = CLIST_ITER_INCREMENT(_cl_compiler_token_type_pair)(
                   parameter_list_iter)) {
            _cl_compiler_token_type_pair *parameter =
              CLIST_ITER_GET_DATA(_cl_compiler_token_type_pair)(parameter_list_iter);
            
            struct _cl_compiler_token_type_pair element = {
              parameter->token_type, parameter->token };
            
            CLIST_PUSH_BACK(_cl_compiler_token_type_pair)(
                token_list_after_replacing_argument, &element);
          }
        } else {
          /* try to preprocess the parameter tokens and put them into
           * token_list_after_replacing_argument. */
          error_code = clCompilerFrontendMacroSubstOfTokenArray(
              frontend,
              token_list_after_replacing_argument,
              parameter_list,
              handled_defined,
              NULL);
          if (error_code != COMPILER_NO_ERROR) {
            goto finish;
          }
        }
      } else {
        /* this token doesn't match any arguments, put it into the
         * token_list_after_replacing_argument directly. */
        struct _cl_compiler_token_type_pair element = {
          CLIST_ITER_GET_DATA(_cl_compiler_token_type_pair)(e)->token_type,
          CLIST_ITER_GET_DATA(_cl_compiler_token_type_pair)(e)->token };      
        
        CLIST_PUSH_BACK(_cl_compiler_token_type_pair)(
            token_list_after_replacing_argument, &element);
      }
    } else {
      struct _cl_compiler_token_type_pair element = {
        CLIST_ITER_GET_DATA(_cl_compiler_token_type_pair)(e)->token_type,
        CLIST_ITER_GET_DATA(_cl_compiler_token_type_pair)(e)->token };      
      
      CLIST_PUSH_BACK(_cl_compiler_token_type_pair)(
          token_list_after_replacing_argument, &element);
    }
    prev_e = e;
    e = CLIST_ITER_INCREMENT(_cl_compiler_token_type_pair)(e);
  }
  
 finish:
  return error_code;
}

/* do macro substitution of current token.
 * 'handled_defined' is the list of all
 * macros we got inside to avoid recursing.
 * Return CL_FALSE if no substitution needs to be done */
static cl_compiler_error_code
clCompilerFrontendMacroSubstOfToken(
    cl_compiler_frontend frontend,
    CLIST_TYPE(_cl_compiler_token_type_pair) final_preprocessed_token_list,
    cl_compiler_token_type const token_type,
    cl_compiler_token token,
    CVECTOR_TYPE(cl_compiler_token) handled_defined,
    CLIST_TYPE(_token_list_and_its_iter) macro_subst_input_source,
    cl_bool * const substitution_happened) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  
  CLIST_TYPE(_cl_compiler_token_type_pair) source_token_list = NULL;
  CLIST_TYPE(_cl_compiler_token_type_pair) token_list_after_replacing_argument = NULL;
  CVECTOR_TYPE(CLIST_TYPE(_cl_compiler_token_type_pair)) parameter_list_vector = NULL;
  
  ASSERT(substitution_happened != NULL);
  
  /* if symbol is a macro, prepare substitution */
  /* special macros */
  if (TOKEN_TYPE___LINE__ == token_type) {
    cstring cstr = CSTRING_NEW();
    
    CSTRING_PRINTF(cstr, "%d", frontend->read_buffer->line_num);
    
    {
      cl_compiler_token token = clCompilerTokenNew(
          frontend, TOKEN_TYPE_PREPROCESSOR_NUMBER, CSTRING_RAW_DATA(cstr));
      
      {
        _cl_compiler_token_type_pair element = { TOKEN_TYPE_PREPROCESSOR_NUMBER, token };
        CLIST_PUSH_BACK(_cl_compiler_token_type_pair)(final_preprocessed_token_list, &element);
      }
    }
    
    CSTRING_DELETE(cstr);
    cstr = NULL;
  } else if (TOKEN_TYPE___FILE__ == token_type) {
    char * const filename = clCompilerReadBufferGetFilename(frontend->read_buffer);
    cl_compiler_token token = clCompilerTokenNew(frontend, TOKEN_TYPE_STRING, filename);
    
    {
      _cl_compiler_token_type_pair element = { TOKEN_TYPE_STRING, token };
      CLIST_PUSH_BACK(_cl_compiler_token_type_pair)(final_preprocessed_token_list, &element);
    }
  } else if ((TOKEN_TYPE___DATE__ == token_type) ||
             (TOKEN_TYPE___TIME__ == token_type)) {
    cstring cstr = CSTRING_NEW();
    
    time_t ti;
    time(&ti);
    
    {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
      struct tm *tm = localtime(&ti);
#ifdef _MSC_VER
#pragma warning(pop)
#endif
      if (TOKEN_TYPE___DATE__ == token_type) {
        CSTRING_PRINTF(cstr, "%s %2d %d",
                       monthName[tm->tm_mon],
                       tm->tm_mday, tm->tm_year + 1900);
      } else {
        CSTRING_PRINTF(cstr, "%02d:%02d:%02d", 
                       tm->tm_hour, tm->tm_min, tm->tm_sec);
      }
    }
    
    {
      cl_compiler_token token = clCompilerTokenNew(frontend, TOKEN_TYPE_STRING, CSTRING_RAW_DATA(cstr));
      
      {
        _cl_compiler_token_type_pair element = { TOKEN_TYPE_STRING, token };
        CLIST_PUSH_BACK(_cl_compiler_token_type_pair)(final_preprocessed_token_list, &element);
      }
    }
    
    CSTRING_DELETE(cstr);
    cstr = NULL;
  } else {
    source_token_list = CLIST_NEW(_cl_compiler_token_type_pair)();
    
    ASSERT(TOKEN_TYPE_IDENTIFIER == token_type);
    ASSERT(token != NULL);
    
    CLIST_COPY(_cl_compiler_token_type_pair)(
        source_token_list, clCompilerTokenGetDefinedTokens((cl_compiler_identifier_token)token));
    
    if (MACRO_TYPE_FUNCTION_LIKE == clCompilerTokenGetMacroType((cl_compiler_identifier_token)token)) {
      cl_bool get_a_token = CL_FALSE;
      
      CVECTOR_TYPE(_cl_compiler_token_type_pair) argument_vector = NULL;
      CVECTOR_ITER_TYPE(_cl_compiler_token_type_pair) argument_vector_iter = NULL;
      
      while (CL_FALSE == get_a_token) {
        if (CLIST_SIZE(_token_list_and_its_iter)(macro_subst_input_source) != 0) {
          _token_list_and_its_iter * const list_and_iter =
            CLIST_BACK(_token_list_and_its_iter)(macro_subst_input_source);
          
          if (*(list_and_iter->iter) == CLIST_END(_cl_compiler_token_type_pair)(
                  list_and_iter->list)) {
            CLIST_POP_BACK(_token_list_and_its_iter)(macro_subst_input_source);
            get_a_token = CL_FALSE;
          } else {
            _cl_compiler_token_type_pair * const token_type_pair = 
              CLIST_ITER_GET_DATA(_cl_compiler_token_type_pair)(
                  *list_and_iter->iter);
            
            frontend->token_type = token_type_pair->token_type;
            frontend->token = token_type_pair->token;
            
            CLIST_ITER_INCREMENT(_cl_compiler_token_type_pair)(
                *list_and_iter->iter);
            
            get_a_token = CL_TRUE;
          }
        } else {
          error_code =
            clCompilerFrontendGetNextTokenWithoutMacroSubst(frontend, CAN_NOT_RETURN_SPACE_TOKEN, NULL);
          if (error_code != COMPILER_NO_ERROR) {
            goto finish;
          }
          get_a_token = CL_TRUE;
        }
      }
      
      /* check if the next token is '(' */
      if (frontend->token_type != TOKEN_TYPE_LEFT_PAREN) {
        /* no macro subst */
        (*substitution_happened) = CL_FALSE;
        error_code = COMPILER_NO_ERROR;
        goto finish;
      }
      
      argument_vector = clCompilerTokenGetDefinedArguments((cl_compiler_identifier_token)token);
      argument_vector_iter = CVECTOR_BEGIN(_cl_compiler_token_type_pair)(argument_vector);
      parameter_list_vector = CVECTOR_NEW(CLIST_TYPE(_cl_compiler_token_type_pair))(0);
      
      /* get the next token just after the '(' */
      error_code =
        clCompilerFrontendGetNextTokenWithoutMacroSubst(frontend, CAN_NOT_RETURN_SPACE_TOKEN, NULL);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      
      for (;;) {
        CLIST_TYPE(_cl_compiler_token_type_pair) parameter_list = NULL;
        
        if (/* check if we finished all arguments */
            (argument_vector_iter ==
             CVECTOR_END(_cl_compiler_token_type_pair)(argument_vector)) &&
            /* and we have found the ')' */
            (TOKEN_TYPE_RIGHT_PAREN == frontend->token_type)) {
          /* finish finding all parameters */
          break;
        }
        
        if (argument_vector_iter ==
            CVECTOR_END(_cl_compiler_token_type_pair)(argument_vector)) {
          /* if we are in here, it means we have processed all the arguments,
           * but still can not find the ')'. It means the macro invocation uses
           * too many parameters. */
          ASSERT(0 && "used with too many args");
        }
        
        /* create a parameter. Because a parameter may consist more than one
         * token, we have to use a container to store the parameter tokens. */
        parameter_list = CLIST_NEW(_cl_compiler_token_type_pair)();
        ASSERT(parameter_list != NULL);
        CVECTOR_PUSH_BACK(CLIST_TYPE(_cl_compiler_token_type_pair))(
            parameter_list_vector, &parameter_list);
        
        /* get all tokens in a parameter (corresponding to an argument) */
        {
          cl_uint parameter_level = 0;
          /* find out each parameter one at a time.
           * Ex: #define macro(a, b, c) ...
           *     macro(1, 2, 3)
           * The following while loop gather '1', '2', '3' tokens
           * one at a time. */
          while (
              /* if we are in the nested parameter leve,
               * ex: macro(1, (3, 5), 6)
               *               ^
               * we are not finish the parameter finding. */
              ((parameter_level > 0) || (
                  /* ')' means we are finish the current nested parameter level or 
                   * the overall parameter finding. */
                  (frontend->token_type != TOKEN_TYPE_RIGHT_PAREN) /* ) */ &&
                  /* ',' means we are finish finding a parameter. */
                  (frontend->token_type != TOKEN_TYPE_COMMA) /* , */)) && 
              (frontend->token_type != TOKEN_TYPE_EOF)) {
            if (TOKEN_TYPE_LEFT_PAREN == frontend->token_type) {
              ++parameter_level;
            } else if (TOKEN_TYPE_RIGHT_PAREN == frontend->token_type) {
              --parameter_level;
            } else if (TOKEN_TYPE_LINEFEED == frontend->token_type) {
              frontend->token_type = TOKEN_TYPE_WHITE_SPACE;
            }
            
            if (frontend->token_type != TOKEN_TYPE_WHITE_SPACE) {
              /* we are find a token in a parameter. */
              _cl_compiler_token_type_pair e = {
                frontend->token_type, frontend->token };
              
              CLIST_PUSH_BACK(_cl_compiler_token_type_pair)(
                  parameter_list, &e);
            }
            
            error_code = clCompilerFrontendGetNextTokenWithoutMacroSubst(
                frontend, CAN_NOT_RETURN_SPACE_TOKEN, NULL);
            if (error_code != COMPILER_NO_ERROR) {
              goto finish;
            }
          }
        }
        
        /* finish getting all tokens in a parameter (corresponding to an argument),
         * advance to the next argument. */
        argument_vector_iter = CVECTOR_ITER_INCREMENT(_cl_compiler_token_type_pair)(
            argument_vector_iter);
        if (TOKEN_TYPE_RIGHT_PAREN == frontend->token_type) {
          break;
        }
        if (frontend->token_type != TOKEN_TYPE_COMMA) {
          ASSERT(0 && "expect ,");
        }
        error_code = clCompilerFrontendGetNextTokenWithoutMacroSubst(
            frontend, CAN_NOT_RETURN_SPACE_TOKEN, NULL);
        if (error_code != COMPILER_NO_ERROR) {
          goto finish;
        }
      }
      if (argument_vector_iter !=
          CVECTOR_END(_cl_compiler_token_type_pair)(argument_vector)) {
        ASSERT(0 && "used with too few args");
      }
      
      {
        token_list_after_replacing_argument =
          CLIST_NEW(_cl_compiler_token_type_pair)();
        
        error_code = clCompilerFrontendMacroSubstOfArgument(
            frontend,
            token_list_after_replacing_argument,
            source_token_list,
            argument_vector,
            parameter_list_vector,
            handled_defined);
        if (error_code != COMPILER_NO_ERROR) {
          goto finish;
        }
        
        CLIST_DELETE(_cl_compiler_token_type_pair)(source_token_list);
        source_token_list = NULL;
        
        {
          CVECTOR_ITER_TYPE(CLIST_TYPE(_cl_compiler_token_type_pair)) iter =
            CVECTOR_BEGIN(CLIST_TYPE(_cl_compiler_token_type_pair))(parameter_list_vector);
          while (iter != CVECTOR_END(
                     CLIST_TYPE(_cl_compiler_token_type_pair))(parameter_list_vector)) {
            CLIST_TYPE(_cl_compiler_token_type_pair) parameter =
              *CVECTOR_ITER_GET_DATA(CLIST_TYPE(_cl_compiler_token_type_pair))(iter);
            CLIST_DELETE(_cl_compiler_token_type_pair)(parameter);
            
            iter = CVECTOR_ITER_INCREMENT(CLIST_TYPE(_cl_compiler_token_type_pair))(iter);
          }
          CVECTOR_DELETE(CLIST_TYPE(_cl_compiler_token_type_pair))(parameter_list_vector);
          parameter_list_vector = NULL;
        }
      }
    } else {
      token_list_after_replacing_argument = source_token_list;
      source_token_list = NULL;
    }
    
    CVECTOR_PUSH_BACK(cl_compiler_token)(handled_defined, &token);
    
    error_code = clCompilerFrontendMacroSubstOfTokenArray(
        frontend,
        final_preprocessed_token_list,
        token_list_after_replacing_argument,
        handled_defined,
        macro_subst_input_source);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    
    CVECTOR_POP_BACK(cl_compiler_token)(handled_defined);
    
    CLIST_DELETE(_cl_compiler_token_type_pair)(token_list_after_replacing_argument);
    token_list_after_replacing_argument = NULL;
  }
  
  (*substitution_happened) = CL_TRUE;
  
 finish:
  if (token_list_after_replacing_argument != NULL) {
    CLIST_DELETE(_cl_compiler_token_type_pair)(token_list_after_replacing_argument);
  }
  if (source_token_list != NULL) {
    CLIST_DELETE(_cl_compiler_token_type_pair)(source_token_list);
  }
  if (parameter_list_vector != NULL) {
    CVECTOR_ITER_TYPE(CLIST_TYPE(_cl_compiler_token_type_pair)) iter =
      CVECTOR_BEGIN(CLIST_TYPE(_cl_compiler_token_type_pair))(parameter_list_vector);
    
    while (iter != CVECTOR_END(
               CLIST_TYPE(_cl_compiler_token_type_pair))(parameter_list_vector)) {
      CLIST_TYPE(_cl_compiler_token_type_pair) parameter =
        *CVECTOR_ITER_GET_DATA(CLIST_TYPE(_cl_compiler_token_type_pair))(iter);
      CLIST_DELETE(_cl_compiler_token_type_pair)(parameter);
      
      iter = CVECTOR_ITER_INCREMENT(CLIST_TYPE(_cl_compiler_token_type_pair))(iter);
    }
    
    CVECTOR_DELETE(CLIST_TYPE(_cl_compiler_token_type_pair))(parameter_list_vector);
  }
  return error_code;
}

static cl_bool
isCurrentTokenDefined(cl_compiler_frontend frontend) {
  if ((TOKEN_TYPE___FILE__ == frontend->token_type) ||
      (TOKEN_TYPE___LINE__ == frontend->token_type) ||
      (TOKEN_TYPE___DATE__ == frontend->token_type) ||
      (TOKEN_TYPE___TIME__ == frontend->token_type)) {
    return CL_TRUE;
  }
  
  if ((TOKEN_TYPE_IDENTIFIER == frontend->token_type) &&
      (clCompilerTokenGetDefinedTokens((cl_compiler_identifier_token)(frontend->token)) != NULL)) {
    return CL_TRUE;
  }
  
  return CL_FALSE;
}

/* return next token with macro substitution */
cl_compiler_error_code
clCompilerFrontendGetNextTokenWithMacroSubstImpl(
    cl_compiler_frontend frontend,
    cl_bool const is_get) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  
  CVECTOR_TYPE(cl_compiler_token) handled_defined = NULL;
  CLIST_TYPE(_cl_compiler_token_type_pair) final_preprocessed_token_list = NULL;
  CLIST_TYPE(_token_list_and_its_iter) macro_subst_input_source = NULL;
  
  ASSERT(frontend != NULL);
  
  frontend->token_type = TOKEN_TYPE_MEANINGLESS;
  frontend->token = NULL;
  
  while (TOKEN_TYPE_MEANINGLESS == frontend->token_type) {
    cl_compiler_can_return_space_token can_return_space_token;
    cl_bool is_token_preprocessed;
    
    if (frontend->parse_flags & PARSE_FLAG_CAN_RETURN_SPACE_TOKEN) {
      can_return_space_token = CAN_RETURN_SPACE_TOKEN;
    } else {
      can_return_space_token = CAN_NOT_RETURN_SPACE_TOKEN;
    }
    
    error_code = clCompilerFrontendGetNextTokenWithoutMacroSubst(
        frontend, can_return_space_token, &is_token_preprocessed);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    
    if (CL_FALSE == is_token_preprocessed) {
      /* if not reading from macro substituted string, then try
       * to substitute macros */
      if (CL_TRUE == isCurrentTokenDefined(frontend)) {
        /* we have a macro: we try to substitute */
        handled_defined = CVECTOR_NEW(cl_compiler_token)(0);
        final_preprocessed_token_list = CLIST_NEW(_cl_compiler_token_type_pair)();
        macro_subst_input_source = CLIST_NEW(_token_list_and_its_iter)();
        
        {
          cl_bool subst;
          error_code = clCompilerFrontendMacroSubstOfToken(
              frontend,
              final_preprocessed_token_list,
              frontend->token_type,
              frontend->token,
              handled_defined,
              macro_subst_input_source,
              &subst);
          if (error_code != COMPILER_NO_ERROR) {
            goto finish;
          }
          if (CL_TRUE == subst) {
            /* substitution done, NOTE: maybe empty */
            ASSERT(0 == CLIST_SIZE(_cl_compiler_token_type_pair)(frontend->preprocessed_token_list));
            CLIST_SPLICE(_cl_compiler_token_type_pair)(
                frontend->preprocessed_token_list, NULL, final_preprocessed_token_list);
            
            /* reset the token_type to reget the token. */
            frontend->token_type = TOKEN_TYPE_MEANINGLESS;
            frontend->token = NULL;
          }
        }
        
        CVECTOR_DELETE(cl_compiler_token)(handled_defined);
        handled_defined = NULL;
        
        CLIST_DELETE(_cl_compiler_token_type_pair)(final_preprocessed_token_list);
        final_preprocessed_token_list = NULL;
        
        CLIST_DELETE(_token_list_and_its_iter)(macro_subst_input_source);
        macro_subst_input_source = NULL;
      }
    }
  }
  ASSERT(frontend->token_type != TOKEN_TYPE_MEANINGLESS);
  
  if (frontend->token != NULL) {
    if (TOKEN_TYPE_IDENTIFIER == frontend->token->type) {
      ASSERT(NULL == ((cl_compiler_identifier_token)(frontend->token))->defined_tokens);
      ASSERT(NULL == ((cl_compiler_identifier_token)(frontend->token))->defined_arguments);
    }
  }
  
  if (CL_FALSE == is_get) {
    _cl_compiler_token_type_pair e = { frontend->token_type, frontend->token };
    CLIST_PUSH_FRONT(_cl_compiler_token_type_pair)(frontend->preprocessed_token_list, &e);
  }
  
  /* convert preprocessor tokens into C tokens */
  if ((TOKEN_TYPE_PREPROCESSOR_NUMBER == frontend->token_type) &&
      (frontend->parse_flags &
       PARSE_FLAG_CONVERT_PREPROCESSOR_NUMBER_TOKEN_TO_REAL_NUMBER) != 0) {
    cl_compiler_number_token number_token = (cl_compiler_number_token)(frontend->token);
    
    if (NULL == number_token->token_value) {
      cl_compiler_token_value token_value
        = CL_OSAL_CALLOC(sizeof(_cl_compiler_token_value));
      ASSERT(token_value != NULL);
      
      clCompilerFrontendConvertPreprocessorNumberTokenToRealNumber(
          number_token->str,
          &frontend->token_type,
          token_value);
      
      number_token->token_value = token_value;
    }
  }
  
 finish:
  if (handled_defined != NULL) {
    CVECTOR_DELETE(cl_compiler_token)(handled_defined);    
  }
  if (final_preprocessed_token_list != NULL) {
    CLIST_DELETE(_cl_compiler_token_type_pair)(final_preprocessed_token_list);
  }
  if (macro_subst_input_source != NULL) {
    CLIST_DELETE(_token_list_and_its_iter)(macro_subst_input_source);
  }
  return error_code;
}

/* Preprocess the current file */
cl_compiler_error_code
clCompilerFrontendPreprocessFile(cl_compiler_frontend frontend) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  cl_bool first_token_in_a_line;
  cl_int line_ref;
  cl_compiler_read_buffer read_buffer_ref;
  size_t define_start_size;
  
  ASSERT(frontend != NULL);
  
  clCompilerFrontendSetTokenFlags(frontend, TOKEN_FLAG_BEGIN_OF_LINE);
  clCompilerFrontendSetParseFlags(frontend, PARSE_FLAG_CAN_RETURN_SPACE_TOKEN | PARSE_FLAG_CAN_RETURN_LINEFEED_TOKEN);
  
  first_token_in_a_line = CL_TRUE;
  line_ref = 0;
  read_buffer_ref = NULL;
  
  define_start_size = clCompilerFrontendCurrDefinedTokenStackSize(frontend);
  
  for (;;) {
    error_code = clCompilerFrontendGetNextTokenWithMacroSubst(frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    if (TOKEN_TYPE_EOF == frontend->token_type) {
      break;
    } else if (TOKEN_TYPE_LINEFEED == frontend->token_type) {
      if (CL_TRUE == first_token_in_a_line) {
        continue;
      }
      ++line_ref;
      first_token_in_a_line = CL_TRUE;
    } else if (CL_TRUE == first_token_in_a_line) {
      cl_compiler_read_buffer curr_read_buffer = clCompilerFrontendGetCurrReadBuffer(frontend);
      cl_int const curr_line_num = clCompilerReadBufferGetCurrLineNum(curr_read_buffer);
      
      {
        cl_int diff = curr_line_num - line_ref;
        
        if ((curr_read_buffer != read_buffer_ref) || (diff < 0) || (diff >= 8)) {
          clOsalPrintf("# %d \"%s\"\n", curr_line_num,
                       clCompilerReadBufferGetFilename(curr_read_buffer));
        } else {
          while (diff != 0) {
            clOsalPrintf("\n");
            --diff;
          }
        }
      }
      
      read_buffer_ref = curr_read_buffer;
      line_ref = curr_line_num;
      first_token_in_a_line = CL_FALSE;
    }
    if (TOKEN_TYPE_STRING == frontend->token_type) {
      clOsalPrintf("\"%s\"", clCompilerTokenGetString(frontend->token_type, frontend->token));
    } else {
      clOsalPrintf(clCompilerTokenGetString(frontend->token_type, frontend->token));
    }
  }
  
  while (clCompilerFrontendCurrDefinedTokenStackSize(frontend) > define_start_size) {
    CSTACK_POP(cl_compiler_token)(clCompilerFrontendDefinedTokenStack(frontend));
  }
  
 finish:
  return error_code;
}
