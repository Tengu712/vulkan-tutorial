#include "../common/vulkan-tutorial.h"

// A struct for vertex input data.
// NOTE: 頂点情報の構造。
// NOTE: のちのち変更されるため、vulkan-tutorial.hではなくここで定義しておく。
typedef struct Vertex_t {
    float pos[3];
} Vertex;

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
    VkQueue queue;
    vkGetDeviceQueue(device, queue_family_index, 0, &queue);

    // command pool
    VkCommandPool command_pool;
    {
        const VkCommandPoolCreateInfo ci = {
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            NULL,
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            queue_family_index,
        };
        CHECK_VK(vkCreateCommandPool(device, &ci, NULL, &command_pool), "failed to create a command pool.");
    }

    // command buffer
    VkCommandBuffer command_buffer;
    {
        const VkCommandBufferAllocateInfo ai = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            NULL,
            command_pool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            1,
        };
        CHECK_VK(vkAllocateCommandBuffers(device, &ai, &command_buffer), "failed to allocate a command buffer.");
    }

    // fence
    VkFence fence;
    {
        const VkFenceCreateInfo ci = {
            VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            NULL,
            VK_FENCE_CREATE_SIGNALED_BIT,
        };
        CHECK_VK(vkCreateFence(device, &ci, NULL, &fence), "failed to create a fence.");
    }

    // semaphores
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
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR surface_format;
    VkSurfaceCapabilitiesKHR surface_capabilities;
    {
        CHECK_VK(glfwCreateWindowSurface(instance, window, NULL, &surface), "failed to create a surface.");
        uint32_t cnt = 0;
        CHECK_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(phys_device, surface, &cnt, NULL), "failed to get the number of surface formats.");
        VkSurfaceFormatKHR *formats = (VkSurfaceFormatKHR *)malloc(sizeof(VkSurfaceFormatKHR) * cnt);
        CHECK_VK(vkGetPhysicalDeviceSurfaceFormatsKHR(phys_device, surface, &cnt, formats), "failed to get surface formats.");
        int32_t index = -1;
        for (int32_t i = 0; i < cnt; ++i) {
            if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM) {
                index = i;
                break;
            }
        }
        CHECK(index >= 0, "failed to get a surface format index.");
        surface_format = formats[index];
        CHECK_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_device, surface, &surface_capabilities), "failed to get a surface capabilities.");
        free(formats);
    }

    // swapchain
    VkSwapchainKHR swapchain;
    {
        const uint32_t min_image_count = surface_capabilities.minImageCount > 2 ? surface_capabilities.minImageCount : 2;
        const VkSwapchainCreateInfoKHR ci = {
            VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            NULL,
            0,
            surface,
            min_image_count,
            surface_format.format,
            surface_format.colorSpace,
            surface_capabilities.currentExtent,
            1,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VK_SHARING_MODE_EXCLUSIVE,
            0,
            NULL,
            surface_capabilities.currentTransform,
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_PRESENT_MODE_FIFO_KHR,
            VK_TRUE,
            VK_NULL_HANDLE,
        };
        CHECK_VK(vkCreateSwapchainKHR(device, &ci, NULL, &swapchain), "failed to create a swapchain.");
    }

    // image views
    uint32_t image_views_cnt;
    VkImageView *image_views;
    {
        uint32_t images_cnt = 0;
        CHECK_VK(vkGetSwapchainImagesKHR(device, swapchain, &images_cnt, NULL), "failed to get the number of swapchain images.");
        VkImage *images = (VkImage *)malloc(sizeof(VkImage) * images_cnt);
        CHECK_VK(vkGetSwapchainImagesKHR(device, swapchain, &images_cnt, images), "failed to get swapchain images.");
        image_views_cnt = images_cnt;
        image_views = (VkImageView *)malloc(sizeof(VkImageView) * image_views_cnt);
        VkImageViewCreateInfo ci = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            NULL,
            0,
            NULL,
            VK_IMAGE_VIEW_TYPE_2D,
            surface_format.format,
            {
                VK_COMPONENT_SWIZZLE_R,
                VK_COMPONENT_SWIZZLE_G,
                VK_COMPONENT_SWIZZLE_B,
                VK_COMPONENT_SWIZZLE_A,
            },
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
        };
        for (int32_t i = 0; i < images_cnt; ++i) {
            ci.image = images[i];
            CHECK_VK(vkCreateImageView(device, &ci, NULL, &image_views[i]), "failed to create an image view.");
        }
        free(images);
    }

    // render pass
    VkRenderPass render_pass;
    {
        const VkAttachmentDescription attachment_descs[] = {
            {
                0,
                surface_format.format,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            },
        };
        const VkAttachmentReference attachment_refs[] = {
            {
                0,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            },
        };
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
        const VkRenderPassCreateInfo ci = {
            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            NULL,
            0,
            1,
            attachment_descs,
            1,
            subpass_descs,
            0,
            NULL,
        };
        CHECK_VK(vkCreateRenderPass(device, &ci, NULL, &render_pass), "failed to create a render pass.");
    }

    // framebuffers
    VkFramebuffer *framebuffers;
    {
        VkFramebufferCreateInfo ci = {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            NULL,
            0,
            render_pass,
            1,
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

    // shaders
    VkShaderModule vert_shader;
    VkShaderModule frag_shader;
    {
        // vertex shader
        // NOTE: ヴァーテックスシェーダモジュールを作成する。
        int bin_vert_size;
        char *bin_vert = read_bin("./shader.vert.spv", &bin_vert_size);
        CHECK(bin_vert != NULL, "failed to read shader.vert.spv.");
        const VkShaderModuleCreateInfo vert_ci = {
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            NULL,
            0,
            bin_vert_size,
            (const uint32_t*)bin_vert,
        };
        CHECK_VK(vkCreateShaderModule(device, &vert_ci, NULL, &vert_shader), "failed to create a vertex shader module.");
        // fragment shader
        // NOTE: フラグメントシェーダモジュールを作成する。
        int bin_frag_size;
        char *bin_frag = read_bin("./shader.frag.spv", &bin_frag_size);
        CHECK(bin_frag != NULL, "failed to read shader.frag.spv.");
        const VkShaderModuleCreateInfo frag_ci = {
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            NULL,
            0,
            bin_frag_size,
            (const uint32_t*)bin_frag,
        };
        CHECK_VK(vkCreateShaderModule(device, &frag_ci, NULL, &frag_shader), "failed to create a fragment shader module.");
        // NOTE: バイナリ配列は不要なので解放する。
        free(bin_vert);
        free(bin_frag);
    }

    // pipeline
    // NOTE: パイプラインを作る。
    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;
    {
        // NOTE: ここでプッシュコンスタントやディスクリプタをまとめる。
        // NOTE: 04-triangleでは使わないのでなし。
        const VkPipelineLayoutCreateInfo pipeline_layout_ci = {
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            NULL,
            0,
            0,
            NULL,
            0,
            NULL,
        };
        CHECK_VK(vkCreatePipelineLayout(device, &pipeline_layout_ci, NULL, &pipeline_layout), "failed to create a pipeline layout.");

        // shaders
        // NOTE: パイプラインで用いるシェーダをまとめる。
        const VkPipelineShaderStageCreateInfo shader_cis[2] = {
            {
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                NULL,
                0,
                VK_SHADER_STAGE_VERTEX_BIT,
                vert_shader,
                "main",
                NULL,
            },
            {
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                NULL,
                0,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                frag_shader,
                "main",
                NULL,
            },
        };

        // vertex input
        // TODO: バインディングって何？
        // NOTE: 頂点情報のデータサイズの説明。
        // NOTE: 04-triangleではxyzローカル座標しかないので、バインディング数が一つで、サイズはfloat*3。
        const VkVertexInputBindingDescription vert_inp_binding_dcs[] = {
            // TODO: 3つ目の説明を追加する。
            // NOTE: バインディング番号、サイズ、
            { 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX },
        };
        // NOTE: 頂点情報の構造の説明。
        // NOTE: バインディングが複数あってもフラットな配列にする。
        const VkVertexInputAttributeDescription vert_inp_attr_dcs[] = {
            // NOTE: バインディング内のロケーション番号、バインディング番号、形式、オフセット。
            // NOTE: ロケーション番号は、GLSL内の指定に一致させる。
            { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
        };
        // NOTE: 頂点情報の説明をまとめる。
        const VkPipelineVertexInputStateCreateInfo vert_inp_ci = {
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            NULL,
            0,
            1,
            vert_inp_binding_dcs,
            1,
            vert_inp_attr_dcs,
        };

        // input assembly
        // NOTE: インプットアセンブリについて。
        const VkPipelineInputAssemblyStateCreateInfo inp_as_ci = {
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            NULL,
            0,
            // NOTE: ポリゴンを作る際の頂点の結び方。今回はトライアングルリスト。
            // NOTE: 他にTRIANGLE_STRIPやTRIANGLE_FANやがある。
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            VK_FALSE,
        };

        // viewport
        // TODO: 詳しく説明する。
        // NOTE: ビューポートとシザーについて。
        const VkViewport viewports[] = {
            {
                0.0f,
                0.0f,
                surface_capabilities.currentExtent.width,
                surface_capabilities.currentExtent.height,
                0.0f,
                1.0f,
            },
        };
        const VkRect2D scissors[] = {
            { {0, 0}, surface_capabilities.currentExtent },
        };
        const VkPipelineViewportStateCreateInfo viewport_ci = {
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            NULL,
            0,
            1,
            viewports,
            1,
            scissors,
        };

        // rasterization
        // TODO: 他のメンバも説明する。
        // NOTE: ラスタライゼーションの設定。カリングを設定できる。
        const VkPipelineRasterizationStateCreateInfo raster_ci = {
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            NULL,
            0,
            VK_FALSE,
            VK_FALSE,
            // NOTE: ポリゴンを塗り尽くすのか、枠線だけ塗るのか、頂点だけ塗るのかを設定できる。
            VK_POLYGON_MODE_FILL,
            // NOTE: カリングを行うか設定できる。2Dゲームなら無、3Dゲームなら有がいいかも？
            // NOTE: BACK_BITを指定すると「裏」を向いているポリゴンを破棄する。
            VK_CULL_MODE_BACK_BIT,
            // NOTE: OpenGL系では普通、頂点を反時計回りに結ぶと「表」と判断される。
            // NOTE: VK_FRONT_FACE_CLOCKWISEを設定すると、時計回りで「表」に変えられる。
            VK_FRONT_FACE_COUNTER_CLOCKWISE,
            VK_FALSE,
            0.0f,
            0.0f,
            0.0f,
            1.0f,
        };

        // multisample
        // TODO: メンバの説明。
        // NOTE: マルチサンプリングの設定。いわゆるアンチエイリアス。
        // NOTE: 省略するとバリデーションレイヤからエラーが吐かれる。
        const VkPipelineMultisampleStateCreateInfo multisample_ci = {
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            NULL,
            0,
            VK_SAMPLE_COUNT_1_BIT,
            VK_FALSE,
            0.0f,
            NULL,
            VK_FALSE,
            VK_FALSE,
        };

        // color blend
        // NOTE: 色の合成についての設定。透過合成やオーバーレイなどを設定できる。
        // NOTE: 今回は一つのみで、透過合成。色もアルファ値も上に重なる物基準で足して1になるようにする。
        const VkPipelineColorBlendAttachmentState color_blend_states[] = {
            {
                VK_TRUE,
                VK_BLEND_FACTOR_SRC_ALPHA,
                VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                VK_BLEND_OP_ADD,
                VK_BLEND_FACTOR_SRC_ALPHA,
                VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                VK_BLEND_OP_ADD,
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            }
        };
        const VkPipelineColorBlendStateCreateInfo color_blend_ci = {
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            NULL,
            0,
            VK_FALSE,
            (VkLogicOp)0,
            1,
            color_blend_states,
            {0.0f, 0.0f, 0.0f, 0.0f},
        };

        // pipeline
        // TODO: 派生について説明する。
        // NOTE: パイプラインを作る。
        // NOTE: パイプラインは「派生」できるらしい。今回は一つのみで、派生しない。
        const VkGraphicsPipelineCreateInfo cis[] = {
            {
                VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                NULL,
                0,
                2,
                shader_cis,
                &vert_inp_ci,
                &inp_as_ci,
                NULL, // NOTE: テッセレーションのためのメンバ。
                &viewport_ci,
                &raster_ci,
                &multisample_ci,
                NULL, // NOTE: デプス/ステンシルのためのメンバ。
                &color_blend_ci,
                NULL, // TODO: dynamic stateについて詳しく知らない。
                pipeline_layout,
                render_pass,
                0, // NOTE: このパイプラインを用いるサブパスの番号。
                NULL, // NOTE: 派生元のパイプライン。
                0,    // NOTE: 派生元のパイプラインのcreate infoのインデックス。
            },
        };
        CHECK_VK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, cis, NULL, &pipeline), "failed to create a pipeline.");
    }

    // model
    Model model;
    {
        // NOTE: Vulkanのクリッピング座標系は、
        // NOTE:   * x: [-1, 1] left to right
        // NOTE:   * y: [-1, 1] up to down
        // NOTE:   * z: [0, 1] near to far (遠近は頂点座標のwで調整するので、遠くにあっても遠近感は出ない)
        // NOTE: であるため、上向きの三角形の頂点座標は以下のようになる。
        const Vertex vtxs[3] = {
            { { -0.5f,  0.5f, 0.0f } }, // NOTE: 左下。
            { {  0.5f,  0.5f, 0.0f } }, // NOTE: 右下。
            { {  0.0f, -0.5f, 0.0f } }, // NOTE: 中上。
        };
        // NOTE: カリングを考慮して反時計回りに結ぶ。
        const uint32_t idxs[3] = { 0, 1, 2 };
        // NOTE: 自作関数でモデルを作る。詳しくはcommon/buffer.cを参照。
        CHECK_VK(
            create_model(
                device,
                &phys_device_memory_prop,
                3,
                sizeof(Vertex) * 3,
                (const float *)vtxs,
                (const uint32_t *)idxs,
                &model
            ),
            "failed to create a model."
        );
    }

    // mainloop
    while (1) {
        if (glfwWindowShouldClose(window))
            break;
        glfwPollEvents();

        // prepare
        int img_idx;
        WARN_VK(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, wait_semaphore, VK_NULL_HANDLE, &img_idx), "failed to acquire a next image index.");
        WARN_VK(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX), "failed to wait for a fence.");
        WARN_VK(vkResetFences(device, 1, &fence), "failed to reset a fence.");
        WARN_VK(vkResetCommandBuffer(command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT), "failed to reset a command buffer.");

        // begin
        const VkCommandBufferBeginInfo cmd_bi = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            NULL,
            0,
            NULL,
        };
        WARN_VK(vkBeginCommandBuffer(command_buffer, &cmd_bi), "failed to begin to record commands to render.");
        const VkClearValue clear_values[] = {
            {{ SCREEN_CLEAR_RGBA }},
        };
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
        // NOTE: パイプラインを関連付ける。
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        // draw
        // NOTE: モデルを関連付ける。
        // NOTE: 残念ながらコマンドバッファごとに関連付けるので、一つしかモデルがなくとも、毎フレーム行う。
        const VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &model.vertex.buffer, &offset);
        vkCmdBindIndexBuffer(command_buffer, model.index.buffer, offset, VK_INDEX_TYPE_UINT32);
        // NOTE: ドローコール。
        vkCmdDrawIndexed(command_buffer, model.index_cnt, 1, 0, 0, 0);

        // end
        vkCmdEndRenderPass(command_buffer);
        vkEndCommandBuffer(command_buffer);
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
        WARN_VK(vkQueuePresentKHR(queue, &pi), "failed to enqueue present command.");
        WARN_VK(res, "failed to present.");
    }

    // termination
    vkDeviceWaitIdle(device);
    vkFreeMemory(device, model.vertex.memory, NULL);
    vkFreeMemory(device, model.index.memory, NULL);
    vkDestroyBuffer(device, model.vertex.buffer, NULL);
    vkDestroyBuffer(device, model.index.buffer, NULL);
    vkDestroyPipeline(device, pipeline, NULL);
    vkDestroyPipelineLayout(device, pipeline_layout, NULL);
    vkDestroyShaderModule(device, frag_shader, NULL);
    vkDestroyShaderModule(device, vert_shader, NULL);
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
