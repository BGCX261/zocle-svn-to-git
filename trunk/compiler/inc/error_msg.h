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
#ifndef ZOCLE_COMPILER_ERROR_MSG_H_
#define ZOCLE_COMPILER_ERROR_MSG_H_

#include <compiler/inc/compiler.h>
#include <compiler_frontend/inc/frontend.h>

extern void error(cl_compiler_frontend frontend, char const *fmt, ...);
extern void expect(cl_compiler_frontend frontend, char const *msg);
extern void warning(cl_compiler_frontend frontend, char const *fmt, ...);

#endif
