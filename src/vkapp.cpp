#include "vkapp.h"
#include "vkdecl.h"
#include "vulkan/vulkan_core.h"
#include "nwin/vulkan_surface.h"
#include "support.h"
#include "params.h"
#include "io.h"
#include "vertex.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

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
    NWin::Vec2 size;
    win.crtInfo.metrics.size = {720, 480};
    win.ptr = NWin::Window::stCreateWindow(win.crtInfo);
    win.ptr->getDrawAreaSize(size);
    win.drawArea.x = size.x;
    win.drawArea.y = size.y;
    Window::cur = &win;
    win.ptr->setResizeCallback(Window::rszcallback);
    if (!win.ptr) {return 1;}
    initVkData();
    
    GfxParams::inst.msaa = MSAAvalue::x16;


    VulkanSupport::QueueFamIndices qfam; VulkanSupport::findQueues(qfam, data);
    VK_CHECK_EXTENDED(gfxCmdPool.create(data, qfam.gfx), "command pool");
    VK_CHECK_EXTENDED(descPool.create(data), "Descriptor pool");


    VK_CHECK_EXTENDED(renderpass.create(data, win, true, true), "rndpass");
    renderpass.depth.image.changeLyt(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, gfxCmdPool);

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

    pipeline.fillCrtInfo();
    pipeline.crtInfo.stageCount = 2;
    pipeline.crtInfo.pStages    = stages;
    pipeline.crtInfo.renderPass = renderpass.handle;

    pipeline.layoutCrtInfo.setLayoutCount = 1;
    pipeline.layoutCrtInfo.pSetLayouts    = &descPool._lytHandle;

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
    DescSet     descSet{};
    //Creating image texture
    ui32 imgsize;
    ivec2 vecsize;
    i32   channels; 
    stbi_uc* pixels = stbi_load("../res/tex.jpg", &vecsize.x, &vecsize.y, &channels, STBI_rgb_alpha);
    imgsize = vecsize.x * vecsize.y * 4;
    Sampler smpler;
    smpler.fillCrtInfo(data);
    smpler.create(data);

    Buffer tempImg;
    img    img0;
    imgView view;

    tempImg.fillCrtInfo();
    tempImg.crtInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    tempImg.memProp       = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    tempImg.create(data, imgsize);
    void* mapped;
    tempImg.mapMem(&mapped);
    memcpy(mapped, pixels, imgsize);
    tempImg.unmapMem(&mapped);

    img0.fillCrtInfo();
    img0.crtInfo.usage         = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    img0.crtInfo.extent.width  = vecsize.x;
    img0.crtInfo.extent.height = vecsize.y;
    img0.setMaxmmplvl();
    img0.create(data);
    img0.cpyFrom(gfxCmdPool, tempImg, vecsize, 0);
    img0.genmmp(gfxCmdPool, offsetof(VulkanSupport::QueueFamIndices, gfx));

    view.fillCrtInfo(img0);
    view.create(data);

    //img0.changeLyt(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, gfxCmdPool);
    //-----
    float color = 0.5;
    UniBuff unf;
    //descPool._lytBindings.pop_back();
    unf.create(data, sizeof(color));
    descPool.allocDescSet(&descSet);
    VkWriteDescriptorSet wrt{};

    VkDescriptorBufferInfo inf{};
    inf.buffer      = unf._buff.handle; inf.range       = unf._buff._size;
    wrt.pBufferInfo = &inf; 
    wrt.descriptorCount = 1;
    wrt.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descSet.wrt(&wrt, 0);

    VkDescriptorImageInfo imInf{};
    imInf.sampler     = smpler.handle;
    imInf.imageView   = view.handle;
    imInf.imageLayout = img0.crtInfo.initialLayout;    

    wrt = VkWriteDescriptorSet{};
    wrt.descriptorCount = 1;
    wrt.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    wrt.pImageInfo      = &imInf;
    
    descSet.wrt(&wrt, 1);

    unf.wrt(&color);
    //-----

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

    Frame frame;
    frame._data.win = &win;
    frame._data.swpchain = &swpchain;
    frame._data.renderpass = &renderpass;
    frame._data.cmdBuffPool = &gfxCmdPool;
    frame.create(data);


    VkResult res;
    while (win.ptr->shouldLoop()) {        
        //TODO::CONTINUE REFACTOR; SEE FRAME class
        if (!frame.begin()) {
            continue;
        }
        vkCmdBeginRenderPass(frame.cmdBuff.handle, &frame.rdrpassInfo, VK_SUBPASS_CONTENTS_INLINE); //What is third parameter?
        vkCmdBindPipeline(frame.cmdBuff.handle, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
      
        //Dynamic states 

        VkViewport viewport;
        VkRect2D   scissor;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width =  Max<i32>(win.drawArea.x, 5.0);
        viewport.height = Max<i32>(win.drawArea.y, 5.0);
        scissor.extent =  {(ui32)viewport.width, (ui32)viewport.height};
        scissor.offset = { 0,0 };
        vkCmdSetViewport(frame.cmdBuff.handle, 0, 1, &viewport);
        vkCmdSetScissor(frame.cmdBuff.handle, 0, 1, &scissor);

        VkDeviceSize voff = 0;
        color += 0.01;
        if (color > 1.0)
            color = 0.0;
        unf.wrt(&color);
        vkCmdBindDescriptorSets(frame.cmdBuff.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline._layout, 0, 1, &descSet.handle, 0, nullptr);
        vkCmdBindVertexBuffers(frame.cmdBuff.handle, 0, 1, &vobj.buff.handle, &voff);
        vkCmdBindIndexBuffer(frame.cmdBuff.handle, vobj.indexBuff.handle, voff, VkIndexType::VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(frame.cmdBuff.handle, sizeof(indices) / sizeof(indices[0]), 1, 0, 0, 0);
        vkCmdEndRenderPass(frame.cmdBuff.handle);

        frame.end();
    }
    frame.dstr();
    vobj.dstr();
    unf.dstr();
    smpler.dstr();
    tempImg.dstr();
    img0.dstr();
    view.dstr();
    return 0;
}

i32 Vkapp::dstr() {

    dbgMsg.dstr();

    swpchain.dstr();
    
    renderpass.dstr();

    gfxCmdPool.dstr();

    descPool.dstr();

    pipeline.dstr();

    vkDestroySurfaceKHR(data.inst, data.srfc, nullptr);

    vkDestroyDevice(data.dvc, nullptr);

    vkDestroyInstance(data.inst, nullptr);

    NWin::Window::stDestroyWindow(win.ptr);

    return 0;
}

