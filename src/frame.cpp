#include "frame.h" 

void Swapchain::create(VulkanData& vkdata) {
    VkResult res;
    VkSwapchainCreateInfoKHR crtInfo;

    crtInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    crtInfo.oldSwapchain
    _vkdata = vkdata;


}

void Swapchain::dstr() {

}
