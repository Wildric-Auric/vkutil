#include "support.h"
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
            temp = i + 1;
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
    
    bool layerSupport(const char* layer) {
        ui32 count;
        VkLayerProperties* prop;
        
        vkEnumerateInstanceLayerProperties(&count, nullptr);
        prop = new VkLayerProperties[count];
        vkEnumerateInstanceLayerProperties(&count, prop);
        for (ui32 i = 0; i < count; ++i) {
            if (!strcmp(layer, prop[i].layerName)) {
                delete[] prop;
                return 1;
            }
        }
        delete[] prop;
        return 0;
    }

    ui32 layersSupport(const char** array, ui32 arrCount) { 
        ui32 count;
        VkLayerProperties* prop;
        ui32 ret  = 0;
        ui32 temp = 0; 
        vkEnumerateInstanceLayerProperties(&count, nullptr);
        prop = new VkLayerProperties[count];
        vkEnumerateInstanceLayerProperties(&count, prop);
        for (ui32 i = 0; i < arrCount; ++i) {
            temp = i + 1;
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


    VkPhysicalDevice selPhyDvc(VkPhysicalDevice* arr, ui32 count) {
        return arr[0];
    }
    
    void findQueues(QueueFamIndices& ind, VulkanData& vkdata) {
        ui32 count; 
        VkBool32                sup;
        VkQueueFamilyProperties prop;
        vkGetPhysicalDeviceQueueFamilyProperties(vkdata.phyDvc, &count, nullptr);
        VkQueueFamilyProperties* famArr = new VkQueueFamilyProperties[count];
        vkGetPhysicalDeviceQueueFamilyProperties(vkdata.phyDvc, &count, famArr);

        for (ui32 i = 0; i < count; ++i) {
            const VkQueueFamilyProperties& q = famArr[i];
            if (q.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
                ind.gfx = i;
            if (q.queueFlags & VK_QUEUE_TRANSFER_BIT)
                ind.trs = i;
            if (q.queueFlags & VK_QUEUE_COMPUTE_BIT) 
                ind.com = i;
            vkGetPhysicalDeviceSurfaceSupportKHR(vkdata.phyDvc, i, vkdata.srfc, &sup);
            if (sup) {
                ind.pre = i;
            }
        }
        delete[] famArr;
    }
}
