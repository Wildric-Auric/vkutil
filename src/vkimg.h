#pragma once
#include "vulkan/vulkan.h"
#include "vkdecl.h"
#include "globals.h"

class imgView {
    public:
    VkImageViewCreateInfo& fillCrtInfo();
    VkResult create(const VulkanData&);
    void dstr();


    VkImageViewCreateInfo crtInfo{};
    VkImageView  handle  = nullptr;
    VulkanData  _vkdata;
};

class img {
    public:
    VkImageCreateInfo& fillCrtInfo();
    VkResult create(const VulkanData&);
    void dstr();


    VkImageCreateInfo crtInfo{};
    VkImage handle  = nullptr;
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
