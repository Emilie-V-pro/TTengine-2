
#include "app.hpp"

#include <vulkan/vulkan_core.h>

#include <cmath>
#include <cstdint>
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
#include "sceneV2/mesh.hpp"


// #include "scene/objects/collision_obj.hpp"
// #include "scene/objects/simulation/ObjetSimuleMSS.h"
#include "sceneV2/node.hpp"
#include "sceneV2/objLoader.hpp"
#include "sceneV2/renderable/basicMeshObj.hpp"
#include "sceneV2/scene.hpp"
#include "sceneV2/renderable/staticMeshObj.hpp"
#include "struct.hpp"
#include "swapchain.hpp"

namespace TTe {

void App::init(Device *device, SwapChain *swapchain, Window* window) {

    this->swapchain = swapchain;
    this->device = device;
    renderPass =
    DynamicRenderPass(device, {1280, 720}, {}, swapchain->getswapChainImages().size(), depthAndStencil::DEPTH, swapchain, nullptr);

    ObjLoader objLoader = ObjLoader(device);
    ObjectFileData data = objLoader.loadObject("../data/mesh/lyoko.obj");
    movementController.setCursors(window);
    vkDeviceWaitIdle(*device);

    scene2 = std::make_shared<Scene2>(device);

 
    BVH bvh = BVH("../data/danse.bvh");
    std::shared_ptr<SkeletonObj> skeleton = std::make_shared<SkeletonObj>();
    skeleton->init("../data/motionFSM");


    
    scene2->getMainCamera()->extent = {1280, 720};
    // scene2->addMesh(m2);
    
    StaticMeshObj obj2 = StaticMeshObj();
    obj2.setMeshId(0);



    ImageCreateInfo imageCreateInfo;
    imageCreateInfo.enableMipMap  = true;
    imageCreateInfo.usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT ;
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

    std::cout << "albedo_id : " << albedo_id << " normal_id : " << normal_id << " mr_id : " << mr_id << std::endl;

    uint mat_id = scene2->addMaterial(mat);

    std::cout << "mat_id : " << mat_id << std::endl;

    scene2->addObjectFileData(data);
    scene2->Param("../data/simu/Fichier_Param.simu");
    // ObjetSimuleMSS objMss = ObjetSimuleMSS(device, "../data/simu/Fichier_Param.objet1");

    StaticMeshObj obj3 = StaticMeshObj();
    int i = 0;
    int mapId = scene2->addNode(-1, std::make_shared<Container>());
    for (auto & mesh : data.meshes) {
        obj3.setMeshId(i);
        scene2->addNode(mapId, std::make_shared<StaticMeshObj>(obj3));
        i++;
    }
    std::cout << "mapId : " << mapId << std::endl;

    uint32_t cape_id  = scene2->addNode(-1, std::make_shared<ObjetSimuleMSS>(device, "../data/simu/Fichier_Param.objet1"));

    scene2->addNode(-1, skeleton);

    scene2->updateMaterialBuffer();
    scene2->updateDescriptorSets();

    std::shared_ptr<Node> cape = scene2->getNode(cape_id);
    // cast to ObjetSimuleMSS
    std::shared_ptr<ObjetSimuleMSS> capeSim = std::dynamic_pointer_cast<ObjetSimuleMSS>(cape);
    // capeSim->transform.rot->x = (M_PI/2.0);
    capeSim->attachToNode(0, skeleton->getChild(0)->getChild(0)->getChild(0)->getChild(2)->getChild(0));
    capeSim->attachToNode(69, skeleton->getChild(0)->getChild(0)->getChild(0)->getChild(1)->getChild(0));

    capeSim->attachToNode(19, skeleton->getChild(0)->getChild(0)->getChild(0)->getChild(2));
    capeSim->attachToNode(49, skeleton->getChild(0)->getChild(0)->getChild(0)->getChild(1));
    // capeSim->attachToNode(34, skeleton->getChild(0)->getChild(1)->getChild(0));

    // std::shared_ptr<BasicMeshObj> b = std::make_shared<BasicMeshObj>();
    // std::shared_ptr<CollisionObject> c = std::make_shared<CollisionObject>(CollisionObject::cube);
    // b->setShape(Cube);
    
    // b->transform.pos = glm::vec3(3.5,-2,-1.5);
    // c->transform.scale = glm::vec3(1.08);
    // auto iddd = scene2->addNode(-1, b);
    // scene2->addNode(iddd, c);

    capeSim->setMaterial(mat_id);
}

void App::resize(int width, int height) { 
    renderPass.resize({static_cast<uint32_t>(width), static_cast<uint32_t>(height)}); 
    scene2->getMainCamera()->extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}
void App::update(float deltaTime, CommandBuffer &cmdBuffer, Window &windowObj) {
    tick++;
    float maxDT = 1.0f / 144.0f;
    //if dt < 1/120, we wait
    // if (deltaTime < maxDT) {
    //     std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>((maxDT - deltaTime) * 1000)));
    //     deltaTime = maxDT;
    // }
    time += deltaTime;


    movementController.moveInPlaneXZ(&windowObj, deltaTime, scene2->getMainCamera());
 // scene.updateCameraBuffer();
    // calcul time
    scene2->updateFromInput(&windowObj, deltaTime);
    scene2->updateSim(deltaTime, time, tick);

    scene2->updateCameraBuffer();
}
void App::renderFrame(float deltatTime, CommandBuffer &cmdBuffer, uint32_t curentFrameIndex) {
    // static float blend;
    static float color[3] = {1.0f, 1.0f, 1.0f};
    static float metallic = 0.0f;
    static float roughness = 0.9f;

    static glm::vec3 pos = {0.0f, 0.0f, 0.0f};
    static glm::vec3 rot;
    static float scale = 1.0f;

    ImGui::Begin("test");
    ImGui::Text("Hello, world!");
    ImGui::Text("time: %f", time);
    ImGui::Text("tick: %d", tick);
    ImGui::Text("FPS: %f", 1.0f / deltatTime);
    ImGui::Text("Camera pos: %f %f %f", scene2->getMainCamera()->transform.pos->x, scene2->getMainCamera()->transform.pos->y,
                scene2->getMainCamera()->transform.pos->z);

    // ImGui::SliderFloat("blending factor", &blend, 0.0f, 1.0f);
    // ImGui::ColorPicker3("KOULEUR", color);
    // ImGui::SliderFloat("metallic", &metallic, 0.0f, 1.0f);
    // ImGui::SliderFloat("roughness", &roughness, 0.0f, 1.0f);

    ImGui::PushItemWidth(80);
        ImGui::Text("pos"); ImGui::SameLine();
        ImGui::SliderFloat("X", &pos.x, -20.0f, 20.0f); ImGui::SameLine();
        ImGui::SliderFloat("Y", &pos.y, -20.0f, 20.0f); ImGui::SameLine();
        ImGui::SliderFloat("Z", &pos.z, -20.0f, 20.0f);
    ImGui::PopItemWidth();

    ImGui::PushItemWidth(80);
        ImGui::Text("rot"); ImGui::SameLine();
        ImGui::SliderFloat("rX", &rot.x, 0.0f, 5.0f); ImGui::SameLine();
        ImGui::SliderFloat("rY", &rot.y, 0.0f, 5.0f); ImGui::SameLine();
        ImGui::SliderFloat("rZ", &rot.z, 0.0f, 5.0f);
    
    ImGui::PopItemWidth();

    ImGui::PushItemWidth(80);
  
        ImGui::SliderFloat("scale", &scale, 0.0f, 2.0f);

    // ImGui::DragFloat("const char *label", float *v)
    ImGui::End();

    

    // scene2->getMainCamera()->transform.pos->x = blend;
    scene2->getMaterials()[0].color = glm::vec4(color[0], color[1], color[2], 1.0f);
    scene2->getMaterials()[0].metallic = metallic;
    scene2->getMaterials()[0].roughness = roughness;
    scene2->getMaterials()[1].roughness = roughness;
    scene2->getMaterials()[2].roughness = roughness;
    scene2->getMaterials()[3].roughness = roughness;
    scene2->getMaterials()[4].roughness = roughness;
    scene2->getMaterials()[5].roughness = roughness;
    scene2->updateMaterialBuffer();

    scene2->getNode(2)->transform.pos = pos;
    scene2->getNode(2)->transform.rot = rot;
    scene2->getNode(2)->transform.scale = glm::vec3(scale);

    renderPass.beginRenderPass(cmdBuffer, curentFrameIndex);

    renderPass.setDepthAndStencil(cmdBuffer, false);
    scene2->renderSkybox(cmdBuffer);
    renderPass.setDepthAndStencil(cmdBuffer, true);

    scene2->render(cmdBuffer);
    

    renderPass.endRenderPass(cmdBuffer);


}
}  // namespace TTe