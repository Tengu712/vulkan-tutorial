#include "../common/vulkan-tutorial.h"

#include <math.h>

// A struct for vertex input data.
typedef struct Vertex_t {
    float pos[3];
    float uv[2];
} Vertex;

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
    const uint32_t render_pass_attachments_count = 2; // NOTE: アタッチメントを増やすので。
    const VkFormat depth_format = VK_FORMAT_D32_SFLOAT; // NOTE: デプスバッファ作成時で使うので。
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
            // NOTE: デプスバッファを設定する。
            {
                0,
                depth_format,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            },
        };
        const VkAttachmentReference color_refs[] = {
            {
                0,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            },
        };
        // NOTE: デプスバッファの参照を設定する。
        // NOTE: 各サブパスに一つ設定するため、一つ。
        const VkAttachmentReference depth_ref = {
            1,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };
        const VkSubpassDescription subpass_descs[] = {
            {
                0,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                0,
                NULL,
                1,
                color_refs,
                NULL,
                &depth_ref, // NOTE: ここも忘れずに。
                0,
                NULL,
            },
        };
        // NOTE: 
        const VkSubpassDependency dependencies[] = {
            {
                0,
                0,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                0,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_DEPENDENCY_BY_REGION_BIT,
            },
        };
        const VkRenderPassCreateInfo ci = {
            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            NULL,
            0,
            2, // NOTE: 忘れずに。
            attachment_descs,
            1,
            subpass_descs,
            1,
            dependencies,
        };
        CHECK_VK(vkCreateRenderPass(device, &ci, NULL, &render_pass), "failed to create a render pass.");
    }

    // depth buffer
    // NOTE: 
    Texture *depth_buffers = (Texture *)malloc(sizeof(Texture) * image_views_cnt);
    for (int i = 0; i < image_views_cnt; ++i) {
        CHECK_VK(
            create_texture(
                device,
                &phys_device_memory_prop,
                depth_format,
                surface_capabilities.currentExtent.width,
                surface_capabilities.currentExtent.height,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT,
                &depth_buffers[i]
            ),
            "failed to create a depth buffer."
        );
    }

    // framebuffers
    VkFramebuffer *framebuffers;
    {
        VkFramebufferCreateInfo ci = {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            NULL,
            0,
            render_pass,
            render_pass_attachments_count,
            NULL,
            surface_capabilities.currentExtent.width,
            surface_capabilities.currentExtent.height,
            1,
        };
        framebuffers = (VkFramebuffer *)malloc(sizeof(VkFramebuffer) * image_views_cnt);
        for (int32_t i = 0; i < image_views_cnt; ++i) {
            VkImageView attachments[] = { image_views[i], depth_buffers[i].view };
            ci.pAttachments = attachments;
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

    // sampler
    VkSampler sampler;
    {
        const VkSamplerCreateInfo ci = {
            VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            NULL,
            0,
            VK_FILTER_LINEAR,
            VK_FILTER_LINEAR,
            VK_SAMPLER_MIPMAP_MODE_NEAREST,
            VK_SAMPLER_ADDRESS_MODE_REPEAT,
            VK_SAMPLER_ADDRESS_MODE_REPEAT,
            VK_SAMPLER_ADDRESS_MODE_REPEAT,
            0.0,
            0,
            1.0,
            0,
            VK_COMPARE_OP_NEVER,
            0.0,
            0.0,
            VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
            0,
        };
        CHECK_VK(vkCreateSampler(device, &ci, NULL, &sampler), "failed to create a sampler.");
    }

    // descriptor sets
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet descriptor_set;
    {
        // descriptor layout
        const VkDescriptorSetLayoutBinding desc_set_layout_binds[] = {
            {
                0,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                1,
                VK_SHADER_STAGE_VERTEX_BIT,
                NULL,
            },
            {
                1,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                1,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                NULL,
            },
        };
        const VkDescriptorSetLayoutCreateInfo desc_set_layout_ci = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            NULL,
            0,
            2,
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
        const VkDescriptorSetAllocateInfo ai = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            NULL,
            descriptor_pool,
            1,
            &descriptor_set_layout,
        };
        CHECK_VK(vkAllocateDescriptorSets(device, &ai, &descriptor_set), "failed to allocate a descriptor set.");
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
            { 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX },
        };
        const VkVertexInputAttributeDescription vert_inp_attr_dcs[] = {
            { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
            { 1, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 3 },
        };
        const VkPipelineVertexInputStateCreateInfo vert_inp_ci = {
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            NULL,
            0,
            1,
            vert_inp_binding_dcs,
            2,
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

        // depth stencil
        // TODO: 
        // NOTE: デプス/ステンシルテストの設定をする。
        const VkPipelineDepthStencilStateCreateInfo depth_stencil_ci = {
            VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            NULL,
            0,
            VK_TRUE,
            VK_TRUE,
            VK_COMPARE_OP_LESS_OR_EQUAL,
            VK_FALSE,
            VK_FALSE,
            { 0 },
            { 0 },
            0.0f,
            0.0f,
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
                &depth_stencil_ci, // NOTE: ここも忘れずに。
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
    Buffer uniform_buffer;
    Texture img_tex;
    {
        // camera
        const float div_tanpov = 1.0f / tan(3.1415f / 4.0f);
        const float div_depth = 1.0f / (1000.0f - 100.0f);
        CameraData camera = {
            {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 320.0f, 1.0f,
            },
            {
                div_tanpov,                     0.0f,                         0.0f, 0.0f,
                      0.0f, div_tanpov * 4.0f / 3.0f,                         0.0f, 0.0f,
                      0.0f,                     0.0f,          1000.0f * div_depth, 1.0f,
                      0.0f,                     0.0f, 100.0f * 1000.0f * div_depth, 0.0f,
            },
        };
        CHECK_VK(
            create_buffer(
                device,
                &phys_device_memory_prop,
                sizeof(CameraData),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &uniform_buffer
            ),
            "failed to create a uniform buffer."
        );
        CHECK_VK(
            map_memory(
                device,
                uniform_buffer.memory,
                (void *)&camera,
                sizeof(CameraData)
            ),
            "failed to map a camera data to a uniform buffer."
        );
        // image
        CHECK_VK(
            create_image_texture_from_file(device, &phys_device_memory_prop, command_pool, queue, "../img/shape.png", &img_tex),
            "failed to create a image texture."
        );
        // update
        const VkDescriptorBufferInfo bi = {
            uniform_buffer.buffer,
            0,
            VK_WHOLE_SIZE,
        };
        const VkDescriptorImageInfo ii = {
            sampler,
            img_tex.view,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
        const VkWriteDescriptorSet write_desc_sets[] = {
            {
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                NULL,
                descriptor_set,
                0,
                0,
                1,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                NULL,
                &bi,
                NULL,
            },
            {
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                NULL,
                descriptor_set,
                1,
                0,
                1,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                &ii,
                NULL,
                NULL,
            },
        };
        vkUpdateDescriptorSets(device, 2, write_desc_sets, 0, NULL);
    }

    // models
    Model cube;
    {
        const Vertex vtxs[24] = {
            // NOTE: 前
            { { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f } },
            { {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f } },
            { {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f } },
            { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f } },
            // NOTE: 奥
            { {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f } },
            { { -0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f } },
            { { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f } },
            { {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f } },
            // NOTE: 左
            { { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f } },
            { { -0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f } },
            { { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f } },
            { { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f } },
            // NOTE: 右
            { {  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f } },
            { {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f } },
            { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f } },
            { {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f } },
            // NOTE: 上
            { { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f } },
            { {  0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f } },
            { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f } },
            { { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f } },
            // NOTE: 下
            { { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f } },
            { {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f } },
            { {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f } },
            { { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f } },
        };
        const uint32_t idxs[36] = {
             0,  1,  2,  0,  2,  3,
             4,  5,  6,  4,  6,  7,
             8,  9, 10,  8, 10, 11,
            12, 13, 14, 12, 14, 15,
            16, 17, 18, 16, 18, 19,
            20, 21, 22, 20, 22, 23,
        };
        CHECK_VK(
            create_model(
                device,
                &phys_device_memory_prop,
                36,
                sizeof(Vertex) * 24,
                (const float *)vtxs,
                (const uint32_t *)idxs,
                &cube
            ),
            "failed to create a model."
        );
    }
    Model square;
    {
        const Vertex vtxs[] = {
            { { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f } },
            { {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f } },
            { {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f } },
            { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f } },
        };
        const uint32_t idxs[] = {
            0, 1, 2, 0, 2, 3,
        };
        CHECK_VK(
            create_model(
                device,
                &phys_device_memory_prop,
                6,
                sizeof(Vertex) * 4,
                (const float *)vtxs,
                (const uint32_t *)idxs,
                &square
            ),
            "failed to create a model."
        );
    }

    // mainloop
    uint32_t pre_image_idx = 0;
    uint32_t cur_image_idx = 0;
    PushConstant pc_cube = {
        { 160.0f, 160.0f, 160.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 0.0f },
    };
    PushConstant pc_square = {
        { 320.0f, 320.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 0.0f },
    };
    while (1) {
        if (glfwWindowShouldClose(window))
            break;
        glfwPollEvents();

        pc_cube.rot[0] += 0.01;
        pc_cube.rot[1] += 0.01;
        pc_cube.rot[2] += 0.01;

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
        const VkClearValue clear_values[] = {
            { SCREEN_CLEAR_RGBA },
            { 1.0f, 0.0f }, // NOTE: デプスバッファのクリア値。
        };
        const VkRenderPassBeginInfo rp_bi = {
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            NULL,
            render_pass,
            framebuffers[cur_image_idx],
            { {0, 0}, surface_capabilities.currentExtent },
            2, // NOTE: 忘れずに。
            clear_values,
        };
        vkCmdBeginRenderPass(command, &rp_bi, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        // draw a cube
        const VkDeviceSize offset1 = 0;
        vkCmdBindVertexBuffers(command, 0, 1, &cube.vertex.buffer, &offset1);
        vkCmdBindIndexBuffer(command, cube.index.buffer, offset1, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(
            command,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline_layout,
            0,
            1,
            &descriptor_set,
            0,
            NULL
        );
        vkCmdPushConstants(command, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant), (const void *)&pc_cube);
        vkCmdDrawIndexed(command, cube.index_cnt, 1, 0, 0, 0);

        // draw a square
        const VkDeviceSize offset2 = 0;
        vkCmdBindVertexBuffers(command, 0, 1, &square.vertex.buffer, &offset2);
        vkCmdBindIndexBuffer(command, square.index.buffer, offset2, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(
            command,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline_layout,
            0,
            1,
            &descriptor_set,
            0,
            NULL
        );
        vkCmdPushConstants(command, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant), (const void *)&pc_square);
        vkCmdDrawIndexed(command, square.index_cnt, 1, 0, 0, 0);

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
    vkFreeMemory(device, square.vertex.memory, NULL);
    vkFreeMemory(device, square.index.memory, NULL);
    vkDestroyBuffer(device, square.vertex.buffer, NULL);
    vkDestroyBuffer(device, square.index.buffer, NULL);
    vkFreeMemory(device, cube.vertex.memory, NULL);
    vkFreeMemory(device, cube.index.memory, NULL);
    vkDestroyBuffer(device, cube.vertex.buffer, NULL);
    vkDestroyBuffer(device, cube.index.buffer, NULL);
    vkFreeMemory(device, uniform_buffer.memory, NULL);
    vkDestroyBuffer(device, uniform_buffer.buffer, NULL);
    vkFreeMemory(device, img_tex.memory, NULL);
    vkDestroyImageView(device, img_tex.view, NULL);
    vkDestroyImage(device, img_tex.image, NULL);
    vkDestroyPipeline(device, pipeline, NULL);
    vkDestroyPipelineLayout(device, pipeline_layout, NULL);
    vkDestroyDescriptorPool(device, descriptor_pool, NULL);
    vkDestroyDescriptorSetLayout(device, descriptor_set_layout, NULL);
    vkDestroySampler(device, sampler, NULL);
    vkDestroyShaderModule(device, frag_shader, NULL);
    vkDestroyShaderModule(device, vert_shader, NULL);
    for (uint32_t i = 0; i < image_views_cnt; ++i) {
        vkDestroySemaphore(device, frame_data[i].semaphore, NULL);
        vkDestroyFence(device, frame_data[i].fence, NULL);
        vkFreeCommandBuffers(device, command_pool, 1, &frame_data[i].command_buffer);
        vkDestroyFramebuffer(device, framebuffers[i], NULL);
        vkDestroyImageView(device, image_views[i], NULL);
        vkFreeMemory(device, depth_buffers[i].memory, NULL);
        vkDestroyImageView(device, depth_buffers[i].view, NULL);
        vkDestroyImage(device, depth_buffers[i].image, NULL);
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
