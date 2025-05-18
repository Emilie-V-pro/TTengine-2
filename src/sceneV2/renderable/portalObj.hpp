#pragma once

#include <array>
#include <glm/fwd.hpp>
#include <vector>

#include "GPU_data/image.hpp"
#include "descriptor/descriptorSet.hpp"
#include "device.hpp"
#include "sceneV2/Irenderable.hpp"
#include "sceneV2/mesh.hpp"
#include "sceneV2/node.hpp"
#include "shader/pipeline/graphic_pipeline.hpp"
#include "utils.hpp"
namespace TTe {
class PortalObj : public IRenderable, public Node {
   public:
    PortalObj(Device *device);

    ~PortalObj();

    void render(CommandBuffer &cmd, RenderData &renderData);
    void placePortal(glm::vec3 normal, glm::vec3 pos);
   static void resize(Device *device, std::vector<std::vector<std::vector<Image>>> &portalATextures, std::vector<std::vector<std::vector<Image>>> &portalBTextures);

       private :



    static GraphicPipeline portalPipeline;
    static std::array<DescriptorSet, MAX_FRAMES_IN_FLIGHT> portalDescriptorSets;

    Mesh portalMesh;
    Device *device;
};
}  // namespace TTe