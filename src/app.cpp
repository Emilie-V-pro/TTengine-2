
#include "app.hpp"
#include <glm/fwd.hpp>
#include "buffer.hpp"

#include "commandBuffer/commandPool_handler.hpp"
#include "device.hpp"
#include "image.hpp"
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

    // Buffer b (&d, sizeof(uint32_t), 10, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    // Buffer b2 = std::move(b);
    // uint32_t data[10] = {1,2,3,4,5,6,7,8,9,10};
    // b2.writeToBuffer(&data, sizeof(data) * 10, 0);

    ImageCreateInfo ici;
    ici.width = 1280;
    ici.height = 720;
    ici.format = VK_FORMAT_R8G8B8A8_SRGB;
    ici.usageFlags =  VK_IMAGE_USAGE_SAMPLED_BIT;
    ici.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
    ici.filename.push_back("../textures/posx.jpg");
    ici.filename.push_back("../textures/negx.jpg");
    ici.filename.push_back("../textures/posy.jpg");
    ici.filename.push_back("../textures/negy.jpg");
    ici.filename.push_back("../textures/posz.jpg");
    ici.filename.push_back("../textures/negz.jpg");
    ici.isCubeTexture = true;
    ici.layers = 6;
    Image im(&d, ici);

    
    vkDeviceWaitIdle(d);
    CommandPoolHandler::cleanUnusedPools(); 
    int i = 1;
}

void App::run() {
    
}

}