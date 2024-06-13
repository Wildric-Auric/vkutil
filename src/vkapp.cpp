#include "vkapp.h"
#include "vulkan/vulkan_core.h"
#include "nwin/vulkan_surface.h"
#include "support.h"
#include "params.h"
#include <string>

i32 Vkapp::init() {
    //Open window
    win.ptr = NWin::Window::stCreateWindow(win.crtInfo);    
    if (!win.ptr) {return 1;}
    initVkData();

    return 0;
}

VkResult createLogicalDevice(VulkanData& data) {
    VkDeviceCreateInfo crtInfo{};
    VkPhysicalDeviceFeatures feat;
    std::vector<VkDeviceQueueCreateInfo> queueFamCrtInfo;
    std::vector<ui32> lut;

    feat.samplerAnisotropy = GfxParams::inst.anisotropy; 

    VulkanSupport::QueueFamIndices fam;
    VulkanSupport::findQueues(fam, data);

    crtInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    crtInfo.pNext = 0;

    for (int offset = 0; offset < sizeof(VulkanSupport::QueueFamIndices); offset += sizeof(ui32)) {
        const ui32 index = *((arch*)&fam + offset);
        if (index == -1 || std::find(lut.begin(), lut.end(), index) != lut.end() ) 
            continue;
        lut.push_back(index);
        queueFamCrtInfo.push_back({});
        auto& tempCrtInfo = queueFamCrtInfo.back();
        tempCrtInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        f32 prio = 1.0;
        tempCrtInfo.pQueuePriorities = &prio;
        tempCrtInfo.queueCount = 1;
        tempCrtInfo.queueFamilyIndex = index; 
    }
    crtInfo.queueCreateInfoCount = queueFamCrtInfo.size(); 
    crtInfo.pQueueCreateInfos    = queueFamCrtInfo.data();
    //TODO::Finish impl

    return vkCreateDevice(data.phyDvc, &crtInfo, nullptr, &data.dvc);
}

int Vkapp::initVkData() {
    VkResult res;
    VkInstanceCreateInfo crtInfo{};
    VkApplicationInfo    appInfo{};
    
    std::vector<const char*> layers = {};
    std::vector<const char*> ext    = {};

    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = std::to_string((arch)this).c_str();
    appInfo.pEngineName      = "No Engine";
    appInfo.apiVersion       = VK_VERSION_1_0;
    appInfo.engineVersion     = VK_MAKE_VERSION(1,0,0);

    crtInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    crtInfo.pApplicationInfo = &appInfo;
   
    const char** windowExt = NWin::getRequiredExt();
    ext = std::vector<const char*>(windowExt, windowExt + 2);

    if (validationEnabled) {
        if ( !VulkanSupport::extSupport(VK_EXT_DEBUG_UTILS_EXTENSION_NAME) 
             || !VulkanSupport::layerSupport(data, VK_STR_VALIDATION_LAYER) ) {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        layers.push_back(VK_STR_VALIDATION_LAYER);
        ext.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        DebugMessenger dbg;
        VkDebugUtilsMessengerCreateInfoEXT& tempCrt = dbg.fillCrtInfo();
        crtInfo.pNext = &tempCrt;
    }

    crtInfo.enabledLayerCount   = layers.size();
    crtInfo.ppEnabledLayerNames = layers.data();

    crtInfo.enabledExtensionCount   = ext.size();             //Hardcoded for now
    crtInfo.ppEnabledExtensionNames = ext.data();

    res = vkCreateInstance(&crtInfo, nullptr, &data.inst);
    
    VK_CHECK_EXTENDED(res, "Failed to create vulkan instance"); 
    //Create Debug Messenger
    dbgMsg.create(data); 
    //Create Surface
    NWin::createSurface(win.ptr, data.inst, data.srfc);
    VK_CHECK_EXTENDED(res, "Failed to create window surface");
    //Physical device
    ui32 count; 
    VkPhysicalDevice* arr;
    vkEnumeratePhysicalDevices(data.inst, &count, nullptr);
    arr = new VkPhysicalDevice[count];
    vkEnumeratePhysicalDevices(data.inst, &count, arr);
    data.phyDvc = VulkanSupport::selPhyDvc(arr, count);
    delete[] arr;
    //Logical device
    createLogicalDevice(data);
    return res; 
}

i32 Vkapp::loop() {
    return 0;
}

i32 Vkapp::dstr() {

    dbgMsg.dstry();
    vkDestroyInstance(data.inst, nullptr);
    NWin::Window::stDestroyWindow(win.ptr);

    return 0;
}


