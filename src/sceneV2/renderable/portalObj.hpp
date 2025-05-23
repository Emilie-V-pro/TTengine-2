#pragma once

#include <array>
#include <cstdint>
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


    struct PushConstantPortal: public PushConstantData {
        uint32_t portalId;
        glm::vec3 portalColor;
        uint32_t recursionLevel;
        glm::ivec2 screenSize;
    };


    PortalObj();
    static void init(Device *device);

    ~PortalObj();

    void render(CommandBuffer &cmd, RenderData &renderData);
    void placePortal(glm::vec3 normal, glm::vec3 pos, glm::vec3 campos);
    glm::vec3 normal;
   static void resize(Device *device, std::vector<std::vector<std::vector<Image>>> &portalATextures, std::vector<std::vector<std::vector<Image>>> &portalBTextures);
   glm::vec3 portalColor = glm::vec3(0, 0, 0);
    int portalId = 0;
       private :

    static GraphicPipeline portalPipeline;
    static std::array<DescriptorSet, MAX_FRAMES_IN_FLIGHT> portalDescriptorSets;

    static Mesh portalMesh;
    static Device *device;
    static glm::ivec2 screen_size;
};
}  // namespace TTe