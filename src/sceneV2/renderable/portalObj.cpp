
#include "portalObj.hpp"
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/matrix.hpp>
#include <vector>

#include "descriptor/descriptorSet.hpp"
#include "device.hpp"
#include "sceneV2/mesh.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace TTe {

std::array<DescriptorSet, MAX_FRAMES_IN_FLIGHT> PortalObj::portalDescriptorSets;
GraphicPipeline PortalObj::portalPipeline;
Mesh PortalObj::portalMesh;
Device *PortalObj::device = nullptr;
glm::ivec2 PortalObj::screen_size = {1280, 720};


PortalObj::PortalObj() {
    
}

void PortalObj::init(Device *device) {
    //TODO : replace with gltf loader when implemented
portalMesh = Mesh(device, Mesh::Plane, 1);

    GraphicPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.fragmentShaderFile = "portal.frag";
    pipelineCreateInfo.vexterShaderFile = "portal.vert";
    portalPipeline = GraphicPipeline(device, pipelineCreateInfo);
}

PortalObj::~PortalObj() {}

void PortalObj::render(CommandBuffer &cmd, RenderData &renderData) {
    if(renderData.recursionLevel != 0){
        if(renderData.cameraId % 2 == 0 && !portalId){
           return;
        } else if(renderData.cameraId % 2 == 1 && portalId){
            return;
        }
    }
    if(renderData.binded_pipeline != &portalPipeline){
        portalPipeline.bindPipeline(cmd);
        renderData.binded_pipeline = &portalPipeline;
    }

    Mesh &mesh = portalMesh;

    renderData.binded_mesh = &mesh;
    mesh.bindMesh(cmd);

    PushConstantPortal pc = {{wMatrix(), wNormalMatrix(), renderData.portal_pos, renderData.cameraId, renderData.portal_normal} , renderData.recursionLevel * 2 +   1 -portalId, portalColor,renderData.recursionLevel, screen_size};

    vkCmdPushConstants(cmd, renderData.binded_pipeline->getPipelineLayout(), renderData.binded_pipeline->getPushConstantStage(), 0, sizeof(PushConstantPortal), &pc);
    std::vector<DescriptorSet*> descriptorSet = {&portalDescriptorSets[renderData.frameIndex]};
    // bind descriptor set
    DescriptorSet::bindDescriptorSet(cmd, descriptorSet, renderData.binded_pipeline->getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS, 1);

    vkCmdDrawIndexed(cmd, mesh.nbIndicies(), 1, 0, 0, 0);
}

void PortalObj::resize(Device *device,
    std::vector<std::vector<std::vector<Image>>> &portalATextures, std::vector<std::vector<std::vector<Image>>> &portalBTextures) {
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        std::vector<VkDescriptorImageInfo> imagesInfo;
        DescriptorSet portalDescriptorSet = DescriptorSet(device, portalPipeline.getDescriptorSetLayout(1));
        for (int j = 0; j < portalATextures.size(); j++) {
            imagesInfo.push_back(portalATextures[j][i][0].getDescriptorImageInfo(samplerType::LINEAR));
            imagesInfo.push_back(portalBTextures[j][i][0].getDescriptorImageInfo(samplerType::LINEAR));
        }
        portalDescriptorSet.writeImagesDescriptor(0, imagesInfo);
        portalDescriptorSets[i] = portalDescriptorSet;
    }
    screen_size = {portalATextures[0][0][0].getWidth(), portalATextures[0][0][0].getHeight()};
}

void PortalObj::placePortal(glm::vec3 normal, glm::vec3 pos, glm::vec3 campos) {
    this->transform.pos = pos + normal * 0.1f;
    normal = glm::normalize(normal);
    this->normal = normal;
    auto t = normal-glm::vec3(0.0f, 1.0f, 0.0f);

    auto t2 = glm::length(t);
    glm::vec3 up = (t2 > 0.0001 ) ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::normalize(campos - pos) ;
    auto rotationQuat = glm::rotation(up, normal);


    this->transform.rot = glm::eulerAngles(rotationQuat);


    this->worldMatrix = glm::inverse(glm::lookAt(transform.pos.value, this->transform.pos + normal,  up));
    this->dirty = false;
}

}  // namespace TTe