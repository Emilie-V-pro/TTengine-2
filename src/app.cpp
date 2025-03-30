
#include "app.hpp"

#include <vulkan/vulkan_core.h>

#include <cmath>
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

    BVH bvh = BVH("../data/Robot.bvh");
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

    scene2->addNode(-1, std::make_shared<ObjetSimuleMSS>(device, "../data/simu/Fichier_Param.objet1"));

    scene2->addNode(-1, skeleton);

    scene2->updateMaterialBuffer();
    scene2->updateDescriptorSets();


}

void App::resize(int width, int height) { 
    renderPass.resize({static_cast<uint32_t>(width), static_cast<uint32_t>(height)}); 
    scene2->getMainCamera()->extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}
void App::update(float deltaTime, CommandBuffer &cmdBuffer, Window &windowObj) {
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
    auto start = std::chrono::high_resolution_clock::now();
    scene2->updateSim(deltaTime, time);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "updateSim time: " << elapsed.count() << std::endl;
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