#include "../common/vulkan-tutorial.h"

#include <math.h>

// A struct for organizing the layout of push constant data.
typedef struct PushConstant_t {
    float scl[4];
    float rot[4];
    float trs[4];
} PushConstant;

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

    // command buffer, fence, semaphore
    FrameData *frame_data = (FrameData *)malloc(sizeof(FrameData) * image_views_cnt);
    for (uint32_t i = 0; i < image_views_cnt; ++i) {
        // command buffer
        const VkCommandBufferAllocateInfo ai = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            NULL,
            command_pool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            1,
        };
        CHECK_VK(vkAllocateCommandBuffers(device, &ai, &frame_data[i].command_buffer), "failed to allocate a command buffers.");
        // fence
        const VkFenceCreateInfo fence_ci = {
            VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            NULL,
            VK_FENCE_CREATE_SIGNALED_BIT,
        };
        CHECK_VK(vkCreateFence(device, &fence_ci, NULL, &frame_data[i].fence), "failed to create a fence.");
        // semaphore
        const VkSemaphoreCreateInfo semaphore_ci = {
            VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            NULL,
            0,
        };
        CHECK_VK(vkCreateSemaphore(device, &semaphore_ci, NULL, &frame_data[i].semaphore), "failed to create a semaphore.");
    }

    // shaders
    VkShaderModule vert_shader;
    VkShaderModule frag_shader;
    {
        // vertex shader
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
        free(bin_vert);
        free(bin_frag);
    }

    // descriptor sets
    // NOTE: ディスクリプタセッツを作る。
    // NOTE: プッシュコンスタントとは違い、予めメモリを確保し、シェーダで使うデータをそこへ格納しておく。
    // NOTE: つまり、一フレーム中にユニフォームバッファやサンプラーの値を変えたい場合は、その分だけディスクリプタセットを作る必要がある。
    // NOTE: 今回は、描画対象が二つと確定しており、必要なディスクリプタセットが二つと確定しているため、二つ作る。
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet descriptor_sets[2];
    {
        // descriptor layout
        // TODO: メンバについて詳しく書く。
        // NOTE: どの種類が何個あるか指定する。
        const VkDescriptorSetLayoutBinding desc_set_layout_binds[] = {
            {
                0, // NOTE: バインディング番号。
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                1,
                VK_SHADER_STAGE_VERTEX_BIT,
                NULL,
            },
        };
        const VkDescriptorSetLayoutCreateInfo desc_set_layout_ci = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            NULL,
            0,
            1,
            desc_set_layout_binds,
        };
        CHECK_VK(vkCreateDescriptorSetLayout(device, &desc_set_layout_ci, NULL, &descriptor_set_layout), "failed to create a descriptor set layout.");
        // descriptor pool
        const VkDescriptorPoolSize desc_pool_sizes[] = {
            {
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                2,
            },
        };
        const VkDescriptorPoolCreateInfo desc_pool_ci = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            NULL,
            0,
            2,
            1,
            desc_pool_sizes,
        };
        CHECK_VK(vkCreateDescriptorPool(device, &desc_pool_ci, NULL, &descriptor_pool), "failed to create a descriptor pool.");
        for (uint32_t i = 0; i < 2; ++i) {
            const VkDescriptorSetAllocateInfo ai = {
                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                NULL,
                descriptor_pool,
                1,
                &descriptor_set_layout,
            };
            CHECK_VK(vkAllocateDescriptorSets(device, &ai, &descriptor_sets[i]), "failed to allocate a descriptor set.");
        }
    }

    // pipeline
    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;
    {
        const VkPushConstantRange push_constant_ranges[] = {
            {
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(PushConstant),
            },
        };
        const VkPipelineLayoutCreateInfo pipeline_layout_ci = {
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            NULL,
            0,
            1,
            &descriptor_set_layout,
            1,
            push_constant_ranges,
        };
        CHECK_VK(vkCreatePipelineLayout(device, &pipeline_layout_ci, NULL, &pipeline_layout), "failed to create a pipeline layout.");

        // shaders
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
        const VkVertexInputBindingDescription vert_inp_binding_dcs[] = {
            { 0, sizeof(float) * 3, VK_VERTEX_INPUT_RATE_VERTEX },
        };
        const VkVertexInputAttributeDescription vert_inp_attr_dcs[] = {
            { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
        };
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
        const VkPipelineInputAssemblyStateCreateInfo inp_as_ci = {
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            NULL,
            0,
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            VK_FALSE,
        };

        // viewport
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
        const VkPipelineRasterizationStateCreateInfo raster_ci = {
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            NULL,
            0,
            VK_FALSE,
            VK_FALSE,
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_COUNTER_CLOCKWISE,
            VK_FALSE,
            0.0f,
            0.0f,
            0.0f,
            1.0f,
        };

        // multisample
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
        const VkGraphicsPipelineCreateInfo cis[] = {
            {
                VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                NULL,
                0,
                2,
                shader_cis,
                &vert_inp_ci,
                &inp_as_ci,
                NULL,
                &viewport_ci,
                &raster_ci,
                &multisample_ci,
                NULL,
                &color_blend_ci,
                NULL,
                pipeline_layout,
                render_pass,
                0,
                NULL,
                0,
            },
        };
        CHECK_VK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, cis, NULL, &pipeline), "failed to create a pipeline.");
    }

    // descriptor sets for cameras
    // NOTE: カメラをディスクリプタに設定する。
    Buffer uniform_buffers[2];
    {
        // NOTE: ユニフォームバッファに格納するデータを定義する。
        // NOTE: 列優先であることに注意する。
        const float div_tanpov = 1.0f / tan(3.1415f / 4.0f);
        CameraData cameras[2] = {
            // camera 0
            {
                // NOTE: ビュー変換行列。
                // NOTE: 座標が(160, 0, -320)でZ軸正の向きを向いているようなカメラ。
                // NOTE: つまり、被写体をx方向に-160、z方向に320移動する平行移動行列。
                {
                       1.0f, 0.0f,   0.0f, 0.0f,
                       0.0f, 1.0f,   0.0f, 0.0f,
                       0.0f, 0.0f,   1.0f, 0.0f,
                    -160.0f, 0.0f, 320.0f, 1.0f,
                },
                // NOTE: 平行投影行列。
                // NOTE: 幅640、高さ480、深さ1000。
                {
                    2.0f / 640.0f,          0.0f,           0.0f, 0.0f,
                             0.0f, 2.0f / 480.0f,           0.0f, 0.0f,
                             0.0f,          0.0f, 2.0f / 1000.0f, 0.0f,
                             0.0f,          0.0f,           0.0f, 1.0f,
                },
            },
            // camera 1
            {
                // NOTE: ビュー変換行列。
                // NOTE: 座標が(-160, 0, -320)でZ軸正の向きを向いているようなカメラ。
                {
                      1.0f, 0.0f,   0.0f, 0.0f,
                      0.0f, 1.0f,   0.0f, 0.0f,
                      0.0f, 0.0f,   1.0f, 0.0f,
                    160.0f, 0.0f, 320.0f, 1.0f,
                },
                // NOTE: 透視投影行列。
                // NOTE: 視野角90度、アスペクト比4:3、near=0、far=1000。
                {
                    div_tanpov,                     0.0f, 0.0f, 0.0f,
                          0.0f, div_tanpov * 4.0f / 3.0f, 0.0f, 0.0f,
                          0.0f,                     0.0f, 1.0f, 1.0f,
                          0.0f,                     0.0f, 0.0f, 0.0f,
                },
            },
        };
        // NOTE: バッファを作り、値を格納して、ディスクリプタセットを更新する。
        for (int i = 0; i < 2; ++i) {
            // NOTE: バッファを作る。
            CHECK_VK(
                create_buffer(
                    device,
                    &phys_device_memory_prop,
                    sizeof(CameraData),
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    &uniform_buffers[i]
                ),
                "failed to create a uniform buffer."
            );
            // NOTE: バッファに値を格納する。
            CHECK_VK(
                map_memory(
                    device,
                    uniform_buffers[i].memory,
                    (void *)&cameras[i],
                    sizeof(CameraData)
                ),
                "failed to map a camera data to a uniform buffer."
            );
            // NOTE: ディスクリプタセットを更新する。
            const VkDescriptorBufferInfo bi = {
                uniform_buffers[i].buffer,
                0,
                VK_WHOLE_SIZE,
            };
            const VkWriteDescriptorSet write_desc_sets[] = {
                {
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    NULL,
                    descriptor_sets[i],
                    0,
                    0,
                    1,
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    NULL,
                    &bi,
                    NULL,
                },
            };
            vkUpdateDescriptorSets(device, 1, write_desc_sets, 0, NULL);
        }
    }

    // model
    Model model;
    {
        const float vtxs[4][3] = {
            { -0.5f,  0.5f, 0.0f },
            {  0.5f,  0.5f, 0.0f },
            {  0.5f, -0.5f, 0.0f },
            { -0.5f, -0.5f, 0.0f },
        };
        const uint32_t idxs[6] = { 0, 1, 2, 0, 2, 3 };
        CHECK_VK(
            create_model(
                device,
                &phys_device_memory_prop,
                6,
                sizeof(float) * 3 * 4,
                (const float *)vtxs,
                (const uint32_t *)idxs,
                &model
            ),
            "failed to create a model."
        );
    }

    // mainloop
    uint32_t pre_image_idx = 0;
    uint32_t cur_image_idx = 0;
    // NOTE: 視錘台にアスペクト比が反映されているおかげで、1:1のポリゴンが正方形として描かれる。
    // NOTE: その一方、視錘台の原点からの距離とポリゴンの大きさ(および拡大率)に注意しなければならない。
    // NOTE: 今回は視野角90度、アスペクト比4:3の視錘台であるため、z=500の位置では次の範囲が正規範囲に収まる：
    // NOTE:   * x: [-500, 500]
    // NOTE:   * y: [-375, 375]
    // NOTE: 例えば、右下の頂点のローカル座標がx=y=0.5であるため、z=500の位置でx方向に1000倍、y方向に750倍すると、
    // NOTE: クリッピング座標系においてx=1,y=1に位置する。
    // NOTE: 逆に、z=320の位置では、ローカル座標系において1x1である正方形は640x480のキャンバスにおける1ピクセルのように扱える。
    // NOTE: 今回、位置に関してはカメラで調整する。
    PushConstant push_constants[2] = {
        {
            { 160.0, 160.0, 1.0, 0.0 },
            { 0.0, 0.0, 0.0, 0.0 },
            { 0.0, 0.0, 0.0, 0.0 },
        },
        {
            { 160.0, 160.0, 1.0, 0.0 },
            { 0.0, 0.0, 0.0, 0.0 },
            { 0.0, 0.0, 0.0, 0.0 },
        },
    };
    while (1) {
        if (glfwWindowShouldClose(window))
            break;
        glfwPollEvents();

        push_constants[0].rot[0] += 0.01f;
        push_constants[0].rot[1] += 0.01f;
        push_constants[0].rot[2] += 0.01f;
        push_constants[1].rot[0] += 0.01f;
        push_constants[1].rot[1] += 0.01f;
        push_constants[1].rot[2] += 0.01f;

        // prepare
        WARN_VK(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, frame_data[pre_image_idx].semaphore, VK_NULL_HANDLE, &cur_image_idx), "failed to acquire a next image index.");
        WARN_VK(vkWaitForFences(device, 1, &frame_data[cur_image_idx].fence, VK_TRUE, UINT64_MAX), "failed to wait for a fence.");
        WARN_VK(vkResetFences(device, 1, &frame_data[cur_image_idx].fence), "failed to reset a fence.");
        WARN_VK(vkResetCommandBuffer(frame_data[cur_image_idx].command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT), "failed to reset a command buffer.");

        // begin
        const VkCommandBuffer command = frame_data[cur_image_idx].command_buffer;
        const VkCommandBufferBeginInfo cmd_bi = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            NULL,
            0,
            NULL,
        };
        WARN_VK(vkBeginCommandBuffer(command, &cmd_bi), "failed to begin to record commands to render.");
        const VkClearValue clear_value = {{ SCREEN_CLEAR_RGBA }};
        const VkRenderPassBeginInfo rp_bi = {
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            NULL,
            render_pass,
            framebuffers[cur_image_idx],
            { {0, 0}, surface_capabilities.currentExtent },
            1,
            &clear_value,
        };
        vkCmdBeginRenderPass(command, &rp_bi, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        // draw
        // NOTE: 今回モデルは一種類しか使わない。
        const VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(command, 0, 1, &model.vertex.buffer, &offset);
        vkCmdBindIndexBuffer(command, model.index.buffer, offset, VK_INDEX_TYPE_UINT32);
        for (int i = 0; i < 2; ++i) {
            // NOTE: ディスクリプタセットを適応する。
            // NOTE: 一度適応してからずっと同じものが使われるため、なるべく同じものを連続して使えるような順番で描画したほうが良い。
            vkCmdBindDescriptorSets(
                command,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipeline_layout,
                0, // NOTE: セット番号。
                1,
                &descriptor_sets[i],
                0, // NOTE: ダイナミックオフセットの数。
                NULL // NOTE: ダイナミックオフセットへのポインタ。
            );
            vkCmdPushConstants(command, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant), (const void *)&push_constants[i]);
            vkCmdDrawIndexed(command, model.index_cnt, 1, 0, 0, 0);
        }

        // end
        vkCmdEndRenderPass(command);
        vkEndCommandBuffer(command);
        const VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        const VkSubmitInfo submit_info = {
            VK_STRUCTURE_TYPE_SUBMIT_INFO,
            NULL,
            1,
            &frame_data[pre_image_idx].semaphore,
            &wait_stage_mask,
            1,
            &command,
            1,
            &frame_data[cur_image_idx].semaphore,
        };
        WARN_VK(vkQueueSubmit(queue, 1, &submit_info, frame_data[cur_image_idx].fence), "failed to submit commands to queue.");
        VkResult res;
        const VkPresentInfoKHR pi = {
            VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            NULL,
            1,
            &frame_data[cur_image_idx].semaphore,
            1,
            &swapchain,
            &cur_image_idx,
            &res,
        };
        WARN_VK(vkQueuePresentKHR(queue, &pi), "failed to enqueue present command.");
        WARN_VK(res, "failed to present.");

        // finish
        pre_image_idx = cur_image_idx;
    }

    // termination
    vkDeviceWaitIdle(device);
    vkFreeMemory(device, model.vertex.memory, NULL);
    vkFreeMemory(device, model.index.memory, NULL);
    vkDestroyBuffer(device, model.vertex.buffer, NULL);
    vkDestroyBuffer(device, model.index.buffer, NULL);
    for (int i = 0; i < 2; ++i) {
        vkFreeMemory(device, uniform_buffers[i].memory, NULL);
        vkDestroyBuffer(device, uniform_buffers[i].buffer, NULL);
    }
    vkDestroyPipeline(device, pipeline, NULL);
    vkDestroyPipelineLayout(device, pipeline_layout, NULL);
    vkDestroyDescriptorPool(device, descriptor_pool, NULL);
    vkDestroyDescriptorSetLayout(device, descriptor_set_layout, NULL);
    vkDestroyShaderModule(device, frag_shader, NULL);
    vkDestroyShaderModule(device, vert_shader, NULL);
    for (uint32_t i = 0; i < image_views_cnt; ++i) {
        vkDestroySemaphore(device, frame_data[i].semaphore, NULL);
        vkDestroyFence(device, frame_data[i].fence, NULL);
        vkFreeCommandBuffers(device, command_pool, 1, &frame_data[i].command_buffer);
        vkDestroyFramebuffer(device, framebuffers[i], NULL);
        vkDestroyImageView(device, image_views[i], NULL);
    }
    vkDestroyRenderPass(device, render_pass, NULL);
    vkDestroySwapchainKHR(device, swapchain, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyCommandPool(device, command_pool, NULL);
    vkDestroyDevice(device, NULL);
    DESTROY_VULKAN_DEBUG_CALLBACK(instance);
    vkDestroyInstance(instance, NULL);
    glfwTerminate();

    return 0;
}
