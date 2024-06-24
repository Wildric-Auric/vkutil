#pragma once

#include "globals.h"
#include "nwin/window.h"
#include "validation.h"
#include "vkdecl.h"
#include "frame.h"
#include "pipeline.h"
#include "cmd_buffer.h"

class Vkapp {
public: 
    Vkapp() = default;

    //Globally, it does the following operations:
    //1.Opens a window
    //2.Calls initVkData
    int init();
    int loop();
    int dstr();
    
    //Initializes data member by creating:
    //1.Instance
    //2.Debug Messenger (if validation layer enabled)
    //3.Physical Device
    //4.Logical  Device 
    int initVkData();

    Window win; 
    VulkanData data;
    bool validationEnabled = 1;
    DebugMessenger dbgMsg;


    Frame         frame;
    Swapchain     swpchain;
    Pipeline      pipeline;
    Renderpass    renderpass;
    CmdBufferPool gfxCmdPool;
};
