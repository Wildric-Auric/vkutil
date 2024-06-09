#include "vkapp.h"

i32 Vkapp::init() {
    //Open window
    win.ptr = NWin::Window::stCreateWindow(win.crtInfo);    
    if (!win.ptr) {return 1;}


    return 0;
}

i32 Vkapp::loop() {
    return 0;
}

i32 Vkapp::dstr() {
    NWin::Window::stDestroyWindow(win.ptr);

    return 0;
}


