
#include "portalObj.hpp"

#include "descriptor/descriptorSet.hpp"
#include "device.hpp"
#include "sceneV2/mesh.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace TTe {

std::array<DescriptorSet, MAX_FRAMES_IN_FLIGHT> PortalObj::portalDescriptorSets;
GraphicPipeline PortalObj::portalPipeline;
PortalObj::PortalObj(Device *device) {
    portalMesh = Mesh(device, "../data/mesh/portal.obj", Buffer::BufferType::GPU_ONLY);

    GraphicPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.fragmentShaderFile = "hello_scene.frag";
    pipelineCreateInfo.vexterShaderFile = "hello_scene.vert";
    portalPipeline = GraphicPipeline(device, pipelineCreateInfo);
}

PortalObj::~PortalObj() {}

void PortalObj::render(CommandBuffer &cmd, RenderData &renderData) {

     if(renderData.binded_pipeline != renderData.default_pipeline){
        renderData.binded_pipeline->bindPipeline(cmd);
        renderData.binded_pipeline = renderData.default_pipeline;
    }

    Mesh &mesh = portalMesh;

    renderData.binded_mesh = &mesh;
    mesh.bindMesh(cmd);
    PushConstantData pc = {wMatrix(), wNormalMatrix(), 0};

    vkCmdPushConstants(cmd, renderData.binded_pipeline->getPipelineLayout(), renderData.binded_pipeline->getPushConstantStage(), 0, sizeof(PushConstantData), &pc);

    vkCmdDrawIndexed(cmd, mesh.nbIndicies(), 1, 0, 0, 0);
}

void PortalObj::resize(Device *device,
    std::vector<std::vector<std::vector<Image>>> &portalATextures, std::vector<std::vector<std::vector<Image>>> &portalBTextures) {
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        std::vector<VkDescriptorImageInfo> imagesInfo;
        DescriptorSet portalDescriptorSet = DescriptorSet(device, portalPipeline.getDescriptorSetLayout(1));
        for (int j = 0; j < portalATextures.size(); j++) {
            imagesInfo.push_back(portalATextures[j][i][1].getDescriptorImageInfo(samplerType::LINEAR));
            imagesInfo.push_back(portalBTextures[j][i][1].getDescriptorImageInfo(samplerType::LINEAR));
        }
        portalDescriptorSet.writeImagesDescriptor(0, imagesInfo);
        portalDescriptorSets[i] = portalDescriptorSet;
    }
}

void PortalObj::placePortal(glm::vec3 normal, glm::vec3 pos) {
    this->transform.pos = pos + normal * 0.01f;
    normal = glm::normalize(normal);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    auto rotationQuat = glm::rotation(up, normal);
    this->transform.rot = glm::eulerAngles(rotationQuat);
}

}  // namespace TTe