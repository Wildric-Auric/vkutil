#pragma once
#include "vkdecl.h"
#include "globals.h"

class CmdBufferPool {
public:

    VkResult create(const VulkanData&, i32); 
    VkResult allocCmdBuff(VkCommandBuffer* outarr, ui32 arrLen);
    void     freeCmdBuff(VkCommandBuffer cmdBuff);
    void     dstr(); 
    
    VkCommandBuffer execBegin();
    void execEnd(VkCommandBuffer, i32 queueIndex = -1);

    VulkanData _vkdata;
    VkCommandPool handle;
    VkCommandPoolCreateInfo crtInfo{};
};


