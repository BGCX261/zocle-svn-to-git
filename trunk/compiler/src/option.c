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

#include <compiler/inc/compiler.h>
#include <compiler/inc/error_msg.h>
#include <compiler_frontend/inc/frontend.h>

#define CL_COMMAND_LINE_OPTION_HAS_ARG (1 << 0)
#define CL_COMMAND_LINE_OPTION_NOSEP   (1 << 1) /**< cannot have space before option and arg */

struct _cl_command_line_option {
  char const *name;
  cl_int index;
  cl_int flags;
};
typedef struct _cl_command_line_option const *cl_command_line_option;

enum {
  CL_COMMAND_LINE_OPTION_HELP,
  CL_COMMAND_LINE_OPTION_I,
  CL_COMMAND_LINE_OPTION_D,
  CL_COMMAND_LINE_OPTION_U,
  CL_COMMAND_LINE_OPTION_v,
  CL_COMMAND_LINE_OPTION_E,
};

static const struct _cl_command_line_option options[] = {
  { "h", CL_COMMAND_LINE_OPTION_HELP, 0 },
  { "?", CL_COMMAND_LINE_OPTION_HELP, 0 },
  { "I", CL_COMMAND_LINE_OPTION_I, CL_COMMAND_LINE_OPTION_HAS_ARG },
  { "D", CL_COMMAND_LINE_OPTION_D, CL_COMMAND_LINE_OPTION_HAS_ARG },
  { "U", CL_COMMAND_LINE_OPTION_U, CL_COMMAND_LINE_OPTION_HAS_ARG },
  { "v", CL_COMMAND_LINE_OPTION_v, CL_COMMAND_LINE_OPTION_HAS_ARG | CL_COMMAND_LINE_OPTION_NOSEP },
  { "E", CL_COMMAND_LINE_OPTION_E, 0},
  { NULL },
};

cl_compiler_error_code
clCompilerParseArgs(cl_compiler compiler, int argc, char **argv) {
  cl_compiler_error_code error_code = COMPILER_NO_ERROR;
  int curr_arg_idx;
  
  ASSERT(compiler != NULL);
  
  curr_arg_idx = 1;
  while (curr_arg_idx < argc) {
    char *arg_string = argv[curr_arg_idx];
    ++curr_arg_idx;
    
    if (arg_string[0] != '-') {
      /* add a new file */
      clCompilerAddFileToCompile(compiler, arg_string);
    } else {
      char *optarg;
      
      /* find option in table (match only the first chars */
      cl_command_line_option popt = options;
      char *arg_string_ptr = NULL;
      for (;;) {
        cl_bool found = CL_FALSE;
        char const *option_name_ptr = popt->name;
        if (NULL == option_name_ptr) {
          error(compiler->frontend, "invalid option -- '%s'", arg_string);
          error_code = COMPILER_ERROR__COMMAND_LINE__INVALID_OPTION;
          goto finish;
        }
        arg_string_ptr = arg_string + 1;
        for (;;) {
          if ('\0' == *option_name_ptr) {
            found = CL_TRUE;
            break;
          }
          if (*arg_string_ptr != *option_name_ptr) {
            break;
          }
          ++arg_string_ptr;
          ++option_name_ptr;
        }
        if (CL_TRUE == found) {
          break;
        }
        ++popt;
      }
      
      if (popt->flags & CL_COMMAND_LINE_OPTION_HAS_ARG) {
        if ((*arg_string_ptr != '\0') || (popt->flags & CL_COMMAND_LINE_OPTION_NOSEP)) {
          optarg = arg_string_ptr;
        } else {
          if (curr_arg_idx >= argc) {
            error(compiler->frontend, "argument to '%s' is missing", arg_string);
            error_code = COMPILER_ERROR__COMMAND_LINE__MISSING_OPTION_ARGUMENT;
            goto finish;
          }
          optarg = argv[curr_arg_idx];
          ++curr_arg_idx;
        }
      } else {
        if (*arg_string_ptr != '\0') {
          error_code = COMPILER_ERROR__COMMAND_LINE__INVALID_OPTION;
          goto finish;
        }
        optarg = NULL;
      }
      
      switch (popt->index) {
      case CL_COMMAND_LINE_OPTION_HELP:
        error_code = COMPILER_ERROR__COMMAND_LINE__ENCOUNTER_HELP_OPTION;
        goto finish;
        
      case CL_COMMAND_LINE_OPTION_I:
        if (NULL == compiler->cmd_line_option.include_paths) {
          compiler->cmd_line_option.include_paths = CLIST_NEW(char_ptr)();
        }
        CLIST_PUSH_BACK(char_ptr)(compiler->cmd_line_option.include_paths, &optarg);
        break;
        
      case CL_COMMAND_LINE_OPTION_D:
      {
        char *symbol, *value;
        symbol = (char *)optarg;
        value = strchr(symbol, '=');
        if (value != NULL) {
          *value = '\0';
          ++value;
        }
        if (NULL == compiler->cmd_line_option.define_undefine_symbol) {
          compiler->cmd_line_option.define_undefine_symbol =
            CLIST_NEW(_cl_compiler_cmd_line_define_undefine_symbol)();
        }
        {
          _cl_compiler_cmd_line_define_undefine_symbol symbol_info = {
            CL_TRUE, symbol, value };
          CLIST_PUSH_BACK(_cl_compiler_cmd_line_define_undefine_symbol)(
              compiler->cmd_line_option.define_undefine_symbol, &symbol_info);
        }
      }
      break;
      
      case CL_COMMAND_LINE_OPTION_U:
        if (NULL == compiler->cmd_line_option.define_undefine_symbol) {
          compiler->cmd_line_option.define_undefine_symbol =
            CLIST_NEW(_cl_compiler_cmd_line_define_undefine_symbol)();
        }
        {
          _cl_compiler_cmd_line_define_undefine_symbol symbol_info = {
            CL_FALSE, optarg, NULL };
          CLIST_PUSH_BACK(_cl_compiler_cmd_line_define_undefine_symbol)(
              compiler->cmd_line_option.define_undefine_symbol, &symbol_info);
        }
        break;
        
      case CL_COMMAND_LINE_OPTION_v:
        printf("zcc version %s\n", ZCC_VERSION);
        break;
        
      case CL_COMMAND_LINE_OPTION_E:
        clCompilerSetOutputType(compiler, OUTPUT_TYPE_PREPROCESSOR_ONLY);
        break;
        
      default:
        warning(compiler->frontend, "unsupported option '%s'", arg_string);
        break;
      }
    }
  }
  
 finish:
  return error_code;
}
