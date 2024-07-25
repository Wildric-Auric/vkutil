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
        std::vector<img>     imgs;
        std::vector<imgView>     views;
        std::vector<Framebuffer> fmbuffs;
        VulkanData     _vkdata;
};

struct FramdeData {
    Window*        win;
    Renderpass*    renderpass;
    Swapchain*     swpchain;
    CmdBufferPool* cmdBuffPool;
};

class Frame {
    public:
        VkResult     create(const VulkanData&);
        void         dstr(); 
        bool begin();
        void end();

        VkFence      fenQueueSubmitComplete; //Signaled by  vkQueueSubmit()
        VkSemaphore  semImgAvailable;        //Signaled by vkAcquireNext...
        VkSemaphore  semRdrFinished;         //Signaled by vkQueueSubmit()
        VulkanData   _vkdata; 
        FramdeData   _data;
        
        CmdBuff                  cmdBuff; 
        VkCommandBufferBeginInfo beginInfo{};
        VkRenderPassBeginInfo    rdrpassInfo{};
        VkSubmitInfo             submitInfo{};
        VkPresentInfoKHR         preInfo{};

        VkPipelineStageFlags waitDstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkClearValue clearCol[2] = {
            {1.0f, 0.05f, 0.15f, 1.0f},
            {1.0, 0.0}
        };        

        VkQueue gfxQueue;
        VkQueue preQueue;
        ui32 swpIndex; 
};
