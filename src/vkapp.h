#pragma once

#include "globals.h"
#include "nwin/window.h"
#include "validation.h"
#include "vkdecl.h"


typedef NWin::Window* pWin;

//crtInfo can be modified befored calling VkApp::init() so that it window be customized 
struct Window {
    NWin::WindowCrtInfo crtInfo; //creation info
    pWin ptr = nullptr;
};

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

};
