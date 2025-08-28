
#include "app.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

#include "device.hpp"

#include "sceneV2/loader/gltf_loader.hpp"
#include "sceneV2/main_controller.hpp"
#include "sceneV2/render_data.hpp"
#include "sceneV2/scene.hpp"


namespace TTe {

void App::init(Device* device, DynamicRenderPass *deferredRenderPass, DynamicRenderPass *shadingRenderPass, Window* window) {
    
    this->device = device;
    this->deferredRenderPass = deferredRenderPass;
    this->shadingRenderPass = shadingRenderPass;
    movementController.setCursors(window);



    GLTFLoader gltfLoader(device);
    gltfLoader.load("gltf/SciFiHelmet/glTF/SciFiHelmet.gltf");
    s = gltfLoader.getScene();
    s->initSceneData(deferredRenderPass, shadingRenderPass);
   


    // movementController.setCursors(window);


    // add sphere for show hit

    // scene2->computeBoundingBox();


    // scene2->getMainCamera()->setParent(skeleton.get());

    // movementController.init(device, scene2.get());
}

void App::resize(int width, int height) {
    s->updateRenderPassDescriptorSets();
    s->getMainCamera()->extent = { (uint32_t)width, (uint32_t)height };
}
void App::update(float deltaTime, CommandBuffer &cmdBuffer, Window &windowObj) {

    movementController.moveInPlaneXZ(&windowObj, deltaTime, s->getMainCamera());
    s->updateCameraBuffer();
   
}


void App::renderDeferredFrame(float deltatTime, CommandBuffer &cmdBuffer ,uint32_t render_index, uint32_t swapchainIndex) {
    RenderData r;
    r.frameIndex = render_index;
    r.cameraId = 0;
    r.renderPass = shadingRenderPass;
    deferredRenderPass->beginRenderPass(cmdBuffer, swapchainIndex);
    s->renderDeffered(cmdBuffer, r);
    deferredRenderPass->endRenderPass(cmdBuffer);
   
}

void App::renderShadedFrame(float deltatTime, CommandBuffer &cmdBuffer, uint32_t render_index, uint32_t swapchainIndex) {
    // shadingRenderPass->setClearColor({1.0,0.0,.0});
    // shadingRenderPass->setClearEnable(true);
    // shadingRenderPass->beginRenderPass(cmdBuffer, swapchainIndex);
    // shadingRenderPass->endRenderPass(cmdBuffer);
    RenderData r;
     r.frameIndex = render_index;
    r.cameraId = 0;
    r.swapchainIndex = swapchainIndex;
     r.renderPass = shadingRenderPass;
    s->renderShading(cmdBuffer, r);

}
}  // namespace TTe