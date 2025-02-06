
#include "scene.hpp"
#include <glm/fwd.hpp>
#include <iostream>
#include <vector>
#include "GPU_data/image.hpp"
#include "descriptor/descriptorSet.hpp"
#include "shader/pipeline/graphic_pipeline.hpp"
#include "utils.hpp"

namespace TTe {

Scene::Scene(Device *device) {
    this->device = device;
    camera = Camera();
    // createDescriptorSets();
}

void Scene::render(CommandBuffer &cmd) {

    std::vector<DescriptorSet*> descriptorSets = {&sceneDescriptorSet}; 
    backgroundPipeline.bindPipeline(cmd);
    DescriptorSet::bindDescriptorSet(cmd, descriptorSets, pipeline.getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS);
    vkCmdDraw(cmd, 3, 1, 0, 0);

    //bind pipeline
    pipeline.bindPipeline(cmd);
    //bind descriptor set
    DescriptorSet::bindDescriptorSet(cmd, descriptorSets, pipeline.getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS);
    for(auto & object : objects){
        //bind mesh
        vkCmdBindIndexBuffer(cmd, meshes[object.meshId].getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
        VkBuffer vbuffers[] = {meshes[object.meshId].getVertexBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1,vbuffers, offsets);

        // set push constant

        glm::mat4 model = object.mat4();
        vkCmdPushConstants(cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), 0, sizeof(glm::mat4), &model);
        glm::mat4 normalMatrix = object.normalMatrix();
        vkCmdPushConstants(cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), sizeof(glm::mat4), sizeof(glm::mat4), &normalMatrix);
        // draw

        vkCmdDrawIndexed(cmd, meshes[object.meshId].nbIndicies(), 1, 0, 0, 0);

    }

}

void Scene::updateBuffer() {
    Ubo ubo;
    ubo.projection = camera.getProjectionMatrix(16.0/9.0);
    ubo.view = camera.getViewMatrix();
    ubo.invView = glm::inverse(camera.getViewMatrix());
    
    CameraBuffer.writeToBuffer(&ubo, sizeof(Ubo));
    // ObjectBuffer.writeToBuffer(objects.data(), sizeof(Object) *objects.size(), 0);
    // MaterialBuffer.writeToBuffer(materials.data(), sizeof(Material)*materials.size(), 0);
}

void Scene::updateCameraBuffer() {
    Ubo ubo;
    ubo.projection = camera.getProjectionMatrix(16.0/9.0);
    ubo.view = camera.getViewMatrix();
    ubo.invView = glm::inverse(camera.getViewMatrix());
    // show camera pos in console
    // std::cout << "camera pos : " << camera.translation.x << " " << camera.translation.y << " " << camera.translation.z << std::endl;
    //clear console
    // std::cout << "\033[2J\033[1;1H";
    // std::cout << "camera rot : " << camera.rotation.x << " " << camera.rotation.y << " " << camera.rotation.z << std::endl;
    
    CameraBuffer.writeToBuffer(&ubo, sizeof(Ubo));
}

void Scene::createDescriptorSets() {
    CameraBuffer = Buffer(device, sizeof(Ubo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    ObjectBuffer = Buffer(device, sizeof(Object), objects.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    MaterialBuffer = Buffer(device, sizeof(Material), materials.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Buffer::BufferType::DYNAMIC);

    GraphicPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.fragmentShaderFile = "hello_scene.frag";
    pipelineCreateInfo.vexterShaderFile = "hello_scene.vert";
    pipeline = GraphicPipeline(device, pipelineCreateInfo);

    pipelineCreateInfo.fragmentShaderFile = "bg.frag";
    pipelineCreateInfo.vexterShaderFile = "bg.vert";
    backgroundPipeline = GraphicPipeline(device, pipelineCreateInfo);

    sceneDescriptorSet = DescriptorSet(device, pipeline.getDescriptorSetLayout(0));

    sceneDescriptorSet.writeBufferDescriptor(0,CameraBuffer);
    sceneDescriptorSet.writeBufferDescriptor(1, MaterialBuffer);
    std::vector<VkDescriptorImageInfo> imageInfos;
    for (auto &texture : textures) {
        imageInfos.push_back(texture.getDescriptorImageInfo(samplerType::LINEAR));
        //TODO: add sampler
    }
    sceneDescriptorSet.writeImagesDescriptor(2, imageInfos);
    
}

}