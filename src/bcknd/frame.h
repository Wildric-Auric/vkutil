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
    Attachment*  get(arch index);
    Attachment*  add(bool hasResolve = 0);
    Attachment*  addDepth();

    //Attachment*  add(VkAttachmentDescription&);

    std::vector<Attachment> _container;
    Attachment depth;
    bool _hasDepth       = 0;
    bool _hasResolve     = 0;
};

class Subpass { 
    public:
    VkSubpassDescription desc{};
};

struct StrideData {
    i32 colLen;
    i32 colOffset        = -1; 
    i32 localResOffset   = -1; //Offset has same size as color attachment; 
    i32 localDepthOffset = -1;
    i32 localInputOffset = -1;
    i32 inputLen         =  0;
};

class SubpassContainer {
    public:
    void setup(arch subpassNum, arch totalAttNum);

    void add(const Window& win, const VulkanData& vkdata, AttachmentContainer& atts, VkSubpassDependency** depedencyWithPrevious);
    void addDepthRes(const Window&, const VulkanData&, const VkAttachmentDescription&);
    void addResolveRes(const Window&, const VulkanData&, const VkAttachmentDescription&);
    void addColorRes(const Window&, const VulkanData&, const VkAttachmentDescription&);

    std::vector<AttachmentData>::iterator getStrideColIterBegin(ui32 strideIndex, std::vector<AttachmentData>::iterator* end);
    std::vector<AttachmentData>::iterator getStrideColResolveBegin(ui32 strideIndex, std::vector<AttachmentData>::iterator* end);
    AttachmentData*  getStrideDepth(ui32 strideIndex);

   
    std::vector<VkSubpassDependency>     _dpn;
    std::vector<VkSubpassDescription>    descs;
    //Each stride consists of respective subpass attachments, [color0, color1 ... colorN, res0 ... resnN, depth]
    std::vector<VkAttachmentDescription> attDescs;
    std::vector<VkAttachmentReference>   attRefs;

    std::vector<AttachmentData>          resources;
    std::vector<StrideData>              _strideInfo;

    arch _ptrSPContainer  = 0;
    arch _ptrAttContainer = 0;
};

class Renderpass {
public:
    VkResult create(const VulkanData&, const Window& win);
    bool     setSwpChainHijack(ui32 subpassIndex, ui32 attIndex);
    void     dstr();

    void     dstrRes();

    VkRenderPass      handle;
    VulkanData       _vkdata;
    SubpassContainer _subpasses;

    ui32 _subpassHjckIndex = 0;
    ui32 _attHjckIndex     = 0;
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

struct FrameData {
    Window*        win;
    Renderpass*    renderpass;
    Swapchain*     swpchain;
    CmdBufferPool* cmdBuffPool;
};

class Frame {
    public:
        void         setup(const FrameData&);
        VkResult     create(const VulkanData&);
        void         processSwpchainRec();
        void         dstr(); 
        bool begin();
        void end();

        VkFence      fenQueueSubmitComplete; //Signaled by  vkQueueSubmit()
        VkSemaphore  semImgAvailable;        //Signaled by vkAcquireNext...
        VkSemaphore  semRdrFinished;         //Signaled by vkQueueSubmit()
        VulkanData   _vkdata; 
        FrameData   _data;
        
        CmdBuff                  cmdBuff; 
        VkCommandBufferBeginInfo beginInfo{};
        VkRenderPassBeginInfo    rdrpassInfo{};
        VkSubmitInfo             submitInfo{};
        VkPresentInfoKHR         preInfo{};

        VkPipelineStageFlags waitDstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        std::vector<VkClearValue> clearCol = {
            {1.0f, 0.05f, 0.15f, 1.0f},
            {1.0, 0.0},
        };        

        VkQueue gfxQueue;
        VkQueue preQueue;
        ui32 swpIndex; 
};
