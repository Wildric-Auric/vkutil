#pragma once

#include "vkdecl.h"
#include "vulkan/vulkan_core.h"
#include <vulkan/vulkan.h>
#include "vkimg.h"

class Swapchain {
    public:
        VkResult create(const VulkanData&, const Window& );
        void recreate();
        void dstr();

        void chooseExtent(const Window& win, const VkSurfaceCapabilitiesKHR& cap, VkExtent2D* const outExt  );

        VkSwapchainKHR handle = nullptr;
        std::vector<VkImage>   imgs;
        std::vector<imgView> views;
        VulkanData     _vkdata;
};

class Renderpass {
public:
    VkResult create(const VulkanData&);
    void     dstr();
    VkRenderPass handle;

    VulkanData _vkdata;
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
