
#include "basicMeshObj.hpp"

namespace TTe {

BasicMeshObj::BasicMeshObj() {}

BasicMeshObj::~BasicMeshObj() {}

void BasicMeshObj::render(CommandBuffer &cmd, GraphicPipeline &pipeline, std::vector<Mesh> &meshes,  std::map<BasicShape, Mesh> basicMeshes) {
    Mesh &mesh = basicMeshes[shape];
    mesh.bindMesh(cmd);
    glm::mat4 model = wMatrix();
    vkCmdPushConstants(cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), 0, sizeof(glm::mat4), &model);
    glm::mat4 normalMatrix = wNormalMatrix();
    vkCmdPushConstants(
        cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), sizeof(glm::mat4), sizeof(glm::mat4), &normalMatrix);

    vkCmdDrawIndexed(cmd, mesh.nbIndicies(), 1, 0, 0, 0);
}



}  // namespace TTe