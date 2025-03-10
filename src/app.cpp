
#include "app.hpp"

#include <vulkan/vulkan_core.h>

#include <cmath>
#include <glm/fwd.hpp>

#include <glm/geometric.hpp>
#include <vector>


#include "GPU_data/image.hpp"
#include "device.hpp"
#include "dynamic_renderpass.hpp"
#include "scene/mesh.hpp"
#include "scene/object.hpp"
#include "scene/objects/animatic/BVH.h"
#include "scene/objects/collision_obj.hpp"
#include "scene/objects/simulation/ObjetSimuleMSS.h"
#include "struct.hpp"
#include "swapchain.hpp"

namespace TTe {

void App::init(Device *device, SwapChain *swapchain, Window* window) {
    this->swapchain = swapchain;
    this->device = device;

    movementController.setCursors(window);

    BVH bvh;
    bvh.init("../data/Robot.bvh");

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

    Mesh m2 = Mesh(device, "../data/mesh/cubes.obj");

    scene = Scene(device);
    Object o = Object();
    o.meshId = 0;
    o.transform.pos = {4, -2, -3};
    o.transform.scale = {0.95, 0.95, 0.95};
    scene.objects.push_back(o);
    // scene.meshes.push_back(m2);
    scene.camera.transform.pos = {0, 2, -10};
    scene.camera.extent = {1280, 720};

    scene.meshes.push_back(Mesh(device, BasicShape::Sphere, 2));

    ImageCreateInfo imageCreateInfo;
    imageCreateInfo.filename.push_back("../data/textures/albedo.jpg");
    imageCreateInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageCreateInfo.usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    // imageCreateInfo.enableMipMap = true;

    Image image = Image(device, imageCreateInfo);
    // image.generateMipmaps();

    imageCreateInfo.filename.clear();
    imageCreateInfo.filename.push_back("../data/textures/normal.jpg");
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;

    Image normal = Image(device, imageCreateInfo);

    imageCreateInfo.filename.clear();
    imageCreateInfo.filename.push_back("../data/textures/mr.jpg");

    Image mr = Image(device, imageCreateInfo);

    Material mat = Material();
    mat.color = {1, 1, 1, 1};
    mat.metallic = 0.5;
    mat.roughness = 0.5;
    mat.albedo_tex_id = 0;
    mat.normal_tex_id = 1;
    mat.metallic_roughness_tex_id = 2;

    scene.materials.push_back(mat);
    scene.materials.push_back(mat);
    scene.materials.push_back(mat);
    scene.materials.push_back(mat);
    scene.materials.push_back(mat);
    scene.materials.push_back(mat);


    scene.Param("../data/simu/Fichier_Param.simu");
    auto obj = ObjetSimuleMSS(device, "../data/simu/Fichier_Param.objet1");
    obj.transform.rot = {M_PI/2.0, 0, 0};
    scene.addMssObject(obj);

    CollisionObject co (CollisionObject::sphere);
    scene.collisionObjects.push_back(co);
    scene.collisionObjects[0].transform.pos = {4, -2, -3};
    // scene.collisionObjects[0].scale = {0.1, 0.1, 0.1};
    // scene.addBVH(bvh);

    scene.textures.push_back(image);
    scene.textures.push_back(normal);
    scene.textures.push_back(mr);

    scene.createDescriptorSets();

    
    scene.updateBuffer();

    
}

void App::resize(int width, int height) { 
    renderPass.resize({static_cast<uint32_t>(width), static_cast<uint32_t>(height)}); 
    scene.camera.extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}
void App::update(float deltaTime, CommandBuffer &cmdBuffer, Window &windowObj) {
   
    time += deltaTime;
    printf("time : %f\n", deltaTime);
    //   float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    //   float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    //   float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    // scene.collisionObjects[0].translation = scene.camera.translation;
    scene.updateSimu(deltaTime, time);
    movementController.moveInPlaneXZ(&windowObj, deltaTime, scene.camera);
    renderPass.setClearColor({0.01, 0.01, 0.01});
    // scene.camera.rotation = glm::normalize(glm::vec3(-1));
    // std::cout << time << std::endl;
    // std::cout << "camera rot : " << scene.camera.rotation.x << " " << scene.camera.rotation.y << " " << scene.camera.rotation.z << std::endl;
    scene.updateCameraBuffer();
}
void App::renderFrame(float deltatTime, CommandBuffer &cmdBuffer, uint32_t curentFrameIndex) {
    // copy renderedImage to swapchainImage
    // if (renderedImage) {

    // swapchain->getSwapChainImage(curentFrameIndex).transitionImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &cmdBuffer);

    renderPass.beginRenderPass(cmdBuffer, curentFrameIndex);
    scene.render(cmdBuffer);
    renderPass.endRenderPass(cmdBuffer);

    // Image::blitImage(device, *renderedImage, (*swapchainImages)[curentFrameIndex], &cmdBuffer);
    // cmdBuffer.addRessourceToDestroy(renderedImage);
    // renderedImage = nullptr;
    // testMutex.unlock();

    // testMutex.lock();
    // cmdBuffer.addRessourceToDestroy(new Image(*renderedImage));
    // testMutex.unlock();

    // }
}
}  // namespace TTe