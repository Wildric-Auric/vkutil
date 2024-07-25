#pragma once
#include "vulkan/vulkan.h"
#include "vkdecl.h"
#include "globals.h"
#include "cmd_buffer.h"
#include "buff.h"


class img {
    public:
    VkImageCreateInfo& fillCrtInfo();
    VkResult create(const VulkanData&);
    void dstr();
    VkResult changeLyt(VkImageLayout newlyt, CmdBufferPool& ); 

    void     cpyFrom(CmdBufferPool&, img&    other, const ivec2& size, const ivec2& offset);

    void     cpyFrom(CmdBufferPool&, Buffer& buff, const ivec2& size, ui32 offset);
    
    void     genmmp(CmdBufferPool&, ui32 queueIndex);

    void setMaxmmplvl();

    VkImageCreateInfo crtInfo{};
    VkImage handle  = nullptr;
    VulkanData  _vkdata;
    VkDeviceMemory      _mem;
    VkMemoryPropertyFlags memProp = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
};

class imgView {
    public:
    VkImageViewCreateInfo& fillCrtInfo(const img&);
    VkResult create(const VulkanData&);
    void dstr();


    VkImageViewCreateInfo crtInfo{};
    VkImageView  handle  = nullptr;
    VulkanData  _vkdata;
};


class Framebuffer {
    public:
    VkFramebufferCreateInfo& fillCrtInfo();
    VkResult create(const VulkanData& vkdata, VkRenderPass rdrpass, VkImageView* attchments, ui32 attLen);
    void dstr();

    VkFramebufferCreateInfo crtInfo{};
    VkFramebuffer handle  = nullptr;
    VulkanData  _vkdata;
};

class Sampler {
    public: 
        VkResult create(const VulkanData& vkdata);
        VkSamplerCreateInfo& fillCrtInfo(const VulkanData&);
        void     dstr();

        VkSamplerCreateInfo crtInfo{};
        VkSampler           handle  = nullptr;
        VulkanData          _vkdata;
};

