#pragma once

#include "globals.h"
#include "window.h"

#include "vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"

typedef NWin::Window* pWin;

//crtInfo can be modified befored calling VkApp so that it can be customized 
struct Window {
    NWin::WindowCrtInfo crtInfo; //creation info
    pWin ptr = nullptr;
};

struct VulkanData {
    VkPhysicalDevice phyDvc;
    VkDevice         dvc;
    VkInstance       inst;
};

class Vkapp {
public: 
    Vkapp() = default;

    int init();
    int loop();
    int dstr();

    Window win; 
    VulkanData data;
};
