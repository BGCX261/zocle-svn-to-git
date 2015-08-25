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

CL_API_ENTRY cl_int CL_API_CALL
clRetainMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == memobj) {
    return CL_INVALID_MEM_OBJECT;
  }
  ++memobj->ref_count;
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseMemObject(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == memobj) {
    return CL_INVALID_MEM_OBJECT;
  }
  --memobj->ref_count;
  if (0 == memobj->ref_count) {
    /* TODO: wait commands queued for execution on a command-queue(s) that
     * use this memobj have finished, the memory object is deleted.
     */
  }
  return CL_SUCCESS;
}

void
clCheckMemoryObjectFlags(cl_mem       mem,
                         cl_mem_flags flags,
                         size_t       size,
                         void *       host_ptr,
                         cl_int *     errcode_ret) {
  cl_int return_code = CL_SUCCESS;
  
  if ((flags & CL_MEM_COPY_HOST_PTR) && (flags & CL_MEM_USE_HOST_PTR)) {
    return_code = CL_INVALID_VALUE;
    goto error;
  }
  if (flags & CL_MEM_READ_WRITE) {
    if ((flags & CL_MEM_READ_ONLY)  || (flags & CL_MEM_WRITE_ONLY)) {
      return_code = CL_INVALID_VALUE;
      goto error;
    }
    mem->access_read = CL_TRUE;
    mem->access_write = CL_TRUE;
  }
  if (flags & CL_MEM_READ_ONLY) {
    if (flags & CL_MEM_WRITE_ONLY) {
      return_code = CL_INVALID_VALUE;
      goto error;
    }
    mem->access_read = CL_TRUE;
    mem->access_write = CL_FALSE;
  }
  if (flags & CL_MEM_WRITE_ONLY) {
    mem->access_read = CL_FALSE;
    mem->access_write = CL_TRUE;
  }
  if ((0 == (flags & CL_MEM_USE_HOST_PTR)) &&
      (0 == (flags & CL_MEM_COPY_HOST_PTR))) {
    if (host_ptr != NULL) {
      return_code = CL_INVALID_HOST_PTR;
      goto error;
    }
  }
  if (flags & CL_MEM_USE_HOST_PTR) {
    if (NULL == host_ptr) {
      return_code = CL_INVALID_HOST_PTR;
      goto error;
    }
    mem->mem_ptr = host_ptr;
  }
  if (flags & CL_MEM_ALLOC_HOST_PTR) {
    mem->mem_ptr = CL_OSAL_CALLOC(size);
    if (NULL == mem->mem_ptr) {
      return_code = CL_MEM_OBJECT_ALLOCATION_FAILURE;
      goto error;
    }
  }
  if (flags & CL_MEM_COPY_HOST_PTR) {
    if (NULL == host_ptr) {
      return_code = CL_INVALID_HOST_PTR;
      goto error;
    }
    if (NULL == mem->mem_ptr) {
      mem->mem_ptr = CL_OSAL_CALLOC(size);
      if (NULL == mem->mem_ptr) {
        return_code = CL_MEM_OBJECT_ALLOCATION_FAILURE;
        goto error;
      }
    }
    memcpy(mem->mem_ptr, host_ptr, size);
  }
  
 error:
  if ((mem != NULL) && (mem->mem_ptr != NULL)) {
    CL_OSAL_FREE(mem->mem_ptr);
    mem->mem_ptr = NULL;
  }
  if (errcode_ret != NULL) {
    (*errcode_ret) = return_code;
  }
}

static cl_int
clEnqueueCopyBetweenImageAndBuffer(cl_command_type  command_type,
                                   cl_command_queue command_queue,
                                   cl_mem           image,
                                   cl_mem           buffer, 
                                   const size_t     origin[3],
                                   const size_t     region[3], 
                                   size_t           offset,
                                   cl_uint          num_events_in_wait_list,
                                   const cl_event * event_wait_list,
                                   cl_event *       event) {
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
  if (NULL == buffer) {
    return_code = CL_INVALID_MEM_OBJECT;
    goto error;
  }
  if (command_queue->context != image->context) {
    return_code = CL_INVALID_CONTEXT;
    goto error;
  }
  if (command_queue->context != buffer->context) {
    return_code = CL_INVALID_CONTEXT;
    goto error;
  }
  /* TODO: handle CL_INVALID_VALUE for (origin, region) is out of bounds. */
  if (CL_MEM_OBJECT_IMAGE2D == image->type) {
    if ((origin[2] != 0) || (region[2] != 1)) {
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
  
  cmd->u._cl_copy_between_image_and_buffer_command.image = image;
  cmd->u._cl_copy_between_image_and_buffer_command.buffer = buffer;
  
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

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyImageToBuffer(cl_command_queue command_queue,
                           cl_mem           src_image,
                           cl_mem           dst_buffer, 
                           const size_t     src_origin[3],
                           const size_t     region[3], 
                           size_t           dst_offset,
                           cl_uint          num_events_in_wait_list,
                           const cl_event * event_wait_list,
                           cl_event *       event) CL_API_SUFFIX__VERSION_1_0 {
  return clEnqueueCopyBetweenImageAndBuffer(CL_COMMAND_COPY_IMAGE_TO_BUFFER,
                                            command_queue,
                                            src_image,
                                            dst_buffer,
                                            src_origin,
                                            region,
                                            dst_offset,
                                            num_events_in_wait_list,
                                            event_wait_list,
                                            event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBufferToImage(cl_command_queue command_queue,
                           cl_mem           src_buffer,
                           cl_mem           dst_image, 
                           size_t           src_offset,
                           const size_t     dst_origin[3],
                           const size_t     region[3], 
                           cl_uint          num_events_in_wait_list,
                           const cl_event * event_wait_list,
                           cl_event *       event) CL_API_SUFFIX__VERSION_1_0 {
  return clEnqueueCopyBetweenImageAndBuffer(CL_COMMAND_COPY_BUFFER_TO_IMAGE,
                                            command_queue,
                                            dst_image,
                                            src_buffer,
                                            dst_origin,
                                            region,
                                            src_offset,
                                            num_events_in_wait_list,
                                            event_wait_list,
                                            event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueUnmapMemObject(cl_command_queue  command_queue,
                        cl_mem            memobj,
                        void *            mapped_ptr,
                        cl_uint           num_events_in_wait_list,
                        const cl_event *  event_wait_list,
                        cl_event *        event) CL_API_SUFFIX__VERSION_1_0 {
  cl_uint i;
  cl_command cmd = NULL;
  cl_int return_code = CL_SUCCESS;
  cl_event event_allocated = NULL;
  
  if (NULL == command_queue) {
    return_code = CL_INVALID_COMMAND_QUEUE;
    goto error;
  }
  if (NULL == memobj) {
    return_code = CL_INVALID_MEM_OBJECT;
    goto error;
  }
  if (command_queue->context != memobj->context) {
    return_code = CL_INVALID_CONTEXT;
    goto error;
  }
  /* TODO: handle CL_INVALID_VALUE for if mapped_ptr is not a valid pointer
   * returned by clEnqueueMapBuffer or clEnqueueMapImage for memobj.
   */
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
  cmd->type = CL_COMMAND_UNMAP_MEM_OBJECT;
  cmd->num_events_in_wait_list = num_events_in_wait_list;
  cmd->event_wait_list = event_wait_list;
  
  cmd->u._cl_unmap_memobj_command.memobj = memobj;
  
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

CL_API_ENTRY cl_int CL_API_CALL
clGetMemObjectInfo(cl_mem           memobj,
                   cl_mem_info      param_name, 
                   size_t           param_value_size,
                   void *           param_value,
                   size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == memobj) {
    return CL_INVALID_MEM_OBJECT;
  }
  switch (param_name) {
  case CL_MEM_TYPE:
    if (param_value != NULL) {
      if (param_value_size < sizeof(memobj->type)) {
        return CL_INVALID_VALUE;
      }
      memcpy(param_value, &(memobj->type), sizeof(memobj->type));
    }
    if (param_value_size_ret != NULL) {
      (*param_value_size_ret) = sizeof(memobj->type);
    }
    break;
    
  case CL_MEM_FLAGS:
    if (param_value != NULL) {
      if (param_value_size < sizeof(memobj->flags)) {
        return CL_INVALID_VALUE;
      }
      memcpy(param_value, &(memobj->flags), sizeof(memobj->flags));
    }
    if (param_value_size_ret != NULL) {
      (*param_value_size_ret) = sizeof(memobj->flags);
    }
    break;
    
  case CL_MEM_SIZE:
  case CL_MEM_HOST_PTR:
  case CL_MEM_MAP_COUNT:
  case CL_MEM_REFERENCE_COUNT:
  case CL_MEM_CONTEXT:
    /* TODO: handle them. */
    break;
    
  default:
    return CL_INVALID_VALUE;
  }
  return CL_SUCCESS;
}
