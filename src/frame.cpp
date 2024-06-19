#include "frame.h" 
#include "support.h" 
#include "vkimg.h"
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
