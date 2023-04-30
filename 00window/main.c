#include <stdio.h>

#include <GLFW/glfw3.h>

#define CHECK(p, s, r) if ((p)) { fprintf(stderr, "%s\n", (s)); return (r); }
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_TITLE "Vulkan Tutorial"

// A callback function for GLFW
// NOTE: DX向上のために、GLFWにもコールバック関数を設定しておく。
void glfw_error_callback(int code, const char* description) {
    printf("%s : %d\n", description, code);
}

int main() {
    // window
    // NOTE: GLFWを用いてウィンドウを作成する。
    GLFWwindow* window;
    {
        // NOTE: GLFWの初期化。
        const int res = glfwInit();
        CHECK(res != GLFW_TRUE, "failed to init GLFW", res);
        // NOTE: コールバック関数の設定。
        glfwSetErrorCallback(glfw_error_callback);
        // NOTE: GLFWで作成したウィンドウをVulkanで用いる場合、GLFW_CLIENT_APIをGLFW_NO_APIにする。
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // NOTE: ウィンドウの作成。
        window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
        CHECK(window == NULL, "failed to create a window", 1);
    }

    // mainloop
    // NOTE: ウィンドウを閉じないようにするための無限ループ。
    // NOTE: 現状ではCPUを休止させていないので、ビジーループとなる。
    while (1) {
        // NOTE: 閉じるボタンが押されるなりSIGINTを受け取るなりしたときにループを抜ける。
        if (glfwWindowShouldClose(window))
            break;
        // NOTE: ウィンドウイベントの処理。
        glfwPollEvents();
    }

    // termination
    glfwTerminate();

    return 0;
}