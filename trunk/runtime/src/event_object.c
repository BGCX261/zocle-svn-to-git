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

CL_API_ENTRY cl_int CL_API_CALL
clWaitForEvents(cl_uint             num_events,
                const cl_event *    event_list) CL_API_SUFFIX__VERSION_1_0 {
  cl_int return_code = CL_SUCCESS;
  cl_uint i;
  cl_context context;
  
  if (0 == num_events) {
    return_code = CL_INVALID_VALUE;
    goto error;
  }
  context = event_list[0]->context;
  for (i = (num_events - 1); i > 0; --i) {
    if (NULL == event_list[i]) {
      return_code = CL_INVALID_EVENT;
      goto error;
    }
    if (event_list[i]->context != context) {
      return_code = CL_INVALID_CONTEXT;
      goto error;
    }
  }
  for (i = (num_events - 1); i >= 0; --i) {
    while (1) {
      if ((CL_COMPLETE == event_list[i]->command->execution_status) ||
          (event_list[i]->command->execution_status < 0)) {
        break;
      }
    }
  }
  
 error:
  return return_code;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetEventInfo(cl_event         event,
               cl_event_info    param_name,
               size_t           param_value_size,
               void *           param_value,
               size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == event) {
    return CL_INVALID_EVENT;
  }
  switch (param_name) {
  case CL_EVENT_COMMAND_QUEUE:
  case CL_EVENT_COMMAND_TYPE:
  case CL_EVENT_COMMAND_EXECUTION_STATUS:
  case CL_EVENT_REFERENCE_COUNT:
    /* TODO: handle them. */
    break;
    
  default:
    return CL_INVALID_VALUE;
  }
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainEvent(cl_event event) CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == event) {
    return CL_INVALID_EVENT;
  }
  ++event->ref_count;
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseEvent(cl_event event) CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == event) {
    return CL_INVALID_EVENT;
  }
  --event->ref_count;
  if (0 == event->ref_count) {
    /* TODO: wait the specific command identified by this event has completed
     * (or terminated) and there are no commands in the command-queues
     * of a context that require a wait for this event to complete.
     */
  }
  return CL_SUCCESS;
}
