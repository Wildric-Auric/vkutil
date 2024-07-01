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

