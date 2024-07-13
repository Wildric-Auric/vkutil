#include "frame.h" 
#include "support.h" 
#include "vkimg.h"
#include "params.h"


VkResult Swapchain::create(const VulkanData& vkdata, const Window& win, Renderpass rdrpass, img* depthBuffer) {
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
        fake.handle         = imgs[i];
        views[i].fillCrtInfo(fake);
        views[i].create(_vkdata);

        fmbuffs[i].fillCrtInfo();
        fmbuffs[i].crtInfo.width  = crtInfo.imageExtent.width;
        fmbuffs[i].crtInfo.height = crtInfo.imageExtent.height;
        //TODO::update this when there is MSAA impl 
        imgView dpthView;
        imgView msaaView;
        ui32         cur    = 1;
        ui32         attLen =   1 + (depthBuffer != nullptr);
        std::vector<VkImageView> att(attLen);
        att[0]  = views[i].handle;
        
        if (depthBuffer) {
            dpthView.fillCrtInfo(*depthBuffer);
            dpthView.crtInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            dpthView.create(vkdata);
            att[cur++] = dpthView.handle;
        }

        fmbuffs[i].create(_vkdata, rdrpass.handle, att.data(), attLen);
    }

    return res;
}

void Swapchain::chooseExtent(const Window& win, const VkSurfaceCapabilitiesKHR& cap, VkExtent2D* const outExt  ) {

    if (cap.currentExtent.width != (ui32)(-1)) {
        *outExt = cap.currentExtent;
        return;
    }

    NWin::Vec2 size;
    win.ptr->getDrawAreaSize(size);
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

VkResult Renderpass::create(const VulkanData& vkdata, bool hasDepthAttachment) {
    const uchar COLOR_ATT_INDEX_T   = 0;
    const uchar RESOLVE_ATT_INDEX_T = 1;
    const uchar DEPTH_ATT_INDEX_T   = 2;
    int cur = 0;
    uchar attSize = 1 + (GfxParams::inst.msaa > MSAAvalue::x1) + (uchar)hasDepthAttachment;

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
    colDesc.samples =  (VkSampleCountFlagBits)GfxParams::inst.msaa;
    colDesc.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colDesc.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colDesc.finalLayout    = (GfxParams::inst.msaa > MSAAvalue::x1) ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    if (GfxParams::inst.msaa > MSAAvalue::x1) {
        VkAttachmentDescription& resolveDesc = att[cur++];
        resolveDesc.format  = srfcFmt.format;
        resolveDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        resolveDesc.loadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        resolveDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        resolveDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        resolveDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        resolveDesc.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        resolveDesc.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }

    if (hasDepthAttachment) {
        VkAttachmentDescription& dpthDesc = att[cur++];
        dpthDesc.format        = VK_FORMAT_D32_SFLOAT_S8_UINT;
        dpthDesc.samples       = VK_SAMPLE_COUNT_1_BIT;
        dpthDesc.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;
        dpthDesc.storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
        dpthDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        dpthDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        dpthDesc.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        dpthDesc.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }
    
    //References
    VkAttachmentReference  colAttRef{};
    VkAttachmentReference  resolveAttRef{};
    VkAttachmentReference  dpthAttRef{};
    cur = 0;

    colAttRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colAttRef.attachment = cur++;

    resolveAttRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    dpthAttRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    //Subpass
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colAttRef;

    if (GfxParams::inst.msaa > MSAAvalue::x1) {
        resolveAttRef.attachment = cur++;
        subpass.pResolveAttachments  = &resolveAttRef;
    }

    if (hasDepthAttachment) {
        dpthAttRef.attachment = cur++;
        subpass.pDepthStencilAttachment = &dpthAttRef;
    }

    VkSubpassDependency dpn{};
    dpn.srcSubpass = VK_SUBPASS_EXTERNAL;
    dpn.dstSubpass = 0;

    dpn.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dpn.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT          | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    dpn.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dpn.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT          | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


    crtInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    crtInfo.dependencyCount = 1;
    crtInfo.pDependencies = &dpn;
    crtInfo.subpassCount = 1;
    crtInfo.pSubpasses   = &subpass;
    crtInfo.attachmentCount = att.size();
    crtInfo.pAttachments    = att.data();
    
    return vkCreateRenderPass(_vkdata.dvc, &crtInfo, nullptr, &handle);
}

void Renderpass::dstr() {
    vkDestroyRenderPass(_vkdata.dvc, handle, nullptr);
}

VkResult Frame::create(const VulkanData& vkdata) {
    _vkdata = vkdata;
    VkFenceCreateInfo fenCrtInfo;
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
    return VK_SUCCESS; 
}

void Frame::dstr() {

    VkQueue gfxQueue;
    VulkanSupport::QueueFamIndices qfam; VulkanSupport::findQueues(qfam, _vkdata);
    vkGetDeviceQueue(_vkdata.dvc, qfam.gfx, 0, &gfxQueue);

    vkQueueWaitIdle(gfxQueue);

    vkDestroyFence(_vkdata.dvc, fenQueueSubmitComplete, nullptr);
    vkDestroySemaphore(_vkdata.dvc, semImgAvailable, nullptr);
    vkDestroySemaphore(_vkdata.dvc, semRdrFinished, nullptr);

}









