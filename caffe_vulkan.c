//
// caffe_vulkan.c
//
// Caleb Barger
// 03/01/2025
//

global VulkanContext* vulkan_local_ctx = 0;

global F32 positions_and_colors[] = {
  -0.5, -0.5, 1, 0, 0,
  0.5, -0.5, 0, 1, 0,
  0.5, 0.5, 0, 0, 1,
  -0.5, 0.5, 1, 0, 0,
};

global U32 position_and_color_indices[] = {
  0, 1, 2, 0, 2, 3,
};

internal String8List
vulkan_str8_list_from_extension_properties(
    Arena* arena, VkExtensionProperties* extension_properties, U32 count)
{
  String8List result = {0};
  for (U32 extension_idx = 0;
       extension_idx < count;
       ++extension_idx)
  {
    String8 extension_name =
      str8_from_mem((U8*)extension_properties[extension_idx].extensionName);
    str8_list_push(arena, &result, extension_name);
  }
  return result;
}

internal String8List
vulkan_str8_list_from_layer_properties(
    Arena* arena, VkLayerProperties* layer_properties, U32 count)
{
  String8List result = {0};
  for (U32 layer_idx = 0;
       layer_idx < count;
       ++layer_idx)
  {
    String8 layer_name =
      str8_from_mem((U8*)layer_properties[layer_idx].layerName);
    str8_list_push(arena, &result, layer_name);
  }
  return result;
}

internal B32
vulkan_cstrs_in_list(U8** needles, U32 needle_count, String8List list)
{
  U32 expected = (1 << needle_count) - 1;
  U32 result = 0;
  for (U32 needle_idx = 0;
       needle_idx < needle_count;
       ++needle_idx )
  {
    for (String8Node* curr=list.first;
         curr != 0;
         curr=curr->next)
    {
      if (str8_eql(str8_from_mem(needles[needle_idx]), curr->str))
      {
        result |= (1 << needle_idx);
      }
    }
  }
  return (B32)(result == expected);
}

internal void
vulkan_equip_ctx(VulkanContext* ctx)
{
  vulkan_local_ctx = ctx;
}

internal void
vulkan_create_render_pass()
{
  VkAttachmentDescription color_attachment = {0};
  color_attachment.format = vulkan_local_ctx->surface_format.format;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_ref = {0};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {0};
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;

  VkRenderPassCreateInfo render_pass_info = {0};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = 1;
  render_pass_info.pAttachments = &color_attachment;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;

  if (vulkan_local_ctx->api.vkCreateRenderPass(vulkan_local_ctx->device, &render_pass_info, 0, &vulkan_local_ctx->render_pass) != VK_SUCCESS)
  {
    InvalidPath;
  }
}

internal VkShaderModule
vulkan_create_shader_module_from_file(File f)
{
  VkShaderModule result;
  VkShaderModuleCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = f.size;
  create_info.pCode = f.memory;
  if (vulkan_local_ctx->api.vkCreateShaderModule(vulkan_local_ctx->device, &create_info, 0, &result) != VK_SUCCESS)
  {
    InvalidPath;
  }
  return result;
}

internal VkSurfaceFormatKHR
vulkan_pick_surface_format()
{
  VkSurfaceFormatKHR surface_format = {0}; // Surface format from physical device and surface
  {
    Temp temp = scratch_begin(vulkan_local_ctx->arena);
    U32 formats_count;
    if ((vulkan_local_ctx->api.vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_local_ctx->physical_device, vulkan_local_ctx->surface, &formats_count, 0) != VK_SUCCESS) ||
        (formats_count == 0))
    {
      InvalidPath;
    }
    VkSurfaceFormatKHR* surface_formats =
      (VkSurfaceFormatKHR*)arena_push(temp.arena, sizeof(VkSurfaceFormatKHR)*formats_count);
    if (vulkan_local_ctx->api.vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan_local_ctx->physical_device, vulkan_local_ctx->surface, &formats_count, surface_formats) != VK_SUCCESS)
    {
      InvalidPath;
    }
    B32 choose_any_format = ((formats_count == 1) && (surface_formats[0].format == VK_FORMAT_UNDEFINED));
    if (choose_any_format)
    {
      surface_format = (VkSurfaceFormatKHR){VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR};
    }
    else
    {
      for (U32 surface_format_idx=0;
           surface_format_idx < formats_count;
           ++surface_format_idx)
      {
        VkSurfaceFormatKHR curr_surface_format = surface_formats[surface_format_idx];
        if (curr_surface_format.format == VK_FORMAT_R8G8B8A8_UNORM)
        {
          surface_format = curr_surface_format;
        }
      }
    }
    if (surface_format.format == VK_FORMAT_UNDEFINED)
    {
      surface_format = surface_formats[0];
    }
    scratch_end(temp);
  }
  return surface_format;
}


internal void
vulkan_create_framebuffers()
{
  for (U64 framebuffer_idx=0;
       framebuffer_idx < ArrayCount(vulkan_local_ctx->swapchain_framebuffers);
       ++framebuffer_idx)
  {
    VkImageView attachments[] = {
        vulkan_local_ctx->swapchain_image_views[framebuffer_idx]
    };

    VkFramebufferCreateInfo framebuffer_info = {0};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = vulkan_local_ctx->render_pass;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = attachments;
    framebuffer_info.width = vulkan_local_ctx->swapchain_extent.width;
    framebuffer_info.height = vulkan_local_ctx->swapchain_extent.height;
    framebuffer_info.layers = 1;

    if (vulkan_local_ctx->api.vkCreateFramebuffer(vulkan_local_ctx->device, &framebuffer_info, 0,
          &vulkan_local_ctx->swapchain_framebuffers[framebuffer_idx]) != VK_SUCCESS)
    {
      InvalidPath;
    }
  }
}

internal void
vulkan_create_graphics_pipeline(GameMemory* game_memory)
{
  VulkanAPI* vulkan_api = &vulkan_local_ctx->api;

  // NOTE(calebarg): These are already aligned (Make guarentees about what the
  // platform layer's read_entire_file hands back...)
  File triangle_vert_shader =
    game_memory->DEBUG_platform_read_entire_file(Str8Lit("vert.spv"));
  File triangle_frag_shader =
    game_memory->DEBUG_platform_read_entire_file(Str8Lit("frag.spv"));

  if (((U64)triangle_vert_shader.memory & 0x3) || ((U64)triangle_frag_shader.memory & 0x3))
  {
    AssertMessage("Expeted vertex and fragment shader(s) to be 4-byte aligned.");
  }

  VkShaderModule triangle_vert_shader_module =
    vulkan_create_shader_module_from_file(triangle_vert_shader);
  VkShaderModule triangle_frag_shader_module =
    vulkan_create_shader_module_from_file(triangle_frag_shader);

  VkPipelineShaderStageCreateInfo vert_shader_stage_info = {0};
  vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vert_shader_stage_info.module = triangle_vert_shader_module;
  vert_shader_stage_info.pName = "main";

  VkPipelineShaderStageCreateInfo frag_shader_stage_info = {0};
  frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  frag_shader_stage_info.module = triangle_frag_shader_module;
  frag_shader_stage_info.pName = "main";

  VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

  VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamic_state = {0};
  dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state.dynamicStateCount = ArrayCount(dynamic_states);
  dynamic_state.pDynamicStates = dynamic_states;

  VkVertexInputBindingDescription binding_desc = {0};
  binding_desc.binding = 0;
  binding_desc.stride = sizeof(F32)*5;
  binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VkVertexInputAttributeDescription attribute_descs[2] = {0};
  attribute_descs[0].binding = 0;
  attribute_descs[0].location = 0;
  attribute_descs[0].format = VK_FORMAT_R32G32_SFLOAT;
  attribute_descs[0].offset = 0;

  attribute_descs[1].binding = 0;
  attribute_descs[1].location = 1;
  attribute_descs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  attribute_descs[1].offset = sizeof(Vec2F32);

  VkPipelineVertexInputStateCreateInfo vertex_input_info = {0};
  vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.vertexBindingDescriptionCount = 1;
  vertex_input_info.pVertexBindingDescriptions = &binding_desc;
  vertex_input_info.vertexAttributeDescriptionCount = ArrayCount(attribute_descs);
  vertex_input_info.pVertexAttributeDescriptions = attribute_descs;

  VkPipelineInputAssemblyStateCreateInfo input_assembly = {0};
  input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewport_state = {0};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo rasterizer = {0};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode  = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f;
  rasterizer.depthBiasClamp = 0.0f;
  rasterizer.depthBiasSlopeFactor = 0.0f;

  VkPipelineLayout pipeline_layout = {0};
  VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 0;
  pipeline_layout_info.pSetLayouts = 0;
  pipeline_layout_info.pushConstantRangeCount = 0;
  pipeline_layout_info.pPushConstantRanges = 0;
  if (vulkan_api->vkCreatePipelineLayout(vulkan_local_ctx->device, &pipeline_layout_info, 0, &pipeline_layout) != VK_SUCCESS)
  {
    InvalidPath;
  }

  VkPipelineMultisampleStateCreateInfo multisampling = {0};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;
  multisampling.pSampleMask = 0;
  multisampling.alphaToCoverageEnable = VK_FALSE;
  multisampling.alphaToOneEnable = VK_FALSE;

  VkPipelineColorBlendAttachmentState color_blend_attachment = {0};
  color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT;
  color_blend_attachment.blendEnable = VK_TRUE;
  color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
  color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo color_blending = {0};
  color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blending.logicOpEnable = VK_FALSE;
  color_blending.logicOp = VK_LOGIC_OP_COPY;
  color_blending.attachmentCount = 1;
  color_blending.pAttachments = &color_blend_attachment;
  color_blending.blendConstants[0] = 0.0f;
  color_blending.blendConstants[1] = 0.0f;
  color_blending.blendConstants[2] = 0.0f;
  color_blending.blendConstants[3] = 0.0f;

  VkGraphicsPipelineCreateInfo pipeline_info = {0};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = 2;
  pipeline_info.pStages = shader_stages;
  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &input_assembly;
  pipeline_info.pViewportState = &viewport_state;
  pipeline_info.pRasterizationState = &rasterizer;
  pipeline_info.pMultisampleState = &multisampling;
  pipeline_info.pDepthStencilState = 0;
  pipeline_info.pColorBlendState = &color_blending;
  pipeline_info.pDynamicState = &dynamic_state;
  pipeline_info.layout = pipeline_layout;
  pipeline_info.renderPass = vulkan_local_ctx->render_pass;
  pipeline_info.subpass = 0;
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
  pipeline_info.basePipelineIndex = -1;

  if (vulkan_api->vkCreateGraphicsPipelines(vulkan_local_ctx->device, VK_NULL_HANDLE, 1, &pipeline_info, 0, &vulkan_local_ctx->graphics_pipeline) != VK_SUCCESS)
  {
    InvalidPath;
  }

  vulkan_api->vkDestroyShaderModule(vulkan_local_ctx->device, triangle_frag_shader_module, 0);
  vulkan_api->vkDestroyShaderModule(vulkan_local_ctx->device, triangle_vert_shader_module, 0);
}

internal void
vulkan_record_draw_triangle(U32 image_idx)
{
  VkCommandBuffer cmd_buffer = vulkan_local_ctx->command_buffers[vulkan_local_ctx->swapchain_frame_idx];
  VkImage image = vulkan_local_ctx->swapchain_images[image_idx];

   VkCommandBufferBeginInfo cmd_buffer_begin_info = {
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,  // VkStructureType                        sType
    0,                                            // const void                            *pNext
    0,                                            // VkCommandBufferUsageFlags              flags
    0                                             // const VkCommandBufferInheritanceInfo  *pInheritanceInfo
  };
  if (vulkan_local_ctx->api.vkBeginCommandBuffer(cmd_buffer, &cmd_buffer_begin_info) != VK_SUCCESS)
  {
    InvalidPath;
  }

  VkRenderPassBeginInfo render_pass_info = {0};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass = vulkan_local_ctx->render_pass;
  render_pass_info.framebuffer = vulkan_local_ctx->swapchain_framebuffers[image_idx];
  render_pass_info.renderArea.extent = vulkan_local_ctx->swapchain_extent;

  VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
  render_pass_info.clearValueCount = 1;
  render_pass_info.pClearValues = &clear_color;

  vulkan_local_ctx->api.vkCmdBeginRenderPass(cmd_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

  vulkan_local_ctx->api.vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_local_ctx->graphics_pipeline);

  VkBuffer vertex_buffers[] = {vulkan_local_ctx->device_buffer};
  VkDeviceSize offsets[] = {0};
  vulkan_local_ctx->api.vkCmdBindVertexBuffers(cmd_buffer, 0, 1, vertex_buffers, offsets);

  vulkan_local_ctx->api.vkCmdBindIndexBuffer(cmd_buffer, vulkan_local_ctx->device_buffer,
      (VkDeviceSize)(KB(1)), VK_INDEX_TYPE_UINT32);

  // NOTE(calebarg): Since these are dynamic state fields. These are passed
  // to pipeline during each draw call.
  VkViewport viewport = {0};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (F32)vulkan_local_ctx->swapchain_extent.width;
  viewport.height = (F32)vulkan_local_ctx->swapchain_extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vulkan_local_ctx->api.vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

  VkRect2D scissor = {0};
  scissor.extent = vulkan_local_ctx->swapchain_extent;
  vulkan_local_ctx->api.vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

  vulkan_local_ctx->api.vkCmdDrawIndexed(cmd_buffer, (U32)(ArrayCount(position_and_color_indices)), 1, 0, 0, 0);

  vulkan_local_ctx->api.vkCmdEndRenderPass(cmd_buffer);

  if (vulkan_local_ctx->api.vkEndCommandBuffer(cmd_buffer) != VK_SUCCESS)
  {
    InvalidPath;
  }
}

internal void
vulkan_record_clear_color_command(U32 image_idx)
{
  VkCommandBuffer cmd_buffer = vulkan_local_ctx->command_buffers[vulkan_local_ctx->swapchain_frame_idx];
  VkImage image = vulkan_local_ctx->swapchain_images[image_idx];

  VkCommandBufferBeginInfo cmd_buffer_begin_info = {
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,  // VkStructureType                        sType
    0,                                            // const void                            *pNext
    VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, // VkCommandBufferUsageFlags              flags
    0                                             // const VkCommandBufferInheritanceInfo  *pInheritanceInfo
  };
  VkClearColorValue clear_color = {
    { 1.0f, 0.0f, 0.0f, 1.0f }
  };
  VkImageSubresourceRange image_subresource_range = {
    VK_IMAGE_ASPECT_COLOR_BIT,                    // VkImageAspectFlags                     aspectMask
    0,                                            // uint32_t                               baseMipLevel
    1,                                            // uint32_t                               levelCount
    0,                                            // uint32_t                               baseArrayLayer
    1                                             // uint32_t                               layerCount
  };
  VkImageMemoryBarrier barrier_from_present_to_clear = {
    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType                        sType
    0,                                          // const void                            *pNext
    VK_ACCESS_MEMORY_READ_BIT,                  // VkAccessFlags                          srcAccessMask
    VK_ACCESS_TRANSFER_WRITE_BIT,               // VkAccessFlags                          dstAccessMask
    VK_IMAGE_LAYOUT_UNDEFINED,                  // VkImageLayout                          oldLayout
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,       // VkImageLayout                          newLayout
    vulkan_local_ctx->queue_family_idx,         // uint32_t                               srcQueueFamilyIndex
    vulkan_local_ctx->queue_family_idx,         // uint32_t                               dstQueueFamilyIndex
    image,                                      // VkImage                                image
    image_subresource_range                     // VkImageSubresourceRange                subresourceRange
  };
  VkImageMemoryBarrier barrier_from_clear_to_present = {
    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType                        sType
    0,                                    			// const void                            *pNext
    VK_ACCESS_TRANSFER_WRITE_BIT,               // VkAccessFlags                          srcAccessMask
    VK_ACCESS_MEMORY_READ_BIT,                  // VkAccessFlags                          dstAccessMask
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,       // VkImageLayout                          oldLayout
    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,            // VkImageLayout                          newLayout
    vulkan_local_ctx->queue_family_idx,         // uint32_t                               srcQueueFamilyIndex
    vulkan_local_ctx->queue_family_idx,         // uint32_t                               dstQueueFamilyIndex
    image,                                      // VkImage                                image
    image_subresource_range                     // VkImageSubresourceRange                subresourceRange
  };
  if (vulkan_local_ctx->api.vkBeginCommandBuffer(cmd_buffer, &cmd_buffer_begin_info) != VK_SUCCESS)
  {
    InvalidPath;
  }
  vulkan_local_ctx->api.vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 1, 0, 0, 0, 0, 1, &barrier_from_present_to_clear);
  vulkan_local_ctx->api.vkCmdClearColorImage(cmd_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 1, &image_subresource_range);
  vulkan_local_ctx->api.vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0, 0, 0, 1, &barrier_from_clear_to_present);
  if (vulkan_local_ctx->api.vkEndCommandBuffer(cmd_buffer) != VK_SUCCESS)
  {
    InvalidPath;
  }
}

internal VkSemaphore
vulkan_make_semaphore()
{
	VkSemaphore result;
	VkSemaphoreCreateInfo semaphore_create_info = {0};
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vulkan_local_ctx->api.vkCreateSemaphore(vulkan_local_ctx->device, &semaphore_create_info, 0, &result) != VK_SUCCESS)
	{
    InvalidPath;
	}
	return result;
}

internal VkFence
vulkan_make_fence()
{
	VkFence result;
	VkFenceCreateInfo fence_create_info = {0};
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	if (vulkan_local_ctx->api.vkCreateFence(vulkan_local_ctx->device, &fence_create_info, 0, &result) != VK_SUCCESS)
	{
    InvalidPath;
	}
	return result;
}

internal void
vulkan_create_image_views()
{
  for (U64 swapchain_image_idx=0;
       swapchain_image_idx < ArrayCount(vulkan_local_ctx->swapchain_images);
       ++swapchain_image_idx)
  {
    VkImageViewCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = vulkan_local_ctx->swapchain_images[swapchain_image_idx];
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = vulkan_local_ctx->surface_format.format;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;

    if (vulkan_local_ctx->api.vkCreateImageView(vulkan_local_ctx->device, &create_info, 0,
        &vulkan_local_ctx->swapchain_image_views[swapchain_image_idx]) != VK_SUCCESS)
    {
      InvalidPath;
    }
  }
}

internal void
vulkan_create_swapchain(U32 width, U32 height)
{
  VulkanAPI* vulkan_api = &vulkan_local_ctx->api;

  if (vulkan_local_ctx->api.vkDeviceWaitIdle(vulkan_local_ctx->device) != VK_SUCCESS)
  {
    InvalidPath;
  }

  if (vulkan_local_ctx->swapchain_framebuffers[0])
  {
    for (U64 framebuffer_idx=0;
      framebuffer_idx < ArrayCount(vulkan_local_ctx->swapchain_framebuffers);
      ++framebuffer_idx)
    {
      vulkan_api->vkDestroyFramebuffer(vulkan_local_ctx->device,
          vulkan_local_ctx->swapchain_framebuffers[framebuffer_idx], 0);
    }
  }
  if (vulkan_local_ctx->swapchain_images[0])
  {
    for (U64 image_view_idx=0;
         image_view_idx < ArrayCount(vulkan_local_ctx->swapchain_image_views);
         ++image_view_idx)
    {
      vulkan_api->vkDestroyImageView(vulkan_local_ctx->device,
          vulkan_local_ctx->swapchain_image_views[image_view_idx], 0);
    }
  }

  VkSurfaceCapabilitiesKHR surface_capabilities;
  if (vulkan_local_ctx->api.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan_local_ctx->physical_device, vulkan_local_ctx->surface, &surface_capabilities) != VK_SUCCESS)
  {
    InvalidPath;
  }

  // Sometimes images must be transformed before they are presented (i.e. due to device's orienation
  // being other than default orientation)
  // If the specified transform is other than current transform, presentation engine will transform image
  // during presentation operation; this operation may hit performance on some platforms
  // Here we don't want any transformations to occur so if the identity transform is supported use it
  // otherwise just use the same transform as current transform
  VkSurfaceTransformFlagBitsKHR surface_transform = {0};
  if (surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
  {
    surface_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  }
  else
  {
    surface_transform = surface_capabilities.currentTransform;
  }

  VkPresentModeKHR present_mode = {0};
  {
    Temp temp = scratch_begin(vulkan_local_ctx->arena);
    U32 present_modes_count;
    if ((vulkan_local_ctx->api.vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan_local_ctx->physical_device, vulkan_local_ctx->surface, &present_modes_count, 0) != VK_SUCCESS) ||
        (present_modes_count == 0))
    {
      InvalidPath;
    }
    VkPresentModeKHR* present_modes = ArenaPushArray(temp.arena, VkPresentModeKHR, present_modes_count);
    if (vulkan_local_ctx->api.vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan_local_ctx->physical_device, vulkan_local_ctx->surface, &present_modes_count, present_modes) != VK_SUCCESS)
    {
      InvalidPath;
    }
    for (U32 present_mode_idx=0;
        present_mode_idx < present_modes_count;
        ++present_mode_idx)
    {
      VkPresentModeKHR curr_present_mode = present_modes[present_mode_idx];
      if (curr_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) // Prefer mailbox mode.
      {
        present_mode = curr_present_mode;
        break;
      }
    }
    if (present_mode != VK_PRESENT_MODE_MAILBOX_KHR)
    {
      present_mode = VK_PRESENT_MODE_FIFO_KHR; // Fallback to FIFO if mailbox is not supported.
    }
    scratch_end(temp);
  }

  // Set of images defined in a swap chain may not always be available for application to render to:
  // One may be displayed and one may wait in a queue to be presented
  // If application wants to use more images at the same time it must ask for more images
  U32 image_count = surface_capabilities.minImageCount + 1;
  if ((surface_capabilities.maxImageCount > 0) &&
      (image_count > surface_capabilities.maxImageCount))
  {
    image_count = surface_capabilities.maxImageCount;
  }
  if (ArrayCount(vulkan_local_ctx->swapchain_images) < image_count)
  {
    InvalidPath;
  }

  // Color attachment flag must always be supported
  // We can define other usage flags but we always need to check if they are supported
  if (!(surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
  {
    AssertMessage("VK_IMAGE_USAGE_TRANSFER_DST image usage is not supported by the swap chain!");
  }
  VkImageUsageFlags image_usage_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

  // Special value of surface extent is width == height == -1
  // If this is so we define the size by ourselves but it must fit within defined confines
  if (surface_capabilities.currentExtent.width == -1)
  {
    vulkan_local_ctx->swapchain_extent = (VkExtent2D){width, height};
    if (vulkan_local_ctx->swapchain_extent.width < surface_capabilities.minImageExtent.width)
    {
      vulkan_local_ctx->swapchain_extent.width = surface_capabilities.minImageExtent.width;
    }
    if (vulkan_local_ctx->swapchain_extent.height < surface_capabilities.minImageExtent.height)
    {
      vulkan_local_ctx->swapchain_extent.height = surface_capabilities.minImageExtent.height;
    }
    if (vulkan_local_ctx->swapchain_extent.width > surface_capabilities.maxImageExtent.width)
    {
      vulkan_local_ctx->swapchain_extent.width = surface_capabilities.maxImageExtent.width;
    }
    if (vulkan_local_ctx->swapchain_extent.height > surface_capabilities.maxImageExtent.height)
    {
      vulkan_local_ctx->swapchain_extent.height = surface_capabilities.maxImageExtent.height;
    }
  }
  else
  {
    vulkan_local_ctx->swapchain_extent = surface_capabilities.currentExtent;
  }

  VkSwapchainKHR old_swapchain = vulkan_local_ctx->swapchain;
  VkSwapchainCreateInfoKHR swapchain_create_info = {
    VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,  // VkStructureType                sType
    0,                                        		// void*                       	 	pNext
    0,                                            // VkSwapchainCreateFlagsKHR      flags
    vulkan_local_ctx->surface,   						      // VkSurfaceKHR                   surface
    image_count,                     	  			 		// U32                            minImageCount
    vulkan_local_ctx->surface_format.format,      // VkFormat                       imageFormat
    vulkan_local_ctx->surface_format.colorSpace,  // VkColorSpaceKHR                imageColorSpace
    vulkan_local_ctx->swapchain_extent,           // VkExtent2D                     imageExtent
    1,                                            // U32   	                   		  imageArrayLayers
    image_usage_flags,                            // VkImageUsageFlags              imageUsage
    VK_SHARING_MODE_EXCLUSIVE,                    // VkSharingMode                  imageSharingMode
    0,                                            // U32                        		queueFamilyIndexCount
    0,                                      			// U32*               			  		pQueueFamilyIndices
    surface_transform,                            // VkSurfaceTransformFlagBitsKHR  preTransform
    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,            // VkCompositeAlphaFlagBitsKHR    compositeAlpha
    present_mode,                             		// VkPresentModeKHR               presentMode
    VK_TRUE,                                      // VkBool32                       clipped
    old_swapchain                                 // VkSwapchainKHR                 oldSwapchain
  };
  if (vulkan_local_ctx->api.vkCreateSwapchainKHR(vulkan_local_ctx->device, &swapchain_create_info, 0, &vulkan_local_ctx->swapchain) != VK_SUCCESS)
  {
    InvalidPath;
  }
  vulkan_local_ctx->api.vkDestroySwapchainKHR(vulkan_local_ctx->device, old_swapchain, 0);
  if (vulkan_local_ctx->api.vkGetSwapchainImagesKHR(vulkan_local_ctx->device, vulkan_local_ctx->swapchain, &image_count, vulkan_local_ctx->swapchain_images) != VK_SUCCESS)
  {
    InvalidPath;
  }

  vulkan_create_image_views();
  vulkan_create_framebuffers();
}

internal VkBuffer
vulkan_create_buffer(U64 size, VkBufferUsageFlags usage)
{
  VkBuffer result = {0};

  VkBufferCreateInfo buffer_info = {0};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  if (vulkan_local_ctx->api.vkCreateBuffer(vulkan_local_ctx->device, &buffer_info, 0, &result) != VK_SUCCESS)
  {
    InvalidPath;
  }

  return result;
}

internal VkDeviceMemory
vulkan_allocate_device_memory_from_buffer(VkMemoryPropertyFlags properties, VkBuffer buffer)
{
  VkDeviceMemory result = {0};

  U32 mem_type_idx = 0;
  VkMemoryRequirements mem_requirements;
  VkPhysicalDeviceMemoryProperties mem_properties;
  vulkan_local_ctx->api.vkGetBufferMemoryRequirements(vulkan_local_ctx->device, buffer, &mem_requirements);
  vulkan_local_ctx->api.vkGetPhysicalDeviceMemoryProperties(vulkan_local_ctx->physical_device, &mem_properties);
  VkMemoryPropertyFlags mem_property_flags = properties;
  for (;mem_type_idx < mem_properties.memoryTypeCount; ++mem_type_idx)
  {
    if ((mem_requirements.memoryTypeBits & (1 << mem_type_idx)) &&
        ((mem_properties.memoryTypes[mem_type_idx].propertyFlags & (mem_property_flags)) == mem_property_flags))
    {
      break;
    }
  }
  VkMemoryAllocateInfo alloc_info = {0};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = mem_requirements.size;
  alloc_info.memoryTypeIndex = mem_type_idx;
  if (vulkan_local_ctx->api.vkAllocateMemory(vulkan_local_ctx->device, &alloc_info, 0, &result) != VK_SUCCESS)
  {
    InvalidPath;
  }

  return result;
}

// TODO(calebarg) No need for this function
internal void
vulkan_bind_device_memory_to_buffer(VkBuffer buffer, VkDeviceMemory device_memory)
{
  vulkan_local_ctx->api.vkBindBufferMemory(vulkan_local_ctx->device, buffer, device_memory, 0);
}

internal void
vulkan_init_and_equip(GameMemory* game_memory, VulkanContext* ctx)
{
  vulkan_equip_ctx(ctx);

  vulkan_local_ctx->arena = arena_sub(game_memory->perm_arena, MB(1));
  vulkan_api_load_global_functions(game_memory->platform_load_function, &vulkan_local_ctx->api, game_memory->vulkan_code);

  U8* instance_extensions[] = {(U8*)VK_KHR_SURFACE_EXTENSION_NAME, game_memory->platform_vulkan_surface_extension_name()};
  U8* instance_layers[] = {
    (U8*)"VK_LAYER_KHRONOS_validation",
  };
  {
    Temp temp = scratch_begin(game_memory->scratch_arena);
    U32 extension_count = 0;
    if ((vulkan_local_ctx->api.vkEnumerateInstanceExtensionProperties(0, &extension_count, 0) == VK_SUCCESS) &&
        (extension_count != 0))
    {
      VkExtensionProperties* avail_extensions = ArenaPushArray(temp.arena, VkExtensionProperties, extension_count);
      if (vulkan_local_ctx->api.vkEnumerateInstanceExtensionProperties(0, &extension_count, avail_extensions) == VK_SUCCESS)
      {
        String8List avail_extension_names = vulkan_str8_list_from_extension_properties(temp.arena, avail_extensions, extension_count);
        if (!vulkan_cstrs_in_list(instance_extensions, ArrayCount(instance_extensions), avail_extension_names))
        {
          AssertMessage("Missing extension");
        }
      }
    }
    U32 layer_count = 0;
    if ((vulkan_local_ctx->api.vkEnumerateInstanceLayerProperties(&layer_count, 0) == VK_SUCCESS) &&
        (layer_count != 0))
    {
      VkLayerProperties* avail_layers = ArenaPushArray(temp.arena, VkLayerProperties, layer_count);
      if (vulkan_local_ctx->api.vkEnumerateInstanceLayerProperties(&layer_count, avail_layers) == VK_SUCCESS)
      {
        String8List avail_layer_names = vulkan_str8_list_from_layer_properties(temp.arena, avail_layers, layer_count);
        if (!vulkan_cstrs_in_list(instance_layers, ArrayCount(instance_layers), avail_layer_names))
        {
          AssertMessage("Missing layer");
        }
      }
    }
    scratch_end(temp);
  }
  VkApplicationInfo application_info = {
    VK_STRUCTURE_TYPE_APPLICATION_INFO,             // VkStructureType            sType
    0,                                              // const void                *pNext
    "caffeinated",                                  // const char                *pApplicationName
    VK_MAKE_VERSION( 1, 0, 0 ),                     // uint32_t                   applicationVersion
    "caffeinated engine",                           // const char                *pEngineName
    VK_MAKE_VERSION( 1, 0, 0 ),                     // uint32_t                   engineVersion
    VK_MAKE_VERSION(1, 0, 0),                       // uint32_t                   apiVersion
  };
  VkInstanceCreateInfo instance_create_info = {
    VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,         // VkStructureType            sType
    0,                                              // const void*                pNext
    0,                                              // VkInstanceCreateFlags      flags
    &application_info,                              // const VkApplicationInfo   *pApplicationInfo
    ArrayCount(instance_layers),                    // U32                        enabledLayerCount
    (const char**)instance_layers,                  // const char * const        *ppEnabledLayerNames
    ArrayCount(instance_extensions),                // U32                        enabledExtensionCount
    (const char**)instance_extensions               // const char * const        *ppEnabledExtensionNames
  };
  if (vulkan_local_ctx->api.vkCreateInstance(&instance_create_info, 0, &vulkan_local_ctx->instance) != VK_SUCCESS)
  {
    InvalidPath;
  }

  vulkan_api_load_instance_functions(game_memory->platform_load_function, &vulkan_local_ctx->api, game_memory->vulkan_code, vulkan_local_ctx->instance);

  U32 num_devices = 0;
  if ((vulkan_local_ctx->api.vkEnumeratePhysicalDevices(vulkan_local_ctx->instance, &num_devices, 0) != VK_SUCCESS) ||
      (num_devices == 0))
  {
    InvalidPath;
  }
  if (num_devices != 1)
  {
    AssertMessage("FIXME(calebarg): More than one physical device found!");
  }
  if (vulkan_local_ctx->api.vkEnumeratePhysicalDevices(vulkan_local_ctx->instance, &num_devices, &vulkan_local_ctx->physical_device) != VK_SUCCESS ) {
    InvalidPath;
  }
  VkPhysicalDeviceProperties device_properties;
  vulkan_local_ctx->api.vkGetPhysicalDeviceProperties(vulkan_local_ctx->physical_device, &device_properties);
  VkPhysicalDeviceFeatures   device_features;
  vulkan_local_ctx->api.vkGetPhysicalDeviceFeatures(vulkan_local_ctx->physical_device, &device_features);
  U32 major_version = VK_VERSION_MAJOR(device_properties.apiVersion);
  U32 minor_version = VK_VERSION_MINOR(device_properties.apiVersion);
  U32 patch_version = VK_VERSION_PATCH(device_properties.apiVersion);
  if((major_version < 1) &&
      (device_properties.limits.maxImageDimension2D < 4096))
  {
    AssertMessage("Device properties do not meet the minimum requirements.");
  }

  vulkan_local_ctx->surface = game_memory->platform_vulkan_make_surface(vulkan_local_ctx->instance);

  // NOTE(calebarg): Vulkan spec says that the first queue family must support both graphics and presentation.
  VkQueueFamilyProperties queue_family_properties;
  U32 queue_family_count = 1;
  vulkan_local_ctx->api.vkGetPhysicalDeviceQueueFamilyProperties(vulkan_local_ctx->physical_device, &queue_family_count, &queue_family_properties);
  VkBool32 surface_support = 0;
  vulkan_local_ctx->api.vkGetPhysicalDeviceSurfaceSupportKHR(vulkan_local_ctx->physical_device, 0, vulkan_local_ctx->surface, &surface_support);
  if((queue_family_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
     (surface_support))
  {
    vulkan_local_ctx->queue_family_idx = 0;
  }
  else
  {
    AssertMessage("The queue family must support both graphics and presentation!");
  }

  ////////////////////////////////
  //~calebarg: Device creation

  U8* device_extensions[] = {
    (U8*)VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };
  {
    Temp temp = scratch_begin(vulkan_local_ctx->arena);
    U32 extension_count = 0;
    if ((vulkan_local_ctx->api.vkEnumerateDeviceExtensionProperties(vulkan_local_ctx->physical_device, 0, &extension_count, 0) == VK_SUCCESS) &&
        (extension_count != 0))
    {
      VkExtensionProperties* avail_extensions = ArenaPushArray(temp.arena, VkExtensionProperties, extension_count);
      if (vulkan_local_ctx->api.vkEnumerateDeviceExtensionProperties(vulkan_local_ctx->physical_device, 0, &extension_count, avail_extensions) == VK_SUCCESS)
      {
        String8List avail_extension_names = vulkan_str8_list_from_extension_properties(temp.arena, avail_extensions, extension_count);
        if (!vulkan_cstrs_in_list(device_extensions, ArrayCount(device_extensions), avail_extension_names))
        {
          AssertMessage("Missing extension");
        }
      }
    }
    scratch_end(temp);
  }
  F32 queue_priorities = 1.0;
  VkDeviceQueueCreateInfo queue_create_info = {
    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,     // VkStructureType              sType
    0,                                              // const void                  *pNext
    0,                                              // VkDeviceQueueCreateFlags     flags
    vulkan_local_ctx->queue_family_idx,             // uint32_t                     queueFamilyIndex
    1, 																							// uint32_t                     queueCount
    &queue_priorities                               // const float                 *pQueuePriorities
  };
  VkDeviceCreateInfo device_create_info = {
    VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,           // VkStructureType                    sType
    0,                                              // const void                        *pNext
    0,                                              // VkDeviceCreateFlags                flags
    1,                                              // uint32_t                           queueCreateInfoCount
    &queue_create_info,                             // const VkDeviceQueueCreateInfo     *pQueueCreateInfos
    0,                                              // uint32_t                           enabledLayerCount
    0,                                              // const char * const                *ppEnabledLayerNames
    ArrayCount(device_extensions),                  // uint32_t                           enabledExtensionCount
    (const char**)device_extensions,            		// const char * const                *ppEnabledExtensionNames
    0                                           		// const VkPhysicalDeviceFeatures    *pEnabledFeatures
  };
  if (vulkan_local_ctx->api.vkCreateDevice(vulkan_local_ctx->physical_device, &device_create_info, 0, &vulkan_local_ctx->device) != VK_SUCCESS)
  {
    InvalidPath;
  }
  vulkan_api_load_device_functions(&vulkan_local_ctx->api, vulkan_local_ctx->device);

  vulkan_local_ctx->surface_format =
    vulkan_pick_surface_format();

  vulkan_create_render_pass();
  vulkan_create_swapchain(640, 800); // FIXME(calebarg): Don't bake the size in here.

  U64 buffer_size = KB(8);
  VkBuffer buffer =
    vulkan_create_buffer(buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|VK_BUFFER_USAGE_INDEX_BUFFER_BIT|VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
  VkDeviceMemory device_memory =
    vulkan_allocate_device_memory_from_buffer(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer);

  vulkan_local_ctx->device_buffer_memory = device_memory;
  vulkan_local_ctx->device_buffer = buffer;

  vulkan_local_ctx->api.vkBindBufferMemory(vulkan_local_ctx->device, vulkan_local_ctx->device_buffer, vulkan_local_ctx->device_buffer_memory, 0);
  vulkan_local_ctx->api.vkMapMemory(vulkan_local_ctx->device, vulkan_local_ctx->device_buffer_memory, 0, buffer_size, 0, (void**)&vulkan_local_ctx->mapped_device_memory);

  memcpy(vulkan_local_ctx->mapped_device_memory, positions_and_colors, sizeof(positions_and_colors));
  memcpy(vulkan_local_ctx->mapped_device_memory + KB(1), position_and_color_indices, sizeof(position_and_color_indices));

  VkCommandPoolCreateInfo cmd_pool_create_info = {
    VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,      // VkStructureType              sType
    0,                                               // const void*                  pNext
    VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // VkCommandPoolCreateFlags     flags
    vulkan_local_ctx->queue_family_idx               // U32                          queueFamilyIndex
  };
  if (vulkan_local_ctx->api.vkCreateCommandPool(vulkan_local_ctx->device, &cmd_pool_create_info, 0, &vulkan_local_ctx->command_pool) != VK_SUCCESS)
  {
    InvalidPath;
  }
  VkCommandBufferAllocateInfo cmd_buffer_allocate_info = {
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, // VkStructureType              sType
    0,                                              // const void*                  pNext
    vulkan_local_ctx->command_pool,                 // VkCommandPool                commandPool
    VK_COMMAND_BUFFER_LEVEL_PRIMARY,                // VkCommandBufferLevel         level
    ArrayCount(vulkan_local_ctx->command_buffers)   // U32                          bufferCount
  };
  for (U32 command_buffer_idx=0;
       command_buffer_idx < ArrayCount(vulkan_local_ctx->command_buffers);
       ++command_buffer_idx)
  {
    if (vulkan_local_ctx->api.vkAllocateCommandBuffers(vulkan_local_ctx->device, &cmd_buffer_allocate_info, &vulkan_local_ctx->command_buffers[command_buffer_idx]) != VK_SUCCESS)
    {
      InvalidPath;
    }
  }

  vulkan_local_ctx->frames_in_flight_fences[0] = vulkan_make_fence();
  vulkan_local_ctx->frames_in_flight_fences[1] = vulkan_make_fence();
  vulkan_local_ctx->image_available_semaphores[0] = vulkan_make_semaphore();
  vulkan_local_ctx->image_available_semaphores[1] = vulkan_make_semaphore();
  vulkan_local_ctx->rendering_finished_semaphores[0] = vulkan_make_semaphore();
  vulkan_local_ctx->rendering_finished_semaphores[1] = vulkan_make_semaphore();
}
