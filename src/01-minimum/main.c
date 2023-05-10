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
    // NOTE: ハード側を扱うためにインスタンスを作成する。
    VkInstance instance;
    {
        // NOTE: Vulkan 1.2を用いる。
        const VkApplicationInfo ai = {
            VK_STRUCTURE_TYPE_APPLICATION_INFO,
            NULL,
            "VulkanApplication\0",
            0,
            "VulkanApplication\0",
            VK_MAKE_VERSION(1, 0, 0),
            VK_API_VERSION_1_2,
        };
        // NOTE: レイヤーや拡張を設定する。
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
        // NOTE: インスタンスを作成する。
        CHECK_VK(vkCreateInstance(&ci, NULL, &instance), "failed to create a Vulkan instance.");
    }

    // debug
    // NOTE: Vulkanのバリデーションレイヤーのコールバック関数を設定する。
    SET_VULKAN_DEBUG_CALLBACK(instance);

    // mainloop
    while (1) {
        if (glfwWindowShouldClose(window))
            break;
        glfwPollEvents();
    }

    // termination
    DESTROY_VULKAN_DEBUG_CALLBACK(instance);
    vkDestroyInstance(instance, NULL);
    glfwTerminate();

    return 0;
}
