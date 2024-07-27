#include "bcknd/vkapp.h"
#include "demo.h"

int main() {
    Vkapp app;
    GfxParams::inst.msaa  = MSAAvalue::x1;
    app.validationEnabled = true;
    app.init();

    loop(app);
    return 0;
}
