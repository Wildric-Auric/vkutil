#include "frame.h" 
#include "support.h" 
#include "vkimg.h"
#include "params.h"


VkResult Swapchain::create(const VulkanData& vkdata, const Window& win, Renderpass rdrpass) {
    VkResult res;
    VkSwapchainCreateInfoKHR crtInfo{};
    VkSurfaceFormatKHR srfcFmt;
    ui32 imgCount;

    VulkanSupport::SwpchainCap spec;
    _vkdata = vkdata;
    
    VulkanSupport::getSwapchaincap(_vkdata, spec);
    srfcFmt = spec.srfcFormats[VulkanSupport::selSrfcFmt(spec)];

    imgCount = spec.cap.minImageCount + 1;
    if (spec.cap.maxImageCount > 0 && imgCount > spec.cap.maxImageCount) {
        imgCount = spec.cap.maxImageCount;
    }

    crtInfo.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    crtInfo.surface = vkdata.srfc;

    crtInfo.imageFormat = srfcFmt.format;
    crtInfo.imageColorSpace = srfcFmt.colorSpace;

    crtInfo.minImageCount = imgCount;
    crtInfo.imageArrayLayers = 1; 
    chooseExtent(win, spec.cap, &crtInfo.imageExtent);
    crtInfo.imageUsage  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //Render directly
    crtInfo.presentMode = VulkanSupport::selPresent();

    crtInfo.preTransform   = spec.cap.currentTransform;
    crtInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    crtInfo.clipped        = VK_TRUE;
    crtInfo.oldSwapchain   = VK_NULL_HANDLE;


    VulkanSupport::QueueFamIndices fam;
    VulkanSupport::findQueues(fam, _vkdata);
    //Only present family and gfx have access for now
    i32 indices[] = {
        fam.gfx,
        fam.pre
    };

    if (fam.pre == fam.gfx) {
        crtInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        crtInfo.pQueueFamilyIndices = nullptr;
        crtInfo.queueFamilyIndexCount = 0;
    }
    else {
        crtInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        crtInfo.pQueueFamilyIndices   = (const ui32*)indices;
        crtInfo.queueFamilyIndexCount = sizeof(indices) / sizeof(indices[0]);
    }
    res = vkCreateSwapchainKHR(vkdata.dvc, &crtInfo, nullptr, &handle);
    VK_CHECK_EXTENDED(res, "Failed to create swapchain");
    vkGetSwapchainImagesKHR(vkdata.dvc, handle, &imgCount, nullptr);
    imgs.resize(imgCount);
    vkGetSwapchainImagesKHR(vkdata.dvc, handle, &imgCount, imgs.data());
    Img fake;
    //Create swapchaine image views and framebuffers
    views.resize(imgCount);
    fmbuffs.resize(imgCount);
    for (arch i = 0; i < imgCount; ++i) {
        fake.crtInfo.format = srfcFmt.format;
        fake.crtInfo.samples = (VkSampleCountFlagBits)GfxParams::inst.msaa;
        fake.handle         = imgs[i];
        views[i].fillCrtInfo(fake);
        views[i].create(_vkdata);

        fmbuffs[i].fillCrtInfo();
        fmbuffs[i].crtInfo.width  = crtInfo.imageExtent.width;
        fmbuffs[i].crtInfo.height = crtInfo.imageExtent.height;
        //TODO::Fix support for multiple Renderpasses
        std::vector<VkImageView> att;
        att.push_back(views[i].handle);

        for (AttachmentData& data : rdrpass._subpasses.resources) {
            att.push_back(data.view.handle);
        }

        res = fmbuffs[i].create(_vkdata, rdrpass.handle, att.data(), att.size());
    }
    return res;
}

void Swapchain::chooseExtent(const Window& win, const VkSurfaceCapabilitiesKHR& cap, VkExtent2D* const outExt  ) {

    if (cap.currentExtent.width != (ui32)(-1)) {
        *outExt = cap.currentExtent;
        return;
    }

    ivec2 size = win.drawArea;
    outExt->width = Clamp<ui32>(size.x, cap.minImageExtent.width, cap.maxImageExtent.width);
    outExt->height= Clamp<ui32>(size.y, cap.minImageExtent.height, cap.maxImageExtent.height);
}

void Swapchain::dstr() {
    vkDeviceWaitIdle(_vkdata.dvc);
    for (arch i = 0; i < imgs.size(); ++i) {
        views[i].dstr();
        fmbuffs[i].dstr();
    }
    vkDestroySwapchainKHR(_vkdata.dvc, handle, nullptr);
}

void Attachment::setup() {
    desc.format      = VK_FORMAT_R8G8B8A8_UNORM;
    desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    desc.finalLayout   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    desc.samples       = (VkSampleCountFlagBits)GfxParams::inst.msaa;
    desc.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;
    desc.storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
    desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    desc.stencilStoreOp= VK_ATTACHMENT_STORE_OP_DONT_CARE;
    ref.layout         = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}

Attachment* AttachmentContainer::getDepth() {
    if (_hasDepth) return nullptr;
    return &depth;
}

Attachment* AttachmentContainer::getResolve() {
    if (_hasResolve) return nullptr;
    return &resolve;
}

Attachment* AttachmentContainer::get(arch index) {
    return &_container[index];
}

Attachment* AttachmentContainer::add() {
    _container.push_back({});
    Attachment* att = &_container.back();
    att->setup();
    return att;
}

Attachment*  AttachmentContainer::addDepth() {
    Attachment* att = &depth;
    depth.setup();
    att->desc.format      = VK_FORMAT_D24_UNORM_S8_UINT;
    att->desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    att->desc.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
    att->ref.layout       = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

   _hasDepth = true; 
    return att;
}

Attachment*  AttachmentContainer::addResolve() {
    Attachment* att = &resolve;
    resolve.setup();
    att->desc.samples     = VK_SAMPLE_COUNT_1_BIT;
    att->desc.format      = VK_FORMAT_R8G8B8A8_SRGB;
    att->desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; 
    att->desc.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
    att->ref.layout       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    _hasResolve = true;
    return att;
}

void SubpassContainer::addDepthRes(const Window& win, const VulkanData& _vkdata) {
        resources.push_back({});
        AttachmentData& depth = resources.back();

        depth.image.fillCrtInfo();
        depth.image.crtInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        depth.image.crtInfo.samples       = (VkSampleCountFlagBits)GfxParams::inst.msaa;
        depth.image.crtInfo.format        = VK_FORMAT_D24_UNORM_S8_UINT;
        depth.image.crtInfo.extent.width  =  win.drawArea.x; 
        depth.image.crtInfo.extent.height = win.drawArea.y;
        depth.image.crtInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth.image.create(_vkdata);

        depth.view.fillCrtInfo(depth.image);
        depth.view.crtInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depth.view.create(_vkdata);
}

void SubpassContainer::addResolveRes(const Window& win, const VulkanData& _vkdata) {
       resources.push_back({});
       AttachmentData& msaaA = resources.back();

       msaaA.image.fillCrtInfo();
       msaaA.image.crtInfo.usage   = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
       msaaA.image.crtInfo.samples = (VkSampleCountFlagBits)GfxParams::inst.msaa;
       msaaA.image.crtInfo.extent.width  = win.drawArea.x;
       msaaA.image.crtInfo.extent.height = win.drawArea.y;
       msaaA.image.create(_vkdata);

       msaaA.view.fillCrtInfo(msaaA.image);
       msaaA.view.crtInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
       msaaA.view.create(_vkdata);
}

void SubpassContainer::add(const Window& win, const VulkanData& vkdata, AttachmentContainer& atts, VkSubpassDependency** depedencyWithPrevious) {
    Subpass s;
    ui32 beg = attDescs.size();
    ui32 cur = attDescs.size();
    ui32 resOffset  = 0;
    ui32 dpthOffset = 0;
    for (Attachment& att : atts._container) {
        att.ref.attachment = cur++; 
        attDescs.push_back(att.desc);
        attRefs.push_back(att.ref);
    }

    if (atts._hasResolve) {
        resOffset = cur;
        atts.resolve.ref.attachment = cur++; 
        attDescs.push_back(atts.resolve.desc);
        attRefs.push_back(atts.resolve.ref);
        addResolveRes(win, vkdata);
    }

    if (atts._hasDepth) {
        dpthOffset = cur;
        atts.depth.ref.attachment = cur++;
        attDescs.push_back(atts.depth.desc);
        attRefs.push_back(atts.depth.ref);
        addDepthRes(win, vkdata);
    }

    s.desc.pColorAttachments       = attRefs.data() + beg;
    s.desc.colorAttachmentCount    = atts._container.size();
    s.desc.pResolveAttachments     = resOffset == 0 ? nullptr:attRefs.data() + resOffset; //TODO::Chnage when I have multiple resolve attachments
    s.desc.pDepthStencilAttachment = dpthOffset == 0? nullptr:attRefs.data() + dpthOffset;

    descs.push_back(s.desc);

    _dpn.push_back({});
    VkSubpassDependency& t = _dpn.back();
    if (_dpn.size() == 1) {
        t.srcSubpass           = VK_SUBPASS_EXTERNAL;
        t.dstSubpass           = 0;

        t.srcStageMask         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        t.dstStageMask         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; 

        t.srcAccessMask        = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        t.dstAccessMask        = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }

    else {
        //Default is subpasses wait for previous to finish
        t.srcSubpass           = _dpn.size() - 2;
        t.dstSubpass           = _dpn.size() - 1;

        t.srcStageMask         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        t.dstStageMask         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; 

        t.srcAccessMask        = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        t.dstAccessMask        = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    }
    if (depedencyWithPrevious != nullptr)
        *depedencyWithPrevious = &t;
    return ;
}


VkResult Renderpass::create(const VulkanData& vkdata, const Window& win) {
    VkRenderPassCreateInfo crtInfo{};

    _vkdata = vkdata;
     
    crtInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    crtInfo.dependencyCount = 1;
    crtInfo.pDependencies   = _subpasses._dpn.data();
    crtInfo.subpassCount    = _subpasses.descs.size();
    crtInfo.pSubpasses      = _subpasses.descs.data();
    crtInfo.attachmentCount = _subpasses.attDescs.size();
    crtInfo.pAttachments    = _subpasses.attDescs.data();
    
    VkResult res = vkCreateRenderPass(_vkdata.dvc, &crtInfo, nullptr, &handle);
 
    return res;
}

void Renderpass::dstr() {
    vkDestroyRenderPass(_vkdata.dvc, handle, nullptr);
    dstrRes();
}

void Renderpass::dstrRes() {
    for (AttachmentData& d : _subpasses.resources) {
            d.view.dstr();
            d.image.dstr();
            d.view.crtInfo.format = (VkFormat)0;
    }
}


void Frame::end() {
        VkResult res;
        vkEndCommandBuffer(cmdBuff.handle);
        vkQueueSubmit(cmdBuff.queue, 1, &submitInfo ,fenQueueSubmitComplete);
        res = vkQueuePresentKHR(preQueue, &preInfo);
        
        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || _data.win->consumesignal()) {
            NWin::Vec2 size;
            _data.win->ptr->getDrawAreaSize(size);
            _data.win->drawArea.x = size.x;
            _data.win->drawArea.y = size.y;
            while (size.x == 0 || size.y == 0) {
                _data.win->ptr->update();
                _data.win->ptr->getDrawAreaSize(size);
                _data.win->drawArea.x = size.x;
                _data.win->drawArea.y = size.y;
            }
            _data.swpchain->dstr();
            _data.renderpass->dstrRes();
            //TODO::Finish THIS
//            _data.renderpass->createRes(*_data.win, _data.renderpass->depth.valid, _data.renderpass->msaaA.valid);
//            _data.renderpass->depth.image.changeLyt(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, *_data.cmdBuffPool);
            _data.swpchain->create(_vkdata, *_data.win, *_data.renderpass);
            rdrpassInfo.renderArea.extent = {(ui32)size.x, (ui32)size.y};
        }
 
        _data.win->ptr->_getKeyboard().update();
        _data.win->ptr->update();
}

VkResult Frame::create(const VulkanData& vkdata) {
    _vkdata = vkdata;
    VkFenceCreateInfo     fenCrtInfo;
    VkSemaphoreCreateInfo semCrtInfo;

    fenCrtInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenCrtInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    fenCrtInfo.pNext = nullptr;

    semCrtInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semCrtInfo.pNext = nullptr;
    semCrtInfo.flags = NULL;

    VkResult res = vkCreateFence(_vkdata.dvc, &fenCrtInfo, nullptr, &fenQueueSubmitComplete);
    VkResult res0 = vkCreateSemaphore(_vkdata.dvc, &semCrtInfo, nullptr, &semImgAvailable);
    VkResult res1 = vkCreateSemaphore(_vkdata.dvc, &semCrtInfo, nullptr, &semRdrFinished);

    if (res != VK_SUCCESS) {
        return res; 
    }
    else if (res0 != VK_SUCCESS) {
        return res0; 
    }
    else if (res1 != VK_SUCCESS) {
        return res1;
    }


    VkQueue q = VulkanSupport::getQueue(_vkdata, offsetof(VulkanSupport::QueueFamIndices, gfx));
    _data.cmdBuffPool->allocCmdBuff(&cmdBuff, q); 
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = NULL;
    beginInfo.pNext = nullptr;
    beginInfo.pInheritanceInfo = nullptr;

    NWin::Vec2 size;
    _data.win->ptr->getDrawAreaSize(size);

    rdrpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rdrpassInfo.renderPass  = _data.renderpass->handle;
    rdrpassInfo.renderArea.offset = {0,0};
    rdrpassInfo.renderArea.extent = {(ui32)size.x, (ui32)size.y};

    rdrpassInfo.clearValueCount     = sizeof(clearCol) / sizeof(clearCol[0]);
    rdrpassInfo.pClearValues        = clearCol;

    submitInfo.sType =  VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &cmdBuff.handle;
    submitInfo.waitSemaphoreCount   = 1;

    submitInfo.pWaitSemaphores    = &semImgAvailable;
    submitInfo.pWaitDstStageMask  = &waitDstStage;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &semRdrFinished;

    preInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    preInfo.swapchainCount     = 1;
    preInfo.pSwapchains        = &_data.swpchain->handle;
    preInfo.pImageIndices      = &swpIndex;
    preInfo.waitSemaphoreCount = 1;
    preInfo.pWaitSemaphores    = &semRdrFinished;

    
    VulkanSupport::QueueFamIndices qfam; VulkanSupport::findQueues(qfam, _vkdata);

    vkGetDeviceQueue(_vkdata.dvc, qfam.gfx, 0, &gfxQueue);
    vkGetDeviceQueue(_vkdata.dvc, qfam.pre, 0, &preQueue);

    return VK_SUCCESS;
}

void Frame::dstr() {

    vkQueueWaitIdle(gfxQueue); 
    _data.cmdBuffPool->freeCmdBuff(cmdBuff);

    vkDestroyFence(_vkdata.dvc, fenQueueSubmitComplete, nullptr);
    vkDestroySemaphore(_vkdata.dvc, semImgAvailable, nullptr);
    vkDestroySemaphore(_vkdata.dvc, semRdrFinished, nullptr);
    

}

bool Frame::begin() {
        VkResult res;
        Frame& frame = *this;
        VulkanData& data = _vkdata;
        vkWaitForFences(data.dvc, 1, &frame.fenQueueSubmitComplete, VK_TRUE, UINT64_MAX);
        res = vkAcquireNextImageKHR(data.dvc, _data.swpchain->handle, UINT64_MAX, frame.semImgAvailable, VK_NULL_HANDLE, &swpIndex);
        //Swapchain recreation
        if (res == VK_ERROR_OUT_OF_DATE_KHR) { 
            NWin::Vec2 size;
            _data.win->ptr->getDrawAreaSize(size);
            _data.win->drawArea.x = size.x;
            _data.win->drawArea.y = size.y;
            while (size.x == 0 || size.y == 0) {
                _data.win->ptr->update();
                _data.win->ptr->getDrawAreaSize(size);
                _data.win->drawArea.x = size.x;
                _data.win->drawArea.y = size.y;
            }

            _data.swpchain->dstr();
            _data.renderpass->dstrRes();
            //TODO::Finish this
//            _data.renderpass->createRes(*_data.win,  _data.renderpass->depth.valid, _data.renderpass->msaaA.valid);
//            _data.renderpass->depth.image.changeLyt(
            //VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, *_data.cmdBuffPool);
            _data.swpchain->create(data, *_data.win, *_data.renderpass);
            rdrpassInfo.renderArea.extent = {(ui32)size.x, (ui32)size.y};
            _data.win->ptr->_getKeyboard().update();
            _data.win->ptr->update();
            return 0;
        }
        //TODO::RECREATE SWAPCHAIN IF OUT OF DATE
        vkResetFences(data.dvc, 1, &frame.fenQueueSubmitComplete);
        vkResetCommandBuffer(cmdBuff.handle, 0);
        vkBeginCommandBuffer(cmdBuff.handle, &beginInfo);
        rdrpassInfo.framebuffer = _data.swpchain->fmbuffs[swpIndex].handle;
        return 1;
}
