#include "vkapp.h"
#include "vkdecl.h"
#include "vulkan/vulkan_core.h"
#include "nwin/vulkan_surface.h"
#include "support.h"
#include "params.h"
#include "io.h"

i32 Vkapp::init() {
    //Open window
    win.ptr = NWin::Window::stCreateWindow(win.crtInfo);    
    if (!win.ptr) {return 1;}
    initVkData();
   
    VK_CHECK_EXTENDED(swpchain.create(data, win), "Failed to create swapchain");


    std::vector<char> frag;
    std::vector<char> vert;
    
    io::readBin("..\\build\\bin\\trifrag.spv", frag );
    io::readBin("..\\build\\bin\\trivert.spv", vert );
    Shader fragS, vertS;
    
    fragS.fillCrtInfo((const ui32*)frag.data(), frag.size());
    vertS.fillCrtInfo((const ui32*)vert.data(), vert.size());

    fragS.create(data);
    vertS.create(data);

    fragS.fillStageCrtInfo(VK_SHADER_STAGE_FRAGMENT_BIT);
    vertS.fillStageCrtInfo(VK_SHADER_STAGE_VERTEX_BIT);

    VkPipelineShaderStageCreateInfo stages[] = {
        vertS.stageCrtInfo,
        fragS.stageCrtInfo
    };

    pipeline.fillCrtInfo();
    pipeline.crtInfo.stageCount = 2;
    pipeline.crtInfo.pStages    = stages;

    VK_CHECK_EXTENDED(pipeline.create(data), "Failed to create Pipeline");

    return 0;
}

VkResult createLogicalDevice(VulkanData& data) {
    VkDeviceCreateInfo crtInfo{};
    VkPhysicalDeviceFeatures feat{};
    std::vector<const char*> ext = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    std::vector<VkDeviceQueueCreateInfo> queueFamCrtInfo;
    std::vector<ui32> lut;

    feat.samplerAnisotropy = GfxParams::inst.anisotropy; 

    VulkanSupport::QueueFamIndices fam;
    VulkanSupport::findQueues(fam, data);

    crtInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    crtInfo.pNext = 0;

    for (int offset = 0; offset < sizeof(VulkanSupport::QueueFamIndices); offset += sizeof(fam.gfx)) {
        const i32 index = *( (i32*)((arch)(void*)&fam + offset) );
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
    crtInfo.pEnabledFeatures     = &feat;

    bool validation = 1; //TODO::Refactor this;
    const char* validationLayer = VK_STR_VALIDATION_LAYER;
    if (validation) {
        crtInfo.enabledLayerCount = 1;
        crtInfo.ppEnabledLayerNames = &validationLayer;
    }

    crtInfo.enabledExtensionCount = ext.size();
    crtInfo.ppEnabledExtensionNames = ext.data();

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
             || !VulkanSupport::layerSupport(VK_STR_VALIDATION_LAYER) ) {
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
    bool validation = true;
    if (validation) {
        dbgMsg.fillCrtInfo();
        dbgMsg.create(data); 
    }
    //Create Surface
    NWin::createSurface(win.ptr, data.inst, &data.srfc);
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
    VK_CHECK_EXTENDED(
            createLogicalDevice(data), 
            "Failed to create logical device"
            )
    return res; 
}

i32 Vkapp::loop() {
    return 0;
}

i32 Vkapp::dstr() {

    dbgMsg.dstr();

    swpchain.dstr();

    vkDestroySurfaceKHR(data.inst, data.srfc, nullptr);
    vkDestroyDevice(data.dvc, nullptr);

    vkDestroyInstance(data.inst, nullptr);
    NWin::Window::stDestroyWindow(win.ptr);

    return 0;
}


