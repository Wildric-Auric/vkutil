#pragma once
#include "vulkan/vulkan.h"
#include "nwin/window.h"

#define VK_STR_VALIDATION_LAYER "VK_LAYER_KHRONOS_validation"

typedef NWin::Window* pWin;

struct VulkanData {
    VkPhysicalDevice phyDvc;
    VkDevice         dvc;
    VkInstance       inst;
    VkSurfaceKHR     srfc;
};

//crtInfo can be modified befored calling VkApp::init() so that it window be customized 
struct Window {
    NWin::WindowCrtInfo crtInfo; //creation info
    pWin ptr = nullptr;
};
