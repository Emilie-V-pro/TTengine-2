
#include "app.hpp"

#include <GLFW/glfw3.h>

#include <cstdint>
#include <glm/ext/scalar_common.hpp>
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

void App::init(Device *p_device, DynamicRenderPass *p_deferred_renderpass, DynamicRenderPass *p_shading_renderpass, Window *p_window) {
    this->m_device = p_device;
    this->m_deferred_renderpass = p_deferred_renderpass;
    this->m_shading_renderpass = p_shading_renderpass;
    m_movement_controller.setCursors(p_window);

    GLTFLoader gltf_loader(m_device);
    // gltfLoader.load("gltf/ABeautifulGame/glTF/ABeautifulGame.gltf");
    auto start = std::chrono::high_resolution_clock::now();
    gltf_loader.load("gltf/Sponza/glTF/Sponza.gltf");

    // gltfLoader.load("gltf/mc2/mc.gltf");

    // gltf_loader.load("gltf/mc/mc.gltf");
    s = gltf_loader.getScene();
    // s = new Scene(device);


    auto sun = std::make_shared<Light>();
    sun->m_type = Light::DIRECTIONAL;
    sun->transform.rot = glm::normalize(glm::vec3(1.0f,1.0f,0.0f));
    sun->color = glm::vec3(1.0f, 1.0f, 0.9f);
    sun->intensity = 1.0f;
    sun->shadows_enabled = true;
    s->addNode(-1, sun);


    s->initSceneData(m_deferred_renderpass, m_shading_renderpass);
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
        l->m_type = Light::POINT;
        l->transform.pos = glm::vec3{distribution(gen), distribution2(gen), distribution(gen)};
        l->color = glm::vec3{distribution3(gen), distribution3(gen), distribution3(gen)};
        l->intensity = 100.f;
        s->addNode(-1, l);
        m_light_accelerations.push_back(glm::vec3(0, 0, 0));
        m_light_speeds.push_back(glm::vec3(0, 0, 0));
    }
    s->updateLightBuffer();
    std::vector<std::shared_ptr<Light>> &P = s->getLights();
    for (int i = 0; i < MAX_LIGHTS; ++i) {
        P[i]->updateOnchangeFunc();
    }

    // m_movement_controller.setCursors(window);

    // add sphere for show hit

    // scene2->computeBoundingBox();

    // scene2->getMainCamera()->setParent(skeleton.get());

    // m_movement_controller.init(device, scene2.get());
}

App::~App() { delete s; }

void App::resize(int p_width, int p_height) {
    s->updateRenderPassDescriptorSets();
    s->getMainCamera()->extent = {(uint32_t)p_width, (uint32_t)p_height};
}

void App::update(float p_delta_time, CommandBuffer &p_cmd_buffer, Window &p_window_obj) {
    m_movement_controller.moveInPlaneXZ(&p_window_obj, p_delta_time, s->getMainCamera());
    std::default_random_engine gen;
    std::uniform_real_distribution<double> distribution3(-0.1, 0.1);

    std::vector<std::shared_ptr<Light>> &P = s->getLights();
    for (int i = 0; i < MAX_LIGHTS; ++i) {
        m_light_accelerations[i] = glm::vec3(distribution3(gen), distribution3(gen), distribution3(gen));
        m_light_speeds[i] = glm::min((m_light_speeds[i] + p_delta_time * m_light_accelerations[i]) * 1.f, glm::vec3( 5.0f));
        P[i]->transform.pos = P[i]->transform.pos + p_delta_time * m_light_speeds[i];

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

    if (glfwGetKey(p_window_obj, GLFW_KEY_P) == GLFW_PRESS) {
        CommandBuffer render_cmd_buffer =

            std::move(CommandPoolHandler::getCommandPool(m_device, m_device->getRenderQueue())->createCommandBuffer(1)[0]);
        render_cmd_buffer.beginCommandBuffer();

        DynamicRenderPass temp =
            DynamicRenderPass(m_device, {4096, 4096}, {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM}, 1, DEPTH, nullptr, nullptr);
        RenderData r;
        r.frame_index = 0;
        r.camera_id = 0;
        r.render_pass = &temp;
        // s->getMainCamera()->extent = {1,1};
        temp.beginRenderPass(render_cmd_buffer, 0);
        s->renderDeffered(render_cmd_buffer, r);
        temp.endRenderPass(render_cmd_buffer);
        render_cmd_buffer.endCommandBuffer();
        render_cmd_buffer.submitCommandBuffer({}, {}, nullptr, true);


        temp.savedRenderPass(0);
    }

    if(glfwGetKey(p_window_obj, GLFW_KEY_C) == GLFW_PRESS){
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
            update_culling[i] = true;
        }
    }
}

void App::renderDeferredFrame(float p_deltat_time, CommandBuffer &p_cmd_buffer, uint32_t p_render_index, uint32_t p_swapchain_index) {
    RenderData r;
    r.frame_index = p_render_index;
    r.swapchain_index = p_swapchain_index;
    r.camera_id = 0;
    r.render_pass = m_shading_renderpass;

    if(update_culling[p_render_index]){
        r.update_culling = true;
        update_culling[p_render_index] = false;
    }
    s->updateCameraBuffer(p_render_index);


    s->renderDeffered(p_cmd_buffer, r);
    s->renderShadowMaps(p_cmd_buffer, r);
   

}

void App::renderShadedFrame(float p_deltat_time, CommandBuffer &p_cmd_buffer, uint32_t p_render_index, uint32_t p_swapchain_index) {
    // m_shading_renderpass->setClearColor({1.0,0.0,.0});
    // m_shading_renderpass->setClearEnable(true);
    // m_shading_renderpass->beginRenderPass(p_cmd_buffer, p_swapchain_index);
    // m_shading_renderpass->endRenderPass(p_cmd_buffer);
    RenderData r;
    r.frame_index = p_render_index;
    r.camera_id = 0;
    r.swapchain_index = p_swapchain_index;
    r.render_pass = m_shading_renderpass;
    s->renderShading(p_cmd_buffer, r);
}
}  // namespace TTe