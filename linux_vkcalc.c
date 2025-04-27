//
// linux_vkcalc.c
//
// Caleb Barger
// 04/22/2025
//
// Linux vulkan calculator.
//

#include "base/base_inc.h"
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include "generated/vulkan_api.h"
#include "caffe_platform.h"
#include "caffe_vulkan.h"
#include <sys/mman.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "base/base_inc.c"
#include "generated/vulkan_api.c"
#include "caffe_vulkan.c"

internal void* linux_load_library(String8 path)
{
  void* code = dlopen(path.ptr, RTLD_LAZY);
  if (code == 0)
  {
    AssertMessage("Failed to load code.");
  }
  return code;
}

internal LOAD_FUNCTION(linux_load_function)
{
  void* proc_addr = 0;
  if (!(proc_addr = dlsym(handle, func)))
  {
    AssertMessage("Failed to load function");
  }
  return proc_addr;
}

internal DEBUG_FILE_SIZE(linux_file_size)
{
  U32 result = 1;

  struct stat file_stat;
  if (stat((char*)file_pathz, &file_stat) != -1)
  {
    result = file_stat.st_size;
  }

  return result;
}

internal DEBUG_READ_ENTIRE_FILE(linux_read_entire_file)
{
  File result = {0};

  S32 filedes = open((const char*)file_pathz.ptr, O_RDWR);
  if (filedes != -1)
  {
    result.size = linux_file_size(file_pathz.ptr);
    result.memory = mmap(0, AlignPow2(result.size, 4096), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (result.memory)
    {
      if (read(filedes, result.memory, result.size) != result.size)
      {
        AssertMessage("Didn't read the entire file");
      }
    }
    close(filedes);
  }

  return result;
}

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    return 1;
  }
  String8 expression_str = str8_from_mem(argv[1]);

  void* platform_memory = mmap(0, MB(1), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  if (!platform_memory)
  {
    AssertMessage("Failed to map platform memory");
  }
  Arena* linux_arena = arena_from_memory(platform_memory, MB(1));
  ThreadCTX thread_ctx = {0};
  thread_ctx.arenas[0] = arena_sub(linux_arena, KB(80));
  thread_ctx.arenas[1] = arena_sub(linux_arena, KB(80));
  equip_thread_ctx(&thread_ctx);

  VulkanContext vulkan_ctx = {0};
  vulkan_equip_ctx(&vulkan_ctx);

  void* vulkan_code = linux_load_library(Str8Lit("libvulkan.so"));
  if (!vulkan_code)
  {
    AssertMessage("Failed to load vulkan library.");
  }
  vulkan_api_load_global_functions(linux_load_function, &vulkan_ctx.api, vulkan_code);

  char* instance_layers[] = {
    "VK_LAYER_KHRONOS_validation",
  };
  char* instance_extensions[] = {
    "VK_EXT_debug_utils",
  };
  {
    Temp temp = temp_begin(0, 0);
    U32 layer_count = 0;
    if ((vulkan_ctx.api.vkEnumerateInstanceLayerProperties(&layer_count, 0) == VK_SUCCESS) &&
        (layer_count != 0))
    {
      VkLayerProperties* avail_layers = ArenaPushArray(temp.arena, VkLayerProperties, layer_count);
      if (vulkan_ctx.api.vkEnumerateInstanceLayerProperties(&layer_count, avail_layers) == VK_SUCCESS)
      {
        String8List avail_layer_names = vulkan_str8_list_from_layer_properties(temp.arena, avail_layers, layer_count);
        if (!vulkan_cstrs_in_list(instance_layers, ArrayCount(instance_layers), avail_layer_names))
        {
          AssertMessage("Missing layer");
        }
      }
    }
    temp_end(temp);
  }

  VkApplicationInfo application_info = {0};
  application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  application_info.pApplicationName = "vkcalc";
  application_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);

  VkInstanceCreateInfo instance_create_info = {0};
  instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_create_info.pApplicationInfo = &application_info;
  instance_create_info.enabledLayerCount = ArrayCount(instance_layers);
  instance_create_info.ppEnabledLayerNames = (const char**)instance_layers;
  instance_create_info.enabledExtensionCount = ArrayCount(instance_extensions);
  instance_create_info.ppEnabledExtensionNames = (const char**)instance_extensions;
  if (vulkan_ctx.api.vkCreateInstance(&instance_create_info, 0, &vulkan_ctx.instance) != VK_SUCCESS)
  {
    InvalidPath;
  }
  vulkan_api_load_instance_functions(linux_load_function, &vulkan_ctx.api, vulkan_code, vulkan_ctx.instance);
  U32 num_devices = 0;
  if ((vulkan_ctx.api.vkEnumeratePhysicalDevices(vulkan_ctx.instance, &num_devices, 0) != VK_SUCCESS) ||
      (num_devices == 0))
  {
    InvalidPath;
  }
  if (num_devices != 1)
  {
    AssertMessage("FIXME(calebarg): More than one physical device found!");
  }
  if (vulkan_ctx.api.vkEnumeratePhysicalDevices(vulkan_ctx.instance, &num_devices, &vulkan_ctx.physical_device) != VK_SUCCESS ) {
    InvalidPath;
  }
  VkPhysicalDeviceProperties device_properties;
  vulkan_ctx.api.vkGetPhysicalDeviceProperties(vulkan_ctx.physical_device, &device_properties);
  VkPhysicalDeviceFeatures   device_features;
  vulkan_ctx.api.vkGetPhysicalDeviceFeatures(vulkan_ctx.physical_device, &device_features);
  U32 major_version = VK_VERSION_MAJOR(device_properties.apiVersion);
  U32 minor_version = VK_VERSION_MINOR(device_properties.apiVersion);
  U32 patch_version = VK_VERSION_PATCH(device_properties.apiVersion);
  if((major_version < 1) &&
      (device_properties.limits.maxImageDimension2D < 4096))
  {
    AssertMessage("device properties do not meet the minimum requirements");
  }

  // NOTE(calebarg): The first queue family must support both graphics, presentation and compute.
  VkQueueFamilyProperties queue_family_properties;
  U32 queue_family_count = 1;
  vulkan_ctx.api.vkGetPhysicalDeviceQueueFamilyProperties(vulkan_ctx.physical_device, &queue_family_count, &queue_family_properties);
  if ((queue_family_properties.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0)
  {
    vulkan_ctx.queue_family_idx = 0;
  }

  F32 queue_priorities = 1.0;
  VkDeviceQueueCreateInfo queue_create_info = {0};
  queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_create_info.queueFamilyIndex = vulkan_ctx.queue_family_idx;
  queue_create_info.queueCount = 1;
  queue_create_info.pQueuePriorities = &queue_priorities;

  VkDeviceCreateInfo device_create_info = {0};
  device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_create_info.queueCreateInfoCount = 1;
  device_create_info.queueCreateInfos = &queue_create_info;
  if (vulkan_ctx.api.vkCreateDevice(vulkan_ctx.physical_device, &device_create_info, 0, &vulkan_ctx.device) != VK_SUCCESS)
  {
    InvalidPath;
  }
  vulkan_api_load_device_functions(&vulkan_ctx.api, vulkan_ctx.device);

  VkCommandPoolCreateInfo cmd_pool_create_info = {0};
  cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cmd_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  cmd_pool_create_info.queueFamilyIndex = vulkan_ctx.queue_family_idx;
  if (vulkan_ctx.api.vkCreateCommandPool(vulkan_ctx.device, &cmd_pool_create_info, 0, &vulkan_ctx.command_pool) != VK_SUCCESS)
  {
    InvalidPath;
  }

  VkCommandBufferAllocateInfo cmd_buffer_allocate_info = {0};
  cmd_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmd_buffer_allocate_info.commandPool = vulkan_ctx.command_pool;
  cmd_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cmd_buffer_allocate_info.bufferCount = 1;
  VkCommandBuffer command_buffer = {0};
  if (vulkan_ctx.api.vkAllocateCommandBuffers(vulkan_ctx.device, &cmd_buffer_allocate_info, &command_buffer) != VK_SUCCESS)
  {
    InvalidPath;
  }
  VkFence compute_done_fence = vulkan_make_fence();

  VkBuffer shader_storage_buffer =
    vulkan_create_buffer(KB(4), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT|VK_BUFFER_USAGE_TRANSFER_DST_BIT);
  VkDeviceMemory shader_storage_buffer_memory =
    vulkan_allocate_device_memory_from_buffer(
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        shader_storage_buffer);
  if (vulkan_ctx.api.vkBindBufferMemory(
      vulkan_ctx.device, shader_storage_buffer, shader_storage_buffer_memory, 0) != VK_SUCCESS)
  {
    InvalidPath;
  }

  void* mapped_device_memory = {0};
  if (vulkan_ctx.api.vkMapMemory(vulkan_ctx.device, shader_storage_buffer_memory, 0, VK_WHOLE_SIZE, 0, &mapped_device_memory) != VK_SUCCESS)
  {
    InvalidPath;
  }
  S32* mapped_device_memory_ints = (S32*)mapped_device_memory;
  for (U64 byte_idx=0;
       byte_idx < expression_str.len;
       ++byte_idx)
  {
    *mapped_device_memory_ints++ = (S32)expression_str.ptr[byte_idx];
  }

  VkDescriptorSetLayoutBinding descriptor_layout_binding = {0};
  descriptor_layout_binding.binding = 0;
  descriptor_layout_binding.descriptorCount = 1;
  descriptor_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptor_layout_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {0};
  descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptor_set_layout_create_info.bindingCount = 1;
  descriptor_set_layout_create_info.pBindings = &descriptor_layout_binding;

  VkDescriptorSetLayout compute_descriptor_set_layout = {0};
  if (vulkan_ctx.api.vkCreateDescriptorSetLayout(vulkan_ctx.device, &descriptor_set_layout_create_info, 0, &compute_descriptor_set_layout) != VK_SUCCESS)
  {
    InvalidPath;
  }

  VkDescriptorPoolSize descriptor_pool_size = {0};
  descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptor_pool_size.descriptorCount = 1;

  VkDescriptorPoolCreateInfo descriptor_pool_create_info = {0};
  descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptor_pool_create_info.maxSets = 1;
  descriptor_pool_create_info.poolSizeCount = 1;
  descriptor_pool_create_info.pPoolSizes = &descriptor_pool_size;

  VkDescriptorPool descriptor_pool = {0};
  if (vulkan_ctx.api.vkCreateDescriptorPool(vulkan_ctx.device, &descriptor_pool_create_info, 0, &descriptor_pool) != VK_SUCCESS)
  {
    InvalidPath;
  }

  VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {0};
  descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  descriptor_set_allocate_info.descriptorPool = descriptor_pool;
  descriptor_set_allocate_info.descriptorSetCount = 1;
  descriptor_set_allocate_info.pSetLayouts = &compute_descriptor_set_layout;

  VkDescriptorSet compute_descriptor_set = {0};
  if (vulkan_ctx.api.vkAllocateDescriptorSets(vulkan_ctx.device, &descriptor_set_allocate_info, &compute_descriptor_set) != VK_SUCCESS)
  {
    InvalidPath;
  }

  VkDescriptorBufferInfo descriptor_buffer_info = {0};
  descriptor_buffer_info.buffer = shader_storage_buffer;
  descriptor_buffer_info.offset = 0;
  descriptor_buffer_info.range = KB(4);

  VkWriteDescriptorSet write_descriptor_set = {0};
  write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write_descriptor_set.dstSet = compute_descriptor_set;
  write_descriptor_set.dstBinding = 0;
  write_descriptor_set.descriptorCount = 1;
  write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  write_descriptor_set.pBufferInfo = &descriptor_buffer_info;

  vulkan_ctx.api.vkUpdateDescriptorSets(vulkan_ctx.device, 1, &write_descriptor_set, 0, 0);

	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {0};
  pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_create_info.setLayoutCount = 1;
  pipeline_layout_create_info.pSetLayouts = &compute_descriptor_set_layout;

  VkPipelineLayout compute_pipeline_layout = {0};
  if (vulkan_ctx.api.vkCreatePipelineLayout(vulkan_ctx.device, &pipeline_layout_create_info, 0, &compute_pipeline_layout) != VK_SUCCESS)
  {
    InvalidPath;
  }

  File compute_shader =
    linux_read_entire_file(Str8Lit("compute.spv"));
  U64 compute_shader_memory_addr = (U64)(compute_shader.memory);
  if (compute_shader_memory_addr & 0x3)
  {
    AssertMessage("shader memory needs to be 4 byte aligned");
  }
  VkShaderModule compute_shader_module =
    vulkan_create_shader_module_from_file(compute_shader);

  VkPipelineShaderStageCreateInfo shader_stage_create_info = {0};
  shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stage_create_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  shader_stage_create_info.module = compute_shader_module;
  shader_stage_create_info.pName = "main";

  VkComputePipelineCreateInfo compute_pipeline_create_info = {0};
  compute_pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  compute_pipeline_create_info.stage = shader_stage_create_info;
  compute_pipeline_create_info.layout = compute_pipeline_layout;

  VkPipeline compute_pipeline = {0};
  if (vulkan_ctx.api.vkCreateComputePipelines(vulkan_ctx.device, 0, 1, &compute_pipeline_create_info, 0, &compute_pipeline) != VK_SUCCESS)
  {
    InvalidPath;
  }

  VkCommandBufferBeginInfo command_buffer_begin_info = {0};
  command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vulkan_ctx.api.vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info) != VK_SUCCESS)
  {
    InvalidPath;
  }

  vulkan_ctx.api.vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline);
  vulkan_ctx.api.vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline_layout, 0, 1, &compute_descriptor_set, 0, 0);
  vulkan_ctx.api.vkCmdDispatch(command_buffer, 1, 1, 1);

  if (vulkan_ctx.api.vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
  {
    InvalidPath;
  }

  VkSubmitInfo submit_info = {0};
  VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.pWaitDstStageMask = &wait_dst_stage_mask;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer;

  vulkan_ctx.api.vkResetFences(vulkan_ctx.device, 1, &compute_done_fence);

  VkQueue compute_queue = {0};
  vulkan_ctx.api.vkGetDeviceQueue(vulkan_ctx.device, vulkan_ctx.queue_family_idx, 0, &compute_queue);
  if (vulkan_ctx.api.vkQueueSubmit(compute_queue, 1, &submit_info, compute_done_fence) != VK_SUCCESS)
  {
    InvalidPath;
  }

  if (vulkan_ctx.api.vkWaitForFences(vulkan_ctx.device, 1, &compute_done_fence, VK_TRUE, (U64)-1) != VK_SUCCESS)
  {
    InvalidPath;
  }

  S32* out_ints = (S32*)mapped_device_memory + 512;
#if 0
  for (U64 row_idx=0; row_idx < 10; ++row_idx)
  {
    for (U64 col_idx=0; col_idx < 10; ++col_idx)
    {
      printf("0x%05x, ", out_ints[row_idx*10 + col_idx]);
    }
    printf("\n");
  }
#else
  printf("%d\n", *out_ints);
#endif
}

