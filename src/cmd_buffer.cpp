#include "cmd_buffer.h"
#include "support.h"


VkResult CmdBufferPool::create(const VulkanData& vkdata, i32 queueFamilyIndex) {
    _vkdata = vkdata;
    VulkanSupport::QueueFamIndices qfam;
    VulkanSupport::findQueues(qfam, _vkdata);    

    crtInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    crtInfo.queueFamilyIndex = queueFamilyIndex;
    crtInfo.flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    
    return vkCreateCommandPool(vkdata.dvc, &crtInfo, nullptr, &handle);
}

void     CmdBufferPool::dstr() {
    vkDestroyCommandPool(_vkdata.dvc, handle, nullptr);
}


VkResult CmdBufferPool::allocCmdBuff(VkCommandBuffer* outarr, ui32 arrLen) {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = handle;
  allocInfo.commandBufferCount = arrLen; 
 
  return vkAllocateCommandBuffers(_vkdata.dvc, &allocInfo, outarr);
}

void CmdBufferPool::freeCmdBuff(VkCommandBuffer cmdBuff) {
    vkFreeCommandBuffers(_vkdata.dvc, handle, 1, &cmdBuff);
}

VkCommandBuffer CmdBufferPool::execBegin() {
    VkCommandBufferAllocateInfo allocInfo{};
    VkCommandBufferBeginInfo beginInfo{};
    VkCommandBuffer buff;

    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    allocInfo.commandPool        = handle;

    vkAllocateCommandBuffers(_vkdata.dvc, &allocInfo, &buff);
    
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(buff, &beginInfo);

    return buff;
}

void CmdBufferPool::execEnd(VkCommandBuffer buff, i32 index) {
    VkQueue q;
    VkSubmitInfo info{};
    if (index == -1) {
        VulkanSupport::QueueFamIndices qfam; 
        VulkanSupport::findQueues(qfam, _vkdata);
        index = qfam.gfx;
    }
    vkGetDeviceQueue(_vkdata.dvc, index, 0, &q);

    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.commandBufferCount = 1;
    info.pCommandBuffers = &buff;
    
    vkEndCommandBuffer(buff);
    vkQueueSubmit(q, 1, &info, NULL);
    vkQueueWaitIdle(q); //TODO::Use fence instead 
    vkFreeCommandBuffers(_vkdata.dvc, handle, 1, &buff);
}



