
#include "scene.hpp"

#include <glm/fwd.hpp>
#include <vector>

#include "GPU_data/image.hpp"
#include "descriptor/descriptorSet.hpp"
#include "scene/camera.hpp"
#include "scene/object.hpp"
#include "scene/objects/simulateObj.hpp"
#include "shader/pipeline/graphic_pipeline.hpp"
#include "struct.hpp"
#include "utils.hpp"

namespace TTe {

Scene::Scene(Device *device) {
    this->device = device;
    camera = Camera();
    sphere = Mesh(device, BasicShape::Sphere, 2);

    // createDescriptorSets();
}

void Scene::addBVH(BVH &bvh) {
    int objectOffset = this->objects.size();
    std::vector<Object> skeleton;
    for (int i = 0; i < bvh.getNumberOfJoint(); i++) {
        Object o;
        auto &jointMat = bvh.getJoint(i);

        o.parentID = jointMat.getParentId();
        o.scale = glm::vec3(5.);
        jointMat.getOffset(o.translation.x, o.translation.y, o.translation.z);
        skeleton.push_back(o);
    }

    for (auto &node : skeleton) {
        auto actualNode = node;
        node.worldMatrix = node.getTranslationRotationMatrix();
        node.worldNormalMatrix = node.getNormalTranslationRotationMatrix();
        while (actualNode.parentID != -1) {
            actualNode = skeleton[actualNode.parentID];
            node.worldMatrix = actualNode.getTranslationRotationMatrix() * node.worldMatrix;
            node.worldNormalMatrix = actualNode.getNormalTranslationRotationMatrix() * node.worldNormalMatrix;
        }
    }

    this->animaticOBJ.push_back({skeleton, bvh});
}


void Scene::addMssObject(ObjetSimuleMSS &mss) {
    mss.initObjetSimule();
    mss.initMeshObjet();
    mssObjects.push_back(mss);
}

void Scene::render(CommandBuffer &cmd) {
    std::vector<DescriptorSet *> descriptorSets = {&backgroundDescriptorSet};
    backgroundPipeline.bindPipeline(cmd);
    DescriptorSet::bindDescriptorSet(cmd, descriptorSets, pipeline.getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS);


   
    vkCmdPushConstants(
        cmd, backgroundPipeline.getPipelineLayout(), backgroundPipeline.getPushConstantStage(), 0, sizeof(glm::ivec2), &camera.extent);
    vkCmdDraw(cmd, 3, 1, 0, 0);

    descriptorSets = {&sceneDescriptorSet};

    // bind pipeline
    pipeline.bindPipeline(cmd);
    // bind descriptor set
    DescriptorSet::bindDescriptorSet(cmd, descriptorSets, pipeline.getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS);
    for (auto &object : objects) {
        // bind mesh
        vkCmdBindIndexBuffer(cmd, meshes[object.meshId].getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
        VkBuffer vbuffers[] = {meshes[object.meshId].getVertexBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1, vbuffers, offsets);

        // set push constant
        glm::mat4 model = object.mat4();
        vkCmdPushConstants(cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), 0, sizeof(glm::mat4), &model);
        glm::mat4 normalMatrix = object.normalMatrix();
        vkCmdPushConstants(
            cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), sizeof(glm::mat4), sizeof(glm::mat4), &normalMatrix);
        // draw

        vkCmdDrawIndexed(cmd, meshes[object.meshId].nbIndicies(), 1, 0, 0, 0);
    }
    vkCmdBindIndexBuffer(cmd, sphere.getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
    VkBuffer vbuffers[] = {sphere.getVertexBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, vbuffers, offsets);

    for (auto animeObj : animaticOBJ) {
        auto &skeleton = animeObj.first;
        auto &bvh = animeObj.second;
        for (auto &node : skeleton) {
            // bind mesh
            // set push constant
            glm::mat4 model = node.worldMatrix * node.getScaledMatrix();
            vkCmdPushConstants(cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), 0, sizeof(glm::mat4), &model);
            glm::mat4 normalMatrix = node.worldNormalMatrix;
            vkCmdPushConstants(
                cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), sizeof(glm::mat4), sizeof(glm::mat4), &normalMatrix);
            // draw
            vkCmdDrawIndexed(cmd, sphere.nbIndicies(), 1, 0, 0, 0);
        }
    }

    for(auto &mss : mssObjects){
        mss.mesh.bindMesh(cmd);

        glm::mat4 model = mss.mat4();
        vkCmdPushConstants(cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), 0, sizeof(glm::mat4), &model);
        glm::mat4 normalMatrix = mss.normalMatrix();
        vkCmdPushConstants(
            cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), sizeof(glm::mat4), sizeof(glm::mat4), &normalMatrix);

        vkCmdDrawIndexed(cmd, mss.mesh.nbIndicies(), 1, 0, 0, 0);

    }
}

void Scene::updateSimu(float dt, float t){
    for(auto &obj : mssObjects){
        obj.Simulation(gravity, _visco, 0, dt, t, collisionObjects);
    }
}

void Scene::updateBuffer() {
    Ubo ubo;
    ubo.projection = camera.getProjectionMatrix();
    ubo.view = camera.getViewMatrix();
    ubo.invView = glm::inverse(camera.getViewMatrix());

    CameraBuffer.writeToBuffer(&ubo, sizeof(Ubo));
    // ObjectBuffer.writeToBuffer(objects.data(), sizeof(Object) *objects.size(), 0);
    // MaterialBuffer.writeToBuffer(materials.data(), sizeof(Material)*materials.size(), 0);
}

void Scene::updateCameraBuffer() {
    Ubo ubo;
    ubo.projection = camera.getProjectionMatrix();
    ubo.view = camera.getViewMatrix();
    ubo.invView = glm::inverse(camera.getViewMatrix());
    CameraBuffer.writeToBuffer(&ubo, sizeof(Ubo));
}

void Scene::createDescriptorSets() {
    CameraBuffer = Buffer(device, sizeof(Ubo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    MaterialBuffer = Buffer(device, sizeof(MaterialGPU), materials.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    std::vector<MaterialGPU> materialsGPU;

    for (auto &material : materials) {
        materialsGPU.push_back({material.color, material.metallic, material.roughness, material.albedo_tex_id, material.metallic_roughness_tex_id, material.normal_tex_id});
    }

    MaterialBuffer.writeToBuffer(materialsGPU.data(), sizeof(materialsGPU) * materialsGPU.size(), 0);
    GraphicPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.fragmentShaderFile = "hello_scene.frag";
    pipelineCreateInfo.vexterShaderFile = "hello_scene.vert";
    pipeline = GraphicPipeline(device, pipelineCreateInfo);

    pipelineCreateInfo.fragmentShaderFile = "bg.frag";
    pipelineCreateInfo.vexterShaderFile = "bg.vert";
    backgroundPipeline = GraphicPipeline(device, pipelineCreateInfo);

    ImageCreateInfo cubeTextureCreateInfo;
    cubeTextureCreateInfo.filename.push_back("../data/textures/posx.jpg");
    cubeTextureCreateInfo.filename.push_back("../data/textures/negx.jpg");
    cubeTextureCreateInfo.filename.push_back("../data/textures/posy.jpg");
    cubeTextureCreateInfo.filename.push_back("../data/textures/negy.jpg");
    cubeTextureCreateInfo.filename.push_back("../data/textures/posz.jpg");
    cubeTextureCreateInfo.filename.push_back("../data/textures/negz.jpg");
    cubeTextureCreateInfo.usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    cubeTextureCreateInfo.isCubeTexture = true;
    cubeTexture = Image(device, cubeTextureCreateInfo);

    backgroundDescriptorSet = DescriptorSet(device, backgroundPipeline.getDescriptorSetLayout(0));
    backgroundDescriptorSet.writeBufferDescriptor(0, CameraBuffer);
    backgroundDescriptorSet.writeImageDescriptor(1, cubeTexture.getDescriptorImageInfo(samplerType::LINEAR));

    

    sceneDescriptorSet = DescriptorSet(device, pipeline.getDescriptorSetLayout(0));

    sceneDescriptorSet.writeBufferDescriptor(0, CameraBuffer);
    sceneDescriptorSet.writeBufferDescriptor(1, MaterialBuffer);

    if (textures.size() == 0) {
        ImageCreateInfo imageCreateInfo;
        imageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageCreateInfo.width = 1;
        imageCreateInfo.height = 1;
        imageCreateInfo.usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageCreateInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        textures.push_back(Image(device, imageCreateInfo));
    }
    std::vector<VkDescriptorImageInfo> imageInfos;
    for (auto &texture : textures) {
        imageInfos.push_back(texture.getDescriptorImageInfo(samplerType::LINEAR));
    }
    sceneDescriptorSet.writeImagesDescriptor(2, imageInfos);
    sceneDescriptorSet.writeImageDescriptor(3, cubeTexture.getDescriptorImageInfo(samplerType::LINEAR));
}

}  // namespace TTe