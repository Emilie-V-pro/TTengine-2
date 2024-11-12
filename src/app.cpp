
#include "app.hpp"
#include "device.hpp"
#include "window.hpp"

namespace TTe {

void App::init() {
    Window w{1280,720, "mon napli"};
    Device d = Device(w);
}

void App::run() {
}

}