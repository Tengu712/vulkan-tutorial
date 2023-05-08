#pragma once

#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#define CHECK(p, s) if (!(p)) { fprintf(stderr, "[ Error   ] %s\n", (s)); return 1; }
#define CHECK_VK(p, s) if ((p) != VK_SUCCESS) { fprintf(stderr, "[ Error   ] %s\n", (s)); return (p); }
#define CHECK_RETURN(p) { if (!(p)) return VK_ERROR_UNKNOWN; }
#define CHECK_RETURN_VK(p) { VkResult res = (p); if (res != VK_SUCCESS) return res; }
#define WARN_VK(p, s) if ((p) != VK_SUCCESS) printf("[ Warning ] %s\n", (s));
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_TITLE "Vulkan Tutorial"
#define SCREEN_CLEAR_RGBA { 0.25f, 0.25f, 0.25f, 1.0f }
#define DEVICE_EXT_NAMES_CNT 1
#define DEVICE_EXT_NAMES { "VK_KHR_swapchain" }
#define MAX_SHADER_BIN_SIZE 8192

#ifdef _WIN32
#    define INST_EXT_NAME_FOR_SURFACE "VK_KHR_win32_surface"
#elif __linux__
#    define INST_EXT_NAME_FOR_SURFACE "VK_KHR_xcb_surface"
#endif

#ifndef RELEASE_BUILD
#    define SET_GLFW_ERROR_CALLBACK() set_glfw_error_callback()
#    define SET_VULKAN_DEBUG_CALLBACK(p) CHECK_VK(set_vulkan_debug_callback((p)), "failed to set Vulkan debug callback")
#    define DESTROY_VULKAN_DEBUG_CALLBACK(p) destroy_vulkan_debug_callback((p))
#    define INST_EXT_NAMES_CNT 4
#    define INST_EXT_NAMES { "VK_EXT_debug_report", "VK_EXT_debug_utils", "VK_KHR_surface", INST_EXT_NAME_FOR_SURFACE }
#    define INST_LAYER_NAMES_CNT 1
#    define INST_LAYER_NAMES { "VK_LAYER_KHRONOS_validation" }
#else
#    define SET_GLFW_ERROR_CALLBACK()
#    define SET_VULKAN_DEBUG_CALLBACK(p)
#    define DESTROY_VULKAN_DEBUG_CALLBACK(p)
#    define INST_EXT_NAMES_CNT 1
#    define INST_EXT_NAMES { "VK_KHR_surface" }
#    define INST_LAYER_NAMES_CNT 0
#    define INST_LAYER_NAMES { }
#endif

typedef struct FrameData_t {
    VkCommandBuffer command_buffer;
    VkFence fence;
    VkSemaphore semaphore;
} FrameData;

typedef struct Buffer_t {
    VkBuffer buffer;
    VkDeviceMemory memory;
} Buffer;

typedef struct Model_t {
    uint32_t index_cnt;
    Buffer vertex;
    Buffer index;
} Model;

typedef struct CameraData_t {
    float view[16];
    float proj[16];
} CameraData;

typedef struct ImageTexture_t {
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
} ImageTexture;

#ifndef RELEASE_BUILD
    void set_glfw_error_callback();
    VkResult set_vulkan_debug_callback(const VkInstance instance);
    void destroy_vulkan_debug_callback(const VkInstance instance);
#endif

char *read_bin(const char *path, int *p_size);

int32_t get_memory_type_index(
    const VkPhysicalDeviceMemoryProperties *mem_prop,
    const VkMemoryRequirements *reqs,
    VkMemoryPropertyFlags flags
);
VkResult create_buffer(
    const VkDevice device,
    const VkPhysicalDeviceMemoryProperties *mem_prop,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags flags,
    Buffer *out
);
VkResult map_memory(const VkDevice device, const VkDeviceMemory device_memory, const void *data, int32_t size);
VkResult create_model(
    const VkDevice device,
    const VkPhysicalDeviceMemoryProperties *mem_prop,
    uint32_t index_cnt,
    size_t vtxs_size,
    const float *vtxs,
    const uint32_t *idxs,
    Model *out
);

VkResult create_image_texture_from_file(
    const VkDevice device,
    const VkPhysicalDeviceMemoryProperties *mem_prop,
    const VkCommandPool command_pool,
    const VkQueue queue,
    const char *path,
    ImageTexture *out
);
