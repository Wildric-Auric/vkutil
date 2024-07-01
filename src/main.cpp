#include "vkapp.h"


int main() {
    Vkapp app;
    app.init();
    app.loop();
    app.dstr();
    return 0;
}
