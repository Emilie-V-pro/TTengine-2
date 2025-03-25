
#include "staticMeshObj.hpp"

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



}  // namespace TTe