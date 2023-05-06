#include "../common/vulkan-tutorial.h"

int main() {
    // window
    // NOTE: GLFWを用いてウィンドウを作成する。
    GLFWwindow* window;
    {
        // NOTE: GLFWの初期化。
        const int res = glfwInit();
        CHECK(res == GLFW_TRUE, "failed to init GLFW.");
        // NOTE: コールバック関数の設定。
        // NOTE: リリース時には不要なのでマクロで行う。詳細はcommon/を参照したい。
        SET_GLFW_ERROR_CALLBACK();
        // NOTE: GLFWで作成したウィンドウをVulkanで用いる場合、GLFW_CLIENT_APIをGLFW_NO_APIにする。
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // NOTE: 簡単化のためにウィンドウのリサイズを禁止する。
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        // NOTE: ウィンドウの作成。
        window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
        CHECK(window != NULL, "failed to create a window.");
    }

    // mainloop
    // NOTE: ウィンドウを閉じないようにするための無限ループ。
    // NOTE: 現状ではCPUを休止させていないので、ビジーループとなる。
    while (1) {
        // NOTE: 終了イベントを検知したときにループを抜ける。
        if (glfwWindowShouldClose(window))
            break;
        // NOTE: ウィンドウイベントの処理。
        glfwPollEvents();
    }

    // termination
    glfwTerminate();

    return 0;
}
