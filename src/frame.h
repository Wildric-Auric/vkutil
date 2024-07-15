#pragma once

#include "vkdecl.h"
#include "vulkan/vulkan_core.h"
#include <vulkan/vulkan.h>
#include "vkimg.h"

struct AttachmentData {
    bool    valid = false;
    img     image;
    imgView view;
};

class Renderpass {
public:
    VkResult create(const VulkanData&, const Window& win, bool hasDepthAttachment = false, bool hasMsaa = false);
    void     dstr();

    VkResult createRes(const Window& win, bool hasDepth = false, bool hasMsaa = false);
    void     dstrRes();

    VkRenderPass handle;
    VulkanData _vkdata;

    AttachmentData depth;
    AttachmentData msaaA;

};

class Swapchain {
    public:
        VkResult create(const VulkanData& vkdata, const Window& win, Renderpass rdrpass);
        void dstr();

        void chooseExtent(const Window& win, const VkSurfaceCapabilitiesKHR& cap, VkExtent2D* const outExt  );

        VkSwapchainKHR handle = nullptr;
        std::vector<VkImage>     imgs;
        std::vector<imgView>     views;
        std::vector<Framebuffer> fmbuffs;
        VulkanData     _vkdata;
};

class Frame {
    public:
        VkResult     create(const VulkanData&);
        void         dstr(); 

        VkFence      fenQueueSubmitComplete; //Signaled by  vkQueueSubmit()
        VkSemaphore  semImgAvailable;        //Signaled by vkAcquireNext...
        VkSemaphore  semRdrFinished;         //Signaled by vkQueueSubmit()
        VulkanData   _vkdata; 
};
