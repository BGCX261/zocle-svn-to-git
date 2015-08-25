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

#include <compiler_frontend/inc/ast_node.h>

#define INDENT_LEVEL_WIDTH (2)

static char const *astNodeTypeString[] = {
  "increment operator (prefix) (++)",
  "decrement operator (prefix) (--)",
  "increment operator (postfix) (++)",
  "decrement operator (postfix) (--)",
  
  "address operator (&)",
  "indirection operator (*)",
  
  "positive operator (+)",
  "negative operator (-)",
  "logical negation operator (!)",
  "sizeof operator (sizeof)",
  
  "binary addition operator (+)",
  "binary subtraction operator (-)",
  "binary multiply operator (*)",
  "binary division operator (/)",
  "binary modulo operator (%)",

  "less than operator (<)",
  "less equal operator (<=)",
  "greater than operator (>)",
  "greater equal operator (>=)",
  "equal operator (==)",
  "not equal operator (!=)",

  "bitwise left shift operator (<<)",
  "bitwise right shift operator (>>)",
  "bitwise complement operator (~)",
  "bitwise AND operator (&)",
  "bitwise XOR operator (^)",
  "bitwise OR operator (|)",

  "logical AND operator (%%)",
  "logical OR operator (||)",
  
  "if",
  
  "array subscripting ([])",
  "compound literal",
  "typename",
  "init list",
  "concrete type member (.)",
  "pointer type member (->)",
  "function call",
  "cast to type",
  
  "identifier",
  
  "constant int",
  "constant unsigned int",
  
  "constant long int",
  "constant unsigned long int",
  
  "constant longlong int",
  "constant unsigned longlong int",
  
  "constant float",
  "constant double",
  "constant long double",
  
  "string",
  "long string",
  
  "char",
  "long char",
  
  "typedef",
  "extern",
  "static",
  "auto",
  "register",
  
  "const",
  "restrict",
  "volatile",
  
  "inline",
  
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
  "_Imaginary"
};

cl_compiler_ast_node
clCompilerFrontendAstNodeNew(cl_compiler_ast_node_type const type) {
  cl_compiler_ast_node ast_node;
  
  switch (type) {
  case AST_NODE_TYPE_IDENTIFIER:
    
  case AST_NODE_TYPE_CONSTANT_INT:
  case AST_NODE_TYPE_CONSTANT_UNSIGNED_INT:
  case AST_NODE_TYPE_CONSTANT_LONG_INT:
  case AST_NODE_TYPE_CONSTANT_UNSIGNED_LONG_INT:
  case AST_NODE_TYPE_CONSTANT_LONGLONG_INT:
  case AST_NODE_TYPE_CONSTANT_UNSIGNED_LONGLONG_INT:
  case AST_NODE_TYPE_CONSTANT_FLOAT:
  case AST_NODE_TYPE_CONSTANT_DOUBLE:
  case AST_NODE_TYPE_CONSTANT_LONG_DOUBLE:
    
  case AST_NODE_TYPE_CONCRETE_TYPE_MEMBER:
  case AST_NODE_TYPE_PTR_TYPE_MEMBER:
  {
    cl_compiler_ast_node_with_token ast_node_with_token =
      CL_OSAL_CALLOC(sizeof(struct _cl_compiler_ast_node_with_token));
    ASSERT(ast_node_with_token != NULL);
    ast_node = (cl_compiler_ast_node)ast_node_with_token;
  }
  break;
  
  default:
    ast_node = CL_OSAL_CALLOC(sizeof(struct _cl_compiler_ast_node));
    break;
  }
  ASSERT(ast_node != NULL);
  
  ast_node->type = type;
  ast_node->parent = NULL;
  memset(&(ast_node->children), 0, sizeof(ast_node->children));
  
  return ast_node;
}

static cl_bool
search_ast_node_in_children_compare_func(
    cl_compiler_ast_node *target_ast_node,
    cl_compiler_ast_node source_ast_node) {
  if (*target_ast_node == source_ast_node) {
    return CL_TRUE;
  } else {
    return CL_FALSE;
  }
}

CLIST_ITER_TYPE(cl_compiler_ast_node)
clCompilerFrontendAstNodeSearch(CLIST_TYPE(cl_compiler_ast_node) clist,
                                cl_compiler_ast_node target_ast_node) {
  CLIST_ITER_TYPE(cl_compiler_ast_node) iter = CLIST_SEARCH(cl_compiler_ast_node)(
      clist, target_ast_node, search_ast_node_in_children_compare_func);
  return iter;
}

void
clCompilerFrontendAstNodeDelete(cl_compiler_ast_node ast_node) {
  ASSERT(ast_node != NULL);
  
  {
    /* delete the children */
    CLIST_ITER_TYPE(cl_compiler_ast_node) iter;
    
    for (iter = CLIST_BEGIN(cl_compiler_ast_node)(&(ast_node->children));
         iter != CLIST_END(cl_compiler_ast_node)(&(ast_node->children));
         iter = CLIST_ITER_INCREMENT(cl_compiler_ast_node)(iter)) {
      clCompilerFrontendAstNodeDelete(*CLIST_ITER_GET_DATA(cl_compiler_ast_node)(iter));
    }
    CLIST_CLEAR(cl_compiler_ast_node)(&(ast_node->children));
  }
  
  CL_OSAL_FREE(ast_node);
}

void
clCompilerFrontendAstNodeAddChildren(cl_compiler_ast_node ast_node,
                                     cl_compiler_ast_node children) {
  ASSERT(ast_node != NULL);
  ASSERT(children != NULL);
  ASSERT(NULL == children->parent);
  
  CLIST_PUSH_BACK(cl_compiler_ast_node)(&(ast_node->children), &children);
  children->parent = ast_node;
}

void
clCompilerFrontendAstNodeAddChildrenList(cl_compiler_ast_node ast_node,
                                         CLIST_TYPE(cl_compiler_ast_node) children) {
  ASSERT(ast_node != NULL);
  ASSERT(children != NULL);
  CLIST_SPLICE(cl_compiler_ast_node)(&(ast_node->children), NULL, children);
  
  /* set children's parent to 'ast_node' */
  {
    CLIST_ITER_TYPE(cl_compiler_ast_node) iter;
    for (iter = CLIST_BEGIN(cl_compiler_ast_node)(&(ast_node->children));
         iter != CLIST_END(cl_compiler_ast_node)(&(ast_node->children));
         iter = CLIST_ITER_INCREMENT(cl_compiler_ast_node)(iter)) {
      (*CLIST_ITER_GET_DATA(cl_compiler_ast_node)(iter))->parent = ast_node;
    }
  }
}

void
clCompilerFrontendAstNodeFreeChildren(CLIST_TYPE(cl_compiler_ast_node) children) {
  CLIST_ITER_TYPE(cl_compiler_ast_node) iter;
  
  ASSERT(children != NULL);
  
  for (iter = CLIST_BEGIN(cl_compiler_ast_node)(children);
       iter != CLIST_END(cl_compiler_ast_node)(children);
       iter = CLIST_ITER_INCREMENT(cl_compiler_ast_node)(iter)) {
    clCompilerFrontendAstNodeDelete(*CLIST_ITER_GET_DATA(cl_compiler_ast_node)(iter));
  }
  
  CLIST_CLEAR(cl_compiler_ast_node)(children);
}

static void
printIndent(size_t const indent) {
  size_t i;
  for (i = 0; i < (indent * INDENT_LEVEL_WIDTH); ++i) {
    clOsalPrintf("%c", ' ');
  }
}

void
clCompilerFrontendAstNodeDumpInfo(
    cl_compiler_ast_node ast_node,
    size_t const indent) {
  ASSERT(ast_node != NULL);
  
  printIndent(indent);
  
  clOsalPrintf("<%s>", astNodeTypeString[ast_node->type]);
  switch (ast_node->type) {
  case AST_NODE_TYPE_IDENTIFIER:
  {
    cl_compiler_identifier_token token =
      (cl_compiler_identifier_token)(((cl_compiler_ast_node_with_token)ast_node)->token);
    ASSERT(token != NULL);
    clOsalPrintf(": %s", token->str);
  }
  break;
  case AST_NODE_TYPE_CONSTANT_INT:
  {
    cl_compiler_number_token token =
      (cl_compiler_number_token)(((cl_compiler_ast_node_with_token)ast_node)->token);
    ASSERT(token != NULL);
    clOsalPrintf(": %d", token->token_value->int_value);
  }
  break;
  case AST_NODE_TYPE_CONSTANT_UNSIGNED_INT:
  {
    cl_compiler_number_token token =
      (cl_compiler_number_token)(((cl_compiler_ast_node_with_token)ast_node)->token);
    ASSERT(token != NULL);
    clOsalPrintf(": %u", token->token_value->uint_value);
  }
  break;
  case AST_NODE_TYPE_CONSTANT_LONG_INT:
  {
    cl_compiler_number_token token =
      (cl_compiler_number_token)(((cl_compiler_ast_node_with_token)ast_node)->token);
    ASSERT(token != NULL);
    clOsalPrintf(": %ld", token->token_value->long_value);
  }
  break;
  case AST_NODE_TYPE_CONSTANT_UNSIGNED_LONG_INT:
  {
    cl_compiler_number_token token =
      (cl_compiler_number_token)(((cl_compiler_ast_node_with_token)ast_node)->token);
    ASSERT(token != NULL);
    clOsalPrintf(": %lu", token->token_value->ulong_value);
  }
  break;
  case AST_NODE_TYPE_CONSTANT_LONGLONG_INT:
  {
    cl_compiler_number_token token =
      (cl_compiler_number_token)(((cl_compiler_ast_node_with_token)ast_node)->token);
    ASSERT(token != NULL);
    clOsalPrintf(": %lld", token->token_value->longlong_value);
  }
  break;
  case AST_NODE_TYPE_CONSTANT_UNSIGNED_LONGLONG_INT:
  {
    cl_compiler_number_token token =
      (cl_compiler_number_token)(((cl_compiler_ast_node_with_token)ast_node)->token);
    ASSERT(token != NULL);
    clOsalPrintf(": %llu", token->token_value->ulonglong_value);
  }
  break;
  case AST_NODE_TYPE_CONSTANT_FLOAT:
  {
    cl_compiler_number_token token =
      (cl_compiler_number_token)(((cl_compiler_ast_node_with_token)ast_node)->token);
    ASSERT(token != NULL);
    clOsalPrintf(": %f", token->token_value->float_value);
  }
  break;
  case AST_NODE_TYPE_CONSTANT_DOUBLE:
  {
    cl_compiler_number_token token =
      (cl_compiler_number_token)(((cl_compiler_ast_node_with_token)ast_node)->token);
    ASSERT(token != NULL);
    clOsalPrintf(": %f", token->token_value->double_value);
  }
  break;
  case AST_NODE_TYPE_CONSTANT_LONG_DOUBLE:
  {
    cl_compiler_number_token token =
      (cl_compiler_number_token)(((cl_compiler_ast_node_with_token)ast_node)->token);
    ASSERT(token != NULL);
    clOsalPrintf(": %Lf", token->token_value->longdouble_value);
  }
  break;
  case AST_NODE_TYPE_STRING:
  {
    cl_compiler_string_token token =
      (cl_compiler_string_token)(((cl_compiler_ast_node_with_token)ast_node)->token);
    ASSERT(token != NULL);
    clOsalPrintf(": %s", token->str);
  }
  break;
  case AST_NODE_TYPE_LONG_STRING:
    ASSERT(0 && "TODO");
    break;
  case AST_NODE_TYPE_CHAR:
  {
    cl_compiler_string_token token =
      (cl_compiler_string_token)(((cl_compiler_ast_node_with_token)ast_node)->token);
    ASSERT(token != NULL);
    clOsalPrintf(": %s", token->str);
  }
  break;
  case AST_NODE_TYPE_LONG_CHAR:
    ASSERT(0 && "TODO");
    break;
    
  case AST_NODE_TYPE_COMPOUND_LITERAL:
  case AST_NODE_TYPE_TYPENAME:
  case AST_NODE_TYPE_INIT_LIST:
  case AST_NODE_TYPE_CONCRETE_TYPE_MEMBER:
  case AST_NODE_TYPE_PTR_TYPE_MEMBER:
  case AST_NODE_TYPE_FUNCTION_CALL:
  case AST_NODE_TYPE_CAST_TO_TYPE:
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
  case AST_NODE_TYPE_ADDITION_OPERATOR:            /* +  */
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
    
  case AST_NODE_TYPE_KEYWORD_TYPEDEF:
  case AST_NODE_TYPE_KEYWORD_EXTERN:
  case AST_NODE_TYPE_KEYWORD_STATIC:
  case AST_NODE_TYPE_KEYWORD_AUTO:
  case AST_NODE_TYPE_KEYWORD_REGISTER:

  case AST_NODE_TYPE_KEYWORD_CONST:
  case AST_NODE_TYPE_KEYWORD_RESTRICT:
  case AST_NODE_TYPE_KEYWORD_VOLATILE:
    
  case AST_NODE_TYPE_KEYWORD_INLINE:
    
  case AST_NODE_TYPE_KEYWORD_VOID:
  case AST_NODE_TYPE_KEYWORD_CHAR:
  case AST_NODE_TYPE_KEYWORD_SHORT:
  case AST_NODE_TYPE_KEYWORD_INT:
  case AST_NODE_TYPE_KEYWORD_LONG:
  case AST_NODE_TYPE_KEYWORD_FLOAT:
  case AST_NODE_TYPE_KEYWORD_DOUBLE:
  case AST_NODE_TYPE_KEYWORD_SIGNED:
  case AST_NODE_TYPE_KEYWORD_UNSIGNED:
  case AST_NODE_TYPE_KEYWORD__BOOL:
  case AST_NODE_TYPE_KEYWORD__COMPLEX:
  case AST_NODE_TYPE_KEYWORD__IMAGINARY:
    break;
    
  default:
    ASSERT(0 && "should not reach here");
    break;
  }
  clOsalPrintf("\n");
  
  {
    CLIST_ITER_TYPE(cl_compiler_ast_node) iter;
    for (iter = CLIST_BEGIN(cl_compiler_ast_node)(&(ast_node->children));
         iter != CLIST_END(cl_compiler_ast_node)(&(ast_node->children));
         iter = CLIST_ITER_INCREMENT(cl_compiler_ast_node)(iter)) {
      clCompilerFrontendAstNodeDumpInfo(*CLIST_ITER_GET_DATA(cl_compiler_ast_node)(iter), indent + 1);
    }
  }
}

cl_bool
clCompilerFrontendAstNodeIsNumericConstant(
    cl_compiler_ast_node ast_node) {
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
  case AST_NODE_TYPE_ADDITION_OPERATOR:            /* +  */
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
    return CL_FALSE;
    
  case AST_NODE_TYPE_CONSTANT_INT:
  case AST_NODE_TYPE_CONSTANT_UNSIGNED_INT:
  case AST_NODE_TYPE_CONSTANT_LONG_INT:
  case AST_NODE_TYPE_CONSTANT_UNSIGNED_LONG_INT:
  case AST_NODE_TYPE_CONSTANT_LONGLONG_INT:
  case AST_NODE_TYPE_CONSTANT_UNSIGNED_LONGLONG_INT:
  case AST_NODE_TYPE_CONSTANT_FLOAT:
  case AST_NODE_TYPE_CONSTANT_DOUBLE:
  case AST_NODE_TYPE_CONSTANT_LONG_DOUBLE:
    return CL_TRUE;
    
  default:
    ASSERT(0 && "should not reach here");
    return CL_TRUE;
  }
}

cl_bool
clCompilerFrontendAstNodeIsIntegerConstant(
    cl_compiler_ast_node ast_node) {
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
  case AST_NODE_TYPE_ADDITION_OPERATOR:            /* +  */
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
  case AST_NODE_TYPE_CONSTANT_FLOAT:
  case AST_NODE_TYPE_CONSTANT_DOUBLE:
  case AST_NODE_TYPE_CONSTANT_LONG_DOUBLE:
    return CL_FALSE;
    
  case AST_NODE_TYPE_CONSTANT_INT:
  case AST_NODE_TYPE_CONSTANT_UNSIGNED_INT:
  case AST_NODE_TYPE_CONSTANT_LONG_INT:
  case AST_NODE_TYPE_CONSTANT_UNSIGNED_LONG_INT:
  case AST_NODE_TYPE_CONSTANT_LONGLONG_INT:
  case AST_NODE_TYPE_CONSTANT_UNSIGNED_LONGLONG_INT:
    return CL_TRUE;
    
  default:
    ASSERT(0 && "should not reach here");
    return CL_TRUE;
  }
}

int64_t
clCompilerFrontendIntegerContantAstNodeGetValue(cl_compiler_ast_node ast_node) {
  cl_compiler_number_token token;
  
  ASSERT(ast_node != NULL);
  ASSERT(CL_TRUE == clCompilerFrontendAstNodeIsIntegerConstant(ast_node));
  
  token = (cl_compiler_number_token)(((cl_compiler_ast_node_with_token)ast_node)->token);
  
  switch (ast_node->type) {
  case AST_NODE_TYPE_CONSTANT_INT: return token->token_value->int_value;
  case AST_NODE_TYPE_CONSTANT_UNSIGNED_INT: return token->token_value->uint_value;
  case AST_NODE_TYPE_CONSTANT_LONG_INT: return token->token_value->long_value;
  case AST_NODE_TYPE_CONSTANT_UNSIGNED_LONG_INT: return token->token_value->ulong_value;
  case AST_NODE_TYPE_CONSTANT_LONGLONG_INT: return token->token_value->longlong_value;
  case AST_NODE_TYPE_CONSTANT_UNSIGNED_LONGLONG_INT: return token->token_value->ulonglong_value;
  default: ASSERT(0 && "should not reach here"); return 0;
  }
}

cl_bool
clCompilerFrontendAstNodeAllChildrenIsNumericConstant(
    cl_compiler_ast_node ast_node) {
  CLIST_ITER_TYPE(cl_compiler_ast_node) iter;
  for (iter = CLIST_BEGIN(cl_compiler_ast_node)(&(ast_node->children));
       iter != CLIST_END(cl_compiler_ast_node)(&(ast_node->children));
       iter = CLIST_ITER_INCREMENT(cl_compiler_ast_node)(iter)) {
    if (CL_FALSE == clCompilerFrontendAstNodeIsNumericConstant(ast_node)) {
      return CL_FALSE;
    }
  }
  return CL_TRUE;
}
