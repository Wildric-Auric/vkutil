#pragma once

#include "nwin/vulkan_surface.h"
#include "vulkan/vulkan_core.h"

#include "bcknd/vkapp.h"
#include "bcknd/vkdecl.h"
#include "bcknd/support.h"
#include "bcknd/params.h"
#include "bcknd/io.h"
#include "bcknd/vertex.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

inline i32 loop(Vkapp& vkapp) {
    Pipeline      pipeline;

    std::vector<char> frag;
    std::vector<char> vert;
    io::readBin("..\\build\\bin\\trifrag.spv", frag );
    io::readBin("..\\build\\bin\\trivert.spv", vert );
    Shader fragS, vertS;
    fragS.fillCrtInfo((const ui32*)frag.data(), frag.size());
    vertS.fillCrtInfo((const ui32*)vert.data(), vert.size());
    VK_CHECK_EXTENDED(fragS.create(vkapp.data), "fragshader");
    VK_CHECK_EXTENDED(vertS.create(vkapp.data), "vertshader");
    fragS.fillStageCrtInfo(VK_SHADER_STAGE_FRAGMENT_BIT);
    vertS.fillStageCrtInfo(VK_SHADER_STAGE_VERTEX_BIT);

    VkPipelineShaderStageCreateInfo stages[] = {
        vertS.stageCrtInfo,
        fragS.stageCrtInfo
    };

    pipeline.fillCrtInfo();
    pipeline.crtInfo.stageCount = 2;
    pipeline.crtInfo.pStages    = stages;
    pipeline.crtInfo.renderPass = vkapp.renderpass.handle;

    pipeline.layoutCrtInfo.setLayoutCount = 1;
    pipeline.layoutCrtInfo.pSetLayouts    = &vkapp.descPool._lytHandle;

    VK_CHECK_EXTENDED(pipeline.create(vkapp.data), "Failed to create Pipeline");
    
    fragS.dstr();
    vertS.dstr();
    DescSet     descSet{};
    //Creating image texture
    ui32 imgsize;
    ivec2 vecsize;
    i32   channels; 
    stbi_uc* pixels = stbi_load("../res/tex.jpg", &vecsize.x, &vecsize.y, &channels, STBI_rgb_alpha);
    imgsize = vecsize.x * vecsize.y * 4;

    Sampler smpler;
    smpler.fillCrtInfo(vkapp.data);
    smpler.create(vkapp.data);

    Buffer tempImg;
    img    img0;
    imgView view;

    tempImg.fillCrtInfo();
    tempImg.crtInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    tempImg.memProp       = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    tempImg.create(vkapp.data, imgsize);
    void* mapped;
    tempImg.mapMem(&mapped);
    memcpy(mapped, pixels, imgsize);
    tempImg.unmapMem(&mapped);

    img0.fillCrtInfo();
    img0.crtInfo.usage         = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    img0.crtInfo.extent.width  = vecsize.x;
    img0.crtInfo.extent.height = vecsize.y;
    img0.setMaxmmplvl();
    img0.create(vkapp.data);
    img0.cpyFrom(vkapp.gfxCmdPool, tempImg, vecsize, 0);
    img0.genmmp(vkapp.gfxCmdPool, offsetof(VulkanSupport::QueueFamIndices, gfx));

    view.fillCrtInfo(img0);
    view.create(vkapp.data);

    //img0.changeLyt(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, gfxCmdPool);
    //-----
    float color = 0.5;
    UniBuff unf;
    //descPool._lytBindings.pop_back();
    unf.create(vkapp.data, sizeof(color));
    vkapp.descPool.allocDescSet(&descSet);
    VkWriteDescriptorSet wrt{};

    VkDescriptorBufferInfo inf{};
    inf.buffer      = unf._buff.handle; inf.range       = unf._buff._size;
    wrt.pBufferInfo = &inf; 
    wrt.descriptorCount = 1;
    wrt.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descSet.wrt(&wrt, 0);

    VkDescriptorImageInfo imInf{};
    imInf.sampler     = smpler.handle;
    imInf.imageView   = view.handle;
    imInf.imageLayout = img0.crtInfo.initialLayout;    

    wrt = VkWriteDescriptorSet{};
    wrt.descriptorCount = 1;
    wrt.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    wrt.pImageInfo      = &imInf;
    
    descSet.wrt(&wrt, 1);

    unf.wrt(&color);
    //-----

    VertexObject vobj;
    VertexData strides[] = {
        { {-0.5, -0.5, .0},  {0.0, 0.0} },
        { { 0.5, -0.5, 0.0}, {1.0, 0.0} },    
        { {-0.5,  0.5, 0.0}, {0.0, 1.0} },
        { { 0.5,  0.5, 0.0}, {1.0, 1.0} }

    };

    ui32 indices[] = {
        0,1,2,
        2,1,3
    };

    vobj.create(vkapp.data, vkapp.gfxCmdPool, (float*)strides,  sizeof(strides) );
    vobj.createIndexBuff(vkapp.data, vkapp.gfxCmdPool, indices, sizeof(indices));

    Frame frame;
    frame._data.win = &vkapp.win;
    frame._data.swpchain = &vkapp.swpchain;
    frame._data.renderpass = &vkapp.renderpass;
    frame._data.cmdBuffPool = &vkapp.gfxCmdPool;
    frame.create(vkapp.data);


    VkResult res;
    while (vkapp.win.ptr->shouldLoop()) {        
        //TODO::CONTINUE REFACTOR; SEE FRAME class
        if (!frame.begin()) {
            continue;
        }
        vkCmdBeginRenderPass(frame.cmdBuff.handle, &frame.rdrpassInfo, VK_SUBPASS_CONTENTS_INLINE); //What is third parameter?
        vkCmdBindPipeline(frame.cmdBuff.handle, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
      
        //Dynamic states 

        VkViewport viewport;
        VkRect2D   scissor;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width =  Max<i32>(vkapp.win.drawArea.x, 5.0);
        viewport.height = Max<i32>(vkapp.win.drawArea.y, 5.0);
        scissor.extent =  {(ui32)viewport.width, (ui32)viewport.height};
        scissor.offset = { 0,0 };
        vkCmdSetViewport(frame.cmdBuff.handle, 0, 1, &viewport);
        vkCmdSetScissor(frame.cmdBuff.handle, 0, 1, &scissor);

        VkDeviceSize voff = 0;
        color += 0.01;
        if (color > 1.0)
            color = 0.0;
        unf.wrt(&color);
        vkCmdBindDescriptorSets(frame.cmdBuff.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline._layout, 0, 1, &descSet.handle, 0, nullptr);
        vkCmdBindVertexBuffers(frame.cmdBuff.handle, 0, 1, &vobj.buff.handle, &voff);
        vkCmdBindIndexBuffer(frame.cmdBuff.handle, vobj.indexBuff.handle, voff, VkIndexType::VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(frame.cmdBuff.handle, sizeof(indices) / sizeof(indices[0]), 1, 0, 0, 0);
        vkCmdEndRenderPass(frame.cmdBuff.handle);

        frame.end();
    }
    frame.dstr();
    vobj.dstr();
    unf.dstr();
    smpler.dstr();
    tempImg.dstr();
    img0.dstr();
    view.dstr();
    pipeline.dstr();
    return 0;
}
