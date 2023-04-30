#include <stdio.h>

// NOTE: VulkanでGLFWのウィンドウを用いるときは必須のマクロ。
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#define CHECK(p, s, r) if ((p)) { fprintf(stderr, "%s\n", (s)); return (r); }
#define CHECK_VK(p, s) if ((p) != VK_SUCCESS) { fprintf(stderr, (s)); return (p); }
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_TITLE "Vulkan Tutorial"
#define INST_EXT_NAMES_CNT 2
#define INST_EXT_NAMES { "VK_EXT_debug_report", "VK_EXT_debug_utils" }
#define INST_LAYER_NAMES_CNT 1
#define INST_LAYER_NAMES { "VK_LAYER_KHRONOS_validation" }

// A callback function for GLFW
void glfw_error_callback(int code, const char* description) {
    printf("%s : %d\n", description, code);
}

int main() {
    // window
    GLFWwindow* window;
    {
        const int res = glfwInit();
        CHECK(res != GLFW_TRUE, "failed to init GLFW", res);
        glfwSetErrorCallback(glfw_error_callback);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
        CHECK(window == NULL, "failed to create a window", 1);
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
        const int32_t layer_names_cnt = INST_LAYER_NAMES_CNT;
        const int32_t ext_names_cnt = INST_EXT_NAMES_CNT;
        const char *layer_names[] = INST_LAYER_NAMES;
        const char *ext_names[] = INST_EXT_NAMES;
        const VkInstanceCreateInfo ci = {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            NULL,
            0,
            &ai,
            layer_names_cnt,
            layer_names,
            ext_names_cnt,
            ext_names,
        };
        // NOTE: インスタンスを作成する。
        CHECK_VK(vkCreateInstance(&ci, NULL, &instance), "failed to create a Vulkan instance");
    }

    // mainloop
    while (1) {
        if (glfwWindowShouldClose(window))
            break;
        glfwPollEvents();
    }

    // termination
    vkDestroyInstance(instance, NULL);
    glfwTerminate();

    return 0;
}
