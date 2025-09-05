
#include "scene.hpp"

#include <cstdint>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/matrix.hpp>
#include <vector>

#include "GPU_data/buffer.hpp"
#include "GPU_data/image.hpp"
#include "commandBuffer/command_buffer.hpp"
#include "descriptor/descriptorSet.hpp"
#include "device.hpp"
#include "sceneV2/IIndirectRenderable.hpp"
#include "sceneV2/IRenderable.hpp"
#include "sceneV2/Icollider.hpp"
#include "sceneV2/animatic/skeletonObj.hpp"
#include "sceneV2/cameraV2.hpp"
#include "sceneV2/mesh.hpp"
#include "sceneV2/render_data.hpp"
#include "shader/pipeline/compute_pipeline.hpp"
#include "struct.hpp"

namespace TTe {

Scene::Scene(Device *device) : device(device) {
    createPipelines();
    createDescriptorSets();
}

Scene::~Scene() {}

void Scene::initSceneData(DynamicRenderPass *defferedRenderpass, DynamicRenderPass *shadingRenderPass, std::filesystem::path skyboxPath) {
    
    this->defferedRenderpass = defferedRenderpass;
    this->shadingRenderPass = shadingRenderPass;
    ImageCreateInfo skyboxImageCreateInfo;
    skyboxImageCreateInfo.filename.push_back(skyboxPath / "posx.jpg");
    skyboxImageCreateInfo.filename.push_back(skyboxPath / "negx.jpg");
    skyboxImageCreateInfo.filename.push_back(skyboxPath / "posy.jpg");
    skyboxImageCreateInfo.filename.push_back(skyboxPath / "negy.jpg");
    skyboxImageCreateInfo.filename.push_back(skyboxPath / "posz.jpg");
    skyboxImageCreateInfo.filename.push_back(skyboxPath / "negz.jpg");
    skyboxImageCreateInfo.usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    skyboxImageCreateInfo.isCubeTexture = true;
    skyboxImageCreateInfo.enableMipMap = true;
    skyboxImageCreateInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    skyboxImage = Image(device, skyboxImageCreateInfo);

    Mesh cubeMesh(device, Mesh::BasicShape::Cube, 1);
    addStaticMesh(cubeMesh);
    basicMeshes[Mesh::BasicShape::Cube] = &meshes.at(nb_meshes - 1);

    Mesh sphereMesh(device, Mesh::BasicShape::Sphere, 1);
    addStaticMesh(sphereMesh);
    basicMeshes[Mesh::BasicShape::Sphere] = &meshes.at(nb_meshes - 1);

    // Mesh planeMesh(device, Mesh::BasicShape::Plane, 1);
    // addStaticMesh(planeMesh);
    // basicMeshes[Mesh::BasicShape::Plane] = &meshes.back();

    vkDeviceWaitIdle(*device);

    if (cameras.size() == 0) {
        addNode(-1, std::make_shared<CameraV2>());
    }
    createDrawIndirectBuffers();
    updateCameraBuffer();
    updateObjectBuffer();
    updateMaterialBuffer();
    updateDescriptorSets();
    updateRenderPassDescriptorSets();
}

void Scene::renderDeffered(CommandBuffer &cmd, RenderData &renderData) {
    renderData.basicMeshes = basicMeshes;
    skyboxPipeline.bindPipeline(cmd);
    std::vector<DescriptorSet *> descriptorSets = {&sceneDescriptorSet};

    DescriptorSet::bindDescriptorSet(cmd, descriptorSets, skyboxPipeline.getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS);

    basicMeshes[Mesh::Cube]->bindMesh(cmd);
    renderData.renderPass->setDepthAndStencil(cmd, false);

    PushConstantStruct tp{
        objectBuffer.getBufferDeviceAddress(), materialBuffer.getBufferDeviceAddress(),
        cameraBuffer[renderData.frameIndex].getBufferDeviceAddress(), 0};
    renderData.pushConstant = tp;
    // // set push_constant for cam_id
    vkCmdPushConstants(cmd, skyboxPipeline.getPipelineLayout(), skyboxPipeline.getPushConstantStage(), 0, sizeof(PushConstantStruct), &tp);

    vkCmdSetCullMode(cmd, VK_CULL_MODE_NONE);
    vkCmdDrawIndexed(
        cmd, basicMeshes[Mesh::Cube]->nbIndicies(), 1, basicMeshes[Mesh::Cube]->getFirstIndex(), basicMeshes[Mesh::Cube]->getFirstVertex(),
        0);
     vkCmdSetCullMode(cmd, VK_CULL_MODE_BACK_BIT);
    renderData.renderPass->setDepthAndStencil(cmd, true);
    ;

    meshPipeline.bindPipeline(cmd);
    DescriptorSet::bindDescriptorSet(cmd, descriptorSets, meshPipeline.getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS);

    // renderData.basicMeshes = &basicMeshes;
    // renderData.meshes = &meshes;
    // renderData.default_pipeline = &meshPipeline;
    // renderData.binded_pipeline = &meshPipeline;
    // renderData.binded_mesh = &basicMeshes[Mesh::Cube];
    // renderData.descriptorSets.push(sceneDescriptorSet);

    for (auto &renderable : indirectRenderables) {
        renderable->render(cmd, renderData);
    }

    drawIndirectBuffers[renderData.frameIndex].writeToBuffer(
        renderData.drawCommands.data(), renderData.drawCommands.size() * sizeof(VkDrawIndexedIndirectCommand), 0);

    uint32_t drawcount = renderData.drawCommands.size();
    countIndirectBuffers[renderData.frameIndex].writeToBuffer(&drawcount, sizeof(uint32_t), 0);

    vkCmdDrawIndexedIndirectCount(
        cmd, drawIndirectBuffers[renderData.frameIndex], 0, countIndirectBuffers[renderData.frameIndex], 0, drawcount,
        sizeof(VkDrawIndexedIndirectCommand));

    for (auto &renderable : indirectRenderables) {
        renderable->render(cmd, renderData);
    }

    int x = 0;
}

void Scene::renderShading(CommandBuffer &cmd, RenderData &renderData) {
    renderData.basicMeshes = basicMeshes;
    defferedRenderpass->transitionAttachment(renderData.swapchainIndex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmd);
    shadingRenderPass->transitionColorAttachment(renderData.swapchainIndex, VK_IMAGE_LAYOUT_GENERAL, cmd);

    shadingPipeline.bindPipeline(cmd);

    std::vector<DescriptorSet *> descriptorSets = {&deferreDescriptorSet[renderData.swapchainIndex]};
    DescriptorSet::bindDescriptorSet(cmd, descriptorSets, shadingPipeline.getPipelineLayout(), VK_PIPELINE_BIND_POINT_COMPUTE);
    PushConstantStruct tp{
        objectBuffer.getBufferDeviceAddress(), materialBuffer.getBufferDeviceAddress(),
        cameraBuffer[renderData.frameIndex].getBufferDeviceAddress(), 0};
    renderData.pushConstant = tp;
    // // set push_constant for cam_id
    vkCmdPushConstants(
        cmd, shadingPipeline.getPipelineLayout(), shadingPipeline.getPushConstantStage(), 0, sizeof(PushConstantStruct), &tp);

    shadingPipeline.dispatch(cmd, renderData.renderPass->getFrameSize().width, renderData.renderPass->getFrameSize().height);

    defferedRenderpass->transitionColorAttachment(renderData.swapchainIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, cmd);
    defferedRenderpass->transitionDepthAttachment(renderData.swapchainIndex, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, cmd);
    shadingRenderPass->transitionColorAttachment(renderData.swapchainIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, cmd);
    
    shadingRenderPass->setClearEnable(false);
    shadingRenderPass->beginRenderPass(cmd, renderData.swapchainIndex);
    for (auto &renderable : renderables) {
        renderable->render(cmd, renderData);
    }
    shadingRenderPass->endRenderPass(cmd);
    shadingRenderPass->setClearEnable(true);
}

uint32_t Scene::getNewID() {
    if (!freeIDs.empty()) {
        uint32_t id = freeIDs.back();
        freeIDs.pop_back();
        return id;
    }
    return nextID++;

    Device *device = nullptr;
};

uint32_t Scene::addNode(uint32_t Parent_id, std::shared_ptr<Node> node) {
    if (dynamic_cast<IRenderable *>(node.get())) {
        renderables.push_back(std::dynamic_pointer_cast<IRenderable>(node));
    }
    if (dynamic_cast<IIndirectRenderable *>(node.get())) {
        indirectRenderables.push_back(std::dynamic_pointer_cast<IIndirectRenderable>(node));
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

void Scene::removeNode(uint32_t id) {}

uint32_t Scene::addMaterial(Material material) {
    materials.push_back(material);
    return materials.size() - 1;
}

void Scene::addStaticMesh(Mesh &mesh) {
    bool needGPUUpload = false;
    if (mesh.indicies.size() + firstIndexAvailable > indexBuffer.getInstancesCount()) {
        needGPUUpload = true;
        indexBuffer = Buffer(
            device, sizeof(uint32_t), (mesh.indicies.size() + indexBuffer.getInstancesCount()) * 1.5,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Buffer::BufferType::GPU_ONLY);
    }

    if (mesh.verticies.size() + firstVertexAvailable > vertexBuffer.getInstancesCount()) {
        needGPUUpload = true;
        vertexBuffer = Buffer(
            device, sizeof(Vertex), (mesh.verticies.size() + vertexBuffer.getInstancesCount()) * 1.5,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Buffer::BufferType::GPU_ONLY);
    }

    if (needGPUUpload) {
        for (auto &scene_mesh : meshes) {
            if (scene_mesh.second.indicies.size() == 0 || scene_mesh.second.verticies.size() == 0) continue;
            scene_mesh.second.setVertexAndIndexBuffer(indexBuffer, vertexBuffer);
            scene_mesh.second.uploadToGPU();
        }
    }

    mesh.setVertexAndIndexBuffer(firstIndexAvailable, firstVertexAvailable, indexBuffer, vertexBuffer);

    mesh.uploadToGPU();
    firstIndexAvailable += mesh.indicies.size();
    firstVertexAvailable += mesh.verticies.size();

    meshes[nb_meshes] = mesh;
    nb_meshes++;
}

uint32_t Scene::addImage(Image &image) {
    images.push_back(image);
    return images.size() - 1;
}

void Scene::createDrawIndirectBuffers() {
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        drawIndirectBuffers[i] = Buffer(
            device, sizeof(VkDrawIndexedIndirectCommand), 10000, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            Buffer::BufferType::DYNAMIC);

        countIndirectBuffers[i] = Buffer(
            device, sizeof(uint32_t), 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
            Buffer::BufferType::DYNAMIC);
    }
}

void Scene::updateSim(float dt, float t, uint32_t tick) {
    for (auto &animaticObj : animaticObjs) {
        animaticObj->simulation(glm::vec3(0, -9.81, 0), 0.995, tick, dt, t, collisionObjects);
    }
}

void Scene::updateFromInput(Window *window, float dt) {
    for (auto &controlledObject : controlledObjects) {
        controlledObject->updateFromInput(window, dt);
    }
}

void Scene::updateCameraBuffer(uint32_t frameIndex) {
    if (cameras.size() == 0) return;

    if (cameraBuffer[frameIndex].getInstancesCount() < cameras.size()) {
        cameraBuffer[frameIndex] = Buffer(device, sizeof(Ubo), 20, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    }
    std::vector<Ubo> ubos;

    for (auto cam : cameras) {
        Ubo ubo;
        ubo.projection = mainCamera->getProjectionMatrix();
        ubo.view = mainCamera->getViewMatrix();
        ubo.invView = glm::inverse(mainCamera->getViewMatrix());
        ubos.push_back(ubo);
    }

    cameraBuffer[frameIndex].writeToBuffer(ubos.data(), sizeof(Ubo) * ubos.size());
}

struct Object_data {
    glm::mat4 world_matrix;
    glm::mat4 normal_matrix;
    glm::vec3 padding{0};
    uint32_t material_offset = 0;
};

void Scene::updateObjectBuffer() {
    if (objectBuffer.getInstancesCount() < objects.size()) {
        objectBuffer =
            Buffer(device, sizeof(Object_data), objects.size() * 2, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, Buffer::BufferType::DYNAMIC);
        for (auto &obj : objects) {
            obj.second->uploadedToGPU = false;
        }
    }
    for (auto &obj : objects) {
        if (!obj.second->uploadedToGPU) {
            Object_data data;
            data.world_matrix = obj.second->wMatrix();
            data.normal_matrix = obj.second->wNormalMatrix();
            data.material_offset = 0;
            objectBuffer.writeToBuffer(&data, sizeof(Object_data), sizeof(Object_data) * obj.second->getId());
            obj.second->uploadedToGPU = true;
        }
    }
}

void Scene::updateMaterialBuffer() {
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
    }
    materialBuffer.writeToBuffer(materialsGPU.data(), sizeof(MaterialGPU) * materialsGPU.size(), 0);
}

void Scene::createDescriptorSets() { sceneDescriptorSet = DescriptorSet(device, meshPipeline.getDescriptorSetLayout(0)); }

void Scene::updateDescriptorSets() {
    if (images.size() == 0) {
        glm::vec4 *defaultPixel = new glm::vec4(1, 1, 1, 1);
        // create a default texture
        ImageCreateInfo defaultImageCreateInfo;
        defaultImageCreateInfo.width = 1;
        defaultImageCreateInfo.height = 1;
        defaultImageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
        defaultImageCreateInfo.usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
        defaultImageCreateInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        defaultImageCreateInfo.datas.push_back(defaultPixel);
        Image defaultImage = Image(device, defaultImageCreateInfo);
        images.push_back(defaultImage);
    }
    std::vector<VkDescriptorImageInfo> imageInfos;
    for (auto &texture : images) {
        imageInfos.push_back(texture.getDescriptorImageInfo(samplerType::LINEAR));
    }
    sceneDescriptorSet.writeImagesDescriptor(0, imageInfos);
    sceneDescriptorSet.writeImageDescriptor(1, skyboxImage.getDescriptorImageInfo(samplerType::LINEAR));
}

void Scene::updateRenderPassDescriptorSets() {
    if (deferreDescriptorSet.size() == 0) {
        deferreDescriptorSet.reserve(defferedRenderpass->getimageAttachement().size());
        for (int i = 0; i < defferedRenderpass->getimageAttachement().size(); i++) {
            deferreDescriptorSet.push_back(DescriptorSet(device, shadingPipeline.getDescriptorsSetLayout()[0]));
        }

        testBuffer = Buffer(device, sizeof(uint32_t), 921600, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, Buffer::BufferType::GPU_ONLY);
    }
    for (int i = 0; i < defferedRenderpass->getimageAttachement().size(); i++) {
        // color metal
        VkDescriptorImageInfo imageInfo = defferedRenderpass->getimageAttachement()[i][0].getDescriptorImageInfo(samplerType::LINEAR);
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        deferreDescriptorSet[i].writeImageDescriptor(0, imageInfo);

        // normal roughness
        imageInfo = defferedRenderpass->getimageAttachement()[i][1].getDescriptorImageInfo(samplerType::LINEAR);
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        deferreDescriptorSet[i].writeImageDescriptor(1, imageInfo);

        // depth
        imageInfo = defferedRenderpass->getDepthAndStencilAttachement()[i].getDescriptorImageInfo(samplerType::LINEAR);
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        deferreDescriptorSet[i].writeImageDescriptor(2, imageInfo);

        // swapchain
        imageInfo = shadingRenderPass->getimageAttachement()[i][0].getDescriptorImageInfo(samplerType::LINEAR);
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        deferreDescriptorSet[i].writeImageDescriptor(3, imageInfo);
    }
}

void Scene::createPipelines() {
    GraphicPipelineCreateInfo pipelineCreateInfo;
#ifdef DEFAULT_APP_PATH
    pipelineCreateInfo.fragmentShaderFile = "shaders/deffered.frag";
    pipelineCreateInfo.vexterShaderFile = "shaders/deffered.vert";
#else
    pipelineCreateInfo.fragmentShaderFile = "TTengine-2/shaders/deffered.frag";
    pipelineCreateInfo.vexterShaderFile = "TTengine-2/shaders/deffered.vert";
#endif
    meshPipeline = GraphicPipeline(device, pipelineCreateInfo);

#ifdef DEFAULT_APP_PATH
    pipelineCreateInfo.fragmentShaderFile = "shaders/bgV2.frag";
    pipelineCreateInfo.vexterShaderFile = "shaders/bgV2.vert";
#else
    pipelineCreateInfo.fragmentShaderFile = "TTengine-2/shaders/bgV2.frag";
    pipelineCreateInfo.vexterShaderFile = "TTengine-2/shaders/bgV2.vert";
#endif
    skyboxPipeline = GraphicPipeline(device, pipelineCreateInfo);

#ifdef DEFAULT_APP_PATH
    shadingPipeline = ComputePipeline(device, "shaders/shading.comp");
#else
    shadingPipeline = ComputePipeline(device, "TTengine-2/shaders/shading.comp");
#endif
}

}  // namespace TTe