#include "vulkan-tutorial.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

VkResult create_image_texture_from_file(
    const VkDevice device,
    const VkPhysicalDeviceMemoryProperties *mem_prop,
    const VkCommandPool command_pool,
    const VkQueue queue,
    const char *path,
    ImageTexture *out
) {
    // NOTE: 画像ファイルをstbで読み込む。
    int width = 0;
    int height = 0;
    int channel_cnt = 0;
    unsigned char *pixels = stbi_load(path, &width, &height, &channel_cnt, 0);
    CHECK_RETURN(pixels != NULL);
    CHECK_RETURN(channel_cnt == 4);

    // NOTE: 定数定義。
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    const int32_t size = width * height * 4;

    // NOTE: ステージングバッファを作る。
    Buffer staging;
    CHECK_RETURN_VK(
        create_buffer(
            device,
            mem_prop,
            (VkDeviceSize)size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &staging
        )
    );
    CHECK_RETURN_VK(map_memory(device, staging.memory, (void *)pixels, size));

    // NOTE: イメージを作る。
    VkImage image;
    {
        // TODO: もっと詳しく説明する。
        // NOTE: イメージを作る。
        const VkImageCreateInfo ci = {
            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            NULL,
            0,
            VK_IMAGE_TYPE_2D,
            format,
            { width, height, 1 },
            1,
            1,
            VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_SHARING_MODE_EXCLUSIVE,
            0,
            NULL,
            VK_IMAGE_LAYOUT_UNDEFINED,
        };
        CHECK_RETURN_VK(vkCreateImage(device, &ci, NULL, &image));
    }

    // NOTE: メモリをアロケートして、イメージと関連付ける。
    // NOTE: イメージ版のcreate_buffer。
    VkDeviceMemory memory;
    {
        VkMemoryRequirements reqs;
        vkGetImageMemoryRequirements(device, image, &reqs);
        uint32_t memory_type_index = get_memory_type_index(mem_prop, &reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        const VkMemoryAllocateInfo ai = {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            NULL,
            reqs.size,
            memory_type_index,
        };
        CHECK_RETURN_VK(vkAllocateMemory(device, &ai, NULL, &memory));
        CHECK_RETURN_VK(vkBindImageMemory(device, image, memory, 0));
    }

    // NOTE: イメージビューを作る。
    VkImageView view;
    {
        const VkImageViewCreateInfo ci = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            NULL,
            0,
            image,
            VK_IMAGE_VIEW_TYPE_2D,
            format,
            {
                VK_COMPONENT_SWIZZLE_R,
                VK_COMPONENT_SWIZZLE_G,
                VK_COMPONENT_SWIZZLE_B,
                VK_COMPONENT_SWIZZLE_A,
            },
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
        };
        CHECK_RETURN_VK(vkCreateImageView(device, &ci, NULL, &view));
    }

    // NOTE: コマンドバッファを作成し、開始する。
    VkCommandBuffer command;
    {
        // NOTE: コマンドバッファをアロケートする。
        const VkCommandBufferAllocateInfo ai = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            NULL,
            command_pool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            1,
        };
        CHECK_RETURN_VK(vkAllocateCommandBuffers(device, &ai, &command));
        // NOTE: コマンドの記録を開始する。
        const VkCommandBufferBeginInfo bi = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            NULL,
            0,
            NULL,
        };
        CHECK_RETURN_VK(vkBeginCommandBuffer(command, &bi));
    }

    // TODO: もっと詳しく説明する。
    // NOTE: コピーのためのコマンドを記録する。
    {
        VkImageMemoryBarrier image_memory_barrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            NULL,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            image,
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
        };
        vkCmdPipelineBarrier(
            command,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0,
            NULL,
            0,
            NULL,
            1,
            &image_memory_barrier
        );
        const VkBufferImageCopy copy_region = {
            0,
            0,
            0,
            { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
            { 0, 0, 0 },
            { width, height, 1 },
        };
        vkCmdCopyBufferToImage(command, staging.buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
        image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vkCmdPipelineBarrier(
            command,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0,
            NULL,
            0,
            NULL,
            1,
            &image_memory_barrier
        );
    }

    // NOTE: コマンドの記録を終了する。
    vkEndCommandBuffer(command);

    // NOTE: コマンドを提出する。
    {
        const VkSubmitInfo si = {
            VK_STRUCTURE_TYPE_SUBMIT_INFO,
            NULL,
            0,
            NULL,
            NULL,
            1,
            &command,
            0,
            NULL,
        };
        CHECK_RETURN_VK(vkQueueSubmit(queue, 1, &si, VK_NULL_HANDLE));
    }

    // NOTE: コマンドが処理されるのを待つ。フェンスを使えば全体を止めずに待機できる。
    vkDeviceWaitIdle(device);

    // NOTE: 不要なリソースを解放する。
    vkFreeCommandBuffers(device, command_pool, 1, &command);
    vkFreeMemory(device, staging.memory, NULL);
    vkDestroyBuffer(device, staging.buffer, NULL);
    stbi_image_free((void *)pixels);

    // NOTE: 結果を格納する。
    out->image = image;
    out->view = view;
    out->memory = memory;

    return VK_SUCCESS;
}
