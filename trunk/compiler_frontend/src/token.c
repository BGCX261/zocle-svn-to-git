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

#include <compiler_frontend/inc/token.h>
#include <compiler_frontend/inc/frontend.h>

#include <osal/inc/osal.h>

#define HASH_INIT        (1)
#define HASH_FUNC(h, c)  ((h) * 263 + (c))

cl_compiler_token
clCompilerTokenNew(cl_compiler_frontend frontend,
                   cl_compiler_token_type const token_type,
                   char const *str) {
  size_t str_len = 0;
  cl_compiler_token token;
  
  ASSERT(frontend != NULL);
  
  if (str != NULL) {
    str_len = strlen(str) + 1; /* +1 for the NULL terminator */
  }
  
  switch (token_type) {
  case TOKEN_TYPE_IDENTIFIER:
  {
    cl_compiler_identifier_token id_token =
      CL_OSAL_CALLOC(sizeof(struct _cl_compiler_identifier_token) + str_len);
    ASSERT(id_token != NULL);
    
    id_token->flags = 0;
    id_token->defined_tokens = NULL;
    id_token->defined_arguments = NULL;
    id_token->macro_type = MACRO_TYPE_OBJECT_LIKE;
    memcpy(id_token->str, str, str_len);
    
    token = (cl_compiler_token)id_token;
  }
  break;
  
  case TOKEN_TYPE_PREPROCESSOR_NUMBER:
  case TOKEN_TYPE_CONSTANT_INT:
  case TOKEN_TYPE_CONSTANT_UNSIGNED_INT:
  case TOKEN_TYPE_CONSTANT_LONG_INT:
  case TOKEN_TYPE_CONSTANT_UNSIGNED_LONG_INT:
  case TOKEN_TYPE_CONSTANT_LONGLONG_INT:
  case TOKEN_TYPE_CONSTANT_UNSIGNED_LONGLONG_INT:
  case TOKEN_TYPE_CONSTANT_FLOAT:
  case TOKEN_TYPE_CONSTANT_DOUBLE:
  case TOKEN_TYPE_CONSTANT_LONG_DOUBLE:
  {
    cl_compiler_number_token number_token =
      CL_OSAL_CALLOC(sizeof(struct _cl_compiler_number_token) + str_len);
    ASSERT(number_token != NULL);
    
    number_token->token_value = NULL;
    memcpy(number_token->str, str, str_len);
    
    token = (cl_compiler_token)number_token;
  }
  break;
  
  case TOKEN_TYPE_STRING:
  {
    cl_compiler_string_token string_token =
      CL_OSAL_CALLOC(sizeof(struct _cl_compiler_string_token) + str_len);
    ASSERT(string_token != NULL);
    
    memcpy(string_token->str, str, str_len);
    
    token = (cl_compiler_token)string_token;
  }
  break;
  default:
    ASSERT(0 && "should not reach here");
    break;
  }
  
  token->type = token_type;
  token->reference_count = 1;
  
  CLIST_ONE_LEVEL_PUSH_BACK(cl_compiler_token, all_tokens)(frontend->all_tokens, &token);
  
  return token;
}

void
clCompilerTokenDelete(cl_compiler_frontend frontend,
                      cl_compiler_token token) { 
  ASSERT(frontend != NULL);
  ASSERT(token != NULL);
  ASSERT(token->reference_count != 0);
  
  --token->reference_count;
  if (0 == token->reference_count) {
    CLIST_ONE_LEVEL_REMOVE(cl_compiler_token, all_tokens)(frontend->all_tokens, &token);
    
    switch (token->type) {
    case TOKEN_TYPE_IDENTIFIER:
    {
      cl_compiler_identifier_token id_token = (cl_compiler_identifier_token)token;
      if (id_token->defined_tokens != NULL) {
        CLIST_DELETE(_cl_compiler_token_type_pair)(id_token->defined_tokens);
      }
      if (id_token->defined_arguments != NULL) {
        CVECTOR_DELETE(_cl_compiler_token_type_pair)(id_token->defined_arguments);
      }
    }
    break;
    
    case TOKEN_TYPE_PREPROCESSOR_NUMBER:
    case TOKEN_TYPE_CONSTANT_INT:
    case TOKEN_TYPE_CONSTANT_UNSIGNED_INT:
    case TOKEN_TYPE_CONSTANT_LONG_INT:
    case TOKEN_TYPE_CONSTANT_UNSIGNED_LONG_INT:
    case TOKEN_TYPE_CONSTANT_LONGLONG_INT:
    case TOKEN_TYPE_CONSTANT_UNSIGNED_LONGLONG_INT:
    case TOKEN_TYPE_CONSTANT_FLOAT:
    case TOKEN_TYPE_CONSTANT_DOUBLE:
    case TOKEN_TYPE_CONSTANT_LONG_DOUBLE:
    {
      cl_compiler_number_token number_token = (cl_compiler_number_token)token;
      if (number_token->token_value != NULL) {
        CL_OSAL_FREE(number_token->token_value);
      }
    }
    break;
    
    case TOKEN_TYPE_STRING:
    default:
      break;
    }
    CL_OSAL_FREE(token);
  }
}

char const *special_identifier_token_table[] = {
  "include",
  "define",
  "undef",
  "if",
  "ifdef",
  "ifndef",
  "else",
  "elif",
  "endif",
  "defined",
  "warning",
  "error",
  "line",
  "pragma",
  "__LINE__",
  "__FILE__",
  "__DATE__",
  "__TIME__",
  "sizeof",
  "typedef",
  "extern",
  "static",
  "auto",
  "register",
  "void",
  "char",
  "short",
  "int",
  "long",
  "float",
  "double",
  "signed",
  "unsigned",
  "_Bool",
  "_Complex",
  "_Imaginary",
  "const",
  "restrict",
  "volatile",
  "inline",
  "switch",
  "break",
  "case",
  "continue",
  "default",
  "do",
  "for",
  "while",
  "if",
  "else",
  "goto",
  "return",
  "struct",
  "union",
  "enum"
};

cl_compiler_token_type
clCompilerTokenIsSpecialIdentifierToken(char const * const name) {
  size_t i;
  ASSERT(name != NULL);
  for (i = 0; i < sizeof(special_identifier_token_table) / sizeof(char const *); ++i) {
    if (0 == strcmp(name, special_identifier_token_table[i])) {
      return TOKEN_TYPE_INCLUDE + i;
    }
  }
  return TOKEN_TYPE_IDENTIFIER;
}

char const *
clCompilerTokenGetString(cl_compiler_token_type token_type,
                         cl_compiler_token token) {
  switch (token_type) {
  case TOKEN_TYPE_SHARP: return "#";
  case TOKEN_TYPE_TWOSHARPS: return "##";
  case TOKEN_TYPE_DOTS: return "...";
  case TOKEN_TYPE_DOT: return ".";
  case TOKEN_TYPE_LESS_THAN: return "<";
  case TOKEN_TYPE_LESS_EQUAL: return "<=";
  case TOKEN_TYPE_GREATER_THAN: return ">";
  case TOKEN_TYPE_GREATER_EQUAL: return ">=";
  case TOKEN_TYPE_LEFT_SHIFT: return "<<";
  case TOKEN_TYPE_RIGHT_SHIFT: return ">>";
  case TOKEN_TYPE_ADDITION: return "+";
  case TOKEN_TYPE_SUBTRACTION: return "-";
  case TOKEN_TYPE_MULTIPLY: return "*";
  case TOKEN_TYPE_DIVISION: return "/";
  case TOKEN_TYPE_MODULO: return "%";
  case TOKEN_TYPE_INCREMENT: return "++";
  case TOKEN_TYPE_DECREMENT: return "--";
  case TOKEN_TYPE_BITWISE_AND: return "&";
  case TOKEN_TYPE_BITWISE_OR: return "|";
  case TOKEN_TYPE_BITWISE_XOR: return "^";
  case TOKEN_TYPE_LOGICAL_AND: return "&&";
  case TOKEN_TYPE_LOGICAL_OR: return "||";
  case TOKEN_TYPE_ASSIGN: return "=";
  case TOKEN_TYPE_EQUAL: return "==";
  case TOKEN_TYPE_NOT_EQUAL: return "!=";
  case TOKEN_TYPE_ASSIGN_AND: return "&=";
  case TOKEN_TYPE_ASSIGN_OR: return "|=";
  case TOKEN_TYPE_ASSIGN_XOR: return "^=";
  case TOKEN_TYPE_ASSIGN_ADDITION: return "+=";
  case TOKEN_TYPE_ASSIGN_SUBTRACTION: return "-=";
  case TOKEN_TYPE_ASSIGN_MULTIPLY: return "*=";
  case TOKEN_TYPE_ASSIGN_DIVISION: return "/=";
  case TOKEN_TYPE_ASSIGN_MODULO: return "%=";
  case TOKEN_TYPE_ASSIGN_LEFT_SHIFT: return "<<=";
  case TOKEN_TYPE_ASSIGN_RIGHT_SHIFT: return ">>=";
  case TOKEN_TYPE_ARROW: return "->";
  case TOKEN_TYPE_TILDE: return "~";
  case TOKEN_TYPE_QUESTION_MARK: return "?";
  case TOKEN_TYPE_EXCLAMATION_MARK: return "!";
  case TOKEN_TYPE_COLON: return ":";
  case TOKEN_TYPE_SEMICOLON: return ";";
  case TOKEN_TYPE_COMMA: return ",";
  case TOKEN_TYPE_LEFT_PAREN: return "(";
  case TOKEN_TYPE_RIGHT_PAREN: return ")";
  case TOKEN_TYPE_LEFT_BRACKET: return "[";
  case TOKEN_TYPE_RIGHT_BRACKET: return "]";
  case TOKEN_TYPE_LEFT_BRACE: return "{";
  case TOKEN_TYPE_RIGHT_BRACE: return "}";
  case TOKEN_TYPE_SLASH: return "\\";
  case TOKEN_TYPE_LINEFEED: return "\n";
  case TOKEN_TYPE_WHITE_SPACE: return " ";
    
  case TOKEN_TYPE_IDENTIFIER:
    ASSERT(token != NULL);
    return ((cl_compiler_identifier_token)token)->str;
  case TOKEN_TYPE_PREPROCESSOR_NUMBER:
    ASSERT(token != NULL);
    return ((cl_compiler_number_token)token)->str;
  case TOKEN_TYPE_STRING:
  case TOKEN_TYPE_LONG_STRING:
  case TOKEN_TYPE_CHAR:
  case TOKEN_TYPE_LONG_CHAR:
    ASSERT(token != NULL);
    return ((cl_compiler_string_token)token)->str;
    
  case TOKEN_TYPE_KEYWORD_SIZEOF: return "sizeof";
  case TOKEN_TYPE_KEYWORD_TYPEDEF: return "typedef";
  case TOKEN_TYPE_KEYWORD_EXTERN: return "extern";
  case TOKEN_TYPE_KEYWORD_STATIC: return "static";
  case TOKEN_TYPE_KEYWORD_AUTO: return "auto";
  case TOKEN_TYPE_KEYWORD_REGISTER: return "register";
  case TOKEN_TYPE_KEYWORD_VOID: return "void";
  case TOKEN_TYPE_KEYWORD_CHAR: return "char";
  case TOKEN_TYPE_KEYWORD_SHORT: return "short";
  case TOKEN_TYPE_KEYWORD_INT: return "int";
  case TOKEN_TYPE_KEYWORD_LONG: return "long";
  case TOKEN_TYPE_KEYWORD_FLOAT: return "float";
  case TOKEN_TYPE_KEYWORD_DOUBLE: return "double";
  case TOKEN_TYPE_KEYWORD_SIGNED: return "signed";
  case TOKEN_TYPE_KEYWORD_UNSIGNED: return "unsigned";
  case TOKEN_TYPE_KEYWORD__BOOL: return "_Bool";
  case TOKEN_TYPE_KEYWORD__COMPLEX: return "_Complex";
  case TOKEN_TYPE_KEYWORD__IMAGINARY: return "_Imaginary";
  case TOKEN_TYPE_KEYWORD_CONST: return "const";
  case TOKEN_TYPE_KEYWORD_RESTRICT: return "restrict";
  case TOKEN_TYPE_KEYWORD_VOLATILE: return "volatile";
  case TOKEN_TYPE_KEYWORD_INLINE: return "inline";
  case TOKEN_TYPE_KEYWORD_SWITCH: return "switch";
  case TOKEN_TYPE_KEYWORD_BREAK: return "break";
  case TOKEN_TYPE_KEYWORD_CASE: return "case";
  case TOKEN_TYPE_KEYWORD_CONTINUE: return "continue";
  case TOKEN_TYPE_KEYWORD_DEFAULT: return "default";
  case TOKEN_TYPE_KEYWORD_DO: return "do";
  case TOKEN_TYPE_KEYWORD_FOR: return "for";
  case TOKEN_TYPE_KEYWORD_WHILE: return "while";
  case TOKEN_TYPE_KEYWORD_IF: return "if";
  case TOKEN_TYPE_KEYWORD_ELSE: return "else";
  case TOKEN_TYPE_KEYWORD_GOTO: return "goto";
  case TOKEN_TYPE_KEYWORD_RETURN: return "return";
  case TOKEN_TYPE_KEYWORD_STRUCT: return "struct";
  case TOKEN_TYPE_KEYWORD_UNION: return "union";
  case TOKEN_TYPE_KEYWORD_ENUM: return "enum";
    
  case TOKEN_TYPE___LINE__:
  case TOKEN_TYPE___FILE__:
  case TOKEN_TYPE___DATE__:
  case TOKEN_TYPE___TIME__:
  case TOKEN_TYPE_EOF:
  default:
    ASSERT(0);
    return NULL;
  }
}

/* ===============================================
 *   token hash table handling
 * =============================================== */

static cl_int
clCompilerTokenHashFuncByToken(cl_compiler_token token) {
  int h = HASH_INIT;
  char const *s = clCompilerTokenGetString(token->type, token);
  while (*s != 0) {
    h = HASH_FUNC(h, *s);
    ++s;
  }
  return h;
}

static cl_int
clCompilerTokenHashFuncByStr(char const *str) {
  int h = HASH_INIT;
  char const *s = str;
  while (*s != 0) {
    h = HASH_FUNC(h, *s);
    ++s;
  }
  return h;
}

static cl_bool
clCompilerTokenEqualFuncByToken(cl_compiler_token token1,
                                cl_compiler_token token2) {
  ASSERT(token1 != NULL);
  ASSERT(token2 != NULL);
  if (0 == strcmp(clCompilerTokenGetString(token1->type, token1),
                  clCompilerTokenGetString(token2->type, token2))) {
    return CL_TRUE;
  } else {
    return CL_FALSE;
  }
}

static cl_bool
clCompilerTokenEqualFuncByStr(cl_compiler_token token,
                              char const * str) {
  ASSERT(token != NULL);
  ASSERT(str != NULL);
  if (0 == strcmp(clCompilerTokenGetString(token->type, token), str)) {
    return CL_TRUE;
  } else {
    return CL_FALSE;
  }
}

void
clCompilerTokenHashTableSetDefaultFunction(
    HASHTABLE_TYPE(cl_compiler_token) hash_table) {
  ASSERT(hash_table != NULL);
  
  HASHTABLE_SET_HASH_FUNC(cl_compiler_token)(
      hash_table,
      (HASHTABLE_HASH_FUNC_TYPE(cl_compiler_token))clCompilerTokenHashFuncByToken);
  
  HASHTABLE_SET_EQUAL_FUNC(cl_compiler_token)(
      hash_table,
      (HASHTABLE_EQUAL_FUNC_TYPE(cl_compiler_token))clCompilerTokenEqualFuncByToken);
}

cl_compiler_token
clCompilerTokenHashTableSearchByStr(
    HASHTABLE_TYPE(cl_compiler_token) hash_table, char const *str) {
  HASHTABLE_SET_HASH_FUNC(cl_compiler_token)(
      hash_table,
      (HASHTABLE_HASH_FUNC_TYPE(cl_compiler_token))clCompilerTokenHashFuncByStr);
  
  HASHTABLE_SET_EQUAL_FUNC(cl_compiler_token)(
      hash_table,
      (HASHTABLE_EQUAL_FUNC_TYPE(cl_compiler_token))clCompilerTokenEqualFuncByStr);
  
  {
    HASHTABLE_ITER_TYPE(cl_compiler_token) iter = 
      HASHTABLE_SEARCH(cl_compiler_token)(hash_table, (void *)str);
    
    clCompilerTokenHashTableSetDefaultFunction(hash_table);
    
    if (iter != HASHTABLE_END(cl_compiler_token)(hash_table)) {
      return *HASHTABLE_ITER_GET_DATA(cl_compiler_token)(iter);
    } else {
      return NULL;
    }
  }
}

void
clCompilerTokenHashTableDeleteByStr(
    HASHTABLE_TYPE(cl_compiler_token) hash_table, char const *str) {
  ASSERT(hash_table != NULL);
  ASSERT(str != NULL);
  
  {
    cl_compiler_token token = 
      clCompilerTokenHashTableSearchByStr(hash_table, str);
    
    if (token != NULL) {
      HASHTABLE_REMOVE(cl_compiler_token)(hash_table, token);
    }
  }
}
