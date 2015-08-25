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
#ifndef ZOCLE_CL_INTERNAL_H_
#define ZOCLE_CL_INTERNAL_H_

#include <cl.h>
#include <container/inc/cvector.h>

struct _cl_device_id {
  cl_context            context;
};

#ifndef DECLARE_CVECTOR_TYPE_FOR_CL_COMMAND_QUEUE
#define DECLARE_CVECTOR_TYPE_FOR_CL_COMMAND_QUEUE
DECLARE_CVECTOR_TYPE(cl_command_queue)
#endif

#ifndef DECLARE_CVECTOR_TYPE_FOR_CL_COMMAND
#define DECLARE_CVECTOR_TYPE_FOR_CL_COMMAND
DECLARE_CVECTOR_TYPE(cl_command)
#endif

#ifndef DECLARE_CVECTOR_TYPE_FOR_CL_MEM
#define DECLARE_CVECTOR_TYPE_FOR_CL_MEM
DECLARE_CVECTOR_TYPE(cl_mem)
#endif

struct _cl_context {
  logging_fn                  pfn_notify;
  void *                      user_data;
  cl_uint                     ref_count;
  cl_uint                     num_devices;
  cl_device_id *              devices;
  
  CVECTOR_TYPE(cl_command_queue)  command_queues;
  CVECTOR_TYPE(cl_mem)            mem_objs;
};

struct _cl_command {
  cl_command_type type;
  cl_uint execution_status;
  cl_uint num_events_in_wait_list;
  cl_event const *event_wait_list;
  union {
    struct {
      cl_mem buffer;
      void * ptr;
      size_t offset;
      size_t cb;
    } _cl_read_buffer_command;
    struct {
      cl_mem       buffer;
      void const * ptr;
      size_t       offset;
      size_t       cb;
    } _cl_write_buffer_command;
    struct {
      cl_mem src_buffer;
      cl_mem dst_buffer;
      size_t src_offset;
      size_t dst_offset;
      size_t cb;
    } _cl_copy_buffer_command;
    
    struct {
      cl_mem image;
      void * ptr;
    } _cl_read_image_command;
    struct {
      cl_mem       image;
      void const * ptr;
    } _cl_write_image_command;
    struct {
      cl_mem src_image;
      cl_mem dst_image;
    } _cl_copy_image_command;

    struct {
      cl_mem image;
      cl_mem buffer;
    } _cl_copy_between_image_and_buffer_command;
    
    struct {
      cl_mem buffer;
    } _cl_map_buffer_command;
    struct {
      cl_mem image;
    } _cl_map_image_command;
    struct {
      cl_mem memobj;
    } _cl_unmap_memobj_command;
    
    struct {
      cl_kernel kernel;
    } _cl_ndrange_kernel_command;
    struct {
      cl_kernel kernel;
    } _cl_task_command;
    struct {
      void (*user_func)(void *);
    } _cl_native_kernel_command;
  } u;
};

struct _cl_event {
  cl_context           context;
  struct _cl_command * command;
  cl_uint              ref_count;
};

struct _cl_command_queue {
  cl_context            context;
  cl_bool               enable_out_of_order_exec;
  cl_bool               enable_profiling;
  cl_uint               ref_count;
  cl_device_id          device;
  
  CVECTOR_TYPE(cl_command) commands;
};

struct _cl_mem {
  cl_mem_object_type       type;
  cl_context               context;
  cl_mem_flags             flags;
  cl_bool                  access_read;
  cl_bool                  access_write;
  cl_uint                  ref_count;
  void *                   mem_ptr;
  
  /* only used in Image2D */
  cl_image_format const *  image_format;
  size_t                   image_width;
  size_t                   image_height;
  size_t                   image_row_pitch;
};

struct _cl_sampler {
  cl_context         context;
  cl_uint            ref_count;
  cl_addressing_mode addressing_mode;
  cl_filter_mode     filter_mode;
};

struct _cl_program {
  cl_context         context;
  cl_uint            ref_count;
};

struct _cl_kernel {
  cl_uint      ref_count;
  char const * name;
};

extern void
clCheckMemoryObjectFlags(cl_mem       mem,
                         cl_mem_flags flags,
                         size_t       size,
                         void *       host_ptr,
                         cl_int *     errcode_ret);

#endif
