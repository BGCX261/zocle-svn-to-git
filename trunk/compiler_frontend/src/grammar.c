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

#include <compiler_frontend/inc/grammar.h>

#include <compiler/inc/error_msg.h>

#include <compiler_frontend/inc/ast_node.h>
#include <compiler_frontend/inc/frontend.h>

#define PARSE_IDENTIFIER                                                \
  do {                                                                  \
    error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair); \
    if (error_code != COMPILER_NO_ERROR) {                              \
      goto finish;                                                      \
    }                                                                   \
    if (token_type_pair.token_type != TOKEN_TYPE_IDENTIFIER) {          \
      error_code = COMPILER_ERROR__PARSING__ERROR;                      \
      goto finish;                                                      \
    } else {                                                            \
      if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {   \
        ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_IDENTIFIER); \
        ASSERT(ast_node != NULL);                                       \
                                                                        \
        ((cl_compiler_ast_node_with_token)ast_node)->token = token_type_pair.token; \
        CLIST_PUSH_BACK(cl_compiler_ast_node)(&internal_container, &ast_node); \
      }                                                                 \
    }                                                                   \
  } while(0)

static cl_compiler_error_code clCompilerFrontendParseCastExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node),
    cl_bool const is_in_constant_expression);

static cl_compiler_error_code clCompilerFrontendParsePostfixExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node),
    cl_bool const is_in_constant_expression);

static cl_compiler_error_code clCompilerFrontendParsePrimaryExpression(
    cl_compiler_frontend frontend, CLIST_TYPE(cl_compiler_ast_node));

static cl_compiler_error_code clCompilerFrontendParseExpression(
    cl_compiler_frontend frontend, CLIST_TYPE(cl_compiler_ast_node));

static cl_compiler_error_code clCompilerFrontendParseParameterTypeList(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container);

static cl_compiler_error_code clCompilerFrontendParseInitializer(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container);

static cl_compiler_error_code clCompilerFrontendParseCompoundStatement(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container);

/*
 * specifier_qualifier_list
 *   : ( type_qualifier | type_specifier )+
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseSpecifierQualifierList(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  ASSERT(frontend != NULL);
  ASSERT(container != NULL);
  ASSERT(0 && "TODO");
  return error_code;
}

/*
 * type_name
 *   : specifier_qualifier_list abstract_declarator?
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseTypename(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  ASSERT(frontend != NULL);
  ASSERT(container != NULL);
  ASSERT(0 && "TODO");
  return error_code;
}

/*
 * unary_expression
 *   : postfix_expression
 *   | '++' unary_expression
 *   | '--' unary_expression
 *   | unary_operator cast_expression
 *   | 'sizeof' unary_expression
 *   | 'sizeof' '(' type_name ')'
 *   ;
 *
 * unary_operator
 *   : '&'
 *   | '*'
 *   | '+'
 *   | '-'
 *   | '~'
 *   | '!'
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseUnaryExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container,
    cl_bool const is_in_constant_expression) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  
  ASSERT(frontend != NULL);
  
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  switch (token_type_pair.token_type) {
  case TOKEN_TYPE_INCREMENT: /* ++ */
  case TOKEN_TYPE_DECREMENT: /* -- */
    /* C99 spec, p95:
     *
     * "Constant expressions shall not contain assignment, increment, decrement, function-call,
     * or comma operators, except when they are contained within a subexpression that is not
     * evaluated.
     * Ex: The operand of a sizeof operator is usually not evaluated."
     */
    if (CL_TRUE == is_in_constant_expression) {
      error(frontend, "this operator is not allowed in a constant expression");
      error_code = COMPILER_ERROR__PARSING__UNALLOWED_OPERATOR_IN_CONSTANT_EXPRESSION;
      goto finish;
    }
    error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    
    {
      CLIST_CONCRETE_TYPE(cl_compiler_ast_node) unary_expr = {0};
      cl_compiler_ast_node ast_node = NULL;
      
      error_code = clCompilerFrontendParseUnaryExpression(frontend, &unary_expr, is_in_constant_expression);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      if (TOKEN_TYPE_INCREMENT == token_type_pair.token_type) {
        ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_PREFIX_INCREMENT_OPERATOR);
      } else {
        ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_PREFIX_DECREMENT_OPERATOR);
      }
      ASSERT(ast_node != NULL);
      
      clCompilerFrontendAstNodeAddChildrenList(ast_node, &unary_expr);
      CLIST_PUSH_BACK(cl_compiler_ast_node)(container, &ast_node);
    }
    break;
    
  case TOKEN_TYPE_BITWISE_AND:
  case TOKEN_TYPE_MULTIPLY:
  case TOKEN_TYPE_ADDITION:
  case TOKEN_TYPE_SUBTRACTION:
  case TOKEN_TYPE_TILDE:
  case TOKEN_TYPE_EXCLAMATION_MARK:
    error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    
    {
      CLIST_CONCRETE_TYPE(cl_compiler_ast_node) cast_expr = {0};
      cl_compiler_ast_node ast_node = NULL;
      
      error_code = clCompilerFrontendParseCastExpression(frontend, &cast_expr, is_in_constant_expression);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      switch (token_type_pair.token_type) {
      case TOKEN_TYPE_BITWISE_AND: ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_BITWISE_AND_OPERATOR); break;
      case TOKEN_TYPE_MULTIPLY: ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_MULTIPLY_OPERATOR); break;
      case TOKEN_TYPE_ADDITION: ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_ADDITION_OPERATOR); break;
      case TOKEN_TYPE_SUBTRACTION: ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_SUBTRACTION_OPERATOR); break;
      case TOKEN_TYPE_TILDE: ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_BITWISE_COMPLEMENT_OPERATOR); break;
      case TOKEN_TYPE_EXCLAMATION_MARK: ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_LOGICAL_NEGATION_OPERATOR); break;
      default: ASSERT(0); break;
      }
      ASSERT(ast_node != NULL);
      
      clCompilerFrontendAstNodeAddChildrenList(ast_node, &cast_expr);
      CLIST_PUSH_BACK(cl_compiler_ast_node)(container, &ast_node);
    }
    break;
    
  case TOKEN_TYPE_KEYWORD_SIZEOF:
    error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    
    {
      error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      if (TOKEN_TYPE_LEFT_PAREN == token_type_pair.token_type) {
        CLIST_CONCRETE_TYPE(cl_compiler_ast_node) typename = {0};
        
        error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
        if (error_code != COMPILER_NO_ERROR) {
          goto finish;
        }
        
        error_code = clCompilerFrontendParseTypename(frontend, &typename);
        if (error_code != COMPILER_NO_ERROR) {
          goto finish;
        }
        {
          token_type_pair.token_type = TOKEN_TYPE_RIGHT_PAREN;
          error_code = clCompilerFrontendEnsureNextUnparsedToken(frontend, &token_type_pair);
          if (error_code != COMPILER_NO_ERROR) {
            clCompilerFrontendAstNodeFreeChildren(&typename);
          }
          {
            cl_compiler_ast_node ast_node =
              clCompilerFrontendAstNodeNew(AST_NODE_TYPE_SIZEOF_OPERATOR);
            ASSERT(ast_node != NULL);
            
            clCompilerFrontendAstNodeAddChildrenList(ast_node, &typename);
            CLIST_PUSH_BACK(cl_compiler_ast_node)(container, &ast_node);
          }
        }
      } else {
        CLIST_CONCRETE_TYPE(cl_compiler_ast_node) unary_expr = {0};
        error_code = clCompilerFrontendParseUnaryExpression(frontend, &unary_expr, is_in_constant_expression);
        if (error_code != COMPILER_NO_ERROR) {
          goto finish;
        }
        {
          cl_compiler_ast_node ast_node =
            clCompilerFrontendAstNodeNew(AST_NODE_TYPE_SIZEOF_OPERATOR);
          ASSERT(ast_node != NULL);
          
          clCompilerFrontendAstNodeAddChildrenList(ast_node, &unary_expr);
          CLIST_PUSH_BACK(cl_compiler_ast_node)(container, &ast_node);
        }
      }
    }
    break;
    
  default:
    error_code = clCompilerFrontendParsePostfixExpression(frontend, container, is_in_constant_expression);
    break;
  }
  
 finish:
  return error_code;
}

/*
 * initializer_list
 *   : initializer (',' initializer)*
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseInitializerList(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  cl_bool continue_parsing_init_declarator = CL_TRUE;
  
  error_code = clCompilerFrontendParseInitializer(frontend, &internal_container);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  while (CL_TRUE == continue_parsing_init_declarator) {
    error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    if (token_type_pair.token_type != TOKEN_TYPE_COMMA) {
      continue_parsing_init_declarator = CL_FALSE;
    } else {
      error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      error_code = clCompilerFrontendParseInitializer(frontend, &internal_container);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
    }
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  return error_code;
}

static cl_compiler_error_code
clCompilerFrontendParseArgumentExpressionList(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  ASSERT(frontend != NULL);
  ASSERT(0 && "TODO");
  return error_code;
}

/*
 * postfix_expression
 *   :
 *   ( primary_expression
 *   | '(' type-name ')' '{' initializer-list '}'
 *   | '(' type-name ')' '{' initializer-list ',' '}'
 *   )
 *   ( '[' expression ']'
 *   | '(' ')'
 *   | '(' argument_expression_list ')'
 *   | '.' 'identifier'
 *   | '->' 'identifier'
 *   | '++'
 *   | '--'
 *   )*
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParsePostfixExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container,
    cl_bool const is_in_constant_expression) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  cl_bool parse_finish;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  
  ASSERT(frontend != NULL);
  
  /* handle the first part:
   *
   * postfix_expression
   *   :
   *   ( primary_expression
   *   | '(' type-name ')' '{' initializer-list '}'
   *   | '(' type-name ')' '{' initializer-list ',' '}'
   *   )
   */
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  switch (token_type_pair.token_type) {
  case TOKEN_TYPE_LEFT_PAREN: /* '(' */
    ASSERT(0 && "TODO");
    break;
    
  default:
  {
    CLIST_CONCRETE_TYPE(cl_compiler_ast_node) prim_expr = {0};
    
    error_code = clCompilerFrontendParsePrimaryExpression(frontend, &prim_expr);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      CLIST_SPLICE(cl_compiler_ast_node)(&internal_container, NULL, &prim_expr);
    }
  }
  break;
  }
  
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  parse_finish = CL_FALSE;
  while (CL_FALSE == parse_finish) {
    switch (token_type_pair.token_type) {
    case TOKEN_TYPE_LEFT_BRACKET: /* '[' */
    {
      CLIST_CONCRETE_TYPE(cl_compiler_ast_node) expr = {0};
      
      error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      
      error_code = clCompilerFrontendParseExpression(frontend, &expr);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      
      token_type_pair.token_type = TOKEN_TYPE_RIGHT_BRACKET; /* ']' */
      error_code = clCompilerFrontendEnsureNextUnparsedToken(frontend, &token_type_pair);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      
      if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
        cl_compiler_ast_node ast_node =
          clCompilerFrontendAstNodeNew(AST_NODE_TYPE_ARRAY_SUBSCRIPTING);
        ASSERT(ast_node != NULL);
        
        clCompilerFrontendAstNodeAddChildrenList(ast_node, &expr);
        CLIST_PUSH_BACK(cl_compiler_ast_node)(&internal_container, &ast_node);
      }
    }
    break;
    
    case TOKEN_TYPE_DOT: /* '.' */
    case TOKEN_TYPE_ARROW: /* '->' */
    {
      cl_compiler_ast_node ast_node = NULL;
      cl_compiler_token_type token_type = token_type_pair.token_type;
      
      error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      
      /* parse 'identiier' */
      error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      if (token_type_pair.token_type != TOKEN_TYPE_IDENTIFIER) {
        goto finish;
      }
      
      if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
        if (TOKEN_TYPE_DOT == token_type) {
          ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_CONCRETE_TYPE_MEMBER);
        } else {
          ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_PTR_TYPE_MEMBER);
        }
        ((cl_compiler_ast_node_with_token)ast_node)->token = token_type_pair.token;
        ASSERT(ast_node != NULL);
      }
      error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend); /* consume 'identifier' */
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      
      if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
        CLIST_PUSH_BACK(cl_compiler_ast_node)(&internal_container, &ast_node);
      }
    }
    break;
    
    case TOKEN_TYPE_INCREMENT: /* '++' */
    case TOKEN_TYPE_DECREMENT: /* '--' */
    {
      cl_compiler_ast_node ast_node = NULL;
      
      if (CL_TRUE == is_in_constant_expression) {
        error(frontend, "this operator is not allowed in a constant expression");
        error_code = COMPILER_ERROR__PARSING__UNALLOWED_OPERATOR_IN_CONSTANT_EXPRESSION;
        goto finish;
      }
      
      error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      
      if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
        if (TOKEN_TYPE_INCREMENT == token_type_pair.token_type) {
          ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_POSTFIX_INCREMENT_OPERATOR);
        } else {
          ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_POSTFIX_DECREMENT_OPERATOR);
        }
        ASSERT(ast_node != NULL);
        
        CLIST_PUSH_BACK(cl_compiler_ast_node)(&internal_container, &ast_node);
      }
    }
    break;
    
    case TOKEN_TYPE_LEFT_PAREN: /* '(' */
    {
      error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend); /* consume '(' */
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      
      error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      if (TOKEN_TYPE_RIGHT_PAREN == token_type_pair.token_type) { /* ')' */
        cl_compiler_ast_node ast_node = NULL;
        
        error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
        if (error_code != COMPILER_NO_ERROR) {
          goto finish;
        }
        
        if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
          ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_FUNCTION_CALL);
          ASSERT(ast_node != NULL);
          
          CLIST_PUSH_BACK(cl_compiler_ast_node)(&internal_container, &ast_node);
        }
      } else {
        cl_compiler_ast_node ast_node = NULL;
        CLIST_CONCRETE_TYPE(cl_compiler_ast_node) argu_expr_list = {0};
        
        error_code = clCompilerFrontendParseArgumentExpressionList(frontend, &argu_expr_list);
        if (error_code != COMPILER_NO_ERROR) {
          goto finish;
        }
        
        token_type_pair.token_type = TOKEN_TYPE_RIGHT_PAREN; /* ')' */
        error_code = clCompilerFrontendEnsureNextUnparsedToken(frontend, &token_type_pair);
        if (error_code != COMPILER_NO_ERROR) {
          goto finish;
        }
        
        if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
          ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_FUNCTION_CALL);
          ASSERT(ast_node != NULL);
          
          clCompilerFrontendAstNodeAddChildrenList(ast_node, &argu_expr_list);
          CLIST_PUSH_BACK(cl_compiler_ast_node)(&internal_container, &ast_node);
        }
      }
    }
    break;
    
    default:
      parse_finish = CL_TRUE;
      break;
    }
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  clCompilerFrontendAstNodeFreeChildren(&internal_container);
  return error_code;
}

/* cast_expression
 *   : ( '(' type-name ')' )* unary_expression
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseCastExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container,
    cl_bool const is_in_constant_expression) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  _cl_compiler_token_type_pair token_type_pair;
  cl_bool parse_finish = CL_FALSE;
  
  ASSERT(frontend != NULL);
  
  while (CL_FALSE == parse_finish) {
    error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    if (token_type_pair.token_type != TOKEN_TYPE_LEFT_PAREN) {
      parse_finish = CL_TRUE;
    } else {
      error_code = clCompilerFrontendParseTypename(frontend, &internal_container);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
    }
  }
  
  error_code = clCompilerFrontendParseUnaryExpression(frontend, &internal_container, is_in_constant_expression);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  return COMPILER_NO_ERROR;
  
 finish:
  clCompilerFrontendAstNodeFreeChildren(&internal_container);  
  return error_code;
}

/*
 * primary_expression
 *   : identifier
 *   | constant
 *   | string-literal
 *   | '(' expression ')'
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParsePrimaryExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  cl_compiler_ast_node ast_node = NULL;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  
  ASSERT(frontend != NULL);
  
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  switch (token_type_pair.token_type) {
  case TOKEN_TYPE_IDENTIFIER:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_IDENTIFIER);
      goto create_ast_node_done;
    }
    break;
  case TOKEN_TYPE_CONSTANT_INT:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_CONSTANT_INT);
      goto create_ast_node_done;
    }
    break;
  case TOKEN_TYPE_CONSTANT_UNSIGNED_INT:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_CONSTANT_UNSIGNED_INT);
      goto create_ast_node_done;
    }
    break;
  case TOKEN_TYPE_CONSTANT_LONG_INT:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_CONSTANT_LONG_INT);
      goto create_ast_node_done;
    }
    break;
  case TOKEN_TYPE_CONSTANT_UNSIGNED_LONG_INT:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_CONSTANT_UNSIGNED_LONG_INT);
      goto create_ast_node_done;
    }
    break;
  case TOKEN_TYPE_CONSTANT_LONGLONG_INT:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_CONSTANT_LONGLONG_INT);
      goto create_ast_node_done;
    }
    break;
  case TOKEN_TYPE_CONSTANT_UNSIGNED_LONGLONG_INT:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_CONSTANT_UNSIGNED_LONGLONG_INT);
      goto create_ast_node_done;
    }
    break;
  case TOKEN_TYPE_CONSTANT_FLOAT:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_CONSTANT_FLOAT);
      goto create_ast_node_done;
    }
    break;
  case TOKEN_TYPE_CONSTANT_DOUBLE:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_CONSTANT_DOUBLE);
      goto create_ast_node_done;
    }
    break;
  case TOKEN_TYPE_CONSTANT_LONG_DOUBLE:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_CONSTANT_LONG_DOUBLE);
      goto create_ast_node_done;
    }
    break;
  case TOKEN_TYPE_STRING:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_STRING);
      goto create_ast_node_done;
    }
    break;
  case TOKEN_TYPE_LONG_STRING:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_LONG_STRING);
      goto create_ast_node_done;
    }
    break;
  case TOKEN_TYPE_CHAR:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_CHAR);
      goto create_ast_node_done;
    }
    break;
  case TOKEN_TYPE_LONG_CHAR:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_LONG_CHAR);
      
    create_ast_node_done:
      ASSERT(ast_node != NULL);
      ((cl_compiler_ast_node_with_token)ast_node)->token = token_type_pair.token;
      error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      CLIST_PUSH_BACK(cl_compiler_ast_node)(&internal_container, &ast_node);
    }
    break;
    
  case TOKEN_TYPE_LEFT_PAREN: /* '(' */
    error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    
    error_code = clCompilerFrontendParseArgumentExpressionList(frontend, &internal_container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    
    token_type_pair.token_type = TOKEN_TYPE_RIGHT_PAREN; /* ')' */
    error_code = clCompilerFrontendEnsureNextUnparsedToken(frontend, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    break;
    
  default:
    error_code = COMPILER_ERROR__PREPROCESSOR__PARSE_PRIMARY_EXPRESSION_FAILED;
    break;
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  clCompilerFrontendAstNodeFreeChildren(&internal_container);
  return error_code;
}

/**
 * multiplicative_expression
 *   : (cast_expression)('*' cast_expression | '/' cast-expreesion | '%' cast_expression)*
 */
static cl_compiler_error_code
clCompilerFrontendParseMultiplicativeExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container,
    cl_bool const is_in_constant_expression) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) left_cast_expr = {0};
  
  ASSERT(frontend != NULL);
  ASSERT(container != NULL);
  
  /* parse 1st shift expression */
  error_code = clCompilerFrontendParseCastExpression(frontend, &left_cast_expr, is_in_constant_expression);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  /* check if next token is '*' or '/' or '%' */
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  if ((token_type_pair.token_type != TOKEN_TYPE_MULTIPLY) &&
      (token_type_pair.token_type != TOKEN_TYPE_DIVISION) &&
      (token_type_pair.token_type != TOKEN_TYPE_MODULO)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &left_cast_expr);
  } else {
    ASSERT(0 && "TODO");
  }
  return COMPILER_NO_ERROR;
  
 finish:
  clCompilerFrontendAstNodeFreeChildren(&left_cast_expr);
  return error_code;
}

/*
 * additive_expression
 *   : (multiplicative_expression) ('+' multiplicative_expression | '-' multiplicative_expression)*
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseAdditiveExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container,
    cl_bool const is_in_constant_expression) {
  cl_compiler_ast_node top_ast_node = NULL;
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) left_multiplicative_expr = {0};
  
  ASSERT(frontend != NULL);
  ASSERT(container != NULL);
  
  /* parse 1st multiplicative expression */
  error_code = clCompilerFrontendParseMultiplicativeExpression(
      frontend, &left_multiplicative_expr, is_in_constant_expression);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  /* check if next token is '+' or '-' */
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  if ((token_type_pair.token_type != TOKEN_TYPE_ADDITION) &&
      (token_type_pair.token_type != TOKEN_TYPE_SUBTRACTION)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &left_multiplicative_expr);
  } else {
    cl_compiler_ast_node ast_node = NULL;
    /* set to TOKEN_TYPE_MEANINGLESS to be the initial value */
    cl_compiler_token_type prev_token_type = TOKEN_TYPE_MEANINGLESS;
    cl_bool parse_finish = CL_FALSE;
    
    for (;;) {
      CLIST_CONCRETE_TYPE(cl_compiler_ast_node) right_multiplicative_expr = {0};
      
      /* check if next token is '+' or '-' */
      error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      if (token_type_pair.token_type != prev_token_type) {
        /* The current peeking operator doesn't equal to the previous operator,
         * hence, need to create a new operator ast node, such that, ex:
         *
         * a + b - c
         *
         *     -
         *    / \
         *   +   c
         *  / \
         * a   b
         *
         * (To preserve the left association) */
        if ((TOKEN_TYPE_ADDITION == token_type_pair.token_type) ||
            (TOKEN_TYPE_SUBTRACTION == token_type_pair.token_type)) {
          cl_compiler_ast_node tmp_ast_node = NULL;
          /* create a new ast_node */
          if (TOKEN_TYPE_ADDITION == token_type_pair.token_type) {
            tmp_ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_ADDITION_OPERATOR);
          } else if (TOKEN_TYPE_SUBTRACTION == token_type_pair.token_type) {
            tmp_ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_SUBTRACTION_OPERATOR);
          } else {
            ASSERT(0 && "can not reach here.");
          }
          ASSERT(tmp_ast_node != NULL);
          
          if (NULL == ast_node) {
            ast_node = tmp_ast_node;
            top_ast_node = tmp_ast_node;
            /* encounter the 1st '+' or '-' operator */
            clCompilerFrontendAstNodeAddChildrenList(tmp_ast_node, &left_multiplicative_expr);
          } else {
            ASSERT(top_ast_node != NULL);
            /* add ast_node as tmp_ast_node's children, because left association. */
            clCompilerFrontendAstNodeAddChildren(tmp_ast_node, ast_node);
            top_ast_node = tmp_ast_node;
          }
          ASSERT(top_ast_node != NULL);
          ASSERT(ast_node != NULL);
        } else {
          /* encounter a token which is NOT '+' or '-', therefore, finish the parsing this time */
          parse_finish = CL_TRUE;
        }
        prev_token_type = token_type_pair.token_type;
      } else {
        /* current operator is equal to the previous operator, we will add the multiplicative_expr
         * in the previous operator ast node. Ex:
         *
         *       +
         *  / / / \ \ \ 
         * o o o   o o o
         */
      }
      if (CL_TRUE == parse_finish) {
        break;
      }
      /* we found a '+' or a '-' */
      error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend); /* consume '+' or '-' */
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      error_code = clCompilerFrontendParseMultiplicativeExpression(
          frontend, &right_multiplicative_expr, is_in_constant_expression);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      clCompilerFrontendAstNodeAddChildrenList(ast_node, &right_multiplicative_expr);
    }
    CLIST_PUSH_BACK(cl_compiler_ast_node)(container, &ast_node);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  if (top_ast_node != NULL) {
    clCompilerFrontendAstNodeDelete(top_ast_node);
  }
  clCompilerFrontendAstNodeFreeChildren(&left_multiplicative_expr);
  return error_code;
}

/*
 * bitwise-shift_expression
 *   : additive_expression (('<<'|'>>') additive_expression)*
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseBitwiseShiftExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container,
    cl_bool const is_in_constant_expression) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) left_additive_expr = {0};
  
  ASSERT(frontend != NULL);
  ASSERT(container != NULL);
  
  /* parse 1st additive expression */
  error_code = clCompilerFrontendParseAdditiveExpression(
      frontend, &left_additive_expr, is_in_constant_expression);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  /* check if next token is '<<' or '>>' */
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  if ((token_type_pair.token_type != TOKEN_TYPE_LEFT_SHIFT) &&
      (token_type_pair.token_type != TOKEN_TYPE_RIGHT_SHIFT)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &left_additive_expr);
  } else {
    ASSERT(0 && "TODO");
  }
  return COMPILER_NO_ERROR;
  
 finish:
  clCompilerFrontendAstNodeFreeChildren(&left_additive_expr);
  return error_code;
}

/*
 * relational_expression
 *   : bitwise-shift_expression (('<'|'>'|'<='|'>=') bitwise-shift_expression)*
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseRelationalExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container,
    cl_bool const is_in_constant_expression) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) left_shift_expr = {0};
  
  ASSERT(frontend != NULL);
  ASSERT(container != NULL);
  
  /* parse 1st shift expression */
  error_code = clCompilerFrontendParseBitwiseShiftExpression(frontend, &left_shift_expr, is_in_constant_expression);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  /* check if next token is '<' or '>' or '<=' or '>=' */
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  if ((token_type_pair.token_type != TOKEN_TYPE_LESS_THAN) &&
      (token_type_pair.token_type != TOKEN_TYPE_LESS_EQUAL) &&
      (token_type_pair.token_type != TOKEN_TYPE_GREATER_THAN) &&
      (token_type_pair.token_type != TOKEN_TYPE_GREATER_EQUAL)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &left_shift_expr);
  } else {
    ASSERT(0 && "TODO");
  }
  return COMPILER_NO_ERROR;
  
 finish:
  clCompilerFrontendAstNodeFreeChildren(&left_shift_expr);
  return error_code;
}

/*
 * equality_expression
 *   : relational_expression (('=='|'!=') relational_expression)*
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseEqualityExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container,
    cl_bool const is_in_constant_expression) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) left_relational_expr = {0};
  
  ASSERT(frontend != NULL);
  ASSERT(container != NULL);
  
  /* parse 1st relational expression */
  error_code = clCompilerFrontendParseRelationalExpression(frontend, &left_relational_expr, is_in_constant_expression);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  /* check if next token is '==' or '!=' */
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  if ((token_type_pair.token_type != TOKEN_TYPE_EQUAL) &&
      (token_type_pair.token_type != TOKEN_TYPE_NOT_EQUAL)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &left_relational_expr);
  } else {
    ASSERT(0 && "TODO");
  }
  return COMPILER_NO_ERROR;
  
 finish:
  clCompilerFrontendAstNodeFreeChildren(&left_relational_expr);
  return error_code;
}

/*
 * bitwise-and_expression
 *   : equality_expression ('&' equality_expression)*
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseBitwiseAndExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container,
    cl_bool const is_in_constant_expression) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) left_equality_expr = {0};
  
  ASSERT(frontend != NULL);
  ASSERT(container != NULL);
  
  /* parse 1st equality expression */
  error_code = clCompilerFrontendParseEqualityExpression(frontend, &left_equality_expr, is_in_constant_expression);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  /* check if next token is '&' or not */
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  if (token_type_pair.token_type != TOKEN_TYPE_BITWISE_AND) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &left_equality_expr);
  } else {
    ASSERT(0 && "TODO");
  }
  return COMPILER_NO_ERROR;
  
 finish:
  clCompilerFrontendAstNodeFreeChildren(&left_equality_expr);
  return error_code;
}

/*
 * bitwise-xor_expression
 *   : bitwise-and_expression ('^' bitwise-and_expression)*
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseBitwiseXorExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container,
    cl_bool const is_in_constant_expression) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) left_and_expr = {0};
  
  ASSERT(frontend != NULL);
  ASSERT(container != NULL);
  
  /* parse 1st bitwise and expression */
  error_code = clCompilerFrontendParseBitwiseAndExpression(frontend, &left_and_expr, is_in_constant_expression);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  /* check if next token is '^' or not */
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  if (token_type_pair.token_type != TOKEN_TYPE_BITWISE_XOR) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &left_and_expr);
  } else {
    ASSERT(0 && "TODO");
  }
  return COMPILER_NO_ERROR;
  
 finish:
  clCompilerFrontendAstNodeFreeChildren(&left_and_expr);
  return error_code;
}

/*
 * bitwise-or_expression
 *   : bitwise-xor_expression ('|' bitwise-xor_expression)*
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseBitwiseOrExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container,
    cl_bool const is_in_constant_expression) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) left_xor_expr = {0};
  
  ASSERT(frontend != NULL);
  ASSERT(container != NULL);
  
  /* parse 1st bitwise xor expression */
  error_code = clCompilerFrontendParseBitwiseXorExpression(frontend, &left_xor_expr, is_in_constant_expression);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  /* check if next token is '|' or not */
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  if (token_type_pair.token_type != TOKEN_TYPE_BITWISE_OR) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &left_xor_expr);
  } else {
    ASSERT(0 && "TODO");
  }
  return COMPILER_NO_ERROR;
  
 finish:
  clCompilerFrontendAstNodeFreeChildren(&left_xor_expr);
  return error_code;
}

/*
 * logical-and_expression
 *   : bitwise-or_expression ('&&' bitwise-or_expression)*
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseLogicalAndExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container,
    cl_bool const is_in_constant_expression) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) left_or_expr = {0};
  
  ASSERT(frontend != NULL);
  ASSERT(container != NULL);
  
  /* parse 1st bitwise or expression */
  error_code = clCompilerFrontendParseBitwiseOrExpression(frontend, &left_or_expr, is_in_constant_expression);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  /* check if next token is '&&' or not */
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  if (token_type_pair.token_type != TOKEN_TYPE_LOGICAL_AND) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &left_or_expr);
  } else {
    ASSERT(0 && "TODO");
  }
  return COMPILER_NO_ERROR;
  
 finish:
  clCompilerFrontendAstNodeFreeChildren(&left_or_expr);
  return error_code;
}

/*
 * logical-or_expression
 *   : logical-and_expression ('||' logical-and_expression)*
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseLogicalOrExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container,
    cl_bool const is_in_constant_expression) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) left_and_expr = {0};
  
  ASSERT(frontend != NULL);
  ASSERT(container != NULL);
  
  /* parse 1st logical and expression */
  error_code = clCompilerFrontendParseLogicalAndExpression(frontend, &left_and_expr, is_in_constant_expression);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  /* check if next token is '||' or not */
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  if (token_type_pair.token_type != TOKEN_TYPE_LOGICAL_OR) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &left_and_expr);
  } else {
    ASSERT(0 && "TODO");
  }
  return COMPILER_NO_ERROR;
  
 finish:
  clCompilerFrontendAstNodeFreeChildren(&left_and_expr);
  return error_code;
}

/*
 * conditional_expression
 *   : logical-or_expression ('?' expression ':' conditional_expression)?
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseConditionalExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container,
    cl_bool const is_in_constant_expression) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) or_expr = {0};
  
  ASSERT(frontend != NULL);
  ASSERT(container != NULL);
  
  error_code = clCompilerFrontendParseLogicalOrExpression(frontend, &or_expr, is_in_constant_expression);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  /* check if the next unparsed token is '?' or not. */
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  switch (token_type_pair.token_type) {
  case TOKEN_TYPE_QUESTION_MARK:
    error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    {
      cl_compiler_ast_node if_node = NULL;
      CLIST_CONCRETE_TYPE(cl_compiler_ast_node) expr = {0};
      CLIST_CONCRETE_TYPE(cl_compiler_ast_node) conditional_expr = {0};
      
      error_code = clCompilerFrontendParseExpression(frontend, &expr);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      
      token_type_pair.token_type = TOKEN_TYPE_COLON;
      error_code = clCompilerFrontendEnsureNextUnparsedToken(frontend, &token_type_pair);
      if (error_code != COMPILER_NO_ERROR) {
        clCompilerFrontendAstNodeFreeChildren(&expr);
        goto finish;
      }
      
      error_code = clCompilerFrontendParseExpression(frontend, &conditional_expr);
      if (error_code != COMPILER_NO_ERROR) {
        clCompilerFrontendAstNodeFreeChildren(&expr);
        goto finish;
      }
      
      if_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_IF);
      ASSERT(if_node != NULL);
      
      clCompilerFrontendAstNodeAddChildrenList(if_node, &or_expr);
      clCompilerFrontendAstNodeAddChildrenList(if_node, &expr);
      clCompilerFrontendAstNodeAddChildrenList(if_node, &conditional_expr);
      
      CLIST_PUSH_BACK(cl_compiler_ast_node)(container, &if_node);
    }
    break;
    
  default:
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &or_expr);
    break;
  }
  return COMPILER_NO_ERROR;
  
 finish:
  clCompilerFrontendAstNodeFreeChildren(&or_expr);
  return error_code;
}

/*
 * constant_expression
 *   : conditional_expression
 *   ;
 */
cl_compiler_error_code
clCompilerFrontendParseConstantExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  ASSERT(frontend != NULL);
  ASSERT(container != NULL);
  return clCompilerFrontendParseConditionalExpression(frontend, container, CL_TRUE);
}

/*
 * assignment_expression
 *   : unary_expression assignment_operator assignment_expression
 *   | conditional_expression
 *   ;
 *
 * assignment_operator
 *   : '='
 *   | '*='
 *   | '/='
 *   | '%='
 *   | '+='
 *   | '-='
 *   | '<<='
 *   | '>>='
 *   | '&='
 *   | '^='
 *   | '|='
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseAssignmentExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  cl_int alternative = 1;
  
  clCompilerFrontendEnterBackTracking(frontend);
  
  error_code = clCompilerFrontendParseUnaryExpression(frontend, NULL, CL_FALSE);
  if (error_code != COMPILER_NO_ERROR) {
    alternative = 2;
    goto find_alternative;
  }
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  switch (token_type_pair.token_type) {
  case TOKEN_TYPE_ASSIGN:
  case TOKEN_TYPE_ASSIGN_AND:
  case TOKEN_TYPE_ASSIGN_OR:
  case TOKEN_TYPE_ASSIGN_XOR:
  case TOKEN_TYPE_ASSIGN_ADDITION:
  case TOKEN_TYPE_ASSIGN_SUBTRACTION:
  case TOKEN_TYPE_ASSIGN_MULTIPLY:
  case TOKEN_TYPE_ASSIGN_DIVISION:
  case TOKEN_TYPE_ASSIGN_MODULO:
  case TOKEN_TYPE_ASSIGN_LEFT_SHIFT:
  case TOKEN_TYPE_ASSIGN_RIGHT_SHIFT:
    goto find_alternative;
    
  default:
    alternative = 2;
    goto find_alternative;
  }
  
 find_alternative:
  /* stop backtracking mechanism */
  clCompilerFrontendLeaveBackTracking(frontend);
  
  switch (alternative) {
  case 1:
    error_code = clCompilerFrontendParseUnaryExpression(frontend, &internal_container, CL_FALSE);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    switch (token_type_pair.token_type) {
    case TOKEN_TYPE_ASSIGN:
    case TOKEN_TYPE_ASSIGN_AND:
    case TOKEN_TYPE_ASSIGN_OR:
    case TOKEN_TYPE_ASSIGN_XOR:
    case TOKEN_TYPE_ASSIGN_ADDITION:
    case TOKEN_TYPE_ASSIGN_SUBTRACTION:
    case TOKEN_TYPE_ASSIGN_MULTIPLY:
    case TOKEN_TYPE_ASSIGN_DIVISION:
    case TOKEN_TYPE_ASSIGN_MODULO:
    case TOKEN_TYPE_ASSIGN_LEFT_SHIFT:
    case TOKEN_TYPE_ASSIGN_RIGHT_SHIFT:
      error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      break;
    default:
      ASSERT(0 && "should not reach here");
      break;
    }
    error_code = clCompilerFrontendParseAssignmentExpression(frontend, &internal_container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    break;
    
  case 2:
    error_code = clCompilerFrontendParseConditionalExpression(frontend, &internal_container, CL_FALSE);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    break;
  default:
    ASSERT(0);
    break;
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  return error_code;
}

/*
 * expression
 *   : assignment_expression (',' assignment_expression)*
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseExpression(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  cl_bool continue_parsing_assignment_expression = CL_TRUE;
  
  error_code = clCompilerFrontendParseAssignmentExpression(frontend, &internal_container);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  while (CL_TRUE == continue_parsing_assignment_expression) {
    error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    if (token_type_pair.token_type != TOKEN_TYPE_COMMA) {
      continue_parsing_assignment_expression = CL_FALSE;
    } else {
      error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      error_code = clCompilerFrontendParseAssignmentExpression(frontend, &internal_container);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
    }
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  return error_code;
}

/*
 * storage_class_specifier
 *   : 'typedef'
 *   | 'extern'
 *   | 'static'
 *   | 'auto'
 *   | 'register'
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseStorageSpecifier(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  cl_compiler_ast_node ast_node = NULL;
  
  ASSERT(frontend != NULL);
  
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  switch (token_type_pair.token_type) {
  case TOKEN_TYPE_KEYWORD_TYPEDEF:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD_TYPEDEF);
      goto add_to_container;
    }
    break;
  case TOKEN_TYPE_KEYWORD_EXTERN:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD_EXTERN);
      goto add_to_container;
    }
    break;
  case TOKEN_TYPE_KEYWORD_STATIC:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD_STATIC);
      goto add_to_container;
    }
    break;
  case TOKEN_TYPE_KEYWORD_AUTO:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD_AUTO);
      goto add_to_container;
    }
    break;
  case TOKEN_TYPE_KEYWORD_REGISTER:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD_REGISTER);
    add_to_container:
      CLIST_PUSH_BACK(cl_compiler_ast_node)(container, &ast_node);
    }
    break;
  default:
    error_code = COMPILER_ERROR__PARSING__ERROR;
    goto finish;
  }
  error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  return COMPILER_NO_ERROR;
  
 finish:
  return error_code;
}

/*
 * struct_or_union
 *   : 'struct'
 *   | 'union'
 *   ;
 *
 * struct_or_union_specifier
 *   : struct_or_union IDENTIFIER? '{' struct_declaration_list '}'
 *   | struct_or_union IDENTIFIER
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseStructOrUnionSpecifier(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  ASSERT(0 && "TODO");
  return error_code;
}

/*
 * enum_specifier
 *   : 'enum' '{' enumerator_list '}'
 *   : 'enum' '{' enumerator_list ',' '}'
 *   | 'enum' IDENTIFIER '{' enumerator_list '}'
 *   | 'enum' IDENTIFIER '{' enumerator_list ',' '}'
 *   | 'enum' IDENTIFIER
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseEnumSpecifier(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  ASSERT(0 && "TODO");
  return error_code;
}

/*
 * typedef_name
 *   : IDENTIFIER
 *   ;
 *
 * type_specifier
 *   : 'void'
 *   | 'char'
 *   | 'short'
 *   | 'int'
 *   | 'long'
 *   | 'float'
 *   | 'double'
 *   | 'signed'
 *   | 'unsigned'
 *   | '_Bool'
 *   | '_Complex'
 *   | '_Imaginary'
 *   | struct_or_union_specifier
 *   | enum_specifier
 *   | typedef_name
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseTypeSpecifier(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  cl_compiler_ast_node ast_node = NULL;
  
  ASSERT(frontend != NULL);
  
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  switch (token_type_pair.token_type) {
  case TOKEN_TYPE_KEYWORD_VOID:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD_VOID);
      goto add_to_container;
    }
    break;
  case TOKEN_TYPE_KEYWORD_CHAR:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD_CHAR);
      goto add_to_container;
    }
    break;
  case TOKEN_TYPE_KEYWORD_SHORT:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD_SHORT);
      goto add_to_container;
    }
    break;
  case TOKEN_TYPE_KEYWORD_INT:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD_INT);
      goto add_to_container;
    }
    break;
  case TOKEN_TYPE_KEYWORD_LONG:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD_LONG);
      goto add_to_container;
    }
    break;
  case TOKEN_TYPE_KEYWORD_FLOAT:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD_FLOAT);
      goto add_to_container;
    }
    break;
  case TOKEN_TYPE_KEYWORD_DOUBLE:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD_DOUBLE);
      goto add_to_container;
    }
    break;
  case TOKEN_TYPE_KEYWORD_SIGNED:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD_SIGNED);
      goto add_to_container;
    }
    break;
  case TOKEN_TYPE_KEYWORD_UNSIGNED:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD_UNSIGNED);
      goto add_to_container;
    }
    break;
  case TOKEN_TYPE_KEYWORD__BOOL:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD__BOOL);
      goto add_to_container;
    }
    break;
  case TOKEN_TYPE_KEYWORD__COMPLEX:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD__COMPLEX);
      goto add_to_container;
    }
    break;
  case TOKEN_TYPE_KEYWORD__IMAGINARY:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD__IMAGINARY);
    add_to_container:    
      CLIST_PUSH_BACK(cl_compiler_ast_node)(container, &ast_node);
    }
    break;
    
  default:
    error_code = COMPILER_ERROR__PARSING__ERROR;
    goto finish;
  }
  error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  return COMPILER_NO_ERROR;
  
 finish:
  return error_code;
}

/*
 * type_qualifier
 *   : 'const'
 *   | 'restrict'
 *   | 'volatile'
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseTypeQualifier(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  cl_compiler_ast_node ast_node = NULL;
  
  ASSERT(frontend != NULL);
  
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  switch (token_type_pair.token_type) {
  case TOKEN_TYPE_KEYWORD_CONST:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD_CONST);
      goto add_to_container;
    }
    break;
  case TOKEN_TYPE_KEYWORD_RESTRICT:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD_RESTRICT);
      goto add_to_container;
    }
    break;
  case TOKEN_TYPE_KEYWORD_VOLATILE:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD_VOLATILE);
    add_to_container:
      CLIST_PUSH_BACK(cl_compiler_ast_node)(container, &ast_node);
    }
    break;
  default:
    error_code = COMPILER_ERROR__PARSING__ERROR;
    goto finish;
  }
  error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  return COMPILER_NO_ERROR;
  
 finish:
  return error_code;
}

/*
 * function_specifier
 *   : 'inline'
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseFunctionSpecifier(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  cl_compiler_ast_node ast_node = NULL;
  
  ASSERT(frontend != NULL);
  
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  switch (token_type_pair.token_type) {
  case TOKEN_TYPE_KEYWORD_INLINE:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_KEYWORD_INLINE);
      CLIST_PUSH_BACK(cl_compiler_ast_node)(container, &ast_node);
    }
    break;
  default:
    error_code = COMPILER_ERROR__PARSING__ERROR;
    goto finish;
  }
  error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  return COMPILER_NO_ERROR;
  
 finish:
  return error_code;
}

/*
 * declaration_specifiers
 *   : (  storage_class_specifier
 *     |  type_specifier
 *     |  type_qualifier
 *     |  function_specifier
 *     )+
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseDeclarationSpecifiers(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  cl_bool find_one = CL_FALSE;
  
  ASSERT(frontend != NULL);
  
  for (;;) {
    error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    switch (token_type_pair.token_type) {
    case TOKEN_TYPE_KEYWORD_TYPEDEF:
    case TOKEN_TYPE_KEYWORD_EXTERN:
    case TOKEN_TYPE_KEYWORD_STATIC:
    case TOKEN_TYPE_KEYWORD_AUTO:
    case TOKEN_TYPE_KEYWORD_REGISTER:
      error_code = clCompilerFrontendParseStorageSpecifier(frontend, &internal_container);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      break;
    case TOKEN_TYPE_KEYWORD_CONST:
    case TOKEN_TYPE_KEYWORD_RESTRICT:
    case TOKEN_TYPE_KEYWORD_VOLATILE:
      error_code = clCompilerFrontendParseTypeQualifier(frontend, &internal_container);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      break;
    case TOKEN_TYPE_KEYWORD_INLINE:
      error_code = clCompilerFrontendParseFunctionSpecifier(frontend, &internal_container);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      break;
    default:
      error_code = clCompilerFrontendParseTypeSpecifier(frontend, &internal_container);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      break;
    }
    find_one = CL_TRUE;
  }
  
 finish:
  /* If we find at least a valid token, this will not be an error condition. */
  if (CL_TRUE == find_one) {
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
    }
    return COMPILER_NO_ERROR;
  } else {
    return error_code;
  }
}

/*
 * pointer
 *   : '*' type_qualifier+ pointer?
 *   | '*' pointer
 *   | '*'
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParsePointer(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  ASSERT(0 && "TODO");
  return error_code;
}

/*
 * abstract_declarator_suffix
 *   : '[' ']'
 *   | '[' assignment_expression ']'
 *   | '[' '*' ']'
 *   | '(' ')'
 *   | '(' parameter_type_list ')'
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseAbstractDeclaratorSuffix(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  ASSERT(0 && "TODO");
  return error_code;
}

/*
 * direct_abstract_declarator
 *   : ( '(' abstract_declarator ')' | abstract_declarator_suffix ) abstract_declarator_suffix*
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseDirectAbstractDeclarator(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  ASSERT(0 && "TODO");
  return error_code;
}

/*
 * abstract_declarator
 *   : pointer direct_abstract_declarator?
 *   | direct_abstract_declarator
 *   ;
 *
 * Example of abstract_declarator:
 *   *
 *   *[3]
 *   (*)[3]
 *   (*)[*]
 *   *()
 *   (*)(void)
 *   (*const [])(unsigned int, ...)
 */
static cl_compiler_error_code
clCompilerFrontendParseAbstractDeclarator(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  ASSERT(0 && "TODO");
  return error_code;
}

/*
 * identifier_list
 *   : IDENTIFIER (',' IDENTIFIER)*
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseIdentifierList(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  cl_bool continue_parsing_identifier = CL_TRUE;
  cl_compiler_ast_node ast_node = NULL;
  
  PARSE_IDENTIFIER;
  
  while (CL_TRUE == continue_parsing_identifier) {
    error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    if (token_type_pair.token_type != TOKEN_TYPE_COMMA) {
      continue_parsing_identifier = CL_FALSE;
    } else {
      error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      
      PARSE_IDENTIFIER;
    }
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  clCompilerFrontendAstNodeFreeChildren(&internal_container);  
  return error_code;
}

/*
 * declarator_suffix
 *   :   '[' type_qualifier+ ']'
 *   |   '[' assignment_expression ']'
 *   |   '[' type_qualifier+ assignment_expression ']'
 *   |   '[' 'static' assignment_expression ']'
 *   |   '[' 'static' type_qualifier+ assignment_expression ']'
 *   |   '[' type_qualifier+ 'static' assignment_expression ']'
 *   |   '[' type_qualifier+ '*' ']'
 *   |   '[' '*' ']'
 *   |   '[' ']'
 *   |   '(' parameter_type_list ')'
 *   |   '(' identifier_list ')'
 *   |   '(' ')'
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseDeclaratorSuffix(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  cl_compiler_ast_node ast_node = NULL;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  switch (token_type_pair.token_type) {
  case TOKEN_TYPE_LEFT_BRACKET:
    ASSERT(0 && "TODO");
    break;
  case TOKEN_TYPE_LEFT_PAREN:
    error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    if (TOKEN_TYPE_RIGHT_PAREN == token_type_pair.token_type) {
      error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
    } else {
      error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      /* check if the next token is IDENTIFIER, if it is, it might be a IDENTIFIER list. */
      if (TOKEN_TYPE_IDENTIFIER == token_type_pair.token_type) {
        if (0) {
          ASSERT(0 && "TODO: check if this identifier is a user-defined type");
        } else {
          /* The next token is actually a type, parsing parameter_type_list instead. */
          goto parse_parameter_type_list;
        }
      } else {
      parse_parameter_type_list:
        error_code = clCompilerFrontendParseParameterTypeList(frontend, &internal_container);
        if (error_code != COMPILER_NO_ERROR) {
          goto finish;
        }
      }
      token_type_pair.token_type = TOKEN_TYPE_RIGHT_PAREN;
      error_code = clCompilerFrontendEnsureNextUnparsedToken(frontend, &token_type_pair);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
    }
    break;
  default:
    error_code = COMPILER_ERROR__PARSING__ERROR;
    goto finish;
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend) &&
      (container != NULL)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  clCompilerFrontendAstNodeFreeChildren(&internal_container);
  return error_code;
}

static cl_compiler_error_code clCompilerFrontendParseDirectDeclarator(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container);

/*
 * declarator
 *   : pointer? direct_declarator
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseDeclarator(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  _cl_compiler_token_type_pair token_type_pair;
  
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  if (TOKEN_TYPE_MULTIPLY == token_type_pair.token_type) {
    error_code = clCompilerFrontendParsePointer(frontend, &internal_container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
  }
  error_code = clCompilerFrontendParseDirectDeclarator(frontend, &internal_container);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  clCompilerFrontendAstNodeFreeChildren(&internal_container);
  return error_code;
}

/*
 * direct_declarator
 *   : ( IDENTIFIER | '(' declarator ')' ) declarator_suffix*
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseDirectDeclarator(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  cl_compiler_ast_node ast_node = NULL;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  switch (token_type_pair.token_type) {
  case TOKEN_TYPE_IDENTIFIER:
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
      ast_node = clCompilerFrontendAstNodeNew(AST_NODE_TYPE_IDENTIFIER);
      ((cl_compiler_ast_node_with_token)ast_node)->token = token_type_pair.token;
      CLIST_PUSH_BACK(cl_compiler_ast_node)(&internal_container, &ast_node);
    }
    error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    break;
  case TOKEN_TYPE_LEFT_PAREN:
    error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    error_code = clCompilerFrontendParseDeclarator(frontend, &internal_container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    token_type_pair.token_type = TOKEN_TYPE_RIGHT_PAREN;
    error_code = clCompilerFrontendEnsureNextUnparsedToken(frontend, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    break;
  default:
    error_code = COMPILER_ERROR__PARSING__ERROR;
    goto finish;
  }
  
  {
    cl_bool finish_parsing_suffix = CL_FALSE;
    
    while (CL_FALSE == finish_parsing_suffix) {
      error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      switch (token_type_pair.token_type) {
      case TOKEN_TYPE_LEFT_PAREN:
      case TOKEN_TYPE_LEFT_BRACKET:
        error_code = clCompilerFrontendParseDeclaratorSuffix(frontend, &internal_container);
        if (error_code != COMPILER_NO_ERROR) {
          break;
        }
        break;
        
      default:
        finish_parsing_suffix = CL_TRUE;
        break;
      }
    }
    
    if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend) &&
        (container != NULL)) {
      CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
    }
  }
  return COMPILER_NO_ERROR;
  
 finish:
  clCompilerFrontendAstNodeFreeChildren(&internal_container);
  return error_code;
}

/*
 * parameter_declaration
 *   : declaration_specifiers (declarator | abstract_declarator)*
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseParameterDeclaration(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  _cl_compiler_token_type_pair token_type_pair;
  cl_int alternative = 1;
  
  error_code = clCompilerFrontendParseDeclarationSpecifiers(frontend, &internal_container);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  switch (token_type_pair.token_type) {
  case TOKEN_TYPE_COMMA:
    /* this will end a 'parameter_declaration'. */
    break;
  case TOKEN_TYPE_RIGHT_PAREN:
    /* this will end all the 'parameter_declaration's. */
    break;
  default:
  {
    {
      /* start backtracking mechanism */
      clCompilerFrontendEnterBackTracking(frontend);
      
      error_code = clCompilerFrontendParseDeclarator(frontend, NULL);
      if (error_code != COMPILER_NO_ERROR) {
        alternative = 2;
        goto find_alternative;
      }
      
    find_alternative:
      /* stop backtracking mechanism */
      clCompilerFrontendLeaveBackTracking(frontend);
    }
    
    switch (alternative) {
    case 1:
      error_code = clCompilerFrontendParseDeclarator(frontend, &internal_container);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      break;
    case 2:
      error_code = clCompilerFrontendParseAbstractDeclarator(frontend, &internal_container);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      break;
    default:
      ASSERT(0 && "should never reach here");
      break;
    }
  }
  break;
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {  
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  return error_code;
}

/*
 * parameter_list
 *   : parameter_declaration (',' parameter_declaration)*
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseParameterList(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  
  for (;;) {
    error_code = clCompilerFrontendParseParameterDeclaration(frontend, &internal_container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    
    error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    if (TOKEN_TYPE_COMMA == token_type_pair.token_type) {
      error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 1, &token_type_pair);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      if (token_type_pair.token_type != TOKEN_TYPE_DOTS) {
        /* I just see a ',', not ', ...', hence, parse parameter_declaration again */
        error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
        if (error_code != COMPILER_NO_ERROR) {
          goto finish;
        }
      } else {
        break;
      }
    } else {
      break;
    }
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {  
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  return error_code;
}

/*
 * parameter_type_list
 *   : parameter_list (',' '...')?
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseParameterTypeList(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  
  error_code = clCompilerFrontendParseParameterList(frontend, &internal_container);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  if (TOKEN_TYPE_COMMA == token_type_pair.token_type) {
    error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 1, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    if (TOKEN_TYPE_DOTS == token_type_pair.token_type) {
      /* consume the ', ...' */
      error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
    }
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  return error_code;
}

/*
 * initializer
 *   : assignment_expression
 *   | '{' initializer_list ','? '}'
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseInitializer(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  if (TOKEN_TYPE_LEFT_BRACE == token_type_pair.token_type) {
    error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    error_code = clCompilerFrontendParseInitializerList(frontend, &internal_container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    if (TOKEN_TYPE_COMMA == token_type_pair.token_type) {
      error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
    }
    token_type_pair.token_type = TOKEN_TYPE_RIGHT_BRACE;
    error_code = clCompilerFrontendEnsureNextUnparsedToken(frontend, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
  } else {
    error_code = clCompilerFrontendParseAssignmentExpression(frontend, &internal_container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  return error_code;
}

/*
 * init_declarator
 *   : declarator ('=' initializer)?
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseInitDeclarator(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  
  error_code = clCompilerFrontendParseDeclarator(frontend, &internal_container);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  if (TOKEN_TYPE_ASSIGN == token_type_pair.token_type) {
    error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    error_code = clCompilerFrontendParseInitializer(frontend, &internal_container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  return error_code;
}

/*
 * init_declarator_list
 *   : init_declarator (',' init_declarator)*
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseInitDeclaratorList(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  cl_bool continue_parsing_init_declarator = CL_TRUE;
  
  error_code = clCompilerFrontendParseInitDeclarator(frontend, &internal_container);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  while (CL_TRUE == continue_parsing_init_declarator) {
    error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    if (token_type_pair.token_type != TOKEN_TYPE_COMMA) {
      continue_parsing_init_declarator = CL_FALSE;
    } else {
      error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      error_code = clCompilerFrontendParseInitDeclarator(frontend, &internal_container);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
    }
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  return error_code;
}

/*
 * declaration
 *   : declaration_specifiers init_declarator_list? ';'
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseDeclaration(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  
  error_code = clCompilerFrontendParseDeclarationSpecifiers(frontend, &internal_container);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  if (token_type_pair.token_type != TOKEN_TYPE_SEMICOLON) {
    error_code = clCompilerFrontendParseInitDeclaratorList(frontend, &internal_container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
  }
  token_type_pair.token_type = TOKEN_TYPE_SEMICOLON;
  error_code = clCompilerFrontendEnsureNextUnparsedToken(frontend, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  return error_code;
}

/*
 * labeled_statement
 *   : IDENTIFIER ':' statement
 *   | 'case' constant_expression ':' statement
 *   | 'default' ':' statement
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseLabeledStatement(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  ASSERT(0 && "TODO");
  return error_code;
}

/*
 * expression_statement
 *   : ';'
 *   | expression ';'
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseExpressionStatement(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  ASSERT(0 && "TODO");
  return error_code;
}

/*
 * selection_statement
 *   : 'if' '(' expression ')' statement ('else' statement)?
 *   | 'switch' '(' expression ')' statement
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseSelectionStatement(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  ASSERT(0 && "TODO");
  return error_code;
}

/*
  iteration_statement
  : 'while' '(' expression ')' statement
  | 'do' statement 'while' '(' expression ')' ';'
  | 'for' '(' expression? ';' expression? ';' expression? ')' statement
  | 'for' '(' declaration expression? ';' expression? ')' statement
  ;
 */
static cl_compiler_error_code
clCompilerFrontendParseIterationStatement(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  ASSERT(0 && "TODO");
  return error_code;
}

/*
 * jump_statement
 *   : 'goto' IDENTIFIER ';'
 *   | 'continue' ';'
 *   | 'break' ';'
 *   | 'return' ';'
 *   | 'return' expression ';'
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseJumpStatement(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  cl_compiler_ast_node ast_node = NULL;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  switch (token_type_pair.token_type) {
  case TOKEN_TYPE_KEYWORD_GOTO:
    error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    PARSE_IDENTIFIER;
    
    token_type_pair.token_type = TOKEN_TYPE_SEMICOLON;
    error_code = clCompilerFrontendEnsureNextUnparsedToken(frontend, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    break;
  case TOKEN_TYPE_KEYWORD_CONTINUE:
  case TOKEN_TYPE_KEYWORD_BREAK:
    error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    token_type_pair.token_type = TOKEN_TYPE_SEMICOLON;
    error_code = clCompilerFrontendEnsureNextUnparsedToken(frontend, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    break;
  case TOKEN_TYPE_KEYWORD_RETURN:
    error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    if (token_type_pair.token_type != TOKEN_TYPE_SEMICOLON) {
      error_code = clCompilerFrontendParseExpression(frontend, &internal_container);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
    }
    token_type_pair.token_type = TOKEN_TYPE_SEMICOLON;
    error_code = clCompilerFrontendEnsureNextUnparsedToken(frontend, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    break;
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  return error_code;
}

/*
 * statement
 *   : labeled_statement
 *   | compound_statement
 *   | expression_statement
 *   | selection_statement
 *   | iteration_statement
 *   | jump_statement
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseStatement(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  switch (token_type_pair.token_type) {
  case TOKEN_TYPE_LEFT_BRACE:
    error_code = clCompilerFrontendParseCompoundStatement(frontend, &internal_container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    break;    
  case TOKEN_TYPE_KEYWORD_IF:
  case TOKEN_TYPE_KEYWORD_SWITCH:
    error_code = clCompilerFrontendParseSelectionStatement(frontend, &internal_container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    break;
  case TOKEN_TYPE_KEYWORD_WHILE:
  case TOKEN_TYPE_KEYWORD_DO:
  case TOKEN_TYPE_KEYWORD_FOR:
    error_code = clCompilerFrontendParseIterationStatement(frontend, &internal_container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    break;
  case TOKEN_TYPE_KEYWORD_GOTO:
  case TOKEN_TYPE_KEYWORD_CONTINUE:
  case TOKEN_TYPE_KEYWORD_BREAK:
  case TOKEN_TYPE_KEYWORD_RETURN:
    error_code = clCompilerFrontendParseJumpStatement(frontend, &internal_container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    break;
  case TOKEN_TYPE_SEMICOLON:
    error_code = clCompilerFrontendParseExpressionStatement(frontend, &internal_container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    break;
  case TOKEN_TYPE_KEYWORD_CASE:
  case TOKEN_TYPE_KEYWORD_DEFAULT:
    error_code = clCompilerFrontendParseLabeledStatement(frontend, &internal_container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    break;
  case TOKEN_TYPE_IDENTIFIER:
    ASSERT(0 && "TODO: Maybe labeled statement.");
    break;
  default:
    /* The only possibilites is the 'expression' in the expression_statement. */
    error_code = clCompilerFrontendParseExpressionStatement(frontend, &internal_container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    break;
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  return error_code;
}

/*
 * block_item
 *   : declaration
 *   | statement
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseBlockItem(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  switch (token_type_pair.token_type) {
  case TOKEN_TYPE_KEYWORD_TYPEDEF:
  case TOKEN_TYPE_KEYWORD_EXTERN:
  case TOKEN_TYPE_KEYWORD_STATIC:
  case TOKEN_TYPE_KEYWORD_AUTO:
  case TOKEN_TYPE_KEYWORD_REGISTER:
    
  case TOKEN_TYPE_KEYWORD_CONST:
  case TOKEN_TYPE_KEYWORD_RESTRICT:
  case TOKEN_TYPE_KEYWORD_VOLATILE:
    
  case TOKEN_TYPE_KEYWORD_INLINE:
    
  case TOKEN_TYPE_KEYWORD_VOID:
  case TOKEN_TYPE_KEYWORD_CHAR:
  case TOKEN_TYPE_KEYWORD_SHORT:
  case TOKEN_TYPE_KEYWORD_INT:
  case TOKEN_TYPE_KEYWORD_LONG:
  case TOKEN_TYPE_KEYWORD_FLOAT:
  case TOKEN_TYPE_KEYWORD_DOUBLE:
  case TOKEN_TYPE_KEYWORD_SIGNED:
  case TOKEN_TYPE_KEYWORD_UNSIGNED:
  case TOKEN_TYPE_KEYWORD__BOOL:
  case TOKEN_TYPE_KEYWORD__COMPLEX:
  case TOKEN_TYPE_KEYWORD__IMAGINARY:
  case TOKEN_TYPE_KEYWORD_STRUCT:
  case TOKEN_TYPE_KEYWORD_UNION:
  case TOKEN_TYPE_KEYWORD_ENUM:
    error_code = clCompilerFrontendParseDeclaration(frontend, &internal_container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    break;
    
  case TOKEN_TYPE_IDENTIFIER:
    if (0) {
      ASSERT(0 && "TODO: check if this identifier is a user-defined type");
      error_code = clCompilerFrontendParseDeclaration(frontend, &internal_container);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
    } else {
      error_code = clCompilerFrontendParseStatement(frontend, &internal_container);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
    }
    break;
    
  default:
    error_code = clCompilerFrontendParseStatement(frontend, &internal_container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    break;
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  return error_code;
}

/*
 * compound_statement
 *   : '{' '}'
 *   | '{' block_item+ '}'
 *   ;
 */
static cl_compiler_error_code
clCompilerFrontendParseCompoundStatement(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  
  token_type_pair.token_type = TOKEN_TYPE_LEFT_BRACE;
  error_code = clCompilerFrontendEnsureNextUnparsedToken(frontend, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  if (TOKEN_TYPE_RIGHT_BRACE == token_type_pair.token_type) {
    error_code = clCompilerFrontendConsumeNextUnparsedToken(frontend);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
  } else {
    cl_bool finish_parsing_block_item = CL_FALSE;
    
    error_code = clCompilerFrontendParseBlockItem(frontend, &internal_container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    
    while (CL_FALSE == finish_parsing_block_item) {
      error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      
      if (TOKEN_TYPE_RIGHT_BRACE == token_type_pair.token_type) {
        finish_parsing_block_item = CL_TRUE;
      } else {
        error_code = clCompilerFrontendParseBlockItem(frontend, &internal_container);
        if (error_code != COMPILER_NO_ERROR) {
          goto finish;
        }
      }
    }
    
    token_type_pair.token_type = TOKEN_TYPE_RIGHT_BRACE;
    error_code = clCompilerFrontendEnsureNextUnparsedToken(frontend, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  return error_code;
}

/*
 * function_definition
 *   : declaration_specifiers declarator
 *     ( declaration+ compound_statement | compound_statement )
 *   ;
 *
 * <Note 1>
 * In C99, 'declaration_specifiers' is MUST, but in C89, 'declaration_specifiers'
 * is optional.
 * Ex: func(int a) { ... } is valid in C89, but is invalide in C99.
 *     in C99, the correct form is, for example, 'int func(int a)'.
 *
 * <Note 2>
 * The 'declaration+' part is for K&R style syntax.
 * Ex:
 * extern int max(a, b)
 * int a, b;    <----- so-called 'declaration+' part
 * {
 *   ...
 * }
 */
static cl_compiler_error_code
clCompilerFrontendParseFunctionDefinition(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  _cl_compiler_token_type_pair token_type_pair;
  
  error_code = clCompilerFrontendParseDeclarationSpecifiers(frontend, &internal_container);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  error_code = clCompilerFrontendParseDeclarator(frontend, &internal_container);
  if (error_code != COMPILER_NO_ERROR) {
    goto finish;
  }
  
  {
    cl_bool finish_parsing_declaration = CL_FALSE;
    
    while (CL_FALSE == finish_parsing_declaration) {
      error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      if (token_type_pair.token_type != TOKEN_TYPE_LEFT_BRACE) {
        error_code = clCompilerFrontendParseDeclaration(frontend, &internal_container);
        if (error_code != COMPILER_NO_ERROR) {
          goto finish;
        }
      } else {
        finish_parsing_declaration = CL_TRUE;
      }
    }
    
    error_code = clCompilerFrontendParseCompoundStatement(frontend, &internal_container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  return error_code;
}

/*
 * external_declaration
 *   : ( declaration_specifiers declarator declaration* '{' )=> function_definition
 *   | declaration
 *   ;
 *
 * I use the so-called 'syntactic predicate'.
 */
static cl_compiler_error_code
clCompilerFrontendParseExternalDeclaration(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_int alternative = 1;
  _cl_compiler_token_type_pair token_type_pair;
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  
  /* syntactic predicate */
  {
    /* start backtracking mechanism */
    clCompilerFrontendEnterBackTracking(frontend);
    
    error_code = clCompilerFrontendParseDeclarationSpecifiers(frontend, NULL);
    if (error_code != COMPILER_NO_ERROR) {
      alternative = 2;
      goto find_alternative;
    }
    error_code = clCompilerFrontendParseDeclarator(frontend, NULL);
    if (error_code != COMPILER_NO_ERROR) {
      alternative = 2;
      goto find_alternative;
    }
    error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      alternative = 2;
      goto find_alternative;
    }
    for (;;) {
      if (TOKEN_TYPE_LEFT_BRACE == token_type_pair.token_type) {
        alternative = 1;
        goto find_alternative;
      } else {
        error_code = clCompilerFrontendParseDeclaration(frontend, NULL);
        if (error_code != COMPILER_NO_ERROR) {
          alternative = 2;
          goto find_alternative;
        }
      }
    }
    
  find_alternative:
    /* stop backtracking mechanism */
    clCompilerFrontendLeaveBackTracking(frontend);
  }
  
  switch (alternative) {
  case 1:
    error_code = clCompilerFrontendParseFunctionDefinition(frontend, container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    break;
  case 2:
    error_code = clCompilerFrontendParseDeclaration(frontend, container);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    break;
  default:
    ASSERT(0 && "Should not reach here");
    break;
  }
  
 finish:
  return error_code;
}

/*
 * translation_unit
 *   : external_declaration+
 *   ;
 */
cl_compiler_error_code
clCompilerFrontendParseTranslationUnit(
    cl_compiler_frontend frontend,
    CLIST_TYPE(cl_compiler_ast_node) container) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  _cl_compiler_token_type_pair token_type_pair;
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) external_declaration = {0};
  CLIST_CONCRETE_TYPE(cl_compiler_ast_node) internal_container = {0};
  
  ASSERT(frontend != NULL);
  ASSERT(container != NULL);
  
  for (;;) {
    error_code = clCompilerFrontendPeekNextUnparsedToken(frontend, 0, &token_type_pair);
    if (error_code != COMPILER_NO_ERROR) {
      goto finish;
    }
    if (token_type_pair.token_type != TOKEN_TYPE_EOF) {
      error_code = clCompilerFrontendParseExternalDeclaration(frontend, &external_declaration);
      if (error_code != COMPILER_NO_ERROR) {
        goto finish;
      }
      CLIST_SPLICE(cl_compiler_ast_node)(&internal_container, NULL, &external_declaration);
    } else {
      break;
    }
  }
  if (CL_FALSE == clCompilerFrontendIsInBackTracking(frontend)) {
    CLIST_SPLICE(cl_compiler_ast_node)(container, NULL, &internal_container);
  }
  return COMPILER_NO_ERROR;
  
 finish:
  clCompilerFrontendAstNodeFreeChildren(&internal_container);
  return error_code;
}
