#pragma once


#include <vector>
#include "sceneV2/mesh.hpp"
#include "shader/pipeline/graphic_pipeline.hpp"
namespace TTe {
class IRenderable {
   public:
    virtual void render(CommandBuffer &cmd, GraphicPipeline &pipeline, std::vector<Mesh> &meshes,  std::map<BasicShape, Mesh> basicMeshes) = 0;
    void setMaterialOffset(uint offset) { materialOffset = offset; }
   private:
   protected:
    uint materialOffset;
};
}  // namespace TTe