
#include "scene.hpp"
#include <glm/fwd.hpp>
#include <vector>
#include "descriptor/descriptorSet.hpp"
#include "shader/pipeline/graphic_pipeline.hpp"

namespace TTe {

Scene::Scene(Device *device) {
    this->device = device;
    camera = Camera();
    // createDescriptorSets();
}

void Scene::render(CommandBuffer &cmd) {
    //bind pipeline
    pipeline.bindPipeline(cmd);
    //bind descriptor set
    std::vector<DescriptorSet*> descriptorSets = {&sceneDescriptorSet}; 
    DescriptorSet::bindDescriptorSet(cmd, descriptorSets, pipeline.getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS);
    for(auto & object : objects){
        //bind mesh
        vkCmdBindIndexBuffer(cmd, meshes[object.meshId].getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
        VkBuffer vbuffers[] = {meshes[object.meshId].getVertexBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1,vbuffers, offsets);

        // set push constant

        glm::mat4 model = object.mat4();
        vkCmdPushConstants(cmd, pipeline.getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model);
        glm::mat4 normalMatrix = object.normalMatrix();
        vkCmdPushConstants(cmd, pipeline.getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), sizeof(glm::mat4), &normalMatrix);
        // draw

        vkCmdDrawIndexed(cmd, meshes[object.meshId].nbIndicies(), 1, 0, 0, 0);

    }

}

void Scene::updateBuffer() {
    CameraBuffer.writeToBuffer(&camera);
    ObjectBuffer.writeToBuffer(objects.data(), sizeof(Object) *objects.size(), 0);
    MaterialBuffer.writeToBuffer(materials.data(), sizeof(Material)*materials.size(), 0);
}

void Scene::createDescriptorSets() {
    CameraBuffer = Buffer(device, sizeof(Camera), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    ObjectBuffer = Buffer(device, sizeof(Object), objects.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    MaterialBuffer = Buffer(device, sizeof(Material), materials.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Buffer::BufferType::DYNAMIC);

    GraphicPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.fragmentShaderFile = "hello_scene.frag";
    pipelineCreateInfo.vexterShaderFile = "hello_scene.vert";
    pipeline = GraphicPipeline(device, pipelineCreateInfo);

    sceneDescriptorSet = DescriptorSet(device, pipeline.getDescriptorSetLayout(0));

    sceneDescriptorSet.writeBufferDescriptor(0,CameraBuffer);
    sceneDescriptorSet.writeBufferDescriptor(1, MaterialBuffer);
    std::vector<VkDescriptorImageInfo> imageInfos;
    for (auto &texture : textures) {
        imageInfos.push_back(texture.getDescriptorImageInfo());
        //TODO: add sampler
    }
    sceneDescriptorSet.writeImagesDescriptor(2, imageInfos);
    
}

}