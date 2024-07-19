#include "bcknd/vkapp.h"
#include "demo.h"

int main() {
    Vkapp app;
    app.init();
    loop(app);
    app.dstr();
    return 0;
}
