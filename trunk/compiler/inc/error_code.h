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
#ifndef ZOCLE_COMPILER_ERROR_CODE_H_
#define ZOCLE_COMPILER_ERROR_CODE_H_

enum cl_compiler_error_code {
  COMPILER_NO_ERROR = 0,
  
  /* command line */
  COMPILER_ERROR__COMMAND_LINE__ERROR_START,
  
  COMPILER_ERROR__COMMAND_LINE__INVALID_OPTION,
  COMPILER_ERROR__COMMAND_LINE__MISSING_OPTION_ARGUMENT,
  COMPILER_ERROR__COMMAND_LINE__ENCOUNTER_HELP_OPTION,
  COMPILER_ERROR__COMMAND_LINE__FILE_DOES_NOT_EXIST,
  COMPILER_ERROR__COMMAND_LINE__UNSUPPORTED_SOURCE_FILE_TYPE,
  COMPILER_ERROR__COMMAND_LINE__DOES_NOT_SPECIFY_SOURCE_FILES_TO_COMPILE,
  
  COMPILER_ERROR__COMMAND_LINE__ERROR_END,
  
  /* preprocessor */
  COMPILER_ERROR__PREPROCESSOR__START,
  
  COMPILER_ERROR__PREPROCESSOR__ENCOUNTER_ERROR_DIRECTIVE,
  COMPILER_ERROR__PREPROCESSOR__PARSE_PRIMARY_EXPRESSION_FAILED,
  COMPILER_ERROR__PREPROCESSOR__IF_DIRECTIVE_PARAMETER_IS_NOT_INTEGRAL_CONSTANT_TYPE,
  
  COMPILER_ERROR__PREPROCESSOR__END,
  
  /* parsing */
  COMPILER_ERROR__PARSING__START,
  
  COMPILER_ERROR__PARSING__ERROR,
  COMPILER_ERROR__PARSING__UNALLOWED_OPERATOR_IN_CONSTANT_EXPRESSION,
  
  COMPILER_ERROR__PARSING__END
};
typedef enum cl_compiler_error_code cl_compiler_error_code;

#endif
