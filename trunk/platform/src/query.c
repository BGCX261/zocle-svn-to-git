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

#include <string.h>

#define ZOCLE_OPENCL_VERSION "OpenCL 1.0 (ZOCLE)"

CL_API_ENTRY cl_int CL_API_CALL
clGetPlatformInfo(cl_platform_info param_name,
                  size_t           param_value_size,
                  void *           param_value,
                  size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  switch (param_name) {
  case CL_PLATFORM_PROFILE:
    if (param_value != NULL) {
      if (param_value_size < sizeof("FULL_PROFILE")) {
        return CL_INVALID_VALUE;
      } else {
        memcpy(param_value, "FULL_PROFILE", sizeof("FULL_PROFILE"));
      }
    }
    if (param_value_size_ret != NULL) {
      (*param_value_size_ret) = sizeof("FULL_PROFILE");
    }
    break;
    
  case CL_PLATFORM_VERSION:
    if (param_value != NULL) {
      if (param_value_size < sizeof(ZOCLE_OPENCL_VERSION)) {
        return CL_INVALID_VALUE;
      } else {
        memcpy(param_value, ZOCLE_OPENCL_VERSION, sizeof(ZOCLE_OPENCL_VERSION));
      }
    }
    if (param_value_size_ret != NULL) {
      (*param_value_size_ret) = sizeof(ZOCLE_OPENCL_VERSION);
    }
    break;
    
  default:
    return CL_INVALID_VALUE;
  }
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceIDs(cl_device_type   device_type, 
               cl_uint          num_entries, 
               cl_device_id *   devices, 
               cl_uint *        num_devices) CL_API_SUFFIX__VERSION_1_0 {
  cl_uint find_num_devices = 0;
  
  if (NULL == devices) {
    if (NULL == num_devices) {
      return CL_INVALID_VALUE;
    }
  } else {
    if (0 == num_entries) {
      return CL_INVALID_VALUE;
    }
  }
  
  if (CL_DEVICE_TYPE_ALL == (device_type & CL_DEVICE_TYPE_ALL)) {
    device_type &= (~CL_DEVICE_TYPE_DEFAULT);
  }
  if (device_type & CL_DEVICE_TYPE_DEFAULT) {
    device_type &= (~CL_DEVICE_TYPE_DEFAULT);
  }
  if (device_type & CL_DEVICE_TYPE_CPU) {
    device_type &= (~CL_DEVICE_TYPE_CPU);
  }
  if (device_type & CL_DEVICE_TYPE_GPU) {
    device_type &= (~CL_DEVICE_TYPE_GPU);
  }
  if (device_type & CL_DEVICE_TYPE_ACCELERATOR) {
    device_type &= (~CL_DEVICE_TYPE_ACCELERATOR);
  }
  
  if (device_type != 0) {
    return CL_INVALID_DEVICE_TYPE;
  }
  
  if (num_devices != NULL) {
    (*num_devices) = find_num_devices;
  }
  if (0 == find_num_devices) {
    return CL_DEVICE_NOT_FOUND;
  }
  
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceInfo(cl_device_id    device,
                cl_device_info  param_name, 
                size_t          param_value_size, 
                void *          param_value,
                size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  switch (param_name) {
  case CL_DEVICE_TYPE:
  case CL_DEVICE_VENDOR_ID:
  case CL_DEVICE_MAX_COMPUTE_UNITS:
  case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
  case CL_DEVICE_MAX_WORK_ITEM_SIZES:
  case CL_DEVICE_MAX_WORK_GROUP_SIZE:
  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:
  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
  case CL_DEVICE_MAX_CLOCK_FREQUENCY:
  case CL_DEVICE_ADDRESS_BITS:
  case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
  case CL_DEVICE_IMAGE_SUPPORT:
  case CL_DEVICE_MAX_READ_IMAGE_ARGS:
  case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:
  case CL_DEVICE_IMAGE2D_MAX_WIDTH:
  case CL_DEVICE_IMAGE2D_MAX_HEIGHT:
  case CL_DEVICE_IMAGE3D_MAX_WIDTH:
  case CL_DEVICE_IMAGE3D_MAX_HEIGHT:
  case CL_DEVICE_IMAGE3D_MAX_DEPTH:
  case CL_DEVICE_MAX_SAMPLERS:
  case CL_DEVICE_MAX_PARAMETER_SIZE:
  case CL_DEVICE_MEM_BASE_ADDR_ALIGN:
  case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
  case CL_DEVICE_SINGLE_FP_CONFIG:
  case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
  case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
  case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:
  case CL_DEVICE_GLOBAL_MEM_SIZE:
  case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:
  case CL_DEVICE_MAX_CONSTANT_ARGS:
  case CL_DEVICE_LOCAL_MEM_TYPE:
  case CL_DEVICE_LOCAL_MEM_SIZE:
  case CL_DEVICE_ERROR_CORRECTION_SUPPORT:
  case CL_DEVICE_PROFILING_TIMER_RESOLUTION:
  case CL_DEVICE_ENDIAN_LITTLE:
  case CL_DEVICE_AVAILABLE:
  case CL_DEVICE_COMPILER_AVAILABLE:
  case CL_DEVICE_EXECUTION_CAPABILITIES:
  case CL_DEVICE_QUEUE_PROPERTIES:
  case CL_DEVICE_NAME:
  case CL_DEVICE_VENDOR:
  case CL_DRIVER_VERSION:
  case CL_DEVICE_PROFILE:
  case CL_DEVICE_VERSION:
  case CL_DEVICE_EXTENSIONS:
    break;
    
  default:
    return CL_INVALID_DEVICE;
  }
  return CL_SUCCESS;
}
