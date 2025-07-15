
#include "app.hpp"

#include <vulkan/vulkan_core.h>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <memory>
#include <thread>
#include <vector>

#include "GPU_data/image.hpp"
#include "device.hpp"
#include "dynamic_renderpass.hpp"
#include "imgui.h"
#include "sceneV2/animatic/simulation/ObjetSimuleMSS.h"
#include "sceneV2/animatic/skeleton/BVH.h"
#include "sceneV2/animatic/skeletonObj.hpp"
#include "sceneV2/collision/collision_obj.hpp"
#include "sceneV2/container.hpp"
#include "sceneV2/loader/gltf_loader.hpp"
#include "sceneV2/mesh.hpp"

// #include "scene/objects/collision_obj.hpp"
// #include "scene/objects/simulation/ObjetSimuleMSS.h"
#include "sceneV2/node.hpp"
#include "sceneV2/objLoader.hpp"
#include "sceneV2/render_data.hpp"
#include "sceneV2/renderable/basicMeshObj.hpp"
#include "sceneV2/renderable/portalObj.hpp"
#include "sceneV2/renderable/staticMeshObj.hpp"
#include "sceneV2/scene.hpp"
#include "struct.hpp"
#include "swapchain.hpp"

namespace TTe {

void App::init(Device *device, SwapChain *swapchain, Window *window) {
    this->swapchain = swapchain;
    this->device = device;
    renderPass =
        DynamicRenderPass(device, {1280, 720}, {}, swapchain->getswapChainImages().size(), depthAndStencil::DEPTH, swapchain, nullptr);

    VkExtent2D size = {1280, 720};
    PortalObj::init(device);
    std::vector<std::vector<std::vector<Image>>> portalATextures;
    std::vector<std::vector<std::vector<Image>>> portalBTextures;

    GLTFLoader gltfLoader;
    gltfLoader.load("gltf/glTF/Sponza.gltf");

    for (int i = 0; i < 5; i++) {
        portalARenderPasses.push_back(
            DynamicRenderPass(device, size, {VK_FORMAT_R8G8B8A8_SRGB}, swapchain->getswapChainImages().size(), depthAndStencil::DEPTH, nullptr, nullptr));
        portalBRenderPasses.push_back(
            DynamicRenderPass(device, size, {VK_FORMAT_R8G8B8A8_SRGB}, swapchain->getswapChainImages().size(), depthAndStencil::DEPTH, nullptr, nullptr));
        size = {size.width / 2, size.height / 2};
        
        portalATextures.push_back(portalARenderPasses.back().getimageAttachement());
        portalBTextures.push_back(portalBRenderPasses.back().getimageAttachement());
    }
    
    PortalObj::resize(device, portalATextures, portalBTextures);

    ObjLoader objLoader = ObjLoader(device);
    ObjectFileData data = objLoader.loadObject("../data/mesh/cubes.obj");
    movementController.setCursors(window);
    vkDeviceWaitIdle(*device);

    scene2 = std::make_shared<Scene2>(device);

    skeleton = std::make_shared<SkeletonObj>();
    skeleton->init("../data/motionFSM");

    scene2->getMainCamera()->extent = {1280, 720};
    // scene2->addMesh(m2);

    StaticMeshObj obj2 = StaticMeshObj();
    obj2.setMeshId(0);

    ImageCreateInfo imageCreateInfo;
    imageCreateInfo.enableMipMap = true;
    imageCreateInfo.usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.filename.push_back("dt.jpg");

    Image image = Image(device, imageCreateInfo);

    imageCreateInfo.filename[0] = "normal.jpg";
    Image normal = Image(device, imageCreateInfo);

    imageCreateInfo.filename[0] = "mr.jpg";
    Image mr = Image(device, imageCreateInfo);

    uint32_t albedo_id = scene2->addImage(image);
    uint32_t normal_id = scene2->addImage(normal);
    uint32_t mr_id = scene2->addImage(mr);

    Material mat;
    mat.albedo_tex_id = albedo_id;
    mat.normal_tex_id = normal_id;
    mat.metallic_roughness_tex_id = mr_id;

   
    uint mat_id = scene2->addMaterial(mat);



    scene2->addObjectFileData(data);
    scene2->Param("../data/simu/Fichier_Param.simu");
    // ObjetSimuleMSS objMss = ObjetSimuleMSS(device, "../data/simu/Fichier_Param.objet1");

    StaticMeshObj obj3 = StaticMeshObj();
    int i = 0;
    int mapId = scene2->addNode(-1, std::make_shared<Container>());
    // for (auto &mesh : data.meshes) {
    //     obj3.setMeshId(i);
    //     scene2->addNode(mapId, std::make_shared<StaticMeshObj>(obj3));
    //     i++;
    // }
    std::cout << "mapId : " << mapId << std::endl;

    uint32_t cape_id  = scene2->addNode(-1, std::make_shared<ObjetSimuleMSS>(device, "../data/simu/Fichier_Param.objet1"));

    scene2->addNode(-1, skeleton);


    scene2->updateMaterialBuffer();
    scene2->updateDescriptorSets();
    scene2->getMainCamera()->transform.pos = glm::vec3(0.0f, 13.0f, -8.0f);

    std::cout << "\% de leaf sans triangle : " << Mesh::leaf_without_triangle_count * 100.0f / Mesh::leaf_count << std::endl;
    scene2->computeBoundingBox();
    std::shared_ptr<Node> cape = scene2->getNode(cape_id);
    // cast to ObjetSimuleMSS
    std::shared_ptr<ObjetSimuleMSS> capeSim = std::dynamic_pointer_cast<ObjetSimuleMSS>(cape);
    // capeSim->transform.rot= glm::vec3(M_PI/2.0, 0.0f, 0.0f);
    capeSim->attachToNode(0, skeleton->getChild(0)->getChild(0)->getChild(0)->getChild(2)->getChild(0));
    capeSim->attachToNode(69, skeleton->getChild(0)->getChild(0)->getChild(0)->getChild(1)->getChild(0));

    capeSim->attachToNode(25, skeleton->getChild(0)->getChild(0)->getChild(0)->getChild(2));
    capeSim->attachToNode(44, skeleton->getChild(0)->getChild(0)->getChild(0)->getChild(1));
    // capeSim->attachToNode(34, skeleton->getChild(0)->getChild(1)->getChild(0));

    std::shared_ptr<BasicMeshObj> b = std::make_shared<BasicMeshObj>();
    std::shared_ptr<CollisionObject> c = std::make_shared<CollisionObject>(CollisionObject::cube);
    b->setShape(Cube);

    b->transform.pos = glm::vec3(3.5,-2,-1.5);
    b->transform.scale = glm::vec3(2);
    c->transform.scale = glm::vec3(1.09);
    // auto iddd = scene2->addNode(-1, b);
    // scene2->addNode(iddd, c);

    capeSim->setMaterial(mat_id);

    // add sphere for show hit

    scene2->computeBoundingBox();


    scene2->getMainCamera()->setParent(skeleton.get());

    // movementController.init(device, scene2.get());
}

void App::resize(int width, int height) {
    renderPass.resize({static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
    scene2->getMainCamera()->extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};


    VkExtent2D size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    std::vector<std::vector<std::vector<Image>>> portalATextures;
    std::vector<std::vector<std::vector<Image>>> portalBTextures;
    for (int i = 0; i < 5; i++) {
        portalARenderPasses[i].resize(size);
        portalBRenderPasses[i].resize(size);
        portalATextures.push_back(portalARenderPasses[i].getimageAttachement());
        portalBTextures.push_back(portalBRenderPasses[i].getimageAttachement());
    }

    PortalObj::resize(device, portalATextures, portalBTextures);
}
void App::update(float deltaTime, CommandBuffer &cmdBuffer, Window &windowObj) {
    tick++;
    float maxDT = 1.0f / 144.0f;
    // if dt < 1/120, we wait
    // if (deltaTime < maxDT) {
    //     std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>((maxDT - deltaTime) * 1000)));
    //     deltaTime = maxDT;
    // }
    time += deltaTime;
    // printf("%f \n", deltaTime);

    movementController.moveInPlaneXZ(&windowObj, deltaTime, scene2->getMainCamera());
    // scene.updateCameraBuffer();
    // calcul time
    scene2->updateFromInput(&windowObj, deltaTime);
    scene2->updateSim(deltaTime, time, tick);

    // scene2->updateCameraBuffer(near, x_rot);

    scene2->updateCameraBuffer();
}
void App::renderFrame(float deltatTime, CommandBuffer &cmdBuffer, uint32_t curentFrameIndex, uint32_t render_index) {
    // static float blend;
    static float color[3] = {1.0f, 1.0f, 1.0f};
    static float metallic = 0.0f;
    static float roughness = 0.9f;

    static glm::vec3 pos = {0.0f, 0.0f, 0.0f};
    static glm::vec3 rot;
    static float scale = 1.0f;

    static uint camid = 0;

    ImGui::Begin("test");
    ImGui::Text("Hello, world!");
    ImGui::Text("time: %f", time);
    ImGui::Text("tick: %d", tick);
    ImGui::Text("FPS: %f", 1.0f / deltatTime);
    ImGui::Text(
        "Camera pos: %f %f %f", scene2->getMainCamera()->transform.pos->x, scene2->getMainCamera()->transform.pos->y,
        scene2->getMainCamera()->transform.pos->z);

    // ImGui::DragFloat("const char *label", float *v)

    ImGui::Text("%s", ("state: " + skeleton->getStrfromState(skeleton->state)).c_str());
    ImGui::Text("next state: %s", skeleton->getStrfromState(skeleton->nextState).c_str());
    ImGui::End();

    // scene2->getMainCamera()->transform.pos->x = blend;
    scene2->getMaterials()[0].color = glm::vec4(color[0], color[1], color[2], 1.0f);
    scene2->getMaterials()[0].metallic = metallic;

    scene2->updateMaterialBuffer();

    scene2->getNode(2)->transform.pos = pos;
    scene2->getNode(2)->transform.rot = rot;
    scene2->getNode(2)->transform.scale = glm::vec3(scale);



    // render from portal perspective
    RenderData rData;
    rData.recursionLevel = 0;
    rData.cameraId = 1;
    rData.frameIndex = render_index;
    rData.renderPass = &portalARenderPasses[0];

    // rData.portal_pos = movementController.portalObjB->transform.pos.value;
    // rData.portal_normal = movementController.portalObjB->normal;

    // portalARenderPasses[0].beginRenderPass(cmdBuffer, curentFrameIndex);
    // scene2->render(cmdBuffer, rData);
    // portalARenderPasses[0].endRenderPass(cmdBuffer);
    

    rData.cameraId = 2;
    // rData.portal_pos = movementController.portalObjA->transform.pos.value;
    // rData.portal_normal = movementController.portalObjA->normal;
    // rData.renderPass = &portalBRenderPasses[0];
    // portalBRenderPasses[0].beginRenderPass(cmdBuffer, curentFrameIndex);
    // scene2->render(cmdBuffer, rData);
    // portalBRenderPasses[0].endRenderPass(cmdBuffer);

    // put a sync point here to wait for the render pass to finish
    // vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
                        //  nullptr, 0, nullptr);

rData.recursionLevel = 0;
    rData.cameraId = camid;


    renderPass.beginRenderPass(cmdBuffer, curentFrameIndex);
    
    rData.renderPass = &renderPass;

    scene2->render(cmdBuffer, rData);

    renderPass.endRenderPass(cmdBuffer);
}
}  // namespace TTe