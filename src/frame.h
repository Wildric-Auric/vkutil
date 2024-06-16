#pragma once

#include "vkdecl.h"
#include "vulkan/vulkan_core.h"
#include <vulkan/vulkan.h>

class Swapchain {
    public:
        void create(VulkanData&);
        void recreate();
        void dstr();

        VkSwapchainKHR handle;
        VulkanData     _vkdata;
};

class Frame {
    public:

};
