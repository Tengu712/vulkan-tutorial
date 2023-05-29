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
    VkPhysicalDevice phys_device;
    VkPhysicalDeviceMemoryProperties phys_device_memory_prop;
    {
        uint32_t cnt = 0;
        CHECK_VK(vkEnumeratePhysicalDevices(instance, &cnt, NULL), "failed to get the number of physical devices.");
        VkPhysicalDevice *phys_devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * cnt);
        CHECK_VK(vkEnumeratePhysicalDevices(instance, &cnt, phys_devices), "failed to enumerate physical devices.");
        phys_device = phys_devices[0];
        vkGetPhysicalDeviceMemoryProperties(phys_device, &phys_device_memory_prop);
        free(phys_devices);
    }

    // queue family index
    int32_t queue_family_index = -1;
    {
        uint32_t cnt = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &cnt, NULL);
        VkQueueFamilyProperties *props = (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * cnt);
        vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &cnt, props);
        for (int32_t i = 0; i < cnt; ++i) {
            if ((props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) > 0) {
                queue_family_index = i;
                break;
            }
        }
        CHECK(queue_family_index >= 0, "failed to find a queue family index.");
        free(props);
    }

    // device
    VkDevice device;
    {
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

    // queue
    // NOTE: キューファミリの中の最初のキューを選択する。
    // NOTE: コマンドを提出する際に必要なので、ここで。
    VkQueue queue;
    vkGetDeviceQueue(device, queue_family_index, 0, &queue);

    // command pool
    // NOTE: コマンドプールを作成する。
    VkCommandPool command_pool;
    {
        const VkCommandPoolCreateInfo ci = {
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            NULL,
            // NOTE: これを指定すると、各々コマンドバッファをリセットできる。
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            queue_family_index,
        };
        CHECK_VK(vkCreateCommandPool(device, &ci, NULL, &command_pool), "failed to create a command pool.");
    }

    // command buffer
    // NOTE: コマンドバッファを作成する。
    // NOTE: このバッファにコマンドを記録し、バッファごと提出することになる。
    VkCommandBuffer command_buffer;
    {
        const VkCommandBufferAllocateInfo ai = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            NULL,
            command_pool,
            // NOTE: SECONDARYを指定すると、キューには流せないが他のコマンドバッファにコピーできるバッファになる。
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            // NOTE: ここを増やせば一度にアロケートできるが、今回は一つずつアロケートする。
            1,
        };
        CHECK_VK(vkAllocateCommandBuffers(device, &ai, &command_buffer), "failed to allocate a command buffer.");
    }

    // fence
    // NOTE: フェンスを作成する。
    VkFence fence;
    {
        const VkFenceCreateInfo ci = {
            VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            NULL,
            // NOTE: SIGNALED_BITを指定するとシグナルされた状態で作られる。
            VK_FENCE_CREATE_SIGNALED_BIT,
        };
        CHECK_VK(vkCreateFence(device, &ci, NULL, &fence), "failed to create a fence.");
    }

    // semaphores
    // NOTE: セマフォを作成する。
    // NOTE: wait_semaphoreは本来コマンドバッファ処理前に待機するためのセマフォ。vkAquireNextImageKHR関数に必要で、そこでシグナルにして提出時にアンシグナルにする。
    // NOTE: signal_semaphoreはコマンドバッファ処理後にシグナルされるセマフォ。プレゼンテーションキューのプッシュ時に使う。
    VkSemaphore wait_semaphore, signal_semaphore;
    {
        const VkSemaphoreCreateInfo ci = {
            VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            NULL,
            0,
        };
        CHECK_VK(vkCreateSemaphore(device, &ci, NULL, &wait_semaphore), "failed to create a wait semaphore.");
        CHECK_VK(vkCreateSemaphore(device, &ci, NULL, &signal_semaphore), "failed to create a signal semaphore.");
    }

    // surface
    // NOTE: GLFWのウィンドウからサーフェスを作成する。
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR surface_format;
    VkSurfaceCapabilitiesKHR surface_capabilities;
    {
        // NOTE: サーフェスを作成する。
        CHECK_VK(glfwCreateWindowSurface(instance, window, NULL, &surface), "failed to create a surface.");
        // NOTE: フォーマットのリストを取得する。
        uint32_t cnt = 0;
        CHECK_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(phys_device, surface, &cnt, NULL), "failed to get the number of surface formats.");
        VkSurfaceFormatKHR *formats = (VkSurfaceFormatKHR *)malloc(sizeof(VkSurfaceFormatKHR) * cnt);
        CHECK_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(phys_device, surface, &cnt, formats), "failed to get surface formats.");
        // NOTE: VK_FORMAT_B8G8R8A8_UNORMであるようなフォーマットを取得する。
        int32_t index = -1;
        for (int32_t i = 0; i < cnt; ++i) {
            if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM) {
                index = i;
                break;
            }
        }
        CHECK(index >= 0, "failed to get a surface format index.");
        surface_format = formats[index];
        // NOTE: capabilitiesを取得する。
        CHECK_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_device, surface, &surface_capabilities), "failed to get a surface capabilities.");
        // NOTE: リストは不要なので解放する。
        free(formats);
    }

    // swapchain
    // NOTE: 画面を更新するためにスワップチェインを作成する。
    VkSwapchainKHR swapchain;
    {
        // NOTE: ダブルバッファリングのために最少イメージ数を2とする。
        const uint32_t min_image_count = surface_capabilities.minImageCount > 2 ? surface_capabilities.minImageCount : 2;
        // NOTE: スワップチェインを作成する。
        // NOTE: イメージの詳細は概ねサーフェスフォーマットに合わせる。
        const VkSwapchainCreateInfoKHR ci = {
            VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            NULL,
            0,
            surface,
            min_image_count,
            surface_format.format,
            surface_format.colorSpace,
            surface_capabilities.currentExtent,
            // NOTE: スワップチェインが生成する画像のレイヤー数。
            // NOTE: マルチビューやステレオ3D描画を行う場合に増やすらしい。
            1,
            // NOTE: イメージの使用用途。今回はカラーアタッチメント(描画先)として。
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            // NOTE: イメージの共有モード。
            // NOTE: EXCLUSIVEにすると、同時には一つのキューファミリに限定される。また、続く二つのメンバは無視される。
            VK_SHARING_MODE_EXCLUSIVE,
            0,
            NULL,
            surface_capabilities.currentTransform,
            // NOTE: ウィンドウマネージャによるウィンドウ同士合成におけるモード。
            // NOTE: OPAQUE_BIT_KHRを指定するとアルファ値は無視される。
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            // NOTE: スワップチェインのプレゼンテーションモード。
            // NOTE: FIFO_KHRにすると、プレゼンテーション時に垂直同期を取る。
            VK_PRESENT_MODE_FIFO_KHR,
            // NOTE: レンダリング領域外(ウィンドウが重なっている部分等)のレンダリングを破棄するか否か。TRUEで破棄。
            VK_TRUE,
            VK_NULL_HANDLE,
        };
        CHECK_VK(vkCreateSwapchainKHR(device, &ci, NULL, &swapchain), "failed to create a swapchain.");
    }

    // image views
    // NOTE: スワップイメージチェーンのイメージビューを作成する。
    // NOTE: このイメージビューは描画先となる。
    uint32_t image_views_cnt;
    VkImageView *image_views;
    {
        // NOTE: イメージのリストを取得する。
        uint32_t images_cnt = 0;
        CHECK_VK(vkGetSwapchainImagesKHR(device, swapchain, &images_cnt, NULL), "failed to get the number of swapchain images.");
        VkImage *images = (VkImage *)malloc(sizeof(VkImage) * images_cnt);
        CHECK_VK(vkGetSwapchainImagesKHR(device, swapchain, &images_cnt, images), "failed to get swapchain images.");
        // NOTE: イメージを元にイメージビューを作成する。
        // NOTE: イメージビューはイメージを組み合わせられるらしいが、今回は一つのイメージに一つのイメージビュー。
        image_views_cnt = images_cnt;
        image_views = (VkImageView *)malloc(sizeof(VkImageView) * image_views_cnt);
        VkImageViewCreateInfo ci = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            NULL,
            0,
            NULL, // NOTE: ここにイメージが設定される。
            VK_IMAGE_VIEW_TYPE_2D,
            surface_format.format,
            {
                VK_COMPONENT_SWIZZLE_R,
                VK_COMPONENT_SWIZZLE_G,
                VK_COMPONENT_SWIZZLE_B,
                VK_COMPONENT_SWIZZLE_A,
            },
            // NOTE: イメージビューのアスペクトやミップマップやレイヤーを設定できる。
            // NOTE: アスペクトはCOLOR_BIT。たとえば、デプスバッファを作るときは、デプスバッファのイメージビュー作成時にDEPTH_BITを指定する。
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
        };
        for (int32_t i = 0; i < images_cnt; ++i) {
            ci.image = images[i];
            CHECK_VK(vkCreateImageView(device, &ci, NULL, &image_views[i]), "failed to create an image view.");
        }
        // NOTE: イメージのリストは不要なので破棄する。
        free(images);
    }

    // render pass
    // NOTE: 描画工程の全体像であるレンダーパスを作成する。
    // NOTE: 余程妙なことをしなければ単純なレンダーパスで十分？
    VkRenderPass render_pass;
    const uint32_t render_pass_attachments_count = 1;
    {
        // NOTE: アタッチメントの説明。
        // NOTE: アタッチメントとは、端的に言って描画先。たとえばデプステストを行う場合は、デプスバッファも指定する。
        const VkAttachmentDescription attachment_descs[] = {
            {
                0,
                surface_format.format,
                VK_SAMPLE_COUNT_1_BIT,            // NOTE: 1ピクセルにつき1サンプル。
                VK_ATTACHMENT_LOAD_OP_CLEAR,      // NOTE: 色/深度について最初に使われるサブパスの開始時にクリアされる。
                VK_ATTACHMENT_STORE_OP_STORE,     // NOTE: 色/深度について最後に使われるサブパスの終了時にメモリに書き込まれる。
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,  // NOTE: ステンシルについて。無視。
                VK_ATTACHMENT_STORE_OP_DONT_CARE, // NOTE: ステンシルについて。無視。
                VK_IMAGE_LAYOUT_UNDEFINED,        // NOTE: 最初に使う前に変換するレイアウト。ここでは未定義。
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,  // NOTE: レンダーパス終了時に変換されるレイアウト。ここではプレゼンテーションに使えるレイアウト。
            },
        };
        // NOTE: サブパスで用いられるアタッチメントの説明。
        const VkAttachmentReference attachment_refs[] = {
            {
                0, // NOTE: 0番目のアタッチメント。
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // NOTE: アタッチメントのレイアウト。
            },
        };
        // TODO: サブパス自体、およびメンバの解説。
        // NOTE: サブパスの説明。
        const VkSubpassDescription subpass_descs[] = {
            {
                0,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                0,
                NULL,
                1,
                attachment_refs,
                NULL,
                NULL,
                0,
                NULL,
            },
        };
        // NOTE: レンダーパスを作成する。
        const VkRenderPassCreateInfo ci = {
            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            NULL,
            0,
            render_pass_attachments_count,
            attachment_descs,
            1,
            subpass_descs,
            // TODO: サブパス依存の解説。
            // NOTE: 単純なレンダーパスなのでサブパス依存はなし。
            0,
            NULL,
        };
        CHECK_VK(vkCreateRenderPass(device, &ci, NULL, &render_pass), "failed to create a render pass.");
    }

    // framebuffers
    // NOTE: 見かけ上の描画先であるフレームバッファをイメージビューと同じ数だけ作成する。
    VkFramebuffer *framebuffers;
    {
        VkFramebufferCreateInfo ci = {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            NULL,
            0,
            render_pass,
            render_pass_attachments_count,
            // NOTE: ここにイメージビューが指定される。
            // NOTE: 今回はカラーアタッチメントだけだが、デプステストを行う場合はデプスバッファのイメージビューも指定する。
            NULL,
            surface_capabilities.currentExtent.width,
            surface_capabilities.currentExtent.height,
            1,
        };
        framebuffers = (VkFramebuffer *)malloc(sizeof(VkFramebuffer) * image_views_cnt);
        for (int32_t i = 0; i < image_views_cnt; ++i) {
            ci.pAttachments = &image_views[i];
            CHECK_VK(vkCreateFramebuffer(device, &ci, NULL, &framebuffers[i]), "failed to create a framebuffer.");
        }
    }

    // mainloop
    while (1) {
        if (glfwWindowShouldClose(window))
            break;
        glfwPollEvents();

        // prepare
        // NOTE: 使用するイメージを取得する。
        int img_idx;
        WARN_VK(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, wait_semaphore, VK_NULL_HANDLE, &img_idx), "failed to acquire a next image index.");
        // NOTE: イメージに関連付いたフェンスがシグナルされるのを待つ。
        WARN_VK(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX), "failed to wait for a fence.");
        // NOTE: イメージに関連付いたフェンスとコマンドバッファをリセットする。
        WARN_VK(vkResetFences(device, 1, &fence), "failed to reset a fence.");
        WARN_VK(vkResetCommandBuffer(command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT), "failed to reset a command buffer.");

        // begin
        // NOTE: コマンドの記録を開始する。
        const VkCommandBufferBeginInfo cmd_bi = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            NULL,
            0,
            NULL,
        };
        WARN_VK(vkBeginCommandBuffer(command_buffer, &cmd_bi), "failed to begin to record commands to render.");
        // NOTE: 画面のクリア色を指定する。
        // NOTE: 配列であるのは、デプスバッファを利用する場合はそのクリア値も指定することになるため。
        const VkClearValue clear_values[] = {
            {{ SCREEN_CLEAR_RGBA }},
        };
        // NOTE: レンダーパスを開始する。
        const VkRenderPassBeginInfo rp_bi = {
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            NULL,
            render_pass,
            framebuffers[img_idx],
            { {0, 0}, surface_capabilities.currentExtent },
            1,
            clear_values,
        };
        vkCmdBeginRenderPass(command_buffer, &rp_bi, VK_SUBPASS_CONTENTS_INLINE);

        // end
        // NOTE: レンダーパス及びコマンド記録を終了する。
        vkCmdEndRenderPass(command_buffer);
        vkEndCommandBuffer(command_buffer);
        // NOTE: キューにサブミットする。
        const VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        const VkSubmitInfo submit_info = {
            VK_STRUCTURE_TYPE_SUBMIT_INFO,
            NULL,
            1,
            &wait_semaphore,
            &wait_stage_mask,
            1,
            &command_buffer,
            1,
            &signal_semaphore,
        };
        WARN_VK(vkQueueSubmit(queue, 1, &submit_info, fence), "failed to submit commands to queue.");
        // NOTE: プレゼンテーションを行うコマンドを追加する。
        VkResult res;
        const VkPresentInfoKHR pi = {
            VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            NULL,
            1,
            &signal_semaphore,
            1,
            &swapchain,
            &img_idx,
            &res,
        };
        // NOTE: ここで垂直同期が取られる。
        WARN_VK(vkQueuePresentKHR(queue, &pi), "failed to enqueue present command.");
        WARN_VK(res, "failed to present.");
    }

    // termination
    vkDeviceWaitIdle(device);
    for (uint32_t i = 0; i < image_views_cnt; ++i) {
        vkDestroyFramebuffer(device, framebuffers[i], NULL);
        vkDestroyImageView(device, image_views[i], NULL);
    }
    vkDestroyRenderPass(device, render_pass, NULL);
    vkDestroySwapchainKHR(device, swapchain, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroySemaphore(device, signal_semaphore, NULL);
    vkDestroySemaphore(device, wait_semaphore, NULL);
    vkDestroyFence(device, fence, NULL);
    vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
    vkDestroyCommandPool(device, command_pool, NULL);
    vkDestroyDevice(device, NULL);
    DESTROY_VULKAN_DEBUG_CALLBACK(instance);
    vkDestroyInstance(instance, NULL);
    glfwTerminate();

    return 0;
}
