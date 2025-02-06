
#include "app.hpp"

#include <glm/fwd.hpp>
#include <iostream>
#include <utility>
#include <vector>

#include "GPU_data/image.hpp"
#include "commandBuffer/commandPool_handler.hpp"
#include "descriptor/descriptorSet.hpp"
#include "device.hpp"
#include "dynamic_renderpass.hpp"
#include "scene/mesh.hpp"
#include "scene/object.hpp"
#include "shader/pipeline/compute_pipeline.hpp"
#include "swapchain.hpp"
#include "synchronisation/fence.hpp"
#include "utils.hpp"

namespace TTe {

void App::init(Device *device, SwapChain *swapchain) {
    this->swapchain = swapchain;
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
    CommandBufferPool *cbp = CommandPoolHandler::getCommandPool(device, device->getComputeQueue());
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

    renderPass =
        DynamicRenderPass(device, {1280, 720}, {}, swapchain->getswapChainImages().size(), depthAndStencil::DEPTH, swapchain, nullptr);

    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, 0},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, 0},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, 0},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, 0},
    };

    std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};
    Mesh m = Mesh(device, indices, vertices);
    scene = Scene(device);
    Object o = Object();
    o.meshId = 0;
    scene.objects.push_back(o);
    scene.meshes.push_back(m);

    scene.materials.push_back({{1, 1, 1, 1}, -1, -1});
    scene.textures.push_back(image);
    scene.createDescriptorSets();
    scene.updateBuffer();

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

void App::resize(int width, int height) { renderPass.resize({static_cast<uint32_t>(width), static_cast<uint32_t>(height)}); }
void App::update(float deltaTime, CommandBuffer &cmdBuffer, Window &windowObj) {
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

    // random float
    //   float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    //   float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    //   float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    movementController.moveInPlaneXZ(&windowObj, deltaTime, scene.camera);
    renderPass.setClearColor({0.01, 0.01, 0.01});
    scene.updateCameraBuffer();
    //  cmdBuffer.endCommandBuffer();

    // cmdBuffer.submitCommandBuffer({}, {}, nullptr, true);
    // testMutex.lock();
    // // std::cout << "TRANSFERT SHARED_PTR" << std::endl;
    // delete renderedImage;
    // renderedImage = new Image(renderIm);
    // testMutex.unlock();
}
void App::renderFrame(float deltatTime, CommandBuffer &cmdBuffer, uint32_t curentFrameIndex) {
    // copy renderedImage to swapchainImage
    // if (renderedImage) {

    swapchain->getSwapChainImage(curentFrameIndex).transitionImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &cmdBuffer);

    renderPass.beginRenderPass(cmdBuffer, curentFrameIndex);
    scene.render(cmdBuffer);
    renderPass.endRenderPass(cmdBuffer);
    // testMutex.lock();
    // Image::blitImage(device, *renderedImage, (*swapchainImages)[curentFrameIndex], &cmdBuffer);
    // cmdBuffer.addRessourceToDestroy(renderedImage);
    // renderedImage = nullptr;
    // testMutex.unlock();

    // testMutex.lock();
    // Image::blitImage(device, image, (*swapchainImages)[curentFrameIndex], &cmdBuffer);
    // cmdBuffer.addRessourceToDestroy(new Image(*renderedImage));
    // testMutex.unlock();

    // }
}
}  // namespace TTe