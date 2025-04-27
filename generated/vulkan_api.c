//
// vulkan_api.c
//
// Caleb Barger
// 4/23/2025
// DON'T MODIFY. THIS CODE WAS GENERATED WITH linux_code_gen.c
//

typedef PFN_vkVoidFunction VKGetInstanceProcAddr(VkInstance instance, const char* p_name);
global VKGetInstanceProcAddr* vkGetInstanceProcAddr = 0;
global PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr = 0;

internal void
vulkan_api_load_get_instance_proc_addr(LoadFunction* platform_load_function, void* vulkan_code)
{
  vkGetInstanceProcAddr = (VKGetInstanceProcAddr*)platform_load_function(vulkan_code, "vkGetInstanceProcAddr");
}

internal void
vulkan_api_load_global_functions(LoadFunction* platform_load_function, VulkanAPI* api, void* vulkan_code)
{
  if (!vkGetInstanceProcAddr)
  {
    vulkan_api_load_get_instance_proc_addr(platform_load_function, vulkan_code);
  }
  api->vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(0,"vkCreateInstance");
  api->vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)vkGetInstanceProcAddr(0,"vkEnumerateInstanceExtensionProperties");
  api->vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)vkGetInstanceProcAddr(0,"vkEnumerateInstanceLayerProperties");
}

internal void
vulkan_api_load_instance_functions(LoadFunction* platform_load_function, VulkanAPI* api, void* vulkan_code, VkInstance instance)
{
  if (!vkGetInstanceProcAddr)
  {
    vulkan_api_load_get_instance_proc_addr(platform_load_function, vulkan_code);
  }
  vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)vkGetInstanceProcAddr(instance, "vkGetDeviceProcAddr");

  api->vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)vkGetInstanceProcAddr(instance,"vkEnumeratePhysicalDevices");
  api->vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)vkGetInstanceProcAddr(instance,"vkGetPhysicalDeviceProperties");
  api->vkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures)vkGetInstanceProcAddr(instance,"vkGetPhysicalDeviceFeatures");
  api->vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)vkGetInstanceProcAddr(instance,"vkGetPhysicalDeviceQueueFamilyProperties");
  api->vkCreateDevice = (PFN_vkCreateDevice)vkGetInstanceProcAddr(instance,"vkCreateDevice");
  api->vkDestroyInstance = (PFN_vkDestroyInstance)vkGetInstanceProcAddr(instance,"vkDestroyInstance");
  api->vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties)vkGetInstanceProcAddr(instance,"vkEnumerateDeviceExtensionProperties");
  api->vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)vkGetInstanceProcAddr(instance,"vkDestroySurfaceKHR");
  api->vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)vkGetInstanceProcAddr(instance,"vkGetPhysicalDeviceSurfaceSupportKHR");
  api->vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)vkGetInstanceProcAddr(instance,"vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
  api->vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)vkGetInstanceProcAddr(instance,"vkGetPhysicalDeviceSurfaceFormatsKHR");
  api->vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)vkGetInstanceProcAddr(instance,"vkGetPhysicalDeviceSurfacePresentModesKHR");
  api->vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)vkGetInstanceProcAddr(instance,"vkGetPhysicalDeviceMemoryProperties");
}

internal void
vulkan_api_load_device_functions(VulkanAPI* api, VkDevice device)
{
  api->vkGetDeviceQueue = (PFN_vkGetDeviceQueue)vkGetDeviceProcAddr(device,"vkGetDeviceQueue");
  api->vkDestroyDevice = (PFN_vkDestroyDevice)vkGetDeviceProcAddr(device,"vkDestroyDevice");
  api->vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle)vkGetDeviceProcAddr(device,"vkDeviceWaitIdle");
  api->vkCreateSemaphore = (PFN_vkCreateSemaphore)vkGetDeviceProcAddr(device,"vkCreateSemaphore");
  api->vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)vkGetDeviceProcAddr(device,"vkCreateSwapchainKHR");
  api->vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)vkGetDeviceProcAddr(device,"vkDestroySwapchainKHR");
  api->vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)vkGetDeviceProcAddr(device,"vkGetSwapchainImagesKHR");
  api->vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)vkGetDeviceProcAddr(device,"vkAcquireNextImageKHR");
  api->vkQueuePresentKHR = (PFN_vkQueuePresentKHR)vkGetDeviceProcAddr(device,"vkQueuePresentKHR");
  api->vkCreateCommandPool = (PFN_vkCreateCommandPool)vkGetDeviceProcAddr(device,"vkCreateCommandPool");
  api->vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)vkGetDeviceProcAddr(device,"vkAllocateCommandBuffers");
  api->vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer)vkGetDeviceProcAddr(device,"vkBeginCommandBuffer");
  api->vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier)vkGetDeviceProcAddr(device,"vkCmdPipelineBarrier");
  api->vkCmdClearColorImage = (PFN_vkCmdClearColorImage)vkGetDeviceProcAddr(device,"vkCmdClearColorImage");
  api->vkCmdSetViewport = (PFN_vkCmdSetViewport)vkGetDeviceProcAddr(device,"vkCmdSetViewport");
  api->vkCmdSetScissor = (PFN_vkCmdSetScissor)vkGetDeviceProcAddr(device,"vkCmdSetScissor");
  api->vkCmdDraw = (PFN_vkCmdDraw)vkGetDeviceProcAddr(device,"vkCmdDraw");
  api->vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed)vkGetDeviceProcAddr(device,"vkCmdDrawIndexed");
  api->vkEndCommandBuffer = (PFN_vkEndCommandBuffer)vkGetDeviceProcAddr(device,"vkEndCommandBuffer");
  api->vkQueueSubmit = (PFN_vkQueueSubmit)vkGetDeviceProcAddr(device,"vkQueueSubmit");
  api->vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers)vkGetDeviceProcAddr(device,"vkFreeCommandBuffers");
  api->vkDestroyCommandPool = (PFN_vkDestroyCommandPool)vkGetDeviceProcAddr(device,"vkDestroyCommandPool");
  api->vkResetCommandPool = (PFN_vkResetCommandPool)vkGetDeviceProcAddr(device,"vkResetCommandPool");
  api->vkDestroySemaphore = (PFN_vkDestroySemaphore)vkGetDeviceProcAddr(device,"vkDestroySemaphore");
  api->vkCreateFence = (PFN_vkCreateFence)vkGetDeviceProcAddr(device,"vkCreateFence");
  api->vkResetFences = (PFN_vkResetFences)vkGetDeviceProcAddr(device,"vkResetFences");
  api->vkGetFenceStatus = (PFN_vkGetFenceStatus)vkGetDeviceProcAddr(device,"vkGetFenceStatus");
  api->vkWaitForFences = (PFN_vkWaitForFences)vkGetDeviceProcAddr(device,"vkWaitForFences");
  api->vkCreateShaderModule = (PFN_vkCreateShaderModule)vkGetDeviceProcAddr(device,"vkCreateShaderModule");
  api->vkDestroyShaderModule = (PFN_vkDestroyShaderModule)vkGetDeviceProcAddr(device,"vkDestroyShaderModule");
  api->vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout)vkGetDeviceProcAddr(device,"vkCreatePipelineLayout");
  api->vkCreateRenderPass = (PFN_vkCreateRenderPass)vkGetDeviceProcAddr(device,"vkCreateRenderPass");
  api->vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines)vkGetDeviceProcAddr(device,"vkCreateGraphicsPipelines");
  api->vkCreateImageView = (PFN_vkCreateImageView)vkGetDeviceProcAddr(device,"vkCreateImageView");
  api->vkCreateImage = (PFN_vkCreateImage)vkGetDeviceProcAddr(device,"vkCreateImage");
  api->vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)vkGetDeviceProcAddr(device,"vkCmdBeginRenderPass");
  api->vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass)vkGetDeviceProcAddr(device,"vkCmdEndRenderPass");
  api->vkCmdBindPipeline = (PFN_vkCmdBindPipeline)vkGetDeviceProcAddr(device,"vkCmdBindPipeline");
  api->vkCreateFramebuffer = (PFN_vkCreateFramebuffer)vkGetDeviceProcAddr(device,"vkCreateFramebuffer");
  api->vkDestroyFramebuffer = (PFN_vkDestroyFramebuffer)vkGetDeviceProcAddr(device,"vkDestroyFramebuffer");
  api->vkDestroyImageView = (PFN_vkDestroyImageView)vkGetDeviceProcAddr(device,"vkDestroyImageView");
  api->vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers)vkGetDeviceProcAddr(device,"vkCmdBindVertexBuffers");
  api->vkCreateBuffer = (PFN_vkCreateBuffer)vkGetDeviceProcAddr(device,"vkCreateBuffer");
  api->vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)vkGetDeviceProcAddr(device,"vkGetBufferMemoryRequirements");
  api->vkAllocateMemory = (PFN_vkAllocateMemory)vkGetDeviceProcAddr(device,"vkAllocateMemory");
  api->vkBindBufferMemory = (PFN_vkBindBufferMemory)vkGetDeviceProcAddr(device,"vkBindBufferMemory");
  api->vkBindImageMemory = (PFN_vkBindImageMemory)vkGetDeviceProcAddr(device,"vkBindImageMemory");
  api->vkMapMemory = (PFN_vkMapMemory)vkGetDeviceProcAddr(device,"vkMapMemory");
  api->vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer)vkGetDeviceProcAddr(device,"vkCmdBindIndexBuffer");
  api->vkCreateComputePipelines = (PFN_vkCreateComputePipelines)vkGetDeviceProcAddr(device,"vkCreateComputePipelines");
  api->vkCreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout)vkGetDeviceProcAddr(device,"vkCreateDescriptorSetLayout");
  api->vkCreateDescriptorPool = (PFN_vkCreateDescriptorPool)vkGetDeviceProcAddr(device,"vkCreateDescriptorPool");
  api->vkAllocateDescriptorSets = (PFN_vkAllocateDescriptorSets)vkGetDeviceProcAddr(device,"vkAllocateDescriptorSets");
  api->vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets)vkGetDeviceProcAddr(device,"vkUpdateDescriptorSets");
  api->vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets)vkGetDeviceProcAddr(device,"vkCmdBindDescriptorSets");
  api->vkCmdDispatch = (PFN_vkCmdDispatch)vkGetDeviceProcAddr(device,"vkCmdDispatch");
}

