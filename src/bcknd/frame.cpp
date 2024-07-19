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
    img fake;
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
        //TODO::Abstract cur and attLen in renderpass
        ui32         cur    = 0;
        ui32         attLen =   1 + (rdrpass.depth.valid) + (rdrpass.msaaA.valid);
        std::vector<VkImageView> att(attLen);

        if (rdrpass.msaaA.valid) {
            att[cur++] = rdrpass.msaaA.view.handle;
            if (rdrpass.depth.valid) att[cur++] = rdrpass.depth.view.handle;
            att[cur++] = views[i].handle;
        }
        else {
            att[cur++] = views[i].handle;
            if (rdrpass.depth.valid) att[cur++] = rdrpass.depth.view.handle;
        }
            
        res = fmbuffs[i].create(_vkdata, rdrpass.handle, att.data(), attLen);
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

VkResult Renderpass::create(const VulkanData& vkdata, const Window& win, bool hasDepthAttachment, bool msaa) {
    int cur = 0;
    uchar attSize = 1 + (uchar)msaa + (uchar)hasDepthAttachment;

    _vkdata = vkdata;
    VkRenderPassCreateInfo crtInfo{};
    VkSubpassDescription   subpass{};

    VkSurfaceFormatKHR srfcFmt;
    VulkanSupport::SwpchainCap spec;
    VulkanSupport::getSwapchaincap(_vkdata, spec);
    srfcFmt = spec.srfcFormats[VulkanSupport::selSrfcFmt(spec)];

    //Attachments
    std::vector<VkAttachmentDescription> att;
    att.resize(attSize, {});
    cur = 0;

    VkAttachmentDescription& colDesc = att[cur++];
    
    colDesc.format = srfcFmt.format;
    colDesc.samples = msaa ? (VkSampleCountFlagBits)GfxParams::inst.msaa : VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
    colDesc.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colDesc.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colDesc.finalLayout    = (msaa) ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    
    if (hasDepthAttachment) {
        VkAttachmentDescription& dpthDesc = att[cur++];
        dpthDesc.format         = VK_FORMAT_D32_SFLOAT_S8_UINT;
        dpthDesc.samples        = (VkSampleCountFlagBits)GfxParams::inst.msaa;
        dpthDesc.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        dpthDesc.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        dpthDesc.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
        dpthDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        dpthDesc.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        dpthDesc.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    if (msaa) {
        VkAttachmentDescription& resolveDesc = att[cur++];
        resolveDesc.format  = srfcFmt.format;
        resolveDesc.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
        resolveDesc.loadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        resolveDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        resolveDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        resolveDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        resolveDesc.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        resolveDesc.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }

     
    //References
    VkAttachmentReference  colAttRef{};
    VkAttachmentReference  resolveAttRef{};
    VkAttachmentReference  dpthAttRef{};
    cur = 0;

    colAttRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    resolveAttRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    dpthAttRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    //Subpass
    subpass.colorAttachmentCount = 1;

    colAttRef.attachment = cur++;
    subpass.pColorAttachments    = &colAttRef;

    if (hasDepthAttachment) {
        dpthAttRef.attachment = cur++;
        subpass.pDepthStencilAttachment = &dpthAttRef;
    }
    
    if (msaa) {
        resolveAttRef.attachment = cur++;
        subpass.pResolveAttachments  = &resolveAttRef;
    }
    
    VkSubpassDependency dpn{};
    dpn.srcSubpass = VK_SUBPASS_EXTERNAL;
    dpn.dstSubpass = 0;

    dpn.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dpn.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT          | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    dpn.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dpn.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT          | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


    crtInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    crtInfo.dependencyCount = 1;
    crtInfo.pDependencies   = &dpn;
    crtInfo.subpassCount = 1;
    crtInfo.pSubpasses   = &subpass;
    crtInfo.attachmentCount = att.size();
    crtInfo.pAttachments    = att.data();
    
    VkResult res = vkCreateRenderPass(_vkdata.dvc, &crtInfo, nullptr, &handle);
    if (res != VK_SUCCESS) {
        return res;
    }

    res = createRes(win, hasDepthAttachment, msaa);

    return res;
}

void Renderpass::dstr() {
    vkDestroyRenderPass(_vkdata.dvc, handle, nullptr);
    dstrRes();
}

void Renderpass::dstrRes() {
    if (msaaA.valid) {
        msaaA.view.dstr();
        msaaA.image.dstr(); 
    }

    if (depth.valid) {
        depth.view.dstr();
        depth.image.dstr();
    }
}

VkResult Renderpass::createRes(const Window& win, bool hasDepthAttachment, bool msaa) {
    VkResult res = VK_SUCCESS;
    if (msaa) {
       msaaA.valid = true; 
       msaaA.image.fillCrtInfo();
       msaaA.image.crtInfo.usage   = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
       msaaA.image.crtInfo.samples = (VkSampleCountFlagBits)GfxParams::inst.msaa;
       msaaA.image.crtInfo.extent.width  = win.drawArea.x;
       msaaA.image.crtInfo.extent.height = win.drawArea.y;
       res = msaaA.image.create(_vkdata);
       if (res != VK_SUCCESS) return res;

       msaaA.view.fillCrtInfo(msaaA.image);
       msaaA.view.crtInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
       res = msaaA.view.create(_vkdata);
       if (res != VK_SUCCESS) return res;
    }   

    if (hasDepthAttachment) {
        depth.valid = true;
        depth.image.fillCrtInfo();
        depth.image.crtInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        depth.image.crtInfo.samples = (VkSampleCountFlagBits)GfxParams::inst.msaa;
        depth.image.crtInfo.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        depth.image.crtInfo.extent.width =  win.drawArea.x; 
        depth.image.crtInfo.extent.height = win.drawArea.y;
        res = depth.image.create(_vkdata);
        if (res != VK_SUCCESS) return res;

        depth.view.fillCrtInfo(depth.image);
        depth.view.crtInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        res = depth.view.create(_vkdata);
        if (res != VK_SUCCESS) return res;
    }
    return res;
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

            _data.renderpass->createRes(*_data.win, _data.renderpass->depth.valid, _data.renderpass->msaaA.valid);
            _data.renderpass->depth.image.changeLyt(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, *_data.cmdBuffPool);
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

            _data.renderpass->createRes(*_data.win,  _data.renderpass->depth.valid, _data.renderpass->msaaA.valid);
            _data.renderpass->depth.image.changeLyt(
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, *_data.cmdBuffPool);
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
