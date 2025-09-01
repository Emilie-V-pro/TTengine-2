#pragma once


#include <cstdint>
#include <glm/fwd.hpp>
#include <vector>
#include "sceneV2/mesh.hpp"
#include "sceneV2/render_data.hpp"
#include "shader/pipeline/graphic_pipeline.hpp"
namespace TTe {



class IRenderable {
   public:
    virtual void render(CommandBuffer &cmd, RenderData &renderData) = 0;
    void setMaterialOffset(uint offset) { materialOffset = offset; }
   private:
   protected:
    uint materialOffset;
};
}  // namespace TTe