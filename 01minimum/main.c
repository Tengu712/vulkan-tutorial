#include <stdio.h>
#include <vulkan/vulkan.h>

#define CHECK(p, s) if ((p) != VK_SUCCESS) { fprintf(stderr, (s)); return (p); }

int main() {
    // instance
    VkInstance instance;
    {
        const int32_t layer_names_cnt = 1;
        const int32_t ext_names_cnt = 3;
        const char *layer_names[] = { "VK_LAYER_KHRONOS_validation" };
        const char *ext_names[] = {
            "VK_EXT_debug_report",
            "VK_EXT_debug_utils",
            "VK_KHR_surface",
        };
        const VkApplicationInfo app_info = {
            VK_STRUCTURE_TYPE_APPLICATION_INFO,
            NULL,
            "VulkanApplication\0",
            0,
            "VulkanApplication\0",
            VK_MAKE_VERSION(1, 0, 0),
            VK_API_VERSION_1_1,
        };
        const VkInstanceCreateInfo create_info = {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            NULL,
            0,
            &app_info,
            layer_names_cnt,
            layer_names,
            ext_names_cnt,
            ext_names,
        };
        CHECK(vkCreateInstance(&create_info, NULL, &instance), "failed to create instance.");
    }

    // mainloop
    while (1) {}

    // finish
    vkDestroyInstance(instance, NULL);
    return 0;
}
