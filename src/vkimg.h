#pragma once
#include "vulkan/vulkan.h"
#include "vkdecl.h"


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



