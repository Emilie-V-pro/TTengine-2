
#include "staticMeshObj.hpp"
#include <glm/geometric.hpp>

namespace TTe {

StaticMeshObj::StaticMeshObj() {}

StaticMeshObj::~StaticMeshObj() {}

void StaticMeshObj::render(CommandBuffer &cmd, RenderData &renderData) {


    

    VkDrawIndexedIndirectCommand drawCmd;
    drawCmd.firstIndex = mesh->getFirstIndex();
    drawCmd.vertexOffset = mesh->getFirstVertex();
    drawCmd.indexCount = mesh->nbIndicies();
    drawCmd.instanceCount = 1;
    drawCmd.firstInstance = this->id;

    renderData.drawCommands.push_back(drawCmd);

    
}

BoundingBox StaticMeshObj::computeBoundingBox() {
    BoundingBox tmp;
    bbox = mesh->getBoundingBox();

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

    if(bbox.pmin.x == bbox.pmax.x){
            bbox.pmin.x = bbox.pmax.x - 0.000001f;
        }
        if(bbox.pmin.y == bbox.pmax.y){
            bbox.pmin.y = bbox.pmax.y - 0.000001f;
        }
        if(bbox.pmin.z == bbox.pmax.z){
            bbox.pmin.z = bbox.pmax.z - 0.000001f;
        }
    return bbox;
}

SceneHit StaticMeshObj::hit(glm::vec3 &ro, glm::vec3 &rd) {
    //transform ray to local space
    glm::vec3 localRo = glm::inverse(wMatrix()) * glm::vec4(ro, 1.0f);
    glm::vec3 localRd = glm::normalize(glm::inverse(wMatrix()) * glm::vec4(rd, 0.0f));
    
    return mesh->hit(localRo, localRd);
}
}  // namespace TTe