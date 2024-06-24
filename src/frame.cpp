#include "frame.h" 
#include "support.h" 
#include "vkimg.h"
#include "params.h"


VkResult Swapchain::create(const VulkanData& vkdata, const Window& win) {
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

    crtInfo.preTransform = spec.cap.currentTransform;
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

    //Create swapchaine image views
    views.resize(imgCount);
    for (arch i = 0; i < imgCount; ++i) {
        views[i].fillCrtInfo();
        views[i].crtInfo.format = srfcFmt.format;
        views[i].crtInfo.image  = imgs[i];
        views[i].create(_vkdata);
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
    for (arch i = 0; i < imgs.size(); ++i) {
        views[i].dstr();
    }
    vkDestroySwapchainKHR(_vkdata.dvc, handle, nullptr);
}


VkResult Renderpass::create(const VulkanData& vkdata) {
    uchar COLOR_ATT_INDEX   = 0;
    uchar RESOLVE_ATT_INDEX = 1;
    uchar DEPTH_ATT_INDEX   = 2;

    uchar attSize = 1 + (GfxParams::inst.msaa > MSAAvalue::x1);

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

    VkAttachmentDescription& colDesc = att[COLOR_ATT_INDEX];
    
    colDesc.format = srfcFmt.format;
    colDesc.samples =  (VkSampleCountFlagBits)GfxParams::inst.msaa;
    colDesc.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colDesc.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colDesc.finalLayout    = (GfxParams::inst.msaa > MSAAvalue::x1) ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    if (GfxParams::inst.msaa > MSAAvalue::x1) {
        VkAttachmentDescription& resolveDesc = att[RESOLVE_ATT_INDEX];
        resolveDesc.format  = srfcFmt.format;
        resolveDesc.samples = VK_SAMPLE_COUNT_1_BIT;
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

    colAttRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colAttRef.attachment = COLOR_ATT_INDEX;

    resolveAttRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    resolveAttRef.attachment = RESOLVE_ATT_INDEX;

    //Subpass
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colAttRef;
    if (GfxParams::inst.msaa > MSAAvalue::x1) {
        VkAttachmentDescription& resolveDesc = att[RESOLVE_ATT_INDEX];
        subpass.pResolveAttachments  = &resolveAttRef;
    }

    crtInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    crtInfo.subpassCount = 1;
    crtInfo.pSubpasses   = &subpass;
    crtInfo.attachmentCount = att.size();
    crtInfo.pAttachments    = att.data();
    
    return vkCreateRenderPass(_vkdata.dvc, &crtInfo, nullptr, &handle);

#undef COLOR_ATT_INDEX
#undef DEPTH_ATT_INDEX
#undef RESOLVE_ATT_INDEX

}

void Renderpass::dstr() {
    vkDestroyRenderPass(_vkdata.dvc, handle, nullptr);
}



