//
// caffe_vulkan.h
//
// Caleb Barger
// 02/15/2025
//

#ifndef CAFFE_VULKAN_H

typedef struct VulkanContext VulkanContext;
struct VulkanContext
{
  Arena* arena;
  VulkanAPI api;

  VkInstance instance;
  VkDevice device;
  VkPhysicalDevice physical_device;
  VkSurfaceKHR surface;
  VkSurfaceFormatKHR surface_format;

  U32 queue_family_idx;

	VkSwapchainKHR swapchain;
  VkExtent2D swapchain_extent;
  VkImage swapchain_images[4];
  VkImageView swapchain_image_views[4];
  VkFramebuffer swapchain_framebuffers[4];

  VkPipeline graphics_pipeline;
  VkRenderPass render_pass;

  VkImage texture_image; // NOTE(calebarg): Texture profile image.

  U8* mapped_device_memory;
  VkBuffer device_buffer;
  VkDeviceMemory device_buffer_memory;

  U32 swapchain_frame_idx;
  VkCommandPool command_pool;
  VkCommandBuffer command_buffers[2];
	VkFence frames_in_flight_fences[2];
  VkSemaphore image_available_semaphores[2];
  VkSemaphore rendering_finished_semaphores[2];
};

#define CAFFE_VULKAN_H
#endif
