#include "vkapp.h"
#include "vkdecl.h"
#include "vulkan/vulkan_core.h"
#include "nwin/vulkan_surface.h"
#include "support.h"
#include "params.h"
#include "io.h"
#include "vertex.h"

#include <vulkan/vk_enum_string_helper.h>



Window* Window::cur = nullptr;
void Window::rszcallback(NWin::winHandle handle, NWin::Vec2 size) {
    Window::cur->_rszsignal = true; 
}

bool Window::getRszSignal() {
    return _rszsignal;
}

bool Window::consumesignal() {
    bool res = _rszsignal; 
    _rszsignal = false;
    return res;
}
i32 Vkapp::init() {
    //Open window
    win.crtInfo.metrics.size = {500, 500};
    win.ptr = NWin::Window::stCreateWindow(win.crtInfo);    
    Window::cur = &win;
    win.ptr->setResizeCallback(Window::rszcallback);
    if (!win.ptr) {return 1;}
    initVkData();
   
    VK_CHECK_EXTENDED(renderpass.create(data), "rndpass");

    VK_CHECK_EXTENDED(frame.create(data), "frame");

    VK_CHECK_EXTENDED(swpchain.create(data, win, renderpass), "Failed to create swapchain");


    std::vector<char> frag;
    std::vector<char> vert;
    
    io::readBin("..\\build\\bin\\trifrag.spv", frag );
    io::readBin("..\\build\\bin\\trivert.spv", vert );
    Shader fragS, vertS;
    
    fragS.fillCrtInfo((const ui32*)frag.data(), frag.size());
    vertS.fillCrtInfo((const ui32*)vert.data(), vert.size());

    VK_CHECK_EXTENDED(fragS.create(data), "fragshader");
    VK_CHECK_EXTENDED(vertS.create(data), "vertshader");

    fragS.fillStageCrtInfo(VK_SHADER_STAGE_FRAGMENT_BIT);
    vertS.fillStageCrtInfo(VK_SHADER_STAGE_VERTEX_BIT);

    VkPipelineShaderStageCreateInfo stages[] = {
        vertS.stageCrtInfo,
        fragS.stageCrtInfo
    };

    VulkanSupport::QueueFamIndices qfam; VulkanSupport::findQueues(qfam, data);
    VK_CHECK_EXTENDED(gfxCmdPool.create(data, qfam.gfx), "command pool");

    pipeline.fillCrtInfo();
    pipeline.crtInfo.stageCount = 2;
    pipeline.crtInfo.pStages    = stages;
    pipeline.crtInfo.renderPass = renderpass.handle;

    VK_CHECK_EXTENDED(pipeline.create(data), "Failed to create Pipeline");
    
    fragS.dstr();
    vertS.dstr();
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
    appInfo.apiVersion       =  VK_API_VERSION_1_0;
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

//This whole method is to be refactored, locality and verbosity here are only for the sake of testing
//TODO::Submission to queue must in 
i32 Vkapp::loop() {
    CmdBuff                  cmdBuff; 
    VkCommandBufferBeginInfo beginInfo{};
    VkRenderPassBeginInfo    rdrpassInfo{};
    VkSubmitInfo             submitInfo{};
    VkPresentInfoKHR         preInfo{};

    ui32 swpIndex; 

    VkQueue q = VulkanSupport::getQueue(data, offsetof(VulkanSupport::QueueFamIndices, gfx));

    gfxCmdPool.allocCmdBuff(&cmdBuff, q); 

    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = NULL;
    beginInfo.pNext = nullptr;
    beginInfo.pInheritanceInfo = nullptr;

    NWin::Vec2 size;
    win.ptr->getDrawAreaSize(size);

    rdrpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rdrpassInfo.renderPass  = renderpass.handle;
    rdrpassInfo.renderArea.offset = {0,0};
    rdrpassInfo.renderArea.extent = {(ui32)size.x, (ui32)size.y};
    //TODO::add depth clear
    VkClearValue clearCol = {1.0f, 0.05f, 0.15f, 1.0f};
    rdrpassInfo.clearValueCount     = 1;
    rdrpassInfo.pClearValues        = &clearCol;

    submitInfo.sType =  VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &cmdBuff.handle;
    submitInfo.waitSemaphoreCount   = 1;

    submitInfo.pWaitSemaphores    = &frame.semImgAvailable;
    VkPipelineStageFlags stage    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo.pWaitDstStageMask  = &stage;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &frame.semRdrFinished;

    preInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    preInfo.swapchainCount     = 1;
    preInfo.pSwapchains        = &swpchain.handle;
    preInfo.pImageIndices      = &swpIndex;
    preInfo.waitSemaphoreCount = 1;
    preInfo.pWaitSemaphores    = &frame.semRdrFinished;

    VkQueue gfxQueue;
    VkQueue preQueue;
    
    VulkanSupport::QueueFamIndices qfam; VulkanSupport::findQueues(qfam, data);

    vkGetDeviceQueue(data.dvc, qfam.gfx, 0, &gfxQueue);
    vkGetDeviceQueue(data.dvc, qfam.pre, 0, &preQueue);

    VertexObject vobj;
    VertexData strides[] = {
        { {-0.5, -0.5, .0}, {0.0, 0.0} },
        { { 0.5, -0.5, 0.0}, {1.0, 0.0} },    
        { {-0.5,  0.5, 0.0}, {0.0, 1.0} },
        { { 0.5,  0.5, 0.0}, {1.0, 1.0} }

    };

    ui32 indices[] = {
        0,1,2,
        2,1,3
    };

    vobj.create(data, gfxCmdPool, (float*)strides,  sizeof(strides) );
    vobj.createIndexBuff(data, gfxCmdPool, indices, sizeof(indices));

    VkResult res;
    while (win.ptr->shouldLoop()) {
        vkWaitForFences(data.dvc, 1, &frame.fenQueueSubmitComplete, VK_TRUE, UINT64_MAX);

        res = vkAcquireNextImageKHR(data.dvc, swpchain.handle, UINT64_MAX, frame.semImgAvailable, VK_NULL_HANDLE, &swpIndex);

        if (res == VK_ERROR_OUT_OF_DATE_KHR) { 
            swpchain.dstr();
            NWin::Vec2 size;
            win.ptr->getDrawAreaSize(size);
            while (size.x == 0 || size.y == 0) {
                win.ptr->update();
                win.ptr->getDrawAreaSize(size);
            }
            swpchain.create(data, win, renderpass);
            rdrpassInfo.renderArea.extent = {(ui32)size.x, (ui32)size.y};
            win.ptr->_getKeyboard().update();
            win.ptr->update();
            continue;
        }

        //TODO::RECREATE SWAPCHAIN IF OUT OF DATE
        vkResetFences(data.dvc, 1, &frame.fenQueueSubmitComplete);

        vkResetCommandBuffer(cmdBuff.handle, 0);
        vkBeginCommandBuffer(cmdBuff.handle, &beginInfo);
        
        rdrpassInfo.framebuffer = swpchain.fmbuffs[swpIndex].handle;
        vkCmdBeginRenderPass(cmdBuff.handle, &rdrpassInfo, VK_SUBPASS_CONTENTS_INLINE); //What is third parameter?
        vkCmdBindPipeline(cmdBuff.handle, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
      
        //Dynamic states 
        NWin::Vec2 s;
        win.ptr->getDrawAreaSize(size);

        //TODO::The spec states Viewport size must be greater than 0, handle minimization so that viewport isn't to 0, validation layers on Intel
        //Don't report that
        VkViewport viewport;
        VkRect2D   scissor;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width =  Max<i32>(size.x, 5.0);
        viewport.height = Max<i32>(size.y, 5.0);
        scissor.extent =  {(ui32)viewport.width, (ui32)viewport.height};
        scissor.offset = { 0,0 };
        vkCmdSetViewport(cmdBuff.handle, 0, 1, &viewport);
        vkCmdSetScissor(cmdBuff.handle, 0, 1, &scissor);

        VkDeviceSize voff = 0;
        vkCmdBindVertexBuffers(cmdBuff.handle, 0, 1, &vobj.buff.handle, &voff);
        vkCmdBindIndexBuffer(cmdBuff.handle, vobj.indexBuff.handle, voff, VkIndexType::VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmdBuff.handle, sizeof(indices) / sizeof(indices[0]), 1, 0, 0, 0);
        //vkCmdDraw(cmdBuff, sizeof(strides) / sizeof(VertexData) , 1, 0, 0);

        vkCmdEndRenderPass(cmdBuff.handle);
        vkEndCommandBuffer(cmdBuff.handle);

        vkQueueSubmit(cmdBuff.queue, 1, &submitInfo ,frame.fenQueueSubmitComplete);
        

        res = vkQueuePresentKHR(preQueue, &preInfo);
        
        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || win.consumesignal()) {
            swpchain.dstr();
            NWin::Vec2 size;
            win.ptr->getDrawAreaSize(size);
            while (size.x == 0 || size.y == 0) {
                win.ptr->update();
                win.ptr->getDrawAreaSize(size);
            }
            swpchain.create(data, win, renderpass);
            rdrpassInfo.renderArea.extent = {(ui32)size.x, (ui32)size.y};
        }
 
        win.ptr->_getKeyboard().update();
        win.ptr->update();
    }
    
    vkQueueWaitIdle(gfxQueue); 
    gfxCmdPool.freeCmdBuff(cmdBuff);
    vobj.dstr();

    return 0;
}

i32 Vkapp::dstr() {
    frame.dstr();

    dbgMsg.dstr();

    swpchain.dstr();
    
    renderpass.dstr();

    gfxCmdPool.dstr();

    pipeline.dstr();

    vkDestroySurfaceKHR(data.inst, data.srfc, nullptr);

    vkDestroyDevice(data.dvc, nullptr);

    vkDestroyInstance(data.inst, nullptr);

    NWin::Window::stDestroyWindow(win.ptr);

    return 0;
}
