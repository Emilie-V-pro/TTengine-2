
#include "app.hpp"

#include <GLFW/glfw3.h>

#include <cstdint>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <memory>
#include <random>
#include <vector>

#include "commandBuffer/commandPool_handler.hpp"
#include "device.hpp"
#include "dynamic_renderpass.hpp"
#include "sceneV2/light.hpp"
#include "sceneV2/loader/gltf_loader.hpp"
#include "sceneV2/main_controller.hpp"
#include "sceneV2/render_data.hpp"
#include "sceneV2/scene.hpp"

namespace TTe {

void App::init(Device *device, DynamicRenderPass *deferredRenderPass, DynamicRenderPass *shadingRenderPass, Window *window) {
    this->device = device;
    this->deferredRenderPass = deferredRenderPass;
    this->shadingRenderPass = shadingRenderPass;
    movementController.setCursors(window);

    GLTFLoader gltfLoader(device);
    // gltfLoader.load("gltf/ABeautifulGame/glTF/ABeautifulGame.gltf");
    auto start = std::chrono::high_resolution_clock::now();
    gltfLoader.load("gltf/Sponza/glTF/Sponza.gltf");

    // gltfLoader.load("gltf/mc2/mc.gltf");

    // gltfLoader.load("gltf/mc/mc.gltf");
    s = gltfLoader.getScene();
    // s = new Scene(device);
    s->initSceneData(deferredRenderPass, shadingRenderPass);
    s->computeBoundingBox();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    printf("Load gltf time: %ld ms\n", duration);

    std::default_random_engine gen;
    std::uniform_real_distribution<double> distribution(-200.0, 200.0);
    std::uniform_real_distribution<double> distribution2(0.0, 50.0);
    std::uniform_real_distribution<double> distribution3(0.0, 1.0);

    for (int i = 0; i < MAX_LIGHTS; i++) {
        auto l = std::make_shared<Light>();
        l->type = Light::POINT;
        l->transform.pos = glm::vec3{distribution(gen), distribution2(gen), distribution(gen)};
        l->color = glm::vec3{distribution3(gen), distribution3(gen), distribution3(gen)};
        l->intensity = 100.f;
        s->addNode(-1, l);
        lightAccelerations.push_back(glm::vec3(0, 0, 0));
        lightSpeeds.push_back(glm::vec3(0, 0, 0));
    }
    s->updateLightBuffer();
    std::vector<std::shared_ptr<Light>> &P = s->getLights();
    for (int i = 0; i < MAX_LIGHTS; ++i) {
        P[i]->updateOnchangeFunc();
    }

    // movementController.setCursors(window);

    // add sphere for show hit

    // scene2->computeBoundingBox();

    // scene2->getMainCamera()->setParent(skeleton.get());

    // movementController.init(device, scene2.get());
}

App::~App() { delete s; }

void App::resize(int width, int height) {
    s->updateRenderPassDescriptorSets();
    s->getMainCamera()->extent = {(uint32_t)width, (uint32_t)height};
}

void App::update(float deltaTime, CommandBuffer &cmdBuffer, Window &windowObj) {
    movementController.moveInPlaneXZ(&windowObj, deltaTime, s->getMainCamera());
    std::default_random_engine gen;
    std::uniform_real_distribution<double> distribution3(-0.1, 0.1);

    std::vector<std::shared_ptr<Light>> &P = s->getLights();
    for (int i = 0; i < MAX_LIGHTS; ++i) {
        lightAccelerations[i] = glm::vec3(distribution3(gen), distribution3(gen), distribution3(gen));
        lightSpeeds[i] = (lightSpeeds[i] + deltaTime * lightAccelerations[i]) * 1.f;
        P[i]->transform.pos = P[i]->transform.pos + deltaTime * lightSpeeds[i];

        if (P[i]->transform.pos->x > 200 || P[i]->transform.pos->x < -200) {
            P[i]->transform.pos->x = -P[i]->transform.pos->x;
        }
        if (P[i]->transform.pos->y > 50) {
            P[i]->transform.pos->y -= 50;
        }
        else if(P[i]->transform.pos->y < 0){
            P[i]->transform.pos->y +=  50;
        }
        if (P[i]->transform.pos->z > 200 || P[i]->transform.pos->z < -200) {
            P[i]->transform.pos->z = -P[i]->transform.pos->z;
        }
    }
    s->updateLightBuffer();

    if (glfwGetKey(windowObj, GLFW_KEY_P) == GLFW_PRESS) {
        CommandBuffer renderCmdBuffer =

            std::move(CommandPoolHandler::getCommandPool(device, device->getRenderQueue())->createCommandBuffer(1)[0]);
        renderCmdBuffer.beginCommandBuffer();

        DynamicRenderPass temp =
            DynamicRenderPass(device, {4096, 4096}, {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM}, 1, DEPTH, nullptr, nullptr);
        RenderData r;
        r.frameIndex = 0;
        r.cameraId = 0;
        r.renderPass = &temp;
        // s->getMainCamera()->extent = {1,1};
        temp.beginRenderPass(renderCmdBuffer, 0);
        s->renderDeffered(renderCmdBuffer, r);
        temp.endRenderPass(renderCmdBuffer);
        renderCmdBuffer.endCommandBuffer();
        renderCmdBuffer.submitCommandBuffer({}, {}, nullptr, true);

        glm::mat4 view = s->getMainCamera()->getViewMatrix();
        glm::mat4 proj = s->getMainCamera()->getProjectionMatrix();
        temp.savedRenderPass(0);
    }
}

void App::renderDeferredFrame(float deltatTime, CommandBuffer &cmdBuffer, uint32_t render_index, uint32_t swapchainIndex) {
    RenderData r;
    r.frameIndex = render_index;
    r.cameraId = 0;
    r.renderPass = shadingRenderPass;

    s->updateCameraBuffer(render_index);

    deferredRenderPass->beginRenderPass(cmdBuffer, swapchainIndex);
    s->renderDeffered(cmdBuffer, r);
    deferredRenderPass->endRenderPass(cmdBuffer);

    // returnTOWorld /= returnTOWorld.w;
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