
#include "app.hpp"

#include <vulkan/vulkan_core.h>

#include <glm/fwd.hpp>
#include <iostream>
#include <vector>

#include "commandBuffer/commandPool_handler.hpp"
#include "commandBuffer/command_buffer.hpp"
#include "descriptor/descriptorSet.hpp"
#include "device.hpp"
#include "image.hpp"
#include "shader/pipeline/compute_pipeline.hpp"
#include "synchronisation/fence.hpp"

namespace TTe {

void App::init(Device *device, std::vector<Image> &swapchainImages) {
    this->swapchainImages = &swapchainImages;
    this->device = device;

    computePipeline = ComputePipeline(device, "test.comp");

    ImageCreateInfo ici;
    ici.width = 1280;
    ici.height = 720;
    ici.format = VK_FORMAT_R8G8B8A8_SNORM;
    ici.usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    ici.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    image = Image(device, ici);

    descriptorSet = DescriptorSet(device, computePipeline.getDescriptorsSetLayout()[0]);
    descriptorSet.writeImageDescriptor(0, image.getDescriptorImageInfo());

    // create command buffer
    CommandBufferPool  * cbp = CommandPoolHandler::getCommandPool(device, device->getComputeQueue());
    CommandBuffer cmdBuffer = std::move(cbp->createCommandBuffer(1)[0]);
    cmdBuffer.beginCommandBuffer();
    computePipeline.bindPipeline(cmdBuffer);
    std::vector<DescriptorSet *> descriptorSets = {&descriptorSet};
    DescriptorSet::bindDescriptorSet(cmdBuffer, descriptorSets, computePipeline.getPipelineLayout(), VK_PIPELINE_BIND_POINT_COMPUTE);
    glm::vec3 test = glm::vec3(1.0f, 0.0f, 0.0f);
    vkCmdPushConstants(cmdBuffer, computePipeline.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(glm::vec3), &test);
    computePipeline.dispatch(cmdBuffer, 1280, 720, 1);
    image.transferQueueOwnership(cmdBuffer, device->getRenderQueueFamilyIndexFromQueu(device->getRenderQueue()));
    
    cmdBuffer.endCommandBuffer();
    Fence f(device, false);
    cmdBuffer.submitCommandBuffer({}, {}, &f, true);
    f.waitForFence();

    // Window w{1280,720, "mon napli"};
    // Device d = Device(w);
    // // CommandBufferPool cbp (&d, d.getRenderQueue());
    // // CommandBuffer cb = std::move(cbp.createCommandBuffer(1)[0]);
    // // cb.beginCommandBuffer();
    // // cb.endCommandBuffer();
    // // Fence f(&d, false);
    // // std::cout << f.getFenceStatus() << std::endl;
    // // cb.submitCommandBuffer({}, {}, &f, true);
    // // std::cout << f.getFenceStatus() << std::endl;

    // // Buffer b (&d, sizeof(uint32_t), 10, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    // // Buffer b2 = std::move(b);
    // // uint32_t data[10] = {1,2,3,4,5,6,7,8,9,10};
    // // b2.writeToBuffer(&data, sizeof(data) * 10, 0);

    // ImageCreateInfo ici;
    // ici.width = 1280;
    // ici.height = 720;
    // ici.format = VK_FORMAT_R8G8B8A8_SRGB;
    // ici.usageFlags =  VK_IMAGE_USAGE_SAMPLED_BIT;
    // ici.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
    // ici.filename.push_back("../textures/posx.jpg");
    // ici.filename.push_back("../textures/negx.jpg");
    // ici.filename.push_back("../textures/posy.jpg");
    // ici.filename.push_back("../textures/negy.jpg");
    // ici.filename.push_back("../textures/posz.jpg");
    // ici.filename.push_back("../textures/negz.jpg");
    // for(auto f : ici.filename) {
    //     std::cout << f << std::endl;
    // }
    // ici.isCubeTexture = true;
    // ici.layers = 6;
    // Image im(&d, ici);

    // vkDeviceWaitIdle(d);
    // CommandPoolHandler::cleanUnusedPools();
}

void App::resize(int width, int height, std::vector<Image> &swapchainImages) { this->swapchainImages = &swapchainImages; }
void App::update(float deltaTime, CommandBuffer &cmdBuffer) {

    // std::cout << "UPDATE" << std::endl << std::endl;
    // computePipeline.bindPipeline(cmdBuffer);
    // std::vector<DescriptorSet *> descriptorSets = {&descriptorSet};
    // DescriptorSet::bindDescriptorSet(cmdBuffer, descriptorSets, computePipeline.getPipelineLayout(), VK_PIPELINE_BIND_POINT_COMPUTE);
    // glm::vec3 test = glm::vec3(1.0f, 0.0f, 0.0f);
    // vkCmdPushConstants(cmdBuffer, computePipeline.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(glm::vec3), &test);
    // computePipeline.dispatch(cmdBuffer, 1280, 720, 1);

    // ImageCreateInfo ici;
    // ici.width = 1280;
    // ici.height = 720;
    // ici.format = VK_FORMAT_R8G8B8A8_SNORM;
    // ici.usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    // ici.imageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    // Image renderIm (device, ici);

    // Image::copyImage(device, image, renderIm, &cmdBuffer);
    // renderIm.transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, &cmdBuffer);
    // renderIm.transferQueueOwnership(cmdBuffer, device->getRenderQueueFamilyIndexFromQueu(device->getRenderQueue()));

    cmdBuffer.endCommandBuffer();

     cmdBuffer.submitCommandBuffer({}, {}, nullptr, true);
    // testMutex.lock();
    // // std::cout << "TRANSFERT SHARED_PTR" << std::endl;
    // delete renderedImage;
    // renderedImage = new Image(renderIm);
    // testMutex.unlock();
}
void App::renderFrame(float deltatTime, CommandBuffer &cmdBuffer, uint32_t curentFrameIndex) {
    // copy renderedImage to swapchainImage
    // if (renderedImage) {
        
        
        testMutex.lock();
        Image::blitImage(device, image, (*swapchainImages)[curentFrameIndex], &cmdBuffer);
        // cmdBuffer.addRessourceToDestroy(new Image(*renderedImage));
        testMutex.unlock();

    // }
}
}  // namespace TTe