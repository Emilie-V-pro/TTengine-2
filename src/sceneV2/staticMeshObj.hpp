#pragma once

#include "scene/mesh.hpp"
#include "sceneV2/Irenderable.hpp"
#include "sceneV2/node.hpp"
namespace TTe {
class StaticMeshObj : public IRenderable, public Node {

   public:
    StaticMeshObj();
    ~StaticMeshObj();
    
    void render(CommandBuffer &cmd, GraphicPipeline &pipeline, std::vector<Mesh> &meshes, std::map<BasicShape, Mesh> basicMeshes);
    void setMeshId(int id) { meshId = id; }
    private:
    int meshId;
};
}