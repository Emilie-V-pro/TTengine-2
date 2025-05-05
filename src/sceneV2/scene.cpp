
#include "scene.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>

#include "sceneV2/Icollider.hpp"
#include "sceneV2/Irenderable.hpp"
#include "sceneV2/animatic/skeletonObj.hpp"
#include "sceneV2/cameraV2.hpp"
#include "sceneV2/mesh.hpp"
#include "struct.hpp"

namespace TTe {

Scene2::Scene2(Device *device) : device(device) {
    basicMeshes[Sphere] = Mesh(device, Sphere, 4);
    basicMeshes[Cube] = Mesh(device, Cube, 1);

    ImageCreateInfo skyboxImageCreateInfo;
    skyboxImageCreateInfo.filename.push_back("posx.jpg");
    skyboxImageCreateInfo.filename.push_back("negx.jpg");
    skyboxImageCreateInfo.filename.push_back("posy.jpg");
    skyboxImageCreateInfo.filename.push_back("negy.jpg");
    skyboxImageCreateInfo.filename.push_back("posz.jpg");
    skyboxImageCreateInfo.filename.push_back("negz.jpg");
    skyboxImageCreateInfo.usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    skyboxImageCreateInfo.isCubeTexture = true;
    skyboxImageCreateInfo.enableMipMap = true;
    skyboxImage = Image(device, skyboxImageCreateInfo);

    ImageCreateInfo imageCreateInfo;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageCreateInfo.width = 1;
    imageCreateInfo.height = 1;
    imageCreateInfo.usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageCreateInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    images.push_back(Image(device, imageCreateInfo));

    uint32_t whitePixel = 0xFFFFFFFF;

    images[0].writeToImage(&whitePixel, sizeof(uint32_t));

    addNode(-1, std::make_shared<CameraV2>());

    createPipelines();
    updateCameraBuffer();
    updateMaterialBuffer();
    createDescriptorSets();
    updateDescriptorSets();
}

Scene2::~Scene2() {}

void Scene2::render(CommandBuffer &cmd) {
    std::vector<DescriptorSet *> descriptorSets = {&sceneDescriptorSet};

    meshPipeline.bindPipeline(cmd);
    DescriptorSet::bindDescriptorSet(cmd, descriptorSets, meshPipeline.getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS);

    for (auto &renderable : renderables) {
        renderable->render(cmd, meshPipeline, meshes, basicMeshes);
    }
}

void Scene2::renderSkybox(CommandBuffer &cmd) {
    skyboxPipeline.bindPipeline(cmd);
    std::vector<DescriptorSet *> descriptorSets = {&sceneDescriptorSet};
    DescriptorSet::bindDescriptorSet(cmd, descriptorSets, skyboxPipeline.getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS);

    basicMeshes[Cube].bindMesh(cmd);
    vkCmdDrawIndexed(cmd, basicMeshes[Cube].nbIndicies(), 1, 0, 0, 0);
}

uint32_t Scene2::getNewID() {
    if (!freeIDs.empty()) {
        uint32_t id = freeIDs.back();
        freeIDs.pop_back();
        return id;
    }
    return nextID++;

    Device *device = nullptr;
};

uint32_t Scene2::addNode(uint32_t Parent_id, std::shared_ptr<Node> node) {
    if (dynamic_cast<IRenderable *>(node.get())) {
        renderables.push_back(std::dynamic_pointer_cast<IRenderable>(node));
    }
    if (dynamic_cast<CameraV2 *>(node.get())) {
        cameras.push_back(std::dynamic_pointer_cast<CameraV2>(node));
        if (cameras.size() == 1) {
            mainCamera = cameras[0];
        }
    }

    if (dynamic_cast<IAnimatic *>(node.get())) {
        animaticObjs.push_back(std::dynamic_pointer_cast<IAnimatic>(node));
    }

    if (dynamic_cast<ICollider *>(node.get())) {
        collisionObjects.push_back(std::dynamic_pointer_cast<ICollider>(node));
    }

    if (dynamic_cast<IInputController *>(node.get())) {
        controlledObjects.push_back(std::dynamic_pointer_cast<IInputController>(node));
    }

    if (Parent_id == -1) {
        this->addChild(node);
    } else {
        objects[Parent_id]->addChild(node);
    }
    node->setId(getNewID());
    objects[node->getId()] = node;
    return node->getId();
}

void Scene2::removeNode(uint32_t id) {}

uint32_t Scene2::addMaterial(Material material) {
    materials.push_back(material);
    return materials.size() - 1;
}

void Scene2::addMesh(Mesh mesh) { meshes.push_back(mesh); }

void Scene2::addObjectFileData(ObjectFileData &data) {
    for (auto &mesh : data.meshes) {
        mesh.applyMaterialOffset(materials.size());
    }
    for (auto &mesh : data.meshes) {
        addMesh(mesh);
    }
    for (auto &material : data.materials) {
        material.applyTextureOffset(images.size());
    }
    for (auto &material : data.materials) {
        addMaterial(material);
    }
    for (auto &image : data.images) {
        addImage(image);
    }
}

uint32_t Scene2::addImage(Image image) {
    images.push_back(image);
    return images.size() - 1;
}

void Scene2::updateSim(float dt, float t, uint32_t tick) {
    for (auto &animaticObj : animaticObjs) {
        animaticObj->simulation(glm::vec3(0, -9.81, 0), 0.995, tick, dt, t, collisionObjects);
    }
}

void Scene2::updateFromInput(Window *window, float dt) {
    for (auto &controlledObject : controlledObjects) {
        controlledObject->updateFromInput(window, dt);
    }
}

void Scene2::updateCameraBuffer() {
    if (cameraBuffer.getInstancesCount() == 0) {
        cameraBuffer = Buffer(device, sizeof(Ubo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    }
    Ubo ubo;
    ubo.projection = mainCamera->getProjectionMatrix();
    ubo.view = mainCamera->getViewMatrix();
    ubo.invView = glm::inverse(mainCamera->getViewMatrix());
    cameraBuffer.writeToBuffer(&ubo, sizeof(Ubo));
}

void Scene2::updateMaterialBuffer() {
    if (materialBuffer.getInstancesCount() < materials.size() || materials.size() == 0) {
        if (materials.size() == 0) {
            Material mat;
            mat.color = glm::vec4(0.8, 0, 0, 1);
            mat.metallic = 0.8f;
            mat.roughness = 0.9f;
            materials.push_back(mat);
        }
        materialBuffer =
            Buffer(device, sizeof(MaterialGPU), materials.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    }
    std::vector<MaterialGPU> materialsGPU;

    for (auto &material : materials) {
        materialsGPU.push_back(
            {material.color, material.metallic, material.roughness, material.albedo_tex_id, material.metallic_roughness_tex_id,
             material.normal_tex_id});

        if (material.albedo_tex_id == -1) {
            materialsGPU.back().albedo_tex_id = 0;
        }
        if (material.metallic_roughness_tex_id == -1) {
            materialsGPU.back().metallic_roughness_tex_id = 0;
        }
    }
    materialBuffer.writeToBuffer(materialsGPU.data(), sizeof(MaterialGPU) * materialsGPU.size(), 0);
}

void Scene2::createDescriptorSets() { sceneDescriptorSet = DescriptorSet(device, meshPipeline.getDescriptorSetLayout(0)); }

void Scene2::updateDescriptorSets() {
    sceneDescriptorSet.writeBufferDescriptor(0, cameraBuffer);

    sceneDescriptorSet.writeBufferDescriptor(1, materialBuffer);

    if (images.size() == 0) {
        ImageCreateInfo imageCreateInfo;
        imageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageCreateInfo.width = 1;
        imageCreateInfo.height = 1;
        imageCreateInfo.usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageCreateInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        images.push_back(Image(device, imageCreateInfo));
    }
    std::vector<VkDescriptorImageInfo> imageInfos;
    for (auto &texture : images) {
        imageInfos.push_back(texture.getDescriptorImageInfo(samplerType::LINEAR));
    }
    sceneDescriptorSet.writeImagesDescriptor(2, imageInfos);
    sceneDescriptorSet.writeImageDescriptor(3, skyboxImage.getDescriptorImageInfo(samplerType::LINEAR));
}

void Scene2::createPipelines() {
    GraphicPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.fragmentShaderFile = "hello_scene.frag";
    pipelineCreateInfo.vexterShaderFile = "hello_scene.vert";
    meshPipeline = GraphicPipeline(device, pipelineCreateInfo);

    pipelineCreateInfo.fragmentShaderFile = "bgV2.frag";
    pipelineCreateInfo.vexterShaderFile = "bgV2.vert";
    skyboxPipeline = GraphicPipeline(device, pipelineCreateInfo);
}

}  // namespace TTe