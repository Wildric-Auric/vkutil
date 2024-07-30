#pragma once

#include "vkdecl.h"
#include "vulkan/vulkan_core.h"
#include <vulkan/vulkan.h>
#include "vkimg.h"

struct AttachmentData {
    Img     image;
    ImgView view; 
};

class Attachment {
    public:
    void setup();
    VkAttachmentDescription desc{};
    VkAttachmentReference   ref{};
    Img      img;
    ImgView  view;
};

class AttachmentContainer {
    public:
    Attachment*  getDepth();
    Attachment*  getResolve();
    Attachment*  get(arch index);
    Attachment*  add();
    Attachment*  addDepth();
    Attachment*  addResolve();

    //Attachment*  add(VkAttachmentDescription&);

    std::vector<Attachment> _container;
    Attachment depth;
    Attachment resolve; 
    bool _hasDepth       = 0;
    bool _hasResolve     = 0;
};

class Subpass { 
    public:
    VkSubpassDescription desc{};
};

class SubpassContainer {
    public:
    void add(const Window& win, const VulkanData& vkdata, AttachmentContainer& atts, VkSubpassDependency** depedencyWithPrevious);
    void addDepthRes(const Window&, const VulkanData&);
    void addResolveRes(const Window&, const VulkanData&);

   
    std::vector<VkSubpassDependency>     _dpn;
    std::vector<VkSubpassDescription>    descs;

    std::vector<VkAttachmentDescription> attDescs;
    std::vector<VkAttachmentReference>   attRefs;

    std::vector<AttachmentData>          resources;
};

class Renderpass {
public:
    VkResult create(const VulkanData&, const Window& win);
    void     dstr();

    void     dstrRes();

    VkRenderPass      handle;
    VulkanData       _vkdata;
    SubpassContainer _subpasses;
};

class Swapchain {
    public:
        VkResult create(const VulkanData& vkdata, const Window& win, Renderpass rdrpass);
        void dstr();

        void chooseExtent(const Window& win, const VkSurfaceCapabilitiesKHR& cap, VkExtent2D* const outExt  );

        VkSwapchainKHR handle = nullptr;
        std::vector<VkImage>     imgs;
        std::vector<ImgView>     views;
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
