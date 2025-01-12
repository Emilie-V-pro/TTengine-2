
#include "app.hpp"
#include <iostream>
#include "buffer.hpp"
#include "command_buffer.hpp"
#include "device.hpp"
#include "synchronisation/fence.hpp"
#include "window.hpp"

namespace TTe {

void App::init() {
    Window w{1280,720, "mon napli"};
    Device d = Device(w);
    CommandBufferPool cbp (&d, d.getRenderQueue());
    CommandBuffer cb = std::move(cbp.createCommandBuffer(1)[0]);
    cb.beginCommandBuffer();
    cb.endCommandBuffer();
    Fence f(&d, false);
    std::cout << f.getFenceStatus() << std::endl;
    cb.submitCommandBuffer({}, {}, &f, true);
    std::cout << f.getFenceStatus() << std::endl;

    Buffer * b  = new Buffer(&d, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    vkDeviceWaitIdle(d);
    delete b;
    int i = 1;
}

void App::run() {
    
}

}