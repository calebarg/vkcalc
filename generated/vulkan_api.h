//
// vulkan_api.h
//
// Caleb Barger
// 4/23/2025
// DON'T MODIFY. THIS CODE WAS GENERATED WITH linux_code_gen.c
//

#ifndef VULKAN_API_H

typedef struct VulkanAPI VulkanAPI;
struct VulkanAPI {

  // Global functions
  PFN_vkCreateInstance vkCreateInstance;
  PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
  PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;

  // Instance functions
  PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
  PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
  PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
  PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
  PFN_vkCreateDevice vkCreateDevice;
  PFN_vkDestroyInstance vkDestroyInstance;
  PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
  PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
  PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
  PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
  PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
  PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
  PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;

  // Device functions
  PFN_vkGetDeviceQueue vkGetDeviceQueue;
  PFN_vkDestroyDevice vkDestroyDevice;
  PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
  PFN_vkCreateSemaphore vkCreateSemaphore;
  PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
  PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
  PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
  PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
  PFN_vkQueuePresentKHR vkQueuePresentKHR;
  PFN_vkCreateCommandPool vkCreateCommandPool;
  PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
  PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
  PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
  PFN_vkCmdClearColorImage vkCmdClearColorImage;
  PFN_vkCmdSetViewport vkCmdSetViewport;
  PFN_vkCmdSetScissor vkCmdSetScissor;
  PFN_vkCmdDraw vkCmdDraw;
  PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
  PFN_vkEndCommandBuffer vkEndCommandBuffer;
  PFN_vkQueueSubmit vkQueueSubmit;
  PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
  PFN_vkDestroyCommandPool vkDestroyCommandPool;
  PFN_vkResetCommandPool vkResetCommandPool;
  PFN_vkDestroySemaphore vkDestroySemaphore;
  PFN_vkCreateFence vkCreateFence;
  PFN_vkResetFences vkResetFences;
  PFN_vkGetFenceStatus vkGetFenceStatus;
  PFN_vkWaitForFences vkWaitForFences;
  PFN_vkCreateShaderModule vkCreateShaderModule;
  PFN_vkDestroyShaderModule vkDestroyShaderModule;
  PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
  PFN_vkCreateRenderPass vkCreateRenderPass;
  PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
  PFN_vkCreateImageView vkCreateImageView;
  PFN_vkCreateImage vkCreateImage;
  PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
  PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
  PFN_vkCmdBindPipeline vkCmdBindPipeline;
  PFN_vkCreateFramebuffer vkCreateFramebuffer;
  PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
  PFN_vkDestroyImageView vkDestroyImageView;
  PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
  PFN_vkCreateBuffer vkCreateBuffer;
  PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
  PFN_vkAllocateMemory vkAllocateMemory;
  PFN_vkBindBufferMemory vkBindBufferMemory;
  PFN_vkBindImageMemory vkBindImageMemory;
  PFN_vkMapMemory vkMapMemory;
  PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
  PFN_vkCreateComputePipelines vkCreateComputePipelines;
  PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
  PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
  PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
  PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
  PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
  PFN_vkCmdDispatch vkCmdDispatch;
};

#define VULKAN_API_H
#endif