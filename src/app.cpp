
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
#include "sceneV2/animatic/simulation/ObjetSimuleMSS.h"
#include "sceneV2/animatic/skeleton/BVH.h"
#include "sceneV2/animatic/skeletonObj.hpp"
#include "sceneV2/mesh.hpp"


// #include "scene/objects/collision_obj.hpp"
// #include "scene/objects/simulation/ObjetSimuleMSS.h"
#include "sceneV2/node.hpp"
#include "sceneV2/objLoader.hpp"
#include "sceneV2/scene.hpp"
#include "sceneV2/renderable/staticMeshObj.hpp"
#include "struct.hpp"
#include "swapchain.hpp"

namespace TTe {

void App::init(Device *device, SwapChain *swapchain, Window* window) {
    this->swapchain = swapchain;
    this->device = device;
    ObjLoader objLoader = ObjLoader(device);
    ObjectFileData data = objLoader.loadObject("../data/mesh/cubes.obj");
    movementController.setCursors(window);

   

    renderPass =
        DynamicRenderPass(device, {1280, 720}, {}, swapchain->getswapChainImages().size(), depthAndStencil::DEPTH, swapchain, nullptr);

    BVH bvh = BVH("../data/danse.bvh");
    std::shared_ptr<SkeletonObj> skeleton = std::make_shared<SkeletonObj>();
    skeleton->init(bvh);


    scene2 = std::make_shared<Scene2>(device);
    scene2->getMainCamera()->extent = {1280, 720};
    // scene2->addMesh(m2);
    
    StaticMeshObj obj2 = StaticMeshObj();
    obj2.setMeshId(0);

    // scene2->addNode(-1, std::make_shared<StaticMeshObj>(obj2));
    // scene2->addMaterial(mat);
    // scene2->addMaterial(mat);
    // scene2->addMaterial(mat);
    // scene2->addMaterial(mat);
    // scene2->addMaterial(mat);


    // scene2->addImage(image);
    // scene2->addImage(normal);
    // scene2->addImage(mr);
    
    // load sponza sheet texture

    ImageCreateInfo imageCreateInfo;
    imageCreateInfo.enableMipMap  = true;
    imageCreateInfo.usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT ;
    imageCreateInfo.filename.push_back("albedo.jpg");
    
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
    obj3.setMeshId(0);
    scene2->addNode(-1, std::make_shared<StaticMeshObj>(obj3));
    obj3.setMeshId(1);
    scene2->addNode(-1, std::make_shared<StaticMeshObj>(obj3));
    obj3.setMeshId(2);
    scene2->addNode(-1, std::make_shared<StaticMeshObj>(obj3));
    obj3.setMeshId(3);
    scene2->addNode(-1, std::make_shared<StaticMeshObj>(obj3));
    obj3.setMeshId(4);
    scene2->addNode(-1, std::make_shared<StaticMeshObj>(obj3));
    obj3.setMeshId(5);
    scene2->addNode(-1, std::make_shared<StaticMeshObj>(obj3));

    uint32_t cape_id  = scene2->addNode(-1, std::make_shared<ObjetSimuleMSS>(device, "../data/simu/Fichier_Param.objet1"));

    scene2->addNode(-1, skeleton);

    scene2->updateMaterialBuffer();
    scene2->updateDescriptorSets();

    std::shared_ptr<Node> cape = scene2->getNode(cape_id);
    // cast to ObjetSimuleMSS
    std::shared_ptr<ObjetSimuleMSS> capeSim = std::dynamic_pointer_cast<ObjetSimuleMSS>(cape);

    capeSim->attachToNode(0, skeleton->getChild(0)->getChild(1)->getChild(0)->getChild(2)->getChild(0));
    capeSim->attachToNode(69, skeleton->getChild(0)->getChild(1)->getChild(0)->getChild(1)->getChild(0));


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
    scene2->updateSim(deltaTime, time, tick);

    scene2->updateCameraBuffer();
}
void App::renderFrame(float deltatTime, CommandBuffer &cmdBuffer, uint32_t curentFrameIndex) {

    renderPass.beginRenderPass(cmdBuffer, curentFrameIndex);

    renderPass.setDepthAndStencil(cmdBuffer, false);
    scene2->renderSkybox(cmdBuffer);
    renderPass.setDepthAndStencil(cmdBuffer, true);

    scene2->render(cmdBuffer);
    

    renderPass.endRenderPass(cmdBuffer);


}
}  // namespace TTe