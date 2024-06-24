#pragma once
#include "vkdecl.h"
#include "globals.h"

class CmdBufferPool {
public:

    VkResult create(const VulkanData&, i32); 
    VkResult allocCmdBuff(VkCommandBuffer* outarr, ui32 arrLen);
    void     dstr(); 

    VulkanData _vkdata;
    VkCommandPool handle;
    VkCommandPoolCreateInfo crtInfo{};
};


