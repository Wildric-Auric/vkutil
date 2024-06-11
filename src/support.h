#pragma once

#include "globals.h"
#include "vkapp.h"

#include <vulkan/vulkan.h>

namespace VulkanSupport {
    bool extSupport(const char* ext);
    ui32 extsSupport(const char** array, ui32 arrCount);
    
    bool layerSupport(VulkanData& vkdata, const char* layer);
    ui32 layersSupport(VulkanData& vkdata,const char** array, ui32 arrCount);

    VkPhysicalDevice selPhyDvc(VkPhysicalDevice* arr, ui32 count);
}
