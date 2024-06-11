#pragma once
#include "vulkan/vulkan.h"

#define VK_STR_VALIDATION_LAYER "VK_LAYER_KHRONOS_validation"

struct VulkanData {
    VkPhysicalDevice phyDvc;
    VkDevice         dvc;
    VkInstance       inst;
    VkSurfaceKHR     srfc;
};
