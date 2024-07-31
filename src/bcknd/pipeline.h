#pragma once 

#include "vkdecl.h"
#include "globals.h"

class Shader {
    public:
   VkShaderModuleCreateInfo& fillCrtInfo(const ui32* const bytecode, const arch size);  
   VkPipelineShaderStageCreateInfo& fillStageCrtInfo(VkShaderStageFlagBits);
   VkResult create(const VulkanData& vkdata);
   void dstr();

   VkShaderModule handle;
   VkShaderModuleCreateInfo         crtInfo{};
   VkPipelineShaderStageCreateInfo  stageCrtInfo{};
   VulkanData _vkdata;
   VkShaderStageFlagBits _type;
};

class Pipeline {
    public:
    VkResult create(const VulkanData&);
    VkGraphicsPipelineCreateInfo& fillCrtInfo();
    void dstr();

    VkGraphicsPipelineCreateInfo crtInfo{};
    VulkanData _vkdata;
    VkPipelineLayout _layout;
    VkPipeline handle;

    VkPipelineVertexInputStateCreateInfo   vrtxInputState{};
    VkPipelineInputAssemblyStateCreateInfo inputAsmState{};
    VkPipelineRasterizationStateCreateInfo rasterState{};
    VkPipelineDepthStencilStateCreateInfo  depthState{};
    VkPipelineColorBlendStateCreateInfo    blendState{};
    VkPipelineViewportStateCreateInfo      viewportState{};
    VkPipelineTessellationStateCreateInfo  tesState{};
    VkPipelineLayoutCreateInfo             layoutCrtInfo{};


    VkPipelineMultisampleStateCreateInfo _msaaState{};
    VkPipelineDynamicStateCreateInfo     _dynState{};
    std::vector<VkPipelineColorBlendAttachmentState> _colBlendAtt;

    VkDynamicState _dynamicStates[2] = {
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_VIEWPORT  
    };

    VkViewport _viewport{};
    VkRect2D   _scissor{};
};

class ComputePipeline {
    public:
    VkResult create(const VulkanData&);
    VkComputePipelineCreateInfo& fillCrtInfo();
    void dstr();

    VkComputePipelineCreateInfo crtInfo{};
    VkPipelineLayoutCreateInfo  lytCrtInfo{};
    VulkanData                  _vkdata;
    VkPipeline                  handle = nullptr;
    VkPipelineLayout            _lyt   = nullptr;
};
