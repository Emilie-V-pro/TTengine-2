#pragma once



#include <glm/fwd.hpp>
#include "sceneV2/render_data.hpp"
namespace TTe {



class IIndirectRenderable {
   public:
    virtual void render(CommandBuffer &cmd, RenderData &renderData) = 0;
    void setMaterialOffset(uint offset) { materialOffset = offset; }
   private:
   protected:
    uint materialOffset;
};
}  // namespace TTe