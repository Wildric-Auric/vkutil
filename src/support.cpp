#include "vkapp.h"
#include "vulkan/vulkan_core.h"

namespace VulkanSupport {

    bool extSupport(const char* ext) {
        ui32 count;
        VkExtensionProperties* prop;
        
        vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
        prop = new VkExtensionProperties[count];
        vkEnumerateInstanceExtensionProperties(nullptr, &count, prop);
        for (ui32 i = 0; i < count; ++i) {
            if (!strcmp(ext, prop[i].extensionName)) {
                delete[] prop;
                return 1;
            }
        }
        delete[] prop;
        return 0;
    }

    ui32 extsSupport(const char** array, ui32 arrCount) {
        ui32 count;
        VkExtensionProperties* prop;
        ui32 ret  = 0;
        ui32 temp = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
        prop = new VkExtensionProperties[count];
        vkEnumerateInstanceExtensionProperties(nullptr, &count, prop);

        for (ui32 i = 0; i < arrCount; ++i) {
            temp = i;
            for (ui32 j = 0; j < count; ++j) {
                if ( !strcmp(array[i], prop[j].extensionName) ) {
                    temp = 0;
                }
            }
            ret = temp != 0 ? temp : ret;
        }
        delete[] prop;
        return ret;
    }
    
    bool layerSupport(VulkanData& vkdata, const char* layer) {
        ui32 count;
        VkLayerProperties* prop;
        
        vkEnumerateDeviceLayerProperties(vkdata.phyDvc, &count, nullptr);
        prop = new VkLayerProperties[count];
        vkEnumerateDeviceLayerProperties(vkdata.phyDvc, &count, prop);
        for (ui32 i = 0; i < count; ++i) {
            if (!strcmp(layer, prop[i].layerName)) {
                delete[] prop;
                return 1;
            }
        }
        delete[] prop;
        return 0;
    }

    ui32 layersSupport(VulkanData& vkdata,const char** array, ui32 arrCount) { 
        ui32 count;
        VkLayerProperties* prop;
        ui32 ret  = 0;
        ui32 temp = 0; 
        vkEnumerateDeviceLayerProperties(vkdata.phyDvc, &count, nullptr);
        prop = new VkLayerProperties[count];
        vkEnumerateDeviceLayerProperties(vkdata.phyDvc, &count, prop);
        for (ui32 i = 0; i < arrCount; ++i) {
            temp = i;
            for (ui32 j = 0; j < count; ++j) {
                if (!strcmp(array[i], prop[j].layerName)) {
                    temp = 0; 
                }
            }
            ret = temp != 0 ? temp : ret;
        }
        delete[] prop;
        return 0;
    }
}
