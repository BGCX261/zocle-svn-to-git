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

#include <osal/inc/osal.h>
#include <cl_internal.h>

CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithSource(cl_context        context,
                          cl_uint           count,
                          const char **     strings,
                          const size_t *    lengths,
                          cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_int return_code = CL_SUCCESS;
  cl_program program = NULL;
  cl_bool find_device_supports_compiler = CL_FALSE;
  cl_uint i;
  
  if (NULL == context) {
    return_code = CL_INVALID_CONTEXT;
    goto error;
  }
  if (0 == count) {
    return_code = CL_INVALID_VALUE;
    goto error;
  }
  if (NULL == strings) {
    return_code = CL_INVALID_VALUE;
    goto error;
  }
  for (i = (count - 1); i >= 0; --i) {
    if (NULL == strings[i]) {
      return_code = CL_INVALID_VALUE;
      goto error;
    }
  }
  for (i = (context->num_devices - 1); i >= 0; --i) {
    cl_bool compiler_support;
    cl_int result;
    
    result = clGetDeviceInfo(context->devices[i],
                             CL_DEVICE_COMPILER_AVAILABLE,
                             sizeof(compiler_support),
                             &compiler_support,
                             NULL);
    ASSERT(CL_SUCCESS == result);
    if (CL_FALSE == compiler_support) {
      continue;
    } else {
      find_device_supports_compiler = CL_TRUE;
    }
  }
  if (CL_FALSE == find_device_supports_compiler) {
    return_code = CL_INVALID_OPERATION;
    goto error;
  }
  program = CL_OSAL_CALLOC(sizeof(struct _cl_program));
  if (NULL == program) {
    return_code = CL_OUT_OF_HOST_MEMORY;
    goto error;
  }
  program->context = context;
  
  clRetainProgram(program);
  goto success;
  
 error:
  if (program != NULL) {
    CL_OSAL_FREE(program);
    program = NULL;
  }
  
 success:
  if (errcode_ret != NULL) {
    (*errcode_ret) = return_code;
  }
  return program;
}

CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithBinary(cl_context            context,
                          cl_uint               num_devices,
                          const cl_device_id *  device_list,
                          const size_t *        lengths,
                          const char **         binaries,
                          cl_int *              binary_status,
                          cl_int *              errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_int return_code = CL_SUCCESS;
  cl_program program = NULL;
  cl_uint i;
  
  if (NULL == context) {
    return_code = CL_INVALID_CONTEXT;
    goto error;
  }
  if ((0 == num_devices) || (NULL == device_list)) {
    return_code = CL_INVALID_VALUE;
    goto error;
  }
  if ((NULL == lengths) || (NULL == binaries)) {
    return_code = CL_INVALID_VALUE;
    goto error;
  }
  for (i = (context->num_devices - 1); i >= 0; --i) {
    if (0 == lengths[i]) {
      return_code = CL_INVALID_VALUE;
      goto error;
    }
    if (NULL == binaries[i]) {
      return_code = CL_INVALID_VALUE;
      goto error;
    }
    if (device_list[i]->context != context) {
      return_code = CL_INVALID_DEVICE;
      goto error;
    }
  }
  /* TODO: handle CL_INVALID_BINARY */
  program = CL_OSAL_CALLOC(sizeof(struct _cl_program));
  if (NULL == program) {
    return_code = CL_OUT_OF_HOST_MEMORY;
    goto error;
  }
  program->context = context;
  
  clRetainProgram(program);
  goto success;
  
 error:
  if (program != NULL) {
    CL_OSAL_FREE(program);
    program = NULL;
  }
  
 success:
  if (errcode_ret != NULL) {
    (*errcode_ret) = return_code;
  }
  return program;
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainProgram(cl_program program) CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == program) {
    return CL_INVALID_PROGRAM;
  }
  ++program->ref_count;
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseProgram(cl_program program) CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == program) {
    return CL_INVALID_PROGRAM;
  }
  --program->ref_count;
  if (0 == program->ref_count) {
    /* TODO: all kernel objects associated with program have been deleted. */
  }
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clBuildProgram(cl_program           program,
               cl_uint              num_devices,
               const cl_device_id * device_list,
               const char *         options, 
               void (*pfn_notify)(cl_program program, void * user_data),
               void *               user_data) CL_API_SUFFIX__VERSION_1_0 {
  cl_int return_code = CL_SUCCESS;
  
  if (NULL == program) {
    return_code = CL_INVALID_PROGRAM;
    goto error;
  }
  /* TODO: implement this function. */
 error:
  return return_code;
}

CL_API_ENTRY cl_int CL_API_CALL
clUnloadCompiler(void) CL_API_SUFFIX__VERSION_1_0 {
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetProgramInfo(cl_program         program,
                 cl_program_info    param_name,
                 size_t             param_value_size,
                 void *             param_value,
                 size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == program) {
    return CL_INVALID_PROGRAM;
  }
  switch (param_name) {
  case CL_PROGRAM_REFERENCE_COUNT:
  case CL_PROGRAM_CONTEXT:
  case CL_PROGRAM_NUM_DEVICES:
  case CL_PROGRAM_SOURCE:
  case CL_PROGRAM_BINARY_SIZES:
  case CL_PROGRAM_BINARIES:
    /* TODO: handle them. */
    break;
    
  default:
    return CL_INVALID_VALUE;
  }
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetProgramBuildInfo(cl_program            program,
                      cl_device_id          device,
                      cl_program_build_info param_name,
                      size_t                param_value_size,
                      void *                param_value,
                      size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == program) {
    return CL_INVALID_PROGRAM;
  }
  /* TODO: handle CL_INVALID_DEVICE */
  switch (param_name) {
  case CL_PROGRAM_BUILD_STATUS:
  case CL_PROGRAM_BUILD_OPTIONS:
  case CL_PROGRAM_BUILD_LOG:
    /* TODO: handle them. */
    break;
    
  default:
    return CL_INVALID_VALUE;
  }
  return CL_SUCCESS;
}
