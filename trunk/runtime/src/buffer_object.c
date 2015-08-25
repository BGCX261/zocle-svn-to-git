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

#ifndef DEFINE_CVECTOR_TYPE_FOR_CL_MEM
#define DEFINE_CVECTOR_TYPE_FOR_CL_MEM
DEFINE_CVECTOR_TYPE(cl_mem)
#endif

#ifndef DEFINE_CVECTOR_TYPE_FOR_CL_COMMAND
#define DEFINE_CVECTOR_TYPE_FOR_CL_COMMAND
DEFINE_CVECTOR_TYPE(cl_command)
#endif

CL_API_ENTRY cl_mem CL_API_CALL
clCreateBuffer(cl_context   context,
               cl_mem_flags flags,
               size_t       size,
               void *       host_ptr,
               cl_int *     errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_int return_code = CL_SUCCESS;
  cl_mem mem = NULL;
  cl_uint i;
  
  if (NULL == context) {
    return_code = CL_INVALID_CONTEXT;
    goto error;
  }
  if (0 == size) {
    return_code = CL_INVALID_BUFFER_SIZE;
    goto error;
  }
  for (i = (context->num_devices - 1); i >= 0; --i) {
    cl_ulong max_mem_alloc_size;
    cl_int const result = clGetDeviceInfo(context->devices[i],
                                          CL_DEVICE_MAX_MEM_ALLOC_SIZE, 
                                          sizeof(max_mem_alloc_size),
                                          &max_mem_alloc_size,
                                          NULL);
    if (result != CL_SUCCESS) {
      ASSERT(0);
    }
    if (size > max_mem_alloc_size) {
      return_code = CL_INVALID_BUFFER_SIZE;
      goto error;
    }
  }
  mem = CL_OSAL_CALLOC(sizeof(struct _cl_mem));
  if (NULL == mem) {
    return_code = CL_OUT_OF_HOST_MEMORY;
    goto error;
  }
  mem->type = CL_MEM_OBJECT_BUFFER;
  mem->mem_ptr = NULL;
  mem->flags = flags;
  /* Default to read/write */
  mem->access_read = CL_TRUE;
  mem->access_write = CL_TRUE;
  
  clCheckMemoryObjectFlags(mem, flags, size, host_ptr, &return_code);
  if (return_code != CL_SUCCESS) {
    goto error;
  }
  {
    cl_int const result = CVECTOR_PUSH_BACK(cl_mem)(context->mem_objs, &mem);
    switch (result) {
    case CL_SUCCESS:
      break;
    case CL_OUT_OF_HOST_MEMORY:
      return_code = CL_OUT_OF_HOST_MEMORY;
      goto error;
    }
  }
  mem->context = context;
  /* TODO: handle CL_INVALID_OPERATION */
  clRetainMemObject(mem);
  goto success;
  
 error:
  if ((mem != NULL) && (mem->mem_ptr != NULL)) {
    CL_OSAL_FREE(mem->mem_ptr);
    mem->mem_ptr = NULL;
  }
  if (mem != NULL) {
    CL_OSAL_FREE(mem);
    mem = NULL;
  }
  
 success:
  if (errcode_ret != NULL) {
    (*errcode_ret) = return_code;
  }
  return mem;
}

static cl_int
clEnqueueReadWriteBuffer(cl_command_type     command_type,
                         cl_command_queue    command_queue,
                         cl_mem              buffer,
                         cl_bool             blocking,
                         size_t              offset,
                         size_t              cb,
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
  if (NULL == buffer) {
    return_code = CL_INVALID_MEM_OBJECT;
    goto error;
  }
  if (command_queue->context != buffer->context) {
    return_code = CL_INVALID_CONTEXT;
    goto error;
  }
  switch (command_type) {
  case CL_COMMAND_READ_BUFFER:
    if (NULL == read_ptr) {
      return_code = CL_INVALID_VALUE;
      goto error;
    }
    break;
  case CL_COMMAND_WRITE_BUFFER:
    if (NULL == write_ptr) {
      return_code = CL_INVALID_VALUE;
      goto error;
    }
    break;
  default:
    ASSERT(0);
    break;
  }
  /* TODO: handle CL_INVALID_VALUE for (offset,cb) is out of bounds. */
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
  case CL_COMMAND_READ_BUFFER:
    cmd->u._cl_read_buffer_command.ptr = read_ptr;
    cmd->u._cl_read_buffer_command.offset = offset;
    cmd->u._cl_read_buffer_command.cb = cb;
    cmd->u._cl_read_buffer_command.buffer = buffer;
    break;
  case CL_COMMAND_WRITE_BUFFER:
    cmd->u._cl_write_buffer_command.ptr = write_ptr;
    cmd->u._cl_write_buffer_command.offset = offset;
    cmd->u._cl_write_buffer_command.cb = cb;
    cmd->u._cl_write_buffer_command.buffer = buffer;
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
clEnqueueReadBuffer(cl_command_queue    command_queue,
                    cl_mem              buffer,
                    cl_bool             blocking_read,
                    size_t              offset,
                    size_t              cb, 
                    void *              ptr,
                    cl_uint             num_events_in_wait_list,
                    cl_event const *    event_wait_list,
                    cl_event *          event) CL_API_SUFFIX__VERSION_1_0 {
  return clEnqueueReadWriteBuffer(CL_COMMAND_READ_BUFFER, command_queue, buffer, blocking_read,
                                  offset, cb, ptr, NULL, num_events_in_wait_list,
                                  event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBuffer(cl_command_queue   command_queue, 
                     cl_mem             buffer, 
                     cl_bool            blocking_write, 
                     size_t             offset, 
                     size_t             cb, 
                     void const *       ptr, 
                     cl_uint            num_events_in_wait_list, 
                     cl_event const *   event_wait_list, 
                     cl_event *         event) CL_API_SUFFIX__VERSION_1_0 {
  return clEnqueueReadWriteBuffer(CL_COMMAND_WRITE_BUFFER, command_queue, buffer, blocking_write,
                                  offset, cb, NULL, ptr, num_events_in_wait_list,
                                  event_wait_list, event);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBuffer(cl_command_queue    command_queue, 
                    cl_mem              src_buffer,
                    cl_mem              dst_buffer, 
                    size_t              src_offset,
                    size_t              dst_offset,
                    size_t              cb, 
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
  if (NULL == src_buffer) {
    return_code = CL_INVALID_MEM_OBJECT;
    goto error;
  }
  if (NULL == dst_buffer) {
    return_code = CL_INVALID_MEM_OBJECT;
    goto error;
  }
  if (command_queue->context != src_buffer->context) {
    return_code = CL_INVALID_CONTEXT;
    goto error;
  }
  if (command_queue->context != dst_buffer->context) {
    return_code = CL_INVALID_CONTEXT;
    goto error;
  }
  if (src_buffer == dst_buffer) {
    if ((src_offset < dst_offset) && ((src_offset + cb) > dst_offset)) {
      return_code = CL_MEM_COPY_OVERLAP;
      goto error;
    }
    if ((src_offset < (dst_offset + cb)) && ((src_offset + cb) > (dst_offset + cb))) {
      return_code = CL_MEM_COPY_OVERLAP;
      goto error;
    }
    if ((dst_offset < src_offset) && ((dst_offset + cb) > src_offset)) {
      return_code = CL_MEM_COPY_OVERLAP;
      goto error;
    }
    if ((dst_offset < (src_offset + cb)) && ((dst_offset + cb) > (src_offset + cb))) {
      return_code = CL_MEM_COPY_OVERLAP;
      goto error;
    }
  }
  /* TODO: handle CL_INVALID_VALUE for (offset,cb) is out of bounds. */
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
  cmd->type = CL_COMMAND_COPY_BUFFER;
  cmd->num_events_in_wait_list = num_events_in_wait_list;
  cmd->event_wait_list = event_wait_list;
  
  cmd->u._cl_copy_buffer_command.src_buffer = src_buffer;
  cmd->u._cl_copy_buffer_command.dst_buffer = dst_buffer;
  cmd->u._cl_copy_buffer_command.src_offset = src_offset;
  cmd->u._cl_copy_buffer_command.dst_offset = dst_offset;
  cmd->u._cl_copy_buffer_command.cb = cb;
  
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
clEnqueueMapBuffer(cl_command_queue command_queue,
                   cl_mem           buffer,
                   cl_bool          blocking_map, 
                   cl_map_flags     map_flags,
                   size_t           offset,
                   size_t           cb,
                   cl_uint          num_events_in_wait_list,
                   const cl_event * event_wait_list,
                   cl_event *       event,
                   cl_int *         errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_uint i;
  cl_command cmd = NULL;
  cl_int return_code = CL_SUCCESS;
  cl_event event_allocated = NULL;
  
  if (NULL == command_queue) {
    return_code = CL_INVALID_COMMAND_QUEUE;
    goto error;
  }
  if (NULL == buffer) {
    return_code = CL_INVALID_MEM_OBJECT;
    goto error;
  }
  if (command_queue->context != buffer->context) {
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
  /* TODO: handle CL_INVALID_VALUE for (offset,cb) is out of bounds. */
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
  cmd->type = CL_COMMAND_MAP_BUFFER;
  cmd->num_events_in_wait_list = num_events_in_wait_list;
  cmd->event_wait_list = event_wait_list;
  
  cmd->u._cl_map_buffer_command.buffer = buffer;
  
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
  return buffer->mem_ptr;
}
