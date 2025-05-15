
#include "staticMeshObj.hpp"
#include <glm/geometric.hpp>

namespace TTe {

StaticMeshObj::StaticMeshObj() {}

StaticMeshObj::~StaticMeshObj() {}

void StaticMeshObj::render(CommandBuffer &cmd, GraphicPipeline &pipeline, std::vector<Mesh> &meshes,  std::map<BasicShape, Mesh> basicMeshes) {
    Mesh &mesh = meshes[meshId];
    mesh.bindMesh(cmd);
    glm::mat4 model = wMatrix();
    vkCmdPushConstants(cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), 0, sizeof(glm::mat4), &model);
    glm::mat4 normalMatrix = wNormalMatrix();
    vkCmdPushConstants(
        cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), sizeof(glm::mat4), sizeof(glm::mat4), &normalMatrix);

    vkCmdDrawIndexed(cmd, mesh.nbIndicies(), 1, 0, 0, 0);
}

BoundingBox StaticMeshObj::computeBoundingBox() {
    BoundingBox tmp;
    bbox = meshList->at(meshId).getBoundingBox();

    // apply transformation to bounding box
    tmp.pmin = glm::vec3(wMatrix() * glm::vec4(bbox.pmin, 1.0f));
    tmp.pmax = glm::vec3(wMatrix() * glm::vec4(bbox.pmax, 1.0f));
    bbox.pmin = glm::min(tmp.pmin, tmp.pmax);
    bbox.pmax = glm::max(tmp.pmin, tmp.pmax);
    

    for (auto &child : children) {
        BoundingBox childbb = child->computeBoundingBox();
        bbox.pmin = glm::min(bbox.pmin, tmp.pmin);
        bbox.pmax = glm::max(bbox.pmax, tmp.pmax);
    };
    return bbox;
}

SceneHit StaticMeshObj::hit(glm::vec3 &ro, glm::vec3 &rd) {
    //transform ray to local space
    glm::vec3 localRo = glm::inverse(wMatrix()) * glm::vec4(ro, 1.0f);
    glm::vec3 localRd = glm::normalize(glm::inverse(wMatrix()) * glm::vec4(rd, 0.0f));
    
    return meshList->at(meshId).hit(localRo, localRd);
}
}  // namespace TTe