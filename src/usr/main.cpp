#include "bcknd/vkapp.h"
#include "demo.h"

int main() {
    Vkapp app;
    GfxParams::inst.msaa  = MSAAvalue::x16;
    app.validationEnabled = false;
    app.init();

    loop(app);
    return 0;
}
