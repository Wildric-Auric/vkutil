#include "vkimg.h"
#include "params.h"

 VkImageViewCreateInfo& imgView::fillCrtInfo() {
    crtInfo.sType  = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    crtInfo.format = VK_FORMAT_R8G8B8_SRGB;
    crtInfo.image  = VK_NULL_HANDLE;
    crtInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    crtInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    crtInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    crtInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    crtInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY; 
    crtInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    crtInfo.subresourceRange.baseMipLevel = 0;
    crtInfo.subresourceRange.baseArrayLayer = 0;
    crtInfo.subresourceRange.layerCount     = 1;
    crtInfo.subresourceRange.levelCount     = 1;

    return crtInfo;
}

VkResult imgView::create(const VulkanData& vkdata) {
    _vkdata = vkdata;
    return vkCreateImageView(vkdata.dvc, &crtInfo, nullptr, &handle);
}

void imgView::dstr() {
    vkDestroyImageView(_vkdata.dvc, handle, nullptr);
}
    
VkImageCreateInfo& img::fillCrtInfo() {
    crtInfo.sType     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    crtInfo.imageType = VK_IMAGE_TYPE_2D;
    crtInfo.format    = VK_FORMAT_R8G8B8_SRGB;
    crtInfo.tiling    = VK_IMAGE_TILING_OPTIMAL;
    crtInfo.extent.width  = 0;
    crtInfo.extent.height = 0;
    crtInfo.extent.depth = 1;
    crtInfo.mipLevels    = 1;
    crtInfo.arrayLayers  = 1;
    crtInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    crtInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    crtInfo.samples       = (VkSampleCountFlagBits)GfxParams::inst.msaa;
    crtInfo.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    return crtInfo;
};

VkResult img::create(const VulkanData& vkdata) {
    _vkdata = vkdata;
    return vkCreateImage(vkdata.dvc, &crtInfo, nullptr, &handle);
}

void img::dstr() {
    vkDestroyImage(_vkdata.dvc, handle, nullptr);
}

VkFramebufferCreateInfo& Framebuffer::fillCrtInfo() {
    crtInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO; 
    crtInfo.layers = 1;
    return crtInfo;

}

VkResult Framebuffer::create(const VulkanData& vkdata, VkRenderPass rdrpass, VkImageView* attchments, ui32 attLen) {
   _vkdata = vkdata;
   crtInfo.renderPass      = rdrpass;
   crtInfo.attachmentCount = attLen;
   crtInfo.pAttachments    = attchments;

   return vkCreateFramebuffer(_vkdata.dvc, &crtInfo, nullptr, &handle);
}

void Framebuffer::dstr() {
    vkDestroyFramebuffer(_vkdata.dvc, handle, nullptr);
}







