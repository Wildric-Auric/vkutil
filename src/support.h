#pragma once

#include "globals.h"
#include "vkapp.h"

namespace VulkanSupport {
    bool extSupport(const char* ext);
    ui32 extsSupport(const char* array, ui32 arrCount);
    
    bool layerSupport(const char* layer);
    ui32 layersSupport(VulkanData& vkdata,const char** array, ui32 arrCount);
}
