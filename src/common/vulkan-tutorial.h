#pragma once

#include <stdio.h>
#include <stdlib.h>

// GLFWを使う場合は必須のマクロ。
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// GLFW_INCLUDE_VULKANを定義するとglfw3.h内でincludeされるが、気持ちが悪いので、飾りとして。
#include <vulkan/vulkan.h>

// エラーハンドリングのためのマクロ。
#define CHECK(p, s) if (!(p)) { fprintf(stderr, "[ Error   ] %s\n", (s)); return 1; }
#define CHECK_VK(p, s) if ((p) != VK_SUCCESS) { fprintf(stderr, "[ Error   ] %s\n", (s)); return (p); }
#define CHECK_RETURN(p) { if (!(p)) return VK_ERROR_UNKNOWN; }
#define CHECK_RETURN_VK(p) { VkResult res = (p); if (res != VK_SUCCESS) return res; }
#define WARN_VK(p, s) if ((p) != VK_SUCCESS) printf("[ Warning ] %s\n", (s));

// 定数マクロ。
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_TITLE "Vulkan Tutorial"
#define SCREEN_CLEAR_RGBA { 0.25f, 0.25f, 0.25f, 1.0f }
#define DEVICE_EXT_NAMES_CNT 1
#define DEVICE_EXT_NAMES { "VK_KHR_swapchain" }
#define MAX_SHADER_BIN_SIZE 8192

// OS依存の定数マクロ。
#ifdef _WIN32
#    define INST_EXT_NAME_FOR_SURFACE "VK_KHR_win32_surface"
#elif __linux__
#    define INST_EXT_NAME_FOR_SURFACE "VK_KHR_xcb_surface"
#endif

// リリースビルドにデバッグロジックを含めないためのマクロ。
// Tengu712/Vulkan-Tutorial/Makefileを用いる場合は、`make 00 RELEASE=1`のようにすると、RELEASEが定義された状態でビルドされる。
#ifndef RELEASE
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
#    define INST_EXT_NAMES_CNT 2
#    define INST_EXT_NAMES { "VK_KHR_surface", INST_EXT_NAME_FOR_SURFACE }
#    define INST_LAYER_NAMES_CNT 0
#    define INST_LAYER_NAMES { }
#endif

// 1フレームに必要なオブジェクトをまとめた構造体。
// 特に、ダブルバッファリングを管理するために。
typedef struct FrameData_t {
    VkCommandBuffer command_buffer;
    VkFence fence;
    VkSemaphore semaphore;
} FrameData;

// 1バッファに必要なオブジェクトをまとめた構造体。
// 特に、頂点バッファ、インデックスバッファ、ユニフォームバッファのために。
typedef struct Buffer_t {
    VkBuffer buffer;
    VkDeviceMemory memory;
} Buffer;

// 1テクスチャに必要なオブジェクトをまとめた構造体。
// 特に、画像テクスチャ、デプスバッファのために。
typedef struct Texture_t {
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
} Texture;

// 1モデルに必要なオブジェクトをまとめた構造体。
typedef struct Model_t {
    uint32_t index_cnt;
    Buffer vertex;
    Buffer index;
} Model;

// ユニフォームバッファデータのための構造体。
// 名前がCameraDataであるのは、当プロジェクトではカメラとしての役割しか持たないため。
typedef struct CameraData_t {
    float view[16];
    float proj[16];
} CameraData;

// デバッグ関連の関数。
// リリースビルド時には宣言されない。
#ifndef RELEASE
    void set_glfw_error_callback();
    VkResult set_vulkan_debug_callback(const VkInstance instance);
    void destroy_vulkan_debug_callback(const VkInstance instance);
#endif

// pathに存在するファイルをバイナリ形式(char配列)で読み取る関数。
//   - path: ファイルへのパス
//   - p_size: 結果の配列の要素の個数を格納するポインタ
char *read_bin(const char *path, int *p_size);

// バッファを作成するための関数。
//   - device: 論理デバイス
//   - mem_prop: デバイスメモリのプロパティ
//   - size: 確保するバッファのサイズ
//   - usage: バッファの使用目的
//   - flags: バッファのメモリ特性
//   - out: 結果を格納するポインタ
VkResult create_buffer(
    const VkDevice device,
    const VkPhysicalDeviceMemoryProperties *mem_prop,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags flags,
    Buffer *out
);
// テクスチャを作成するための関数。
//   - device: 論理デバイス
//   - mem_prop: デバイスメモリのプロパティ
//   - format: 1テクセルのデータ構造
//   - width: テクスチャ幅
//   - height: テクスチャ高
//   - usage: テクスチャの使用目的
//   - aspect: イメージのアスペクト
VkResult create_texture(
    const VkDevice device,
    const VkPhysicalDeviceMemoryProperties *mem_prop,
    VkFormat format,
    uint32_t width,
    uint32_t height,
    VkImageUsageFlags usage,
    VkImageAspectFlags aspect,
    Texture *out
);
// デバイスメモリにデータをマップする関数。
//   - device: 論理デバイス
//   - device_memory: デバイスメモリ
//   - data: データ
//   - size: データのサイズ(bytes)
VkResult map_memory(const VkDevice device, const VkDeviceMemory device_memory, const void *data, int32_t size);

// モデルを作成する関数。
//   - device: 論理デバイス
//   - mem_prop: デバイスメモリのプロパティ
//   - index_cnt: インデックスの数
//   - vtxs_size: 頂点データのサイズ(bytes)
//   - vtxs: 頂点データ
//   - idxs: インデックスデータ
//   - out: 結果を格納するポインタ
VkResult create_model(
    const VkDevice device,
    const VkPhysicalDeviceMemoryProperties *mem_prop,
    uint32_t index_cnt,
    size_t vtxs_size,
    const float *vtxs,
    const uint32_t *idxs,
    Model *out
);

// ファイルから画像テクスチャを作成する関数。
//   - device: 論理デバイス
//   - mem_prop: デバイスメモリのプロパティ
//   - command_pool: コマンドプール
//   - queue: キュー
//   - path: ファイルへのパス
//   - out: 結果を格納するポインタ
VkResult create_image_texture_from_file(
    const VkDevice device,
    const VkPhysicalDeviceMemoryProperties *mem_prop,
    const VkCommandPool command_pool,
    const VkQueue queue,
    const char *path,
    Texture *out
);
