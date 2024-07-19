#include "bcknd/vkapp.h"
#include "demo.h"

int main() {
    Vkapp app;
    GfxParams::inst.msaa  = MSAAvalue::x1;
    app.validationEnabled = 1;
    app.init();
    loop(app);
    app.dstr();
    return 0;
}
