//
// linux_code_gen.c
//
// Caleb Barger
// 01/14/2024
//
// Code generation for caffeinated game engine.
//
// Vulkan function loader generator metaprogram..
//
// NOTE(calebarg): While this is a linux program, it is meant to
// spit out a header that is PLATFORM AGNOSTIC, the source generated
// doesn't have to be platform agnostic since it's not used in the game code.
//

#include "base/base_inc.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <time.h>

#include "base/base_inc.c"
#if __has_include("generated/checksum.c") // TODO(calebarg): Remove this (newer preprocessor extension)
 #include "generated/checksum.c"
#else
 global U64 cg_checksum = 0;
#endif

global char* global_functions[] = {
  "vkCreateInstance",
  "vkEnumerateInstanceExtensionProperties",
  "vkEnumerateInstanceLayerProperties",
};

global char* instance_functions[] = {
  "vkEnumeratePhysicalDevices",
  "vkGetPhysicalDeviceProperties",
  "vkGetPhysicalDeviceFeatures",
  "vkGetPhysicalDeviceQueueFamilyProperties",
  "vkCreateDevice",
  "vkDestroyInstance",
  "vkEnumerateDeviceExtensionProperties",
  "vkDestroySurfaceKHR",
  "vkGetPhysicalDeviceSurfaceSupportKHR",
  "vkGetPhysicalDeviceSurfaceCapabilitiesKHR",
  "vkGetPhysicalDeviceSurfaceFormatsKHR",
  "vkGetPhysicalDeviceSurfacePresentModesKHR",
  "vkGetPhysicalDeviceMemoryProperties",
};

global char* device_functions[] = {
  "vkGetDeviceQueue",
  "vkDestroyDevice",
  "vkDeviceWaitIdle",
  "vkCreateSemaphore",
  "vkCreateSwapchainKHR",
  "vkDestroySwapchainKHR",
  "vkGetSwapchainImagesKHR",
  "vkAcquireNextImageKHR",
  "vkQueuePresentKHR",
  "vkCreateCommandPool",
  "vkAllocateCommandBuffers",
  "vkBeginCommandBuffer",
  "vkCmdPipelineBarrier",
  "vkCmdClearColorImage",
  "vkCmdSetViewport",
  "vkCmdSetScissor",
  "vkCmdDraw",
  "vkCmdDrawIndexed",
  "vkEndCommandBuffer",
  "vkQueueSubmit",
  "vkFreeCommandBuffers",
  "vkDestroyCommandPool",
  "vkResetCommandPool",
  "vkDestroySemaphore",
  "vkCreateFence",
  "vkResetFences",
	"vkGetFenceStatus",
  "vkWaitForFences",
  "vkCreateShaderModule",
  "vkDestroyShaderModule",
  "vkCreatePipelineLayout",
  "vkCreateRenderPass",
  "vkCreateGraphicsPipelines",
  "vkCreateImageView",
  "vkCreateImage",
  "vkCmdBeginRenderPass",
  "vkCmdEndRenderPass",
  "vkCmdBindPipeline",
  "vkCreateFramebuffer",
  "vkDestroyFramebuffer",
  "vkDestroyImageView",
  "vkCmdBindVertexBuffers",
  "vkCreateBuffer",
  "vkGetBufferMemoryRequirements",
  "vkAllocateMemory",
  "vkBindBufferMemory",
  "vkBindImageMemory",
  "vkMapMemory",
  "vkCmdBindIndexBuffer",
  "vkCreateComputePipelines",
  "vkCreateDescriptorSetLayout",
  "vkCreateDescriptorPool",
  "vkAllocateDescriptorSets",
  "vkUpdateDescriptorSets",
  "vkCmdBindDescriptorSets",
  "vkCmdDispatch",
};

global char** function_groups[] = {
  global_functions,
  instance_functions,
  device_functions,
};

global char* function_group_names[] = {
  "Global",
  "Instance",
  "Device",
};

global U32 function_group_counts[] = {
  ArrayCount(global_functions),
  ArrayCount(instance_functions),
  ArrayCount(device_functions),
};

internal U64
cg_checksum_from_data(U8* data, U64 count)
{
  U64 sum1 = 0;
  U64 sum2 = 0;
  for (U64 byte_idx = 0;
       byte_idx < count;
       ++byte_idx)
  {
    sum1 = (sum1 + data[byte_idx]) % 0xfffffffe;
    sum2 = (sum2 + sum1) % 0xfffffffe;
  }
  return (sum2 << 32) | sum1;
}

internal void
cg_write_file_header(String8 filename, S32 fd)
{
  Temp(0, 0)
  {
    time_t t = time(0);
    struct tm* tm = localtime(&t);
    String8 year_str = str8_from_s32(temp.arena, tm->tm_year + 1900);
    String8 month_str = str8_from_s32(temp.arena, tm->tm_mon + 1);
    String8 day_str = str8_from_s32(temp.arena, tm->tm_mday);

    String8 header = Str8Lit("//\n// ");
    header = str8_cat(temp.arena, header, filename);
    header = str8_cat(temp.arena, header, Str8Lit("\n//\n// Caleb Barger\n// "));
    header = str8_cat(temp.arena, header, month_str);
    header = str8_cat(temp.arena, header, Str8Lit("/"));
    header = str8_cat(temp.arena, header, day_str);
    header = str8_cat(temp.arena, header, Str8Lit("/"));
    header = str8_cat(temp.arena, header, year_str);
    header = str8_cat(temp.arena, header, Str8Lit("\n// DON'T MODIFY. THIS CODE WAS GENERATED WITH " __FILE__ "\n//\n\n"));
    write(fd, header.ptr, header.len);
  }
}

int main()
{
  void* memory = mmap(0, MB(1), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  Arena* arena = arena_from_memory(memory, MB(1));
  ThreadCTX thread_ctx;
  thread_ctx.arenas[0] = arena_sub(arena, KB(32));
  thread_ctx.arenas[1] = arena_sub(arena, KB(32));
  equip_thread_ctx(&thread_ctx);

  String8 generated_dir_path_rel = Str8Lit("./generated");
  {
    struct stat st = {0};
    if (stat((char*)generated_dir_path_rel.ptr, &st) == -1)
    {
      if (mkdir((char*)generated_dir_path_rel.ptr, 0777) == -1)
      {
        InvalidPath;
      }
    }
  }

  U64 checksum = ~cg_checksum; // Can be anything except cg_checksum.
  Temp(0, 0)
  {
    U8* contents = ArenaPushArray(temp.arena, U8, KB(20));
    String8 code_gen_file_path_rel = Str8Lit("./linux_code_gen.c");
    S32 code_gen_fd = open((char*)code_gen_file_path_rel.ptr, O_RDONLY, 0666);
    if (code_gen_fd != -1)
    {
      S64 read_count = read(code_gen_fd, contents, KB(20));
      if (read_count == -1)
      {
        InvalidPath;
      }
      checksum = cg_checksum_from_data(contents, (U64)read_count);
      close(code_gen_fd);
    }
  }
  if (checksum != cg_checksum)
  {
    Temp(0, 0)
    {
      String8 checksum_c_path_rel = str8_catz(arena, str8_cat(temp.arena, generated_dir_path_rel, Str8Lit("/checksum.c")));
      S32 checksum_fd = open((char*)checksum_c_path_rel.ptr, O_CREAT|O_WRONLY|O_TRUNC, 0666);
      if (checksum_fd == -1)
      {
        InvalidPath;
      }
      cg_write_file_header(Str8Lit("checksum.c"), checksum_fd);

      String8 checksum_define = Str8Lit("#define CHECKSUM_C\n");
      write(checksum_fd, checksum_define.ptr, checksum_define.len);

      U8* buf = ArenaPushArray(temp.arena, U8, KB(1));
      S32 formatted_byte_count = snprintf((char*)buf, KB(1), "global U64 cg_checksum = %llu;\n", checksum);
      write(checksum_fd, buf, formatted_byte_count);

      close(checksum_fd);
    }

    String8 vulkan_api_h_rel_path = str8_catz(arena, str8_cat(arena, generated_dir_path_rel, Str8Lit("/vulkan_api.h")));
    S32 hfd = open((char*)vulkan_api_h_rel_path.ptr, O_CREAT|O_WRONLY|O_TRUNC, 0666);
    if (hfd == -1)
    {
      InvalidPath;
    }
    cg_write_file_header(Str8Lit("vulkan_api.h"), hfd);

    String8 top_header_gaurd = Str8Lit("#ifndef VULKAN_API_H\n\n");
    write(hfd, top_header_gaurd.ptr, top_header_gaurd.len);

    String8 api_struct_decl = Str8Lit("typedef struct VulkanAPI VulkanAPI;\nstruct VulkanAPI {\n");
    write(hfd, api_struct_decl.ptr, api_struct_decl.len);

    for (U32 group_idx=0;
         group_idx < ArrayCount(function_groups);
         ++group_idx)
    {
      TempN(temp1, 0, 0)
      {
        String8 group_delim_comment = Str8Lit("\n  // ");
        group_delim_comment = str8_cat(temp1.arena, group_delim_comment, str8_from_mem((U8*)function_group_names[group_idx]));
        group_delim_comment = str8_cat(temp1.arena, group_delim_comment, Str8Lit(" functions\n"));
        write(hfd, group_delim_comment.ptr, group_delim_comment.len);
      }
      for (U32 function_idx=0;
           function_idx < function_group_counts[group_idx];
           ++function_idx)
      {
        TempN(temp2, 0, 0)
        {
          String8 function_name = str8_from_mem((U8*)function_groups[group_idx][function_idx]);
          String8 line = str8_cat(arena, Str8Lit("  PFN_"), function_name);
          line = str8_cat(arena, line, Str8Lit(" "));
          line = str8_cat(arena, line, function_name);
          line = str8_cat(arena, line, Str8Lit(";\n"));
          write(hfd, line.ptr, line.len);
        }
      }
    }

    String8 closing_brace_and_bot_header_gaurd = Str8Lit("};\n\n#define VULKAN_API_H\n#endif");
    write(hfd, closing_brace_and_bot_header_gaurd.ptr, closing_brace_and_bot_header_gaurd.len);

    ////////////////////////////////
    //~ calebarg: Vulkan loader source file

    String8 vulkan_api_c_rel_path = str8_catz(arena, str8_cat(arena, generated_dir_path_rel, Str8Lit("/vulkan_api.c")));
    S32 cfd = open((char*)vulkan_api_c_rel_path.ptr, O_CREAT|O_WRONLY|O_TRUNC, 0666);
    if (cfd == -1)
    {
      InvalidPath;
    }
    cg_write_file_header(Str8Lit("vulkan_api.c"), cfd);

    String8 get_instance_proc_addr_typedef = Str8Lit("typedef PFN_vkVoidFunction VKGetInstanceProcAddr(VkInstance instance, const char* p_name);\n");
    String8 get_instance_proc_addr_decl = Str8Lit("global VKGetInstanceProcAddr* vkGetInstanceProcAddr = 0;\n");
    write(cfd, get_instance_proc_addr_typedef.ptr, get_instance_proc_addr_typedef.len);
    write(cfd, get_instance_proc_addr_decl.ptr, get_instance_proc_addr_decl.len);

    String8 get_device_proc_addr_decl = Str8Lit("global PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr = 0;\n");
    write(cfd, get_device_proc_addr_decl.ptr, get_device_proc_addr_decl.len);

    String8 load_get_instance_proc_function = Str8Lit("\ninternal void\nvulkan_api_load_get_instance_proc_addr(LoadFunction* platform_load_function, void* vulkan_code)\n{\n  vkGetInstanceProcAddr = (VKGetInstanceProcAddr*)platform_load_function(vulkan_code, \"vkGetInstanceProcAddr\");\n}\n\n");
    write(cfd, load_get_instance_proc_function.ptr, load_get_instance_proc_function.len);

    String8 load_get_instance_proc_check =
      Str8Lit("  if (!vkGetInstanceProcAddr)\n  {\n    vulkan_api_load_get_instance_proc_addr(platform_load_function, vulkan_code);\n  }\n");

    for (U32 group_idx=0;
         group_idx < ArrayCount(function_groups);
         ++group_idx)
    {
      String8 load_function_preamble;
      if (group_idx == 0) // Global
      {
        load_function_preamble = Str8Lit("internal void\nvulkan_api_load_global_functions(LoadFunction* platform_load_function, VulkanAPI* api, void* vulkan_code)\n{\n");
        load_function_preamble = str8_cat(arena, load_function_preamble, load_get_instance_proc_check);
      }
      else if (group_idx == 1) // Instance
      {
        load_function_preamble = Str8Lit("internal void\nvulkan_api_load_instance_functions(LoadFunction* platform_load_function, VulkanAPI* api, void* vulkan_code, VkInstance instance)\n{\n");
        load_function_preamble = str8_cat(arena, load_function_preamble, load_get_instance_proc_check);
        load_function_preamble = str8_cat(arena, load_function_preamble, Str8Lit("  vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)vkGetInstanceProcAddr(instance, \"vkGetDeviceProcAddr\");\n\n"));
      }
      else if (group_idx == 2) // Device
      {
        load_function_preamble = Str8Lit("internal void\nvulkan_api_load_device_functions(VulkanAPI* api, VkDevice device)\n{\n");
      }
      else
      {
        AssertMessage("Invalid Path");
      }
      write(cfd, load_function_preamble.ptr, load_function_preamble.len);
      for (U32 function_idx=0;
           function_idx < function_group_counts[group_idx];
           ++function_idx)
      {
        Temp scratch = scratch_begin(arena);
        String8 function_name = str8_from_mem((U8*)function_groups[group_idx][function_idx]);
        String8 line = str8_cat(arena, Str8Lit("  api->"), function_name);
        line = str8_cat(arena, line, Str8Lit(" = "));
        line = str8_cat(arena, line, Str8Lit("(PFN_"));
        line = str8_cat(arena, line, function_name);

        if (group_idx == 0) // Global
        {
          line = str8_cat(arena, line, Str8Lit(")vkGetInstanceProcAddr(0,\""));
        }
        else if (group_idx == 1) // Instance
        {
          line = str8_cat(arena, line, Str8Lit(")vkGetInstanceProcAddr(instance,\""));
        }
        else if (group_idx == 2) // Device
        {
          line = str8_cat(arena, line, Str8Lit(")vkGetDeviceProcAddr(device,\""));
        }
        else
        {
          AssertMessage("Invalid Path");
        }

        line = str8_cat(arena, line, function_name);
        line = str8_cat(arena, line, Str8Lit("\");\n"));
        write(cfd, line.ptr, line.len);
        scratch_end(scratch);
      }
      write(cfd, "}\n\n", 3);
    }
  }
}
