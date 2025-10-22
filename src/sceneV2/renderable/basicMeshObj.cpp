
#include "basicMeshObj.hpp"

namespace TTe {

BasicMeshObj::BasicMeshObj() {}

BasicMeshObj::~BasicMeshObj() {}

void BasicMeshObj::render(CommandBuffer &p_cmd, RenderData &p_render_data) {

    if(p_render_data.binded_pipeline != p_render_data.default_pipeline){
        p_render_data.binded_pipeline->bindPipeline(p_cmd);
        p_render_data.binded_pipeline = p_render_data.default_pipeline;
    }
    
    Mesh *mesh = p_render_data.basic_meshes[m_shape];
    
    VkDrawIndexedIndirectCommand drawCmd;
    drawCmd.firstIndex = mesh->getFirstIndex();
    drawCmd.vertexOffset = mesh->getFirstVertex();
    drawCmd.indexCount = mesh->nbIndicies();
    drawCmd.instanceCount = 1;
    drawCmd.firstInstance = this->m_id;

    p_render_data.draw_commands.push_back(drawCmd);
 }



}  // namespace TTe