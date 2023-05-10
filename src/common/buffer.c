#include "vulkan-tutorial.h"

#include <string.h>

static int32_t get_memory_type_index(
    const VkPhysicalDeviceMemoryProperties *mem_prop,
    const VkMemoryRequirements *reqs,
    VkMemoryPropertyFlags flags
) {
    for (int32_t i = 0; i < mem_prop->memoryTypeCount; ++i) {
        if ((reqs->memoryTypeBits & (1 << i)) && (mem_prop->memoryTypes[i].propertyFlags & flags)) {
            return i;
        }
    }
    return -1;
}

VkResult create_buffer(
    const VkDevice device,
    const VkPhysicalDeviceMemoryProperties *mem_prop,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags flags,
    Buffer *out
) {
    // create a buffer
    VkBufferCreateInfo buffer_create_info = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        NULL,
        0,
        size,
        usage,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
    };
    CHECK_RETURN_VK(vkCreateBuffer(device, &buffer_create_info, NULL, &out->buffer));

    // get memory requirements
    VkMemoryRequirements reqs;
    vkGetBufferMemoryRequirements(device, out->buffer, &reqs);

    // allocate memory
    VkMemoryAllocateInfo allocate_info = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        NULL,
        reqs.size,
        0,
    };
    allocate_info.memoryTypeIndex = get_memory_type_index(mem_prop, &reqs, flags);
    CHECK_RETURN_VK(vkAllocateMemory(device, &allocate_info, NULL, &out->memory));

    // bind buffer with memory
    CHECK_RETURN_VK(vkBindBufferMemory(device, out->buffer, out->memory, 0));

    return VK_SUCCESS;
}

VkResult create_texture(
    const VkDevice device,
    const VkPhysicalDeviceMemoryProperties *mem_prop,
    VkFormat format,
    uint32_t width,
    uint32_t height,
    VkImageUsageFlags usage,
    VkImageAspectFlags aspect,
    Texture *out
) {
    // NOTE: イメージを作る。
    {
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
            usage,
            VK_SHARING_MODE_EXCLUSIVE,
            0,
            NULL,
            VK_IMAGE_LAYOUT_UNDEFINED,
        };
        CHECK_RETURN_VK(vkCreateImage(device, &ci, NULL, &out->image));
    }

    // NOTE: メモリをアロケートして、イメージと関連付ける。
    {
        VkMemoryRequirements reqs;
        vkGetImageMemoryRequirements(device, out->image, &reqs);
        uint32_t memory_type_index = get_memory_type_index(mem_prop, &reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        const VkMemoryAllocateInfo ai = {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            NULL,
            reqs.size,
            memory_type_index,
        };
        CHECK_RETURN_VK(vkAllocateMemory(device, &ai, NULL, &out->memory));
        CHECK_RETURN_VK(vkBindImageMemory(device, out->image, out->memory, 0));
    }

    // NOTE: イメージビューを作る。
    {
        const VkImageViewCreateInfo ci = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            NULL,
            0,
            out->image,
            VK_IMAGE_VIEW_TYPE_2D,
            format,
            {
                VK_COMPONENT_SWIZZLE_R,
                VK_COMPONENT_SWIZZLE_G,
                VK_COMPONENT_SWIZZLE_B,
                VK_COMPONENT_SWIZZLE_A,
            },
            { aspect, 0, 1, 0, 1 },
        };
        CHECK_RETURN_VK(vkCreateImageView(device, &ci, NULL, &out->view));
    }

    return VK_SUCCESS;
}

VkResult map_memory(const VkDevice device, const VkDeviceMemory device_memory, const void *data, int32_t size) {
    void *p;
    CHECK_RETURN_VK(vkMapMemory(device, device_memory, 0, VK_WHOLE_SIZE, 0, &p));
    memcpy(p, data, size);
    vkUnmapMemory(device, device_memory);
    return VK_SUCCESS;
}

VkResult create_model(
    const VkDevice device,
    const VkPhysicalDeviceMemoryProperties *mem_prop,
    uint32_t index_cnt,
    size_t vtxs_size,
    const float *vtxs,
    const uint32_t *idxs,
    Model *out
) {
    out->index_cnt = index_cnt;
    const size_t idxs_size = sizeof(uint32_t) * index_cnt;
    CHECK_RETURN_VK(
        create_buffer(
            device,
            mem_prop,
            vtxs_size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &out->vertex
        )
    );
    CHECK_RETURN_VK(
        create_buffer(
            device,
            mem_prop,
            idxs_size,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &out->index
        )
    );
    CHECK_RETURN_VK(map_memory(device, out->vertex.memory, (void *)vtxs, vtxs_size));
    CHECK_RETURN_VK(map_memory(device, out->index.memory, (void *)idxs, idxs_size));
}
