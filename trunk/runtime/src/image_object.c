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

#ifndef DEFINE_CVECTOR_TYPE_FOR_CL_COMMAND
#define DEFINE_CVECTOR_TYPE_FOR_CL_COMMAND
DEFINE_CVECTOR_TYPE(cl_command)
#endif

static struct _cl_image_format clSupportedImageFormatForReadOnly[] = {
  {CL_RGBA, CL_UNORM_INT8},
  {CL_RGBA, CL_UNORM_INT16},
  {CL_RGBA, CL_SIGNED_INT8},
  {CL_RGBA, CL_SIGNED_INT16},
  {CL_RGBA, CL_SIGNED_INT32},
  {CL_RGBA, CL_UNSIGNED_INT8},
  {CL_RGBA, CL_UNSIGNED_INT16},
  {CL_RGBA, CL_UNSIGNED_INT32},
  {CL_RGBA, CL_HALF_FLOAT},
  {CL_RGBA, CL_FLOAT},
  {CL_BGRA, CL_UNORM_INT8}
};

static struct _cl_image_format clSupportedImageFormatForReadWriteAndWriteOnly[] = {
  {CL_RGBA, CL_UNORM_INT8},
  {CL_RGBA, CL_UNORM_INT16},
  {CL_RGBA, CL_SIGNED_INT8},
  {CL_RGBA, CL_SIGNED_INT16},
  {CL_RGBA, CL_SIGNED_INT32},
  {CL_RGBA, CL_UNSIGNED_INT8},
  {CL_RGBA, CL_UNSIGNED_INT16},
  {CL_RGBA, CL_UNSIGNED_INT32},
  {CL_RGBA, CL_HALF_FLOAT},
  {CL_RGBA, CL_FLOAT},
  {CL_BGRA, CL_UNORM_INT8}
};

static cl_uint
countNumberOfBitOne(cl_uint value) {
  value = ((value >>  1) & 0x55555555) + (value & 0x55555555);
  value = ((value >>  2) & 0x33333333) + (value & 0x33333333);
  value = ((value >>  4) & 0x0F0F0F0F) + (value & 0x0F0F0F0F);
  value = ((value >>  8) & 0x00FF00FF) + (value & 0x00FF00FF);
  value = ((value >> 16) & 0x0000FFFF) + (value & 0x0000FFFF);
  
  return value;
}

static cl_int
clCheckAndDetermineImageElementSize(cl_image_format const * image_format,
                                    cl_uint * element_size_ret) {
  cl_uint element_size = 0;
  ASSERT(image_format != NULL);
  ASSERT(element_size_ret != NULL);
  switch (image_format->image_channel_order) {
  case CL_R: element_size = 1; break;
  case CL_A: element_size = 1; break;
  case CL_RG: element_size = 2; break;
  case CL_RA: element_size = 2; break;
    
  case CL_RGB:
    switch (image_format->image_channel_data_type) {
    case CL_UNORM_SHORT_565:
    case CL_UNORM_SHORT_555:
    case CL_UNORM_INT_101010:
      break;
    case CL_SNORM_INT8:
    case CL_SNORM_INT16:
    case CL_UNORM_INT8:
    case CL_UNORM_INT16:
    case CL_SIGNED_INT8:
    case CL_SIGNED_INT16:
    case CL_SIGNED_INT32:
    case CL_UNSIGNED_INT8:
    case CL_UNSIGNED_INT16:
    case CL_UNSIGNED_INT32:
    case CL_HALF_FLOAT:
    case CL_FLOAT:
      return CL_IMAGE_FORMAT_NOT_SUPPORTED;
    default:
      return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    }
    element_size = 3;
    break;
    
  case CL_RGBA: element_size = 4; break;
    
  case CL_ARGB:
  case CL_BGRA:
    switch (image_format->image_channel_data_type) {
    case CL_SNORM_INT8:
    case CL_UNORM_INT8:
    case CL_SIGNED_INT8:
    case CL_UNSIGNED_INT8:
      break;
    case CL_UNORM_SHORT_565:
    case CL_UNORM_SHORT_555:
    case CL_UNORM_INT_101010:
    case CL_SNORM_INT16:
    case CL_UNORM_INT16:
    case CL_SIGNED_INT16:
    case CL_SIGNED_INT32:
    case CL_UNSIGNED_INT16:
    case CL_UNSIGNED_INT32:
    case CL_HALF_FLOAT:
    case CL_FLOAT:
      return CL_IMAGE_FORMAT_NOT_SUPPORTED;
    default:
      return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    }
    element_size = 4;
    break;
  default:
    return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
  }
  ASSERT(element_size != 0);
  switch (image_format->image_channel_data_type) {
  case CL_SNORM_INT8: element_size *= 1; break;
  case CL_SNORM_INT16: element_size *= 2; break;
  case CL_UNORM_INT8: element_size *= 1; break;
  case CL_UNORM_INT16: element_size *= 2; break;
    
  case CL_UNORM_SHORT_565:
  case CL_UNORM_SHORT_555:
    switch (image_format->image_channel_order) {
    case CL_RGB:
      break;
    case CL_R:
    case CL_A:
    case CL_RG:
    case CL_RA:
    case CL_RGBA:
    case CL_ARGB:
    case CL_BGRA:
      return CL_IMAGE_FORMAT_NOT_SUPPORTED;
    default:
      return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    }
    element_size = 2;
    break;
    
  case CL_UNORM_INT_101010:
    switch (image_format->image_channel_order) {
    case CL_RGB:
      break;
    case CL_R:
    case CL_A:
    case CL_RG:
    case CL_RA:
    case CL_RGBA:
    case CL_ARGB:
    case CL_BGRA:
      return CL_IMAGE_FORMAT_NOT_SUPPORTED;
    default:
      return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    }
    element_size = 4;
    break;
    
  case CL_SIGNED_INT8: element_size *= 1; break;
  case CL_SIGNED_INT16: element_size *= 2; break;
  case CL_SIGNED_INT32: element_size *= 4; break;
  case CL_UNSIGNED_INT8: element_size *= 1; break;
  case CL_UNSIGNED_INT16: element_size *= 2; break;
  case CL_UNSIGNED_INT32: element_size *= 4; break;
  case CL_HALF_FLOAT: element_size *= 2; break;
  case CL_FLOAT: element_size *= 4; break;
  default:
    return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
  }
  if (countNumberOfBitOne(element_size * 8) != 1) {
    /* spec: The number of bits per element determined by the
     * image_channel_data_type and image_channel_order must be a power of two.
     */
    return CL_IMAGE_FORMAT_NOT_SUPPORTED;
  }
  (*element_size_ret) = element_size;
  return CL_SUCCESS;
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateImage2D(cl_context              context,
                cl_mem_flags            flags,
                cl_image_format const * image_format,
                size_t                  image_width,
                size_t                  image_height,
                size_t                  image_row_pitch, 
                void *                  host_ptr,
                cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_mem mem = NULL;
  cl_int return_code = CL_SUCCESS;
  cl_bool find_device_supports_image = CL_FALSE;
  cl_int result;
  cl_uint element_size;
  size_t real_image_row_pitch = image_row_pitch;
  cl_uint i;
  
  if (NULL == context) {
    return_code = CL_INVALID_CONTEXT;
    goto error;
  }
  if ((flags & CL_MEM_COPY_HOST_PTR) && (flags & CL_MEM_USE_HOST_PTR)) {
    return_code = CL_INVALID_VALUE;
    goto error;
  }
  if ((0 == image_width) || (0 == image_height)) {
    return_code = CL_INVALID_IMAGE_SIZE;
    goto error;
  }
  if (NULL == image_format) {
    return_code = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    goto error;
  }
  result = clCheckAndDetermineImageElementSize(image_format, &element_size);
  if (result != CL_SUCCESS) {
    return_code = result;
    goto error;
  }
  if (countNumberOfBitOne(element_size) != 1) {
    /* spec: The size of each element in bytes must be a power of 2. */
    /* TODO: spec does not define which error code to return in this case. */
    return_code = CL_IMAGE_FORMAT_NOT_SUPPORTED;
    goto error;
  }
  if (NULL == host_ptr) {
    ASSERT(0 == image_row_pitch);
  } else {
    if (0 == image_row_pitch) {
      real_image_row_pitch = image_width * element_size;
    } else {
      real_image_row_pitch = image_row_pitch;
      if ((real_image_row_pitch % element_size) != 0) {
        return_code = CL_INVALID_IMAGE_SIZE;
        goto error;
      }
    }
  }
  for (i = (context->num_devices - 1); i >= 0; --i) {
    size_t image2d_max_width, image2d_max_height;
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
    result = clGetDeviceInfo(context->devices[i],
                             CL_DEVICE_IMAGE2D_MAX_WIDTH,
                             sizeof(image2d_max_width),
                             &image2d_max_width,
                             NULL);
    ASSERT(CL_SUCCESS == result);
    if (image_width > image2d_max_width) {
      return_code = CL_INVALID_IMAGE_SIZE;
      goto error;
    }
    result = clGetDeviceInfo(context->devices[i],
                             CL_DEVICE_IMAGE2D_MAX_HEIGHT,
                             sizeof(image2d_max_height),
                             &image2d_max_height,
                             NULL);
    ASSERT(CL_SUCCESS == result);
    if (image_height > image2d_max_height) {
      return_code = CL_INVALID_IMAGE_SIZE;
      goto error;
    }
  }
  if (CL_FALSE == find_device_supports_image) {
    return_code = CL_INVALID_OPERATION;
    goto error;
  }
  mem = CL_OSAL_CALLOC(sizeof(struct _cl_mem));
  if (NULL == mem) {
    return_code = CL_OUT_OF_HOST_MEMORY;
    goto error;
  }
  mem->type = CL_MEM_OBJECT_IMAGE2D;
  mem->mem_ptr = NULL;
  mem->flags = flags;
  /* Default to read/write */
  mem->access_read = CL_TRUE;
  mem->access_write = CL_TRUE;
  
  clCheckMemoryObjectFlags(mem, flags, real_image_row_pitch * image_height, host_ptr, &return_code);
  if (return_code != CL_SUCCESS) {
    goto error;
  }
  
 error:
  if (mem != NULL) {
    CL_OSAL_FREE(mem);
    mem = NULL;
  }
  if (errcode_ret != NULL) {
    (*errcode_ret) = return_code;
  }
  return mem;
}

CL_API_ENTRY cl_mem CL_API_CALL
clCreateImage3D(cl_context              context,
                cl_mem_flags            flags,
                const cl_image_format * image_format,
                size_t                  image_width, 
                size_t                  image_height,
                size_t                  image_depth, 
                size_t                  image_row_pitch, 
                size_t                  image_slice_pitch, 
                void *                  host_ptr,
                cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_mem mem = NULL;
  cl_int return_code = CL_SUCCESS;
  cl_bool find_device_supports_image = CL_FALSE;
  cl_int result;
  cl_uint element_size;
  size_t real_image_row_pitch = image_row_pitch;
  size_t real_image_slice_pitch = image_slice_pitch;
  cl_uint i;
  
  if (NULL == context) {
    return_code = CL_INVALID_CONTEXT;
    goto error;
  }
  if ((flags & CL_MEM_COPY_HOST_PTR) && (flags & CL_MEM_USE_HOST_PTR)) {
    return_code = CL_INVALID_VALUE;
    goto error;
  }
  if ((0 == image_width) || (0 == image_height) || (0 == image_depth)) {
    return_code = CL_INVALID_IMAGE_SIZE;
    goto error;
  }
  /* TODO: spec does not define the error code for this situation. */
  ASSERT(image_depth > 1);
  if (NULL == image_format) {
    return_code = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    goto error;
  }
  result = clCheckAndDetermineImageElementSize(image_format, &element_size);
  if (result != CL_SUCCESS) {
    return_code = result;
    goto error;
  }
  if (countNumberOfBitOne(element_size) != 1) {
    /* spec: The size of each element in bytes must be a power of 2. */
    /* TODO: spec does not define which error code to return in this case. */
    return_code = CL_IMAGE_FORMAT_NOT_SUPPORTED;
    goto error;
  }
  if (NULL == host_ptr) {
    ASSERT(0 == image_row_pitch);
  } else {
    if (0 == image_row_pitch) {
      real_image_row_pitch = image_width * element_size;
    } else {
      real_image_row_pitch = image_row_pitch;
      if ((real_image_row_pitch % element_size) != 0) {
        return_code = CL_INVALID_IMAGE_SIZE;
        goto error;
      }
    }
  }
  if (NULL == host_ptr) {
    ASSERT(0 == image_slice_pitch);
  } else {
    if (0 == image_slice_pitch) {
      real_image_slice_pitch = image_row_pitch * image_height;
    } else {
      real_image_slice_pitch = image_slice_pitch;
      if ((real_image_slice_pitch % image_row_pitch) != 0) {
        return_code = CL_INVALID_IMAGE_SIZE;
        goto error;
      }
    }
  }
  for (i = (context->num_devices - 1); i >= 0; --i) {
    size_t image3d_max_width, image3d_max_height, image3d_max_depth;
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
    result = clGetDeviceInfo(context->devices[i],
                             CL_DEVICE_IMAGE3D_MAX_WIDTH,
                             sizeof(image3d_max_width),
                             &image3d_max_width,
                             NULL);
    ASSERT(CL_SUCCESS == result);
    if (image_width > image3d_max_width) {
      return_code = CL_INVALID_IMAGE_SIZE;
      goto error;
    }
    result = clGetDeviceInfo(context->devices[i],
                             CL_DEVICE_IMAGE3D_MAX_HEIGHT,
                             sizeof(image3d_max_height),
                             &image3d_max_height,
                             NULL);
    ASSERT(CL_SUCCESS == result);
    if (image_height > image3d_max_height) {
      return_code = CL_INVALID_IMAGE_SIZE;
      goto error;
    }
    result = clGetDeviceInfo(context->devices[i],
                             CL_DEVICE_IMAGE3D_MAX_DEPTH,
                             sizeof(image3d_max_depth),
                             &image3d_max_depth,
                             NULL);
    ASSERT(CL_SUCCESS == result);
    if (image_depth > image3d_max_depth) {
      return_code = CL_INVALID_IMAGE_SIZE;
      goto error;
    }
  }
  if (CL_FALSE == find_device_supports_image) {
    return_code = CL_INVALID_OPERATION;
    goto error;
  }
  mem = CL_OSAL_CALLOC(sizeof(struct _cl_mem));
  if (NULL == mem) {
    return_code = CL_OUT_OF_HOST_MEMORY;
    goto error;
  }
  mem->type = CL_MEM_OBJECT_IMAGE3D;
  mem->mem_ptr = NULL;
  mem->flags = flags;
  /* Default to read/write */
  mem->access_read = CL_TRUE;
  mem->access_write = CL_TRUE;
  
  clCheckMemoryObjectFlags(mem, flags, real_image_slice_pitch * image_depth, host_ptr, &return_code);
  if (return_code != CL_SUCCESS) {
    goto error;
  }
  
 error:
  if (mem != NULL) {
    CL_OSAL_FREE(mem);
    mem = NULL;
  }
  if (errcode_ret != NULL) {
    (*errcode_ret) = return_code;
  }
  return mem;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetSupportedImageFormats(cl_context           context,
                           cl_mem_flags         flags,
                           cl_mem_object_type   image_type,
                           cl_uint              num_entries,
                           cl_image_format *    image_formats,
                           cl_uint *            num_image_formats) CL_API_SUFFIX__VERSION_1_0 {  
  cl_bool access_read = CL_TRUE;
  cl_bool access_write = CL_TRUE;
  cl_uint real_num_image_formats = 0;
  
  if (NULL == context) {
    return CL_INVALID_CONTEXT;
  }
  if ((0 == num_entries) && (image_formats != NULL)) {
    return CL_INVALID_VALUE;
  }
  switch (image_type) {
  case CL_MEM_OBJECT_IMAGE2D:
  case CL_MEM_OBJECT_IMAGE3D:
    break;
  default:
    return CL_INVALID_VALUE;
  }
  if (flags & CL_MEM_READ_WRITE) {
    if ((flags & CL_MEM_READ_ONLY)  || (flags & CL_MEM_WRITE_ONLY)) {
      return CL_INVALID_VALUE;
    }
    access_read = CL_TRUE;
    access_write = CL_TRUE;
  }
  if (flags & CL_MEM_READ_ONLY) {
    if (flags & CL_MEM_WRITE_ONLY) {
      return CL_INVALID_VALUE;
    }
    access_read = CL_TRUE;
    access_write = CL_FALSE;
  }
  if (flags & CL_MEM_WRITE_ONLY) {
    access_read = CL_FALSE;
    access_write = CL_TRUE;
  }
  if ((CL_TRUE == access_read) && (CL_FALSE == access_write)) {
    cl_uint i = 0;
    /* read-only */
    while (i < (sizeof(clSupportedImageFormatForReadOnly) / sizeof(struct _cl_image_format))) {
      if ((image_formats != NULL) && (num_entries > real_num_image_formats)) {
        image_formats[real_num_image_formats].image_channel_order =
          clSupportedImageFormatForReadOnly[i].image_channel_order;
        image_formats[real_num_image_formats].image_channel_data_type =
          clSupportedImageFormatForReadOnly[i].image_channel_data_type;
      }
      ++i;
      ++real_num_image_formats;
    }
  } else {
    cl_uint i = 0;
    /* read-write or write-only */
    while (i < (sizeof(clSupportedImageFormatForReadWriteAndWriteOnly) / sizeof(struct _cl_image_format))) {
      if ((image_formats != NULL) && (num_entries > real_num_image_formats)) {
        image_formats[real_num_image_formats].image_channel_order =
          clSupportedImageFormatForReadWriteAndWriteOnly[i].image_channel_order;
        image_formats[real_num_image_formats].image_channel_data_type =
          clSupportedImageFormatForReadWriteAndWriteOnly[i].image_channel_data_type;
      }
      ++i;
      ++real_num_image_formats;
    }
  }
  if (num_image_formats != NULL) {
    (*num_image_formats) = real_num_image_formats;
  }
  return CL_SUCCESS;
}

static cl_int
clEnqueueReadWriteImage(cl_command_type     command_type,
                        cl_command_queue    command_queue,
                        cl_mem              image,
                        cl_bool             blocking,
                        size_t const        origin[3],
                        size_t const        region[3],
                        size_t              row_pitch,
                        size_t              slice_pitch,
                        void *              read_ptr,
                        void const *        write_ptr,
                        cl_uint             num_events_in_wait_list,
                        cl_event const *    event_wait_list,
                        cl_event *          event) CL_API_SUFFIX__VERSION_1_0 {
  cl_uint i;
  cl_command cmd = NULL;
  cl_int return_code = CL_SUCCESS;
  cl_event event_allocated = NULL;
  
  if (NULL == command_queue) {
    return_code = CL_INVALID_COMMAND_QUEUE;
    goto error;
  }
  if (NULL == image) {
    return_code = CL_INVALID_MEM_OBJECT;
    goto error;
  }
  if (command_queue->context != image->context) {
    return_code = CL_INVALID_CONTEXT;
    goto error;
  }
  switch (command_type) {
  case CL_COMMAND_READ_IMAGE:
    if (NULL == read_ptr) {
      return_code = CL_INVALID_VALUE;
      goto error;
    }
    break;
  case CL_COMMAND_WRITE_IMAGE:
    if (NULL == write_ptr) {
      return_code = CL_INVALID_VALUE;
      goto error;
    }
    break;
  default:
    ASSERT(0);
    break;
  }
  /* TODO: handle CL_INVALID_VALUE for (origin, region) is out of bounds. */
  if (CL_MEM_OBJECT_IMAGE2D == image->type) {
    if ((origin[2] != 0) || (region[2] != 1) || (slice_pitch != 0)) {
      return_code = CL_INVALID_VALUE;
      goto error;
    }
  }
  if ((NULL == event_wait_list) && (num_events_in_wait_list > 0)) {
    return_code = CL_INVALID_EVENT_WAIT_LIST;
    goto error;
  }
  if ((event_wait_list != NULL) && (0 == num_events_in_wait_list)) {
    return_code = CL_INVALID_EVENT_WAIT_LIST;
    goto error;
  }
  for (i = (num_events_in_wait_list - 1); i >= 0; --i) {
    if (NULL == event_wait_list[i]) {
      return_code = CL_INVALID_EVENT_WAIT_LIST;
      goto error;
    }
  }
  cmd = CL_OSAL_CALLOC(sizeof(struct _cl_command));
  if (NULL == cmd) {
    return_code = CL_OUT_OF_HOST_MEMORY;
    goto error;
  }
  cmd->type = command_type;
  cmd->num_events_in_wait_list = num_events_in_wait_list;
  cmd->event_wait_list = event_wait_list;
  
  switch (command_type) {
  case CL_COMMAND_READ_IMAGE:
    cmd->u._cl_read_image_command.ptr = read_ptr;
    cmd->u._cl_read_image_command.image = image;
    break;
  case CL_COMMAND_WRITE_IMAGE:
    cmd->u._cl_write_image_command.ptr = write_ptr;
    cmd->u._cl_write_image_command.image = image;
    break;
  default:
    ASSERT(0);
    break;
  }
  
  if (event != NULL) {
    event_allocated = CL_OSAL_CALLOC(sizeof(struct _cl_event));
    if (NULL == event_allocated) {
      return_code = CL_OUT_OF_HOST_MEMORY;
      goto error;
    }
    event_allocated->command = cmd;
    (*event) = event_allocated;
  }
  
  CVECTOR_PUSH_BACK(cl_command)(command_queue->commands, &cmd);
  cmd->execution_status = CL_QUEUED;
  
  if (CL_TRUE == blocking) {
    for (;;) {
      if (CL_COMPLETE == cmd->execution_status) {
        break;
      }
    }
  }
  
  goto success;
  
 error:
  if (cmd != NULL) {
    CL_OSAL_FREE(cmd);
    cmd = NULL;
  }
  if (event_allocated != NULL) {
    CL_OSAL_FREE(event_allocated);
    event_allocated = NULL;
  }
  
 success:
  return return_code;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadImage(cl_command_queue     command_queue,
                   cl_mem               image,
                   cl_bool              blocking_read, 
                   const size_t         origin[3],
                   const size_t         region[3],
                   size_t               row_pitch,
                   size_t               slice_pitch, 
                   void *               ptr,
                   cl_uint              num_events_in_wait_list,
                   const cl_event *     event_wait_list,
                   cl_event *           event) CL_API_SUFFIX__VERSION_1_0 {
  return clEnqueueReadWriteImage(CL_COMMAND_READ_IMAGE,
                                 command_queue,
                                 image,
                                 blocking_read,
                                 origin,
                                 region,
                                 row_pitch,
                                 slice_pitch,
                                 ptr,
                                 NULL,
                                 num_events_in_wait_list,
                                 event_wait_list,
                                 event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteImage(cl_command_queue    command_queue,
                    cl_mem              image,
                    cl_bool             blocking_write, 
                    const size_t        origin[3],
                    const size_t        region[3],
                    size_t              input_row_pitch,
                    size_t              input_slice_pitch, 
                    const void *        ptr,
                    cl_uint             num_events_in_wait_list,
                    const cl_event *    event_wait_list,
                    cl_event *          event) CL_API_SUFFIX__VERSION_1_0 {
  return clEnqueueReadWriteImage(CL_COMMAND_READ_IMAGE,
                                 command_queue,
                                 image,
                                 blocking_write,
                                 origin,
                                 region,
                                 input_row_pitch,
                                 input_slice_pitch,
                                 NULL,
                                 ptr,
                                 num_events_in_wait_list,
                                 event_wait_list,
                                 event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyImage(cl_command_queue     command_queue,
                   cl_mem               src_image,
                   cl_mem               dst_image, 
                   const size_t         src_origin[3],
                   const size_t         dst_origin[3],
                   const size_t         region[3], 
                   cl_uint              num_events_in_wait_list,
                   const cl_event *     event_wait_list,
                   cl_event *           event) CL_API_SUFFIX__VERSION_1_0 {
  cl_uint i;
  cl_command cmd = NULL;
  cl_int return_code = CL_SUCCESS;
  cl_event event_allocated = NULL;
  
  if (NULL == command_queue) {
    return_code = CL_INVALID_COMMAND_QUEUE;
    goto error;
  }
  if (NULL == src_image) {
    return_code = CL_INVALID_MEM_OBJECT;
    goto error;
  }
  if (NULL == dst_image) {
    return_code = CL_INVALID_MEM_OBJECT;
    goto error;
  }
  if (command_queue->context != src_image->context) {
    return_code = CL_INVALID_CONTEXT;
    goto error;
  }
  if (command_queue->context != dst_image->context) {
    return_code = CL_INVALID_CONTEXT;
    goto error;
  }
  if (src_image->image_format->image_channel_order != dst_image->image_format->image_channel_order) {
    return_code = CL_IMAGE_FORMAT_MISMATCH;
    goto error;
  }
  /* TODO: handle CL_INVALID_VALUE for (origin, region) is out of bounds. */
  /* TODO: handle CL_MEM_COPY_OVERLAP */
  if (CL_MEM_OBJECT_IMAGE2D == src_image->type) {
    if ((src_origin[2] != 0) || (region[2] != 1)) {
      return_code = CL_INVALID_VALUE;
      goto error;
    }
  }
  if (CL_MEM_OBJECT_IMAGE2D == dst_image->type) {
    if ((dst_origin[2] != 0) || (region[2] != 1)) {
      return_code = CL_INVALID_VALUE;
      goto error;
    }
  }
  if ((NULL == event_wait_list) && (num_events_in_wait_list > 0)) {
    return_code = CL_INVALID_EVENT_WAIT_LIST;
    goto error;
  }
  if ((event_wait_list != NULL) && (0 == num_events_in_wait_list)) {
    return_code = CL_INVALID_EVENT_WAIT_LIST;
    goto error;
  }
  for (i = (num_events_in_wait_list - 1); i >= 0; --i) {
    if (NULL == event_wait_list[i]) {
      return_code = CL_INVALID_EVENT_WAIT_LIST;
      goto error;
    }
  }
  cmd = CL_OSAL_CALLOC(sizeof(struct _cl_command));
  if (NULL == cmd) {
    return_code = CL_OUT_OF_HOST_MEMORY;
    goto error;
  }
  cmd->type = CL_COMMAND_COPY_IMAGE;
  cmd->num_events_in_wait_list = num_events_in_wait_list;
  cmd->event_wait_list = event_wait_list;
  
  cmd->u._cl_copy_image_command.src_image = src_image;
  cmd->u._cl_copy_image_command.dst_image = dst_image;
  
  if (event != NULL) {
    event_allocated = CL_OSAL_CALLOC(sizeof(struct _cl_event));
    if (NULL == event_allocated) {
      return_code = CL_OUT_OF_HOST_MEMORY;
      goto error;
    }
    event_allocated->command = cmd;
    (*event) = event_allocated;
  }
  
  CVECTOR_PUSH_BACK(cl_command)(command_queue->commands, &cmd);
  cmd->execution_status = CL_QUEUED;
  
  goto success;
  
 error:
  if (cmd != NULL) {
    CL_OSAL_FREE(cmd);
    cmd = NULL;
  }
  if (event_allocated != NULL) {
    CL_OSAL_FREE(event_allocated);
    event_allocated = NULL;
  }
  
 success:
  return return_code;
}

CL_API_ENTRY void * CL_API_CALL
clEnqueueMapImage(cl_command_queue  command_queue,
                  cl_mem            image, 
                  cl_bool           blocking_map, 
                  cl_map_flags      map_flags, 
                  const size_t      origin[3],
                  const size_t      region[3],
                  size_t *          image_row_pitch,
                  size_t *          image_slice_pitch,
                  cl_uint           num_events_in_wait_list,
                  const cl_event *  event_wait_list,
                  cl_event *        event,
                  cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_uint i;
  cl_command cmd = NULL;
  cl_int return_code = CL_SUCCESS;
  cl_event event_allocated = NULL;
  
  if (NULL == command_queue) {
    return_code = CL_INVALID_COMMAND_QUEUE;
    goto error;
  }
  if (NULL == image) {
    return_code = CL_INVALID_MEM_OBJECT;
    goto error;
  }
  if (command_queue->context != image->context) {
    return_code = CL_INVALID_CONTEXT;
    goto error;
  }
  switch (map_flags) {
  case CL_MAP_READ:
  case CL_MAP_WRITE:
  default:
    return_code = CL_INVALID_VALUE;
    goto error;
  }
  /* TODO: handle CL_INVALID_VALUE for (origin,region) is out of bounds. */
  if (CL_MEM_OBJECT_IMAGE2D == image->type) {
    if ((origin[2] != 0) || (region[2] != 1)) {
      return_code = CL_INVALID_VALUE;
      goto error;
    }
  }
  if (NULL == image_row_pitch) {
    return_code = CL_INVALID_VALUE;
    goto error;
  }
  if (CL_MEM_OBJECT_IMAGE3D == image->type) {
    if (NULL == image_slice_pitch) {
      return_code = CL_INVALID_VALUE;
      goto error;
    }
  }
  if ((NULL == event_wait_list) && (num_events_in_wait_list > 0)) {
    return_code = CL_INVALID_EVENT_WAIT_LIST;
    goto error;
  }
  if ((event_wait_list != NULL) && (0 == num_events_in_wait_list)) {
    return_code = CL_INVALID_EVENT_WAIT_LIST;
    goto error;
  }
  for (i = (num_events_in_wait_list - 1); i >= 0; --i) {
    if (NULL == event_wait_list[i]) {
      return_code = CL_INVALID_EVENT_WAIT_LIST;
      goto error;
    }
  }
  cmd = CL_OSAL_CALLOC(sizeof(struct _cl_command));
  if (NULL == cmd) {
    return_code = CL_OUT_OF_HOST_MEMORY;
    goto error;
  }
  cmd->type = CL_COMMAND_MAP_IMAGE;
  cmd->num_events_in_wait_list = num_events_in_wait_list;
  cmd->event_wait_list = event_wait_list;
  
  cmd->u._cl_map_image_command.image = image;
  
  if (event != NULL) {
    event_allocated = CL_OSAL_CALLOC(sizeof(struct _cl_event));
    if (NULL == event_allocated) {
      return_code = CL_OUT_OF_HOST_MEMORY;
      goto error;
    }
    event_allocated->command = cmd;
    (*event) = event_allocated;
  }
  
  CVECTOR_PUSH_BACK(cl_command)(command_queue->commands, &cmd);
  cmd->execution_status = CL_QUEUED;
  
  if (CL_TRUE == blocking_map) {
    for (;;) {
      if (CL_COMPLETE == cmd->execution_status) {
        break;
      }
    }
  }
  
  goto success;
  
 error:
  if (cmd != NULL) {
    CL_OSAL_FREE(cmd);
    cmd = NULL;
  }
  if (event_allocated != NULL) {
    CL_OSAL_FREE(event_allocated);
    event_allocated = NULL;
  }
  
 success:
  /* TODO: consider (offset,cb) */
  return image->mem_ptr;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetImageInfo(cl_mem           image,
               cl_image_info    param_name, 
               size_t           param_value_size,
               void *           param_value,
               size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == image) {
    return CL_INVALID_MEM_OBJECT;
  }
  switch (param_name) {
  case CL_IMAGE_FORMAT:
  case CL_IMAGE_ELEMENT_SIZE:
  case CL_IMAGE_ROW_PITCH:
  case CL_IMAGE_SLICE_PITCH:
  case CL_IMAGE_WIDTH:
  case CL_IMAGE_HEIGHT:
  case CL_IMAGE_DEPTH:
    /* TODO: handle them. */
    break;
    
  default:
    return CL_INVALID_VALUE;
  }
  return CL_SUCCESS;
}
