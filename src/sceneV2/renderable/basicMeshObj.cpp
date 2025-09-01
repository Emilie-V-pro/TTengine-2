
#include "basicMeshObj.hpp"

namespace TTe {

BasicMeshObj::BasicMeshObj() {}

BasicMeshObj::~BasicMeshObj() {}

void BasicMeshObj::render(CommandBuffer &cmd, RenderData &renderData) {

    if(renderData.binded_pipeline != renderData.default_pipeline){
        renderData.binded_pipeline->bindPipeline(cmd);
        renderData.binded_pipeline = renderData.default_pipeline;
    }
    
    Mesh *mesh = renderData.basicMeshes[shape];
    
    VkDrawIndexedIndirectCommand drawCmd;
    drawCmd.firstIndex = mesh->getFirstIndex();
    drawCmd.vertexOffset = mesh->getFirstVertex();
    drawCmd.indexCount = mesh->nbIndicies();
    drawCmd.instanceCount = 1;
    drawCmd.firstInstance = this->id;

    renderData.drawCommands.push_back(drawCmd);
 }



}  // namespace TTe