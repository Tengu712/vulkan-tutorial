#include "../common/vulkan-tutorial.h"

int main() {
    // window
    GLFWwindow* window;
    {
        const int res = glfwInit();
        CHECK(res == GLFW_TRUE, "failed to init GLFW.");
        SET_GLFW_ERROR_CALLBACK();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
        CHECK(window != NULL, "failed to create a window.");
    }

    // instance
    VkInstance instance;
    {
        const VkApplicationInfo ai = {
            VK_STRUCTURE_TYPE_APPLICATION_INFO,
            NULL,
            "VulkanApplication\0",
            0,
            "VulkanApplication\0",
            VK_MAKE_VERSION(1, 0, 0),
            VK_API_VERSION_1_2,
        };
        const char *layer_names[] = INST_LAYER_NAMES;
        const char *ext_names[] = INST_EXT_NAMES;
        const VkInstanceCreateInfo ci = {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            NULL,
            0,
            &ai,
            INST_LAYER_NAMES_CNT,
            layer_names,
            INST_EXT_NAMES_CNT,
            ext_names,
        };
        CHECK_VK(vkCreateInstance(&ci, NULL, &instance), "failed to create a Vulkan instance.");
    }

    // debug
    SET_VULKAN_DEBUG_CALLBACK(instance);

    // physical device
    // NOTE: 物理デバイスを選択する。
    VkPhysicalDevice phys_device;
    VkPhysicalDeviceMemoryProperties phys_device_memory_prop;
    {
        // NOTE: 物理デバイスのリストを取得する。
        uint32_t cnt = 0;
        CHECK_VK(vkEnumeratePhysicalDevices(instance, &cnt, NULL), "failed to get the number of physical devices.");
        VkPhysicalDevice *phys_devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * cnt);
        CHECK_VK(vkEnumeratePhysicalDevices(instance, &cnt, phys_devices), "failed to enumerate physical devices.");
        // NOTE: リストの最初の物理デバイスを選択する。
        // NOTE: 当然ながら、各々のスペックを見て適切に選択すべき。
        phys_device = phys_devices[0];
        // NOTE: 物理デバイスのメモリ詳細を取得する。
        vkGetPhysicalDeviceMemoryProperties(phys_device, &phys_device_memory_prop);
        // NOTE: リストは不要なので解放する。
        free(phys_devices);
    }

    // queue family index
    // NOTE: 論理デバイスを作成するためにキューファミリを選択する。
    // NOTE: 本プログラムでは一つのキューファミリを用いる。
    int32_t queue_family_index = -1;
    {
        // NOTE: キューファミリのリストを取得する。
        uint32_t cnt = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &cnt, NULL);
        VkQueueFamilyProperties *props = (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * cnt);
        vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &cnt, props);
        // NOTE: グラフィックスパイプラインを扱えるキューファミリを選択する。
        // NOTE: 例えばVK_QUEUE_COMPUTE_BITならコンピュートパイプラインをマスクできる。今回は描画目的なので不要。
        for (int32_t i = 0; i < cnt; ++i) {
            if ((props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) > 0) {
                queue_family_index = i;
                break;
            }
        }
        CHECK(queue_family_index >= 0, "failed to find a queue family index.");
        // NOTE: リストは不要なので解放する。
        free(props);
    }

    // device
    // NOTE: 論理デバイスを作成する。
    VkDevice device;
    {
        // NOTE: 論理デバイスで用いるキューの設定。
        const float queue_priorities[] = { 1.0 };
        const VkDeviceQueueCreateInfo queue_cis[] = {
            {
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                NULL,
                0,
                queue_family_index,
                1,
                queue_priorities,
            },
        };
        // NOTE: 論理デバイスを作成する。
        const char *ext_names[] = DEVICE_EXT_NAMES;
        const VkDeviceCreateInfo ci = {
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            NULL,
            0,
            1,
            queue_cis,
            0,
            NULL,
            DEVICE_EXT_NAMES_CNT,
            ext_names,
            NULL,
        };
        CHECK_VK(vkCreateDevice(phys_device, &ci, NULL, &device), "failed to create a device.");
    }

    // mainloop
    while (1) {
        if (glfwWindowShouldClose(window))
            break;
        glfwPollEvents();
    }

    // termination
    vkDeviceWaitIdle(device);
    vkDestroyDevice(device, NULL);
    DESTROY_VULKAN_DEBUG_CALLBACK(instance);
    vkDestroyInstance(instance, NULL);
    glfwTerminate();

    return 0;
}
