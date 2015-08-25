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
#ifndef ZOCLE_COMPILER_FRONTEND_TOKEN_H_
#define ZOCLE_COMPILER_FRONTEND_TOKEN_H_

#include <container/inc/hashtable.h>
#include <container/inc/clist.h>
#include <container/inc/clist_one_level.h>
#include <container/inc/cvector.h>

#include <osal/inc/osal.h>

typedef enum cl_compiler_token_type {
  TOKEN_TYPE_MEANINGLESS,
  
  TOKEN_TYPE_WHITE_SPACE,
  TOKEN_TYPE_LINEFEED,           /**< \n  */
  
  TOKEN_TYPE_SHARP,              /**< #   */
  TOKEN_TYPE_TWOSHARPS,          /**< ##  */
  
  TOKEN_TYPE_DOTS,               /**< ... */
  TOKEN_TYPE_DOT,                /**< .   */
  
  TOKEN_TYPE_LESS_THAN,          /**< <   */
  TOKEN_TYPE_LESS_EQUAL,         /**< <=  */
  TOKEN_TYPE_GREATER_THAN,       /**< >   */
  TOKEN_TYPE_GREATER_EQUAL,      /**< >=  */
  
  TOKEN_TYPE_LEFT_SHIFT,         /**< <<  */
  TOKEN_TYPE_RIGHT_SHIFT,        /**< >>  */
  
  TOKEN_TYPE_ADDITION,           /**< +   */
  TOKEN_TYPE_SUBTRACTION,        /**< -   */
  TOKEN_TYPE_MULTIPLY,           /**< *   */
  TOKEN_TYPE_DIVISION,           /**< /   */
  TOKEN_TYPE_MODULO,             /**< %   */
  
  TOKEN_TYPE_INCREMENT,          /**< ++  */
  TOKEN_TYPE_DECREMENT,          /**< --  */
  
  TOKEN_TYPE_BITWISE_AND,        /**< &   */
  TOKEN_TYPE_BITWISE_OR,         /**< |   */
  TOKEN_TYPE_BITWISE_XOR,        /**< ^   */
  TOKEN_TYPE_LOGICAL_AND,        /**< &&  */
  TOKEN_TYPE_LOGICAL_OR,         /**< ||  */
  
  TOKEN_TYPE_ASSIGN,             /**< =   */
  TOKEN_TYPE_EQUAL,              /**< ==  */
  TOKEN_TYPE_NOT_EQUAL,          /**< !=  */
  TOKEN_TYPE_ASSIGN_AND,         /**< &=  */
  TOKEN_TYPE_ASSIGN_OR,          /**< |=  */
  TOKEN_TYPE_ASSIGN_XOR,         /**< ^=  */
  TOKEN_TYPE_ASSIGN_ADDITION,    /**< +=  */
  TOKEN_TYPE_ASSIGN_SUBTRACTION, /**< -=  */
  TOKEN_TYPE_ASSIGN_MULTIPLY,    /**< *=  */
  TOKEN_TYPE_ASSIGN_DIVISION,    /**< /=  */
  TOKEN_TYPE_ASSIGN_MODULO,      /**< %=  */
  TOKEN_TYPE_ASSIGN_LEFT_SHIFT,  /**< <<= */
  TOKEN_TYPE_ASSIGN_RIGHT_SHIFT, /**< >>= */
  
  TOKEN_TYPE_ARROW,              /**< ->  */
  
  TOKEN_TYPE_TILDE,              /**< ~   */
  TOKEN_TYPE_QUESTION_MARK,      /**< ?   */
  TOKEN_TYPE_EXCLAMATION_MARK,   /**< !   */
  TOKEN_TYPE_COLON,              /**< :   */
  TOKEN_TYPE_SEMICOLON,          /**< ;   */
  TOKEN_TYPE_COMMA,              /**< ,   */
  TOKEN_TYPE_LEFT_PAREN,         /**< (   */
  TOKEN_TYPE_RIGHT_PAREN,        /**< )   */
  TOKEN_TYPE_LEFT_BRACKET,       /**< [   */
  TOKEN_TYPE_RIGHT_BRACKET,      /**< ]   */
  TOKEN_TYPE_LEFT_BRACE,         /**< {   */
  TOKEN_TYPE_RIGHT_BRACE,        /**< }   */
  
  TOKEN_TYPE_SLASH,              /**< /   */
  
  TOKEN_TYPE_PREPROCESSOR_NUMBER,
  
  TOKEN_TYPE_STRING,
  TOKEN_TYPE_LONG_STRING,
  
  TOKEN_TYPE_CHAR,
  TOKEN_TYPE_LONG_CHAR,
  
  TOKEN_TYPE_CONSTANT_INT,
  TOKEN_TYPE_CONSTANT_UNSIGNED_INT,
  
  TOKEN_TYPE_CONSTANT_LONG_INT,
  TOKEN_TYPE_CONSTANT_UNSIGNED_LONG_INT,
  
  TOKEN_TYPE_CONSTANT_LONGLONG_INT,
  TOKEN_TYPE_CONSTANT_UNSIGNED_LONGLONG_INT,
  
  TOKEN_TYPE_CONSTANT_FLOAT,
  TOKEN_TYPE_CONSTANT_DOUBLE,
  TOKEN_TYPE_CONSTANT_LONG_DOUBLE,
  
  TOKEN_TYPE_IDENTIFIER_START,
  
  TOKEN_TYPE_INCLUDE = TOKEN_TYPE_IDENTIFIER_START, /* #include */
  
  TOKEN_TYPE_DEFINE,             /* #define */
  TOKEN_TYPE_UNDEF,              /* #undef */
  TOKEN_TYPE_IF,                 /* #if */
  TOKEN_TYPE_IFDEF,              /* #ifdef */
  TOKEN_TYPE_IFNDEF,             /* #ifndef */
  TOKEN_TYPE_ELSE,               /* #else */
  TOKEN_TYPE_ELIF,               /* #elif */
  TOKEN_TYPE_ENDIF,              /* #endif */
  TOKEN_TYPE_DEFINED,            /* #defined */
  
  TOKEN_TYPE_WARNING,            /* #warning */
  TOKEN_TYPE_ERROR,              /* #error */
  TOKEN_TYPE_LINE,               /* #line */
  
  TOKEN_TYPE_PRAGMA,             /* #pragma */
  
  TOKEN_TYPE___LINE__,           /* __LINE__ */
  TOKEN_TYPE___FILE__,           /* __FILE__ */
  TOKEN_TYPE___DATE__,           /* __DATE__ */
  TOKEN_TYPE___TIME__,           /* __TIME__ */
  
  TOKEN_TYPE_KEYWORD_SIZEOF,     /* sizeof */
  
  TOKEN_TYPE_KEYWORD_TYPEDEF,    /* typedef */
  
  /* storage class specifier */
  TOKEN_TYPE_KEYWORD_EXTERN,     /* extern */
  TOKEN_TYPE_KEYWORD_STATIC,     /* static */
  TOKEN_TYPE_KEYWORD_AUTO,       /* auto */
  TOKEN_TYPE_KEYWORD_REGISTER,   /* register */
  
  TOKEN_TYPE_KEYWORD_VOID,       /* void */
  TOKEN_TYPE_KEYWORD_CHAR,       /* char */
  TOKEN_TYPE_KEYWORD_SHORT,      /* short */
  TOKEN_TYPE_KEYWORD_INT,        /* int */
  TOKEN_TYPE_KEYWORD_LONG,       /* long */
  TOKEN_TYPE_KEYWORD_FLOAT,      /* float */
  TOKEN_TYPE_KEYWORD_DOUBLE,     /* double */
  TOKEN_TYPE_KEYWORD_SIGNED,     /* signed */
  TOKEN_TYPE_KEYWORD_UNSIGNED,   /* unsigned */
  TOKEN_TYPE_KEYWORD__BOOL,      /* _Bool */
  TOKEN_TYPE_KEYWORD__COMPLEX,   /* _Complex */
  TOKEN_TYPE_KEYWORD__IMAGINARY, /* _Imaginary */
  
  TOKEN_TYPE_KEYWORD_CONST,      /* const */
  TOKEN_TYPE_KEYWORD_RESTRICT,   /* restrict */
  TOKEN_TYPE_KEYWORD_VOLATILE,   /* volatile */
  
  TOKEN_TYPE_KEYWORD_INLINE,     /* inline */
  
  TOKEN_TYPE_KEYWORD_SWITCH,     /* switch */
  TOKEN_TYPE_KEYWORD_BREAK,      /* break */
  TOKEN_TYPE_KEYWORD_CASE,       /* case */
  TOKEN_TYPE_KEYWORD_CONTINUE,   /* continue */
  TOKEN_TYPE_KEYWORD_DEFAULT,    /* default */
  
  TOKEN_TYPE_KEYWORD_DO,         /* do */
  TOKEN_TYPE_KEYWORD_FOR,        /* for */
  TOKEN_TYPE_KEYWORD_WHILE,      /* while */
  
  TOKEN_TYPE_KEYWORD_IF,         /* if */
  TOKEN_TYPE_KEYWORD_ELSE,       /* else */
  
  TOKEN_TYPE_KEYWORD_GOTO,       /* goto */
  TOKEN_TYPE_KEYWORD_RETURN,     /* return */
  
  TOKEN_TYPE_KEYWORD_STRUCT,     /* struct */
  TOKEN_TYPE_KEYWORD_UNION,      /* union */
  TOKEN_TYPE_KEYWORD_ENUM,       /* enum */
  
  TOKEN_TYPE_IDENTIFIER,
  
  TOKEN_TYPE_IDENTIFIER_END = TOKEN_TYPE_IDENTIFIER,
  
  TOKEN_TYPE_EOF
} cl_compiler_token_type;

typedef enum cl_compiler_token_flag {
  TOKEN_FLAG_BEGIN_OF_LINE = (1 << 0)   /**< beginning of line before */
} cl_compiler_token_flag;

typedef enum cl_compiler_macro_type {
  MACRO_TYPE_OBJECT_LIKE,
  MACRO_TYPE_FUNCTION_LIKE
} cl_compiler_macro_type;

typedef struct _cl_compiler_token *cl_compiler_token;
typedef struct _cl_compiler_identifier_token *cl_compiler_identifier_token;
typedef struct _cl_compiler_number_token *cl_compiler_number_token;
typedef struct _cl_compiler_string_token *cl_compiler_string_token;

struct _cl_compiler_token_type_pair {
  /** Because not all token_type associate tokens, ex: TOKEN_TYPE_COMMA,
   *  hanec, I don't put the token_type field into the token structure. */
  cl_compiler_token_type  token_type;
  cl_compiler_token       token;
};
typedef struct _cl_compiler_token_type_pair _cl_compiler_token_type_pair;

#ifndef DECLARE_CLIST_TYPE_FOR__CL_COMPILER_TOKEN_TYPE_PAIR
#define DECLARE_CLIST_TYPE_FOR__CL_COMPILER_TOKEN_TYPE_PAIR
DECLARE_CLIST_TYPE(_cl_compiler_token_type_pair);
#endif
#ifndef DEFINE_CLIST_TYPE_FOR__CL_COMPILER_TOKEN_TYPE_PAIR
#define DEFINE_CLIST_TYPE_FOR__CL_COMPILER_TOKEN_TYPE_PAIR
DEFINE_CLIST_TYPE(_cl_compiler_token_type_pair);
#endif

#ifndef DECLARE_CVECTOR_TYPE_FOR__CL_COMPILER_TOKEN_TYPE_PAIR
#define DECLARE_CVECTOR_TYPE_FOR__CL_COMPILER_TOKEN_TYPE_PAIR
DECLARE_CVECTOR_TYPE(_cl_compiler_token_type_pair)
#endif
#ifndef DEFINE_CVECTOR_TYPE_FOR__CL_COMPILER_TOKEN_TYPE_PAIR
#define DEFINE_CVECTOR_TYPE_FOR__CL_COMPILER_TOKEN_TYPE_PAIR
DEFINE_CVECTOR_TYPE(_cl_compiler_token_type_pair)
#endif

#ifndef DECLARE_CLIST_TYPE_FOR_CL_COMPILER_TOKEN
#define DECLARE_CLIST_TYPE_FOR_CL_COMPILER_TOKEN
DECLARE_CLIST_TYPE(cl_compiler_token)
#endif
#ifndef DEFINE_CLIST_TYPE_FOR_CL_COMPILER_TOKEN
#define DEFINE_CLIST_TYPE_FOR_CL_COMPILER_TOKEN
DEFINE_CLIST_TYPE(cl_compiler_token)
#endif

union _cl_compiler_token_value {
  int int_value;
  unsigned int uint_value;
  
  long long_value;
  unsigned long ulong_value;
  
  long long longlong_value;
  unsigned long long ulonglong_value;
  
  float float_value;
  double double_value;
  long double longdouble_value;
};

typedef union _cl_compiler_token_value _cl_compiler_token_value;
typedef union _cl_compiler_token_value *cl_compiler_token_value;

#ifndef DECLARE_CLIST_ONE_LEVEL_TYPE_FOR_CL_COMPILER_TOKEN
#define DECLARE_CLIST_ONE_LEVEL_TYPE_FOR_CL_COMPILER_TOKEN
DECLARE_CLIST_ONE_LEVEL_TYPE(cl_compiler_token, all_tokens)
#endif

struct _cl_compiler_token {
  cl_compiler_token_type type;
  
  cl_uint reference_count;
  CLIST_ONE_LEVEL_ELEMENT_NEEDED_FIELDS(cl_compiler_token, all_tokens)
};

struct _cl_compiler_number_token {
  struct _cl_compiler_token common;
  
  cl_compiler_token_value token_value;
  char str[];
};

struct _cl_compiler_string_token {
  struct _cl_compiler_token common;
  
  char str[];
};

enum {
  IDENTIFIER_TOKEN_FLAG_TYPEDEF = (1 << 1)
};

struct _cl_compiler_identifier_token {
  struct _cl_compiler_token common;
  
  cl_int flags;
  
  cl_compiler_macro_type macro_type;
  /* point to the defined tokens if this token is #define'ed' */
  CLIST_TYPE(_cl_compiler_token_type_pair) defined_tokens;
  /* point to the argument list if this token is #define'ed' */
  CVECTOR_TYPE(_cl_compiler_token_type_pair) defined_arguments;
  
  char str[];
};

#ifndef DEFINE_CLIST_ONE_LEVEL_TYPE_FOR_CL_COMPILER_TOKEN
#define DEFINE_CLIST_ONE_LEVEL_TYPE_FOR_CL_COMPILER_TOKEN
DEFINE_CLIST_ONE_LEVEL_TYPE(cl_compiler_token, all_tokens)
#endif

static inline cl_uint
clCompilerTokenGetReferenceCount(cl_compiler_token token) {
  ASSERT(token != NULL);
  return token->reference_count;
}

static inline void
clCompilerTokenIncrementReference(cl_compiler_token token) {
  ASSERT(token != NULL);
  ++token->reference_count;
}

static inline CLIST_TYPE(_cl_compiler_token_type_pair)
clCompilerTokenGetDefinedTokens(cl_compiler_identifier_token token) {
  ASSERT(token != NULL);
  ASSERT(TOKEN_TYPE_IDENTIFIER == token->common.type);
  return token->defined_tokens;
}

static inline void
clCompilerTokenSetDefinedTokens(
    cl_compiler_identifier_token token,
    CLIST_TYPE(_cl_compiler_token_type_pair) defined_tokens) {
  ASSERT(token != NULL);
  ASSERT(TOKEN_TYPE_IDENTIFIER == token->common.type);
  token->defined_tokens = defined_tokens;
}

static inline CVECTOR_TYPE(_cl_compiler_token_type_pair)
clCompilerTokenGetDefinedArguments(cl_compiler_identifier_token token) {
  ASSERT(token != NULL);
  ASSERT(TOKEN_TYPE_IDENTIFIER == token->common.type);
  return token->defined_arguments;
}

static inline void
clCompilerTokenSetDefinedArguments(
    cl_compiler_identifier_token token,
    CVECTOR_TYPE(_cl_compiler_token_type_pair) defined_arguments) {
  ASSERT(token != NULL);
  ASSERT(TOKEN_TYPE_IDENTIFIER == token->common.type);
  ASSERT(defined_arguments != NULL);
  token->defined_arguments = defined_arguments;
}

static inline cl_compiler_macro_type
clCompilerTokenGetMacroType(cl_compiler_identifier_token token) {
  ASSERT(token != NULL);
  ASSERT(TOKEN_TYPE_IDENTIFIER == token->common.type);
  ASSERT((token->defined_tokens != NULL) &&
         "It it no sense to get the macro type in a non-#define'ed' token");
  return token->macro_type;
}

static inline void
clCompilerTokenSetMacroType(cl_compiler_identifier_token token,
                            cl_compiler_macro_type const macro_type) {
  ASSERT(token != NULL);
  ASSERT(TOKEN_TYPE_IDENTIFIER == token->common.type);
  token->macro_type = macro_type;
}

struct _cl_compiler;
extern cl_compiler_token clCompilerTokenNew(
    struct _cl_compiler_frontend *frontend,
    cl_compiler_token_type const token_type,
    char const *str);
extern void clCompilerTokenDelete(struct _cl_compiler_frontend *frontend, cl_compiler_token token);

#ifndef DECLARE_HASHTABLE_TYPE_FOR_CL_COMPILER_TOKEN
#define DECLARE_HASHTABLE_TYPE_FOR_CL_COMPILER_TOKEN
DECLARE_HASHTABLE_TYPE(cl_compiler_token);
#endif
#ifndef DEFINE_HASHTABLE_TYPE_FOR_CL_COMPILER_TOKEN
#define DEFINE_HASHTABLE_TYPE_FOR_CL_COMPILER_TOKEN
DEFINE_HASHTABLE_TYPE(cl_compiler_token);
#endif

extern cl_compiler_token_type clCompilerTokenIsSpecialIdentifierToken(char const * const name);

extern char const *clCompilerTokenGetString(cl_compiler_token_type token_type,
                                            cl_compiler_token token);

/* ===============================================
 *   token hash table handing
 * =============================================== */

extern void clCompilerTokenHashTableSetDefaultFunction(
    HASHTABLE_TYPE(cl_compiler_token) hash_table);

extern cl_compiler_token clCompilerTokenHashTableSearchByStr(
    HASHTABLE_TYPE(cl_compiler_token) hash_table, char const *str);

extern void clCompilerTokenHashTableDeleteByStr(
    HASHTABLE_TYPE(cl_compiler_token) hash_table, char const *str);

#endif
