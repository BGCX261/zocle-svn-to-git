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

#ifndef DEFINE_CVECTOR_TYPE_FOR_CL_COMMAND_QUEUE
#define DEFINE_CVECTOR_TYPE_FOR_CL_COMMAND_QUEUE
DEFINE_CVECTOR_TYPE(cl_command_queue)
#endif

CL_API_ENTRY cl_command_queue CL_API_CALL
clCreateCommandQueue(
    cl_context                     context, 
    cl_device_id                   device, 
    cl_command_queue_properties    properties,
    cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_command_queue command_queue = NULL;
  cl_uint i;
  cl_int return_code = CL_SUCCESS;
  
  if (NULL == context) {
    return_code = CL_INVALID_CONTEXT;
    goto error;
  }
  for (i = (context->num_devices - 1); i >= 0; --i) {
    if (context->devices[i] == device) {
      command_queue = CL_OSAL_CALLOC(sizeof(struct _cl_command_queue));
      if (NULL == command_queue) {
        return_code = CL_OUT_OF_HOST_MEMORY;
        goto error;
      }
      command_queue->context = context;
      command_queue->device = device;
      if (properties & (CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
                        CL_QUEUE_PROFILING_ENABLE)) {
        return_code = CL_INVALID_VALUE;
        goto error;
      }
      if (properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) {
        command_queue->enable_out_of_order_exec = CL_TRUE;
        properties &= (~CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
      }
      if (properties & CL_QUEUE_PROFILING_ENABLE) {
        command_queue->enable_profiling = CL_TRUE;
        properties &= (~CL_QUEUE_PROFILING_ENABLE);
      }
      command_queue->commands = CVECTOR_NEW(cl_command)(0);
      if (NULL == command_queue->commands) {
        return_code = CL_OUT_OF_HOST_MEMORY;
        goto error;
      }
      clRetainCommandQueue(command_queue);
      {
        cl_int const result =
          CVECTOR_PUSH_BACK(cl_command_queue)(context->command_queues,
                                              &command_queue);
        switch (result) {
        case CL_SUCCESS:
          break;
        case CL_OUT_OF_HOST_MEMORY:
          return_code = CL_OUT_OF_HOST_MEMORY;
          goto error;
        }
      }
      return command_queue;
    }
  }
  return_code = CL_INVALID_DEVICE;
  
 error:
  if ((command_queue != NULL) && (command_queue->commands != NULL)) {
    CVECTOR_DELETE(cl_command)(command_queue->commands);
    command_queue->commands = NULL;
  }
  if (command_queue != NULL) {
    CL_OSAL_FREE(command_queue);
    command_queue = NULL;
  }
  return NULL;
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainCommandQueue(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == command_queue) {
    return CL_INVALID_COMMAND_QUEUE;
  }
  ++command_queue->ref_count;
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseCommandQueue(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == command_queue) {
    return CL_INVALID_COMMAND_QUEUE;
  }
  --command_queue->ref_count;
  if (0 == command_queue->ref_count) {
    /* TODO: wait all commands queued to this command queue are finished. */
  }
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetCommandQueueInfo(cl_command_queue      command_queue,
                      cl_command_queue_info param_name,
                      size_t                param_value_size,
                      void *                param_value,
                      size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == command_queue) {
    return CL_INVALID_COMMAND_QUEUE;
  }
  switch (param_name) {
  case CL_QUEUE_CONTEXT:
    if (param_value != NULL) {
      if (param_value_size < sizeof(command_queue->context)) {
        return CL_INVALID_VALUE;
      }
      memcpy(param_value, command_queue->context, sizeof(command_queue->context));
    }
    if (param_value_size_ret != NULL) {
      (*param_value_size_ret) = sizeof(command_queue->context);
    }
    break;
    
  case CL_QUEUE_DEVICE:
    if (param_value != NULL) {
      if (param_value_size < sizeof(command_queue->device)) {
        return CL_INVALID_VALUE;
      }
      memcpy(param_value, command_queue->device, sizeof(command_queue->device));
    }
    if (param_value_size_ret != NULL) {
      (*param_value_size_ret) = sizeof(command_queue->device);
    }
    break;
    
  case CL_QUEUE_REFERENCE_COUNT:
    if (param_value != NULL) {
      if (param_value_size < sizeof(command_queue->ref_count)) {
        return CL_INVALID_VALUE;
      }
      memcpy(param_value, &(command_queue->ref_count), sizeof(command_queue->ref_count));
    }
    if (param_value_size_ret != NULL) {
      (*param_value_size_ret) = sizeof(command_queue->ref_count);
    }
    break;
    
  case CL_QUEUE_PROPERTIES:
  {
    cl_command_queue_properties properties = 0;
    
    if (CL_TRUE == command_queue->enable_out_of_order_exec) {
      properties |= CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
    }
    if (CL_TRUE == command_queue->enable_profiling) {
      properties |= CL_QUEUE_PROFILING_ENABLE;
    }
    
    if (param_value != NULL) {
      if (param_value_size < sizeof(properties)) {
        return CL_INVALID_VALUE;
      }
      memcpy(param_value, &properties, sizeof(properties));
    }
    if (param_value_size_ret != NULL) {
      (*param_value_size_ret) = sizeof(properties);
    }
  }
  break;
  
  default:
    return CL_INVALID_VALUE;
  }
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clSetCommandQueueProperty(cl_command_queue              command_queue,
                          cl_command_queue_properties   properties, 
                          cl_bool                       enable,
                          cl_command_queue_properties * old_properties) CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == command_queue) {
    return CL_INVALID_COMMAND_QUEUE;
  }
  if (properties & (CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE)) {
    return CL_INVALID_VALUE;
  }
  if (old_properties != NULL) {
    cl_command_queue_properties properties = 0;
    
    if (CL_TRUE == command_queue->enable_out_of_order_exec) {
      properties |= CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
    }
    if (CL_TRUE == command_queue->enable_profiling) {
      properties |= CL_QUEUE_PROFILING_ENABLE;
    }
    (*old_properties) = properties;
  }
  if (properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) {
    if (command_queue->enable_out_of_order_exec != enable) {
      /* TODO: block until all previously queued commands in this command queue have completed. */
    }
    command_queue->enable_out_of_order_exec = enable;
    properties &= (~CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
  }
  if (properties & CL_QUEUE_PROFILING_ENABLE) {
    command_queue->enable_profiling = enable;
    properties &= (~CL_QUEUE_PROFILING_ENABLE);
  }
  return CL_SUCCESS;
}
