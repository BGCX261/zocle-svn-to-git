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
#ifndef ZOCLE_COMPILER_FRONTEND_AST_NODE_H_
#define ZOCLE_COMPILER_FRONTEND_AST_NODE_H_

#include <compiler_frontend/inc/token.h>
#include <compiler_frontend/inc/frontend.h>

#include <container/inc/treenode.h>

typedef enum _cl_compiler_ast_node_type {
  AST_NODE_TYPE_PREFIX_INCREMENT_OPERATOR,    /* ++ */
  AST_NODE_TYPE_PREFIX_DECREMENT_OPERATOR,    /* -- */
  AST_NODE_TYPE_POSTFIX_INCREMENT_OPERATOR,   /* ++ */
  AST_NODE_TYPE_POSTFIX_DECREMENT_OPERATOR,   /* -- */
  
  AST_NODE_TYPE_ADDRESS_OPERATOR,             /* &  */
  AST_NODE_TYPE_INDIRECTION_OPERATOR,         /* *  */
  
  AST_NODE_TYPE_POSITIVE_OPERATOR,            /* +  */
  AST_NODE_TYPE_NEGATIVE_OPERATOR,            /* -  */
  AST_NODE_TYPE_LOGICAL_NEGATION_OPERATOR,    /* !  */
  AST_NODE_TYPE_SIZEOF_OPERATOR,              /* sizeof  */
  
  AST_NODE_TYPE_ADDITION_OPERATOR,            /* +  */
  AST_NODE_TYPE_SUBTRACTION_OPERATOR,         /* -  */
  AST_NODE_TYPE_MULTIPLY_OPERATOR,            /* *  */
  AST_NODE_TYPE_DIVISION_OPERATOR,            /* /  */
  AST_NODE_TYPE_MODULO_OPERATOR,              /* %  */
  
  AST_NODE_TYPE_LESS_THAN_OPERATOR,           /* << */
  AST_NODE_TYPE_LESS_EQUAL_OPERATOR,          /* <= */
  AST_NODE_TYPE_GREATER_THAN_OPERATOR,        /* >> */
  AST_NODE_TYPE_GREATER_EQUAL_OPERATOR,       /* >= */
  AST_NODE_TYPE_EQUAL_OPERATOR,               /* == */
  AST_NODE_TYPE_NOT_EQUAL_OPERATOR,           /* != */
  
  AST_NODE_TYPE_BITWISE_LEFT_SHIFT_OPERATOR,  /* << */
  AST_NODE_TYPE_BITWISE_RIGHT_SHIFT_OPERATOR, /* >> */
  AST_NODE_TYPE_BITWISE_COMPLEMENT_OPERATOR,  /* ~  */
  AST_NODE_TYPE_BITWISE_AND_OPERATOR,         /* &  */
  AST_NODE_TYPE_BITWISE_XOR_OPERATOR,         /* ^  */
  AST_NODE_TYPE_BITWISE_OR_OPERATOR,          /* |  */
  
  AST_NODE_TYPE_LOGICAL_AND_OPERATOR,         /* && */
  AST_NODE_TYPE_LOGICAL_OR_OPERATOR,          /* || */
  
  AST_NODE_TYPE_IF,                           /* if-statement or ?: */
  
  AST_NODE_TYPE_ARRAY_SUBSCRIPTING,           /* [] */
  AST_NODE_TYPE_COMPOUND_LITERAL,
  AST_NODE_TYPE_TYPENAME,
  AST_NODE_TYPE_INIT_LIST,
  AST_NODE_TYPE_CONCRETE_TYPE_MEMBER,
  AST_NODE_TYPE_PTR_TYPE_MEMBER,
  AST_NODE_TYPE_FUNCTION_CALL,
  AST_NODE_TYPE_CAST_TO_TYPE,
  
  AST_NODE_TYPE_IDENTIFIER,
  
  AST_NODE_TYPE_CONSTANT_INT,
  AST_NODE_TYPE_CONSTANT_UNSIGNED_INT,
  
  AST_NODE_TYPE_CONSTANT_LONG_INT,
  AST_NODE_TYPE_CONSTANT_UNSIGNED_LONG_INT,
  
  AST_NODE_TYPE_CONSTANT_LONGLONG_INT,
  AST_NODE_TYPE_CONSTANT_UNSIGNED_LONGLONG_INT,
  
  AST_NODE_TYPE_CONSTANT_FLOAT,
  AST_NODE_TYPE_CONSTANT_DOUBLE,
  AST_NODE_TYPE_CONSTANT_LONG_DOUBLE,
  
  AST_NODE_TYPE_STRING,
  AST_NODE_TYPE_LONG_STRING,
  
  AST_NODE_TYPE_CHAR,
  AST_NODE_TYPE_LONG_CHAR,
  
  AST_NODE_TYPE_KEYWORD_TYPEDEF,
  AST_NODE_TYPE_KEYWORD_EXTERN,
  AST_NODE_TYPE_KEYWORD_STATIC,
  AST_NODE_TYPE_KEYWORD_AUTO,
  AST_NODE_TYPE_KEYWORD_REGISTER,
  
  AST_NODE_TYPE_KEYWORD_CONST,
  AST_NODE_TYPE_KEYWORD_RESTRICT,
  AST_NODE_TYPE_KEYWORD_VOLATILE,
  
  AST_NODE_TYPE_KEYWORD_INLINE,
  
  AST_NODE_TYPE_KEYWORD_VOID,
  AST_NODE_TYPE_KEYWORD_CHAR,
  AST_NODE_TYPE_KEYWORD_SHORT,
  AST_NODE_TYPE_KEYWORD_INT,
  AST_NODE_TYPE_KEYWORD_LONG,
  AST_NODE_TYPE_KEYWORD_FLOAT,
  AST_NODE_TYPE_KEYWORD_DOUBLE,
  AST_NODE_TYPE_KEYWORD_SIGNED,
  AST_NODE_TYPE_KEYWORD_UNSIGNED,
  AST_NODE_TYPE_KEYWORD__BOOL,
  AST_NODE_TYPE_KEYWORD__COMPLEX,
  AST_NODE_TYPE_KEYWORD__IMAGINARY
} cl_compiler_ast_node_type;

typedef struct _cl_compiler_ast_node _cl_compiler_ast_node;
typedef struct _cl_compiler_ast_node *cl_compiler_ast_node;
typedef struct _cl_compiler_ast_node_with_token *cl_compiler_ast_node_with_token;

#ifndef DECLARE_CLIST_TYPE_FOR_CL_COMPILER_AST_NODE
#define DECLARE_CLIST_TYPE_FOR_CL_COMPILER_AST_NODE
DECLARE_CLIST_TYPE(cl_compiler_ast_node)
#endif
#ifndef DEFINE_CLIST_TYPE_FOR_CL_COMPILER_AST_NODE
#define DEFINE_CLIST_TYPE_FOR_CL_COMPILER_AST_NODE
DEFINE_CLIST_TYPE(cl_compiler_ast_node)
#endif

struct _cl_compiler_ast_node {
  cl_compiler_ast_node_type type;
  
  cl_compiler_ast_node  parent;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) children;
};

struct _cl_compiler_ast_node_with_token {
  struct _cl_compiler_ast_node base;
  cl_compiler_token token;
};

static inline cl_compiler_ast_node
clCompilerFrontendAstNodeGetParent(cl_compiler_ast_node ast_node) {
  ASSERT(ast_node != NULL);
  return ast_node->parent;
}

extern cl_compiler_ast_node clCompilerFrontendAstNodeNew(cl_compiler_ast_node_type const type);
extern void clCompilerFrontendAstNodeDelete(cl_compiler_ast_node ast_node);

extern void clCompilerFrontendAstNodeAddChildren(
    cl_compiler_ast_node ast_node, cl_compiler_ast_node children);
extern void clCompilerFrontendAstNodeAddChildrenList(
    cl_compiler_ast_node ast_node, CLIST_TYPE(cl_compiler_ast_node) children);

extern void clCompilerFrontendAstNodeFreeChildren(
    CLIST_TYPE(cl_compiler_ast_node) children);

extern void clCompilerFrontendAstNodeDumpInfo(cl_compiler_ast_node ast_node, size_t const indent);

extern cl_bool clCompilerFrontendAstNodeIsNumericConstant(cl_compiler_ast_node ast_node);
extern cl_bool clCompilerFrontendAstNodeAllChildrenIsNumericConstant(cl_compiler_ast_node ast_node);

extern cl_bool clCompilerFrontendAstNodeIsIntegerConstant(cl_compiler_ast_node ast_node);
extern int64_t clCompilerFrontendIntegerContantAstNodeGetValue(cl_compiler_ast_node ast_node);

extern CLIST_ITER_TYPE(cl_compiler_ast_node) clCompilerFrontendAstNodeSearch(
    CLIST_TYPE(cl_compiler_ast_node) clist, cl_compiler_ast_node target_ast_node);

#endif
