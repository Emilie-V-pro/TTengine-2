
#include "basicMeshObj.hpp"
#include "sceneV2/Irenderable.hpp"

namespace TTe {

BasicMeshObj::BasicMeshObj() {}

BasicMeshObj::~BasicMeshObj() {}

void BasicMeshObj::render(CommandBuffer &cmd, RenderData &renderData) {

    if(renderData.binded_pipeline != renderData.default_pipeline){
        renderData.binded_pipeline->bindPipeline(cmd);
        renderData.binded_pipeline = renderData.default_pipeline;
    }
    
    Mesh &mesh = (*(renderData.basicMeshes))[shape];

    if(renderData.binded_mesh != &mesh){
        renderData.binded_mesh = &mesh;
        renderData.binded_mesh->bindMesh(cmd);
    }
    mesh.bindMesh(cmd);


    PushConstantData pc = {wMatrix(), wNormalMatrix(), renderData.portal_pos, renderData.cameraId, renderData.portal_normal};

    vkCmdPushConstants(
        cmd, renderData.binded_pipeline->getPipelineLayout(), renderData.binded_pipeline->getPushConstantStage(), 0, sizeof(PushConstantData), &pc);

    vkCmdDrawIndexed(cmd, mesh.nbIndicies(), 1, 0, 0, 0);
}



}  // namespace TTe