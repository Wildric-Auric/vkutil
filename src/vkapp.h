#pragma once

#include "globals.h"
#include "nwin/window.h"
#include "validation.h"
#include "vkdecl.h"


typedef NWin::Window* pWin;

//crtInfo can be modified befored calling VkApp so that it can be customized 
struct Window { NWin::WindowCrtInfo crtInfo; //creation info
    pWin ptr = nullptr;
};

class Vkapp {
public: 
    Vkapp() = default;

    int init();
    int loop();
    int dstr();

    int initVkData();

    Window win; 
    VulkanData data;
    bool validationEnabled = 1;
    DebugMessenger dbgMsg;

};
