#include "vulkan-tutorial.h"

#include <string.h>

#define CHECK_RETURN(p) res = (p); if (res != VK_SUCCESS) return res;

int32_t get_memory_type_index(const VkPhysicalDeviceMemoryProperties *mem_prop, VkMemoryRequirements reqs, VkMemoryPropertyFlags flags) {
    for (int32_t i = 0; i < mem_prop->memoryTypeCount; ++i) {
        if ((reqs.memoryTypeBits & (1 << i)) && (mem_prop->memoryTypes[i].propertyFlags & flags)) {
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
    VkBuffer *p_buffer,
    VkDeviceMemory *p_device_memory
) {
    VkResult res;

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
    CHECK_RETURN(vkCreateBuffer(device, &buffer_create_info, NULL, p_buffer));

    // get memory requirements
    VkMemoryRequirements reqs;
    vkGetBufferMemoryRequirements(device, *p_buffer, &reqs);

    // allocate memory
    VkMemoryAllocateInfo allocate_info = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        NULL,
        reqs.size,
        0,
    };
    allocate_info.memoryTypeIndex = get_memory_type_index(mem_prop, reqs, flags);
    CHECK_RETURN(vkAllocateMemory(device, &allocate_info, NULL, p_device_memory));

    // bind buffer with memory
    CHECK_RETURN(vkBindBufferMemory(device, *p_buffer, *p_device_memory, 0));

    return VK_SUCCESS;
}

VkResult map_memory(const VkDevice device, const VkDeviceMemory device_memory, const void *data, int32_t size) {
    VkResult res;
    void *p;
    CHECK_RETURN(vkMapMemory(device, device_memory, 0, VK_WHOLE_SIZE, 0, &p));
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
    VkResult res;
    out->index_cnt = index_cnt;
    const size_t idxs_size = sizeof(uint32_t) * index_cnt;
    CHECK_RETURN(
        create_buffer(
            device,
            mem_prop,
            vtxs_size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &out->vtx_buffer,
            &out->vtx_memory
        )
    );
    CHECK_RETURN(
        create_buffer(
            device,
            mem_prop,
            idxs_size,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &out->idx_buffer,
            &out->idx_memory
        )
    );
    CHECK_RETURN(map_memory(device, out->vtx_memory, (void *)vtxs, vtxs_size));
    CHECK_RETURN(map_memory(device, out->idx_memory, (void *)idxs, idxs_size));
}
