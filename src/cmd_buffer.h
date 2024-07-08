#pragma once
#include "vkdecl.h"
#include "globals.h"

//TODO::an abstraction for command buffer, abstracting queue submission, a comamnd buffer must have 
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


