#pragma once

#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#define CHECK(p, s) if (!(p)) { fprintf(stderr, "[ Error ] %s\n", (s)); return 1; }
#define CHECK_VK(p, s) if ((p) != VK_SUCCESS) { fprintf(stderr, "[ Error ] %s\n", (s)); return (p); }
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_TITLE "Vulkan Tutorial"
#define DEVICE_EXT_NAMES_CNT 0
#define DEVICE_EXT_NAMES { }

#ifndef RELEASE_BUILD
#    define SET_GLFW_ERROR_CALLBACK() set_glfw_error_callback()
#    define SET_VULKAN_DEBUG_CALLBACK(p) CHECK_VK(set_vulkan_debug_callback((p)), "failed to set Vulkan debug callback")
#    define DESTROY_VULKAN_DEBUG_CALLBACK(p) destroy_vulkan_debug_callback((p))
#    define INST_EXT_NAMES_CNT 2
#    define INST_EXT_NAMES { "VK_EXT_debug_report", "VK_EXT_debug_utils" }
#    define INST_LAYER_NAMES_CNT 1
#    define INST_LAYER_NAMES { "VK_LAYER_KHRONOS_validation" }
#else
#    define SET_GLFW_ERROR_CALLBACK()
#    define SET_VULKAN_DEBUG_CALLBACK(p)
#    define DESTROY_VULKAN_DEBUG_CALLBACK(p)
#    define INST_EXT_NAMES_CNT 0
#    define INST_EXT_NAMES { }
#    define INST_LAYER_NAMES_CNT 0
#    define INST_LAYER_NAMES { }
#endif

#ifndef RELEASE_BUILD
    void set_glfw_error_callback();
    VkResult set_vulkan_debug_callback(const VkInstance instance);
    void destroy_vulkan_debug_callback(const VkInstance instance);
#endif
