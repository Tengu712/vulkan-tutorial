#include "vulkan-tutorial.h"

#ifndef RELEASE_BUILD

static PFN_vkCreateDebugReportCallbackEXT g_vkCreateDebugReportCallback;
static PFN_vkDestroyDebugReportCallbackEXT g_vkDestroyDebugReportCallback;
static VkDebugReportCallbackEXT g_debug_report_callback;

static void glfw_error_callback(int code, const char* description) {
    printf("[ GLFW    ] %s : %d\n", description, code);
}

void set_glfw_error_callback() {
    glfwSetErrorCallback(glfw_error_callback);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType,
    uint64_t object,
    size_t location,
    int32_t messageCode,
    const char* pLayerPrefix,
    const char* pMessage,
    void* pUserData
) {
    printf("[ Vulkan  ] %s\n", pMessage);
    return VK_FALSE;
}

VkResult set_vulkan_debug_callback(const VkInstance instance) {
    g_vkCreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    g_vkDestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    VkDebugReportCallbackCreateInfoEXT ci = {
        VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
        NULL,
        VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
        (PFN_vkDebugReportCallbackEXT)&vulkan_debug_callback,
        NULL,
    };
    g_vkCreateDebugReportCallback(instance, &ci, NULL, &g_debug_report_callback);
}

void destroy_vulkan_debug_callback(const VkInstance instance) {
    g_vkDestroyDebugReportCallback(instance, g_debug_report_callback, NULL);
}

#endif
