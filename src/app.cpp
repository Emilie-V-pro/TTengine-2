
#include "app.hpp"
#include <glm/fwd.hpp>
#include <iostream>
#include "buffer.hpp"

#include "commandBuffer/commandPool_handler.hpp"
#include "commandBuffer/command_buffer.hpp"
#include "device.hpp"
#include "synchronisation/fence.hpp"
#include "window.hpp"

namespace TTe {

void App::init() {
    Window w{1280,720, "mon napli"};
    Device d = Device(w);
    // CommandBufferPool cbp (&d, d.getRenderQueue());
    // CommandBuffer cb = std::move(cbp.createCommandBuffer(1)[0]);
    // cb.beginCommandBuffer();
    // cb.endCommandBuffer();
    // Fence f(&d, false);
    // std::cout << f.getFenceStatus() << std::endl;
    // cb.submitCommandBuffer({}, {}, &f, true);
    // std::cout << f.getFenceStatus() << std::endl;

    Buffer b (&d, 100, 10, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Buffer::BufferType::GPU_ONLY);
    Buffer b2 = std::move(b);
    
    vkDeviceWaitIdle(d);
    CommandPoolHandler::cleanUnusedPools(); 
    int i = 1;
}

void App::run() {
    
}

}