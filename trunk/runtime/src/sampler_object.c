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

CL_API_ENTRY cl_sampler CL_API_CALL
clCreateSampler(cl_context          context,
                cl_bool             normalized_coords, 
                cl_addressing_mode  addressing_mode, 
                cl_filter_mode      filter_mode,
                cl_int *            errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_int return_code = CL_SUCCESS;
  cl_sampler sampler = NULL;
  cl_bool find_device_supports_image = CL_FALSE;
  cl_uint i;
  
  if (NULL == context) {
    return_code = CL_INVALID_CONTEXT;
    goto error;
  }
  switch (addressing_mode) {
  case CL_ADDRESS_REPEAT:
  case CL_ADDRESS_CLAMP_TO_EDGE:
  case CL_ADDRESS_CLAMP:
  case CL_ADDRESS_NONE:
    break;
  default:
    return_code = CL_INVALID_VALUE;
    goto error;
  }
  switch (filter_mode) {
  case CL_FILTER_NEAREST:
  case CL_FILTER_LINEAR:
    break;
  default:
    return_code = CL_INVALID_VALUE;
    goto error;
  }
  switch (normalized_coords) {
  case CL_TRUE:
  case CL_FALSE:
    break;
  default:
    return_code = CL_INVALID_VALUE;
    goto error;
  }
  for (i = (context->num_devices - 1); i >= 0; --i) {
    cl_bool image_support;
    cl_int result;
    
    result = clGetDeviceInfo(context->devices[i],
                             CL_DEVICE_IMAGE_SUPPORT,
                             sizeof(image_support),
                             &image_support,
                             NULL);
    ASSERT(CL_SUCCESS == result);
    if (CL_FALSE == image_support) {
      continue;
    } else {
      find_device_supports_image = CL_TRUE;
    }
  }
  if (CL_FALSE == find_device_supports_image) {
    return_code = CL_INVALID_OPERATION;
    goto error;
  }
  sampler = CL_OSAL_CALLOC(sizeof(struct _cl_sampler));
  if (NULL == sampler) {
    return_code = CL_OUT_OF_HOST_MEMORY;
    goto error;
  }
  sampler->addressing_mode = addressing_mode;
  sampler->filter_mode = filter_mode;
  sampler->context = context;
  
  clRetainSampler(sampler);
  goto success;
  
 error:
  if (sampler != NULL) {
    CL_OSAL_FREE(sampler);
    sampler = NULL;
  }
  
 success:
  if (errcode_ret != NULL) {
    (*errcode_ret) = return_code;
  }
  return sampler;
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainSampler(cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == sampler) {
    return CL_INVALID_SAMPLER;
  }
  ++sampler->ref_count;
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseSampler(cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == sampler) {
    return CL_INVALID_SAMPLER;
  }
  --sampler->ref_count;
  if (0 == sampler->ref_count) {
    /* TODO: wait commands queued for execution on a command-queue(s) that
     * use this sampler have finished, the sampler object is deleted.
     */
  }
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetSamplerInfo(cl_sampler         sampler,
                 cl_sampler_info    param_name,
                 size_t             param_value_size,
                 void *             param_value,
                 size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == sampler) {
    return CL_INVALID_SAMPLER;
  }
  switch (param_name) {
  case CL_SAMPLER_REFERENCE_COUNT:
  case CL_SAMPLER_CONTEXT:
  case CL_SAMPLER_ADDRESSING_MODE:
  case CL_SAMPLER_FILTER_MODE:
  case CL_SAMPLER_NORMALIZED_COORDS:
    /* TODO: handle them. */
    break;
    
  default:
    return CL_INVALID_VALUE;
  }
  return CL_SUCCESS;
}
