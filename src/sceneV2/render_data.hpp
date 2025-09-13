#pragma once

#include <sys/types.h>

#include <cstdint>
#include <glm/fwd.hpp>
#include <stack>
#include <vector>

#include "descriptor/descriptorSet.hpp"
#include "dynamic_renderpass.hpp"
#include "sceneV2/cameraV2.hpp"
#include "sceneV2/mesh.hpp"
#include "shader/pipeline.hpp"

namespace TTe {

#pragma pack(push, 1)
struct PushConstantStruct {
    uint64_t objBuffer;
    uint64_t matBuffer;
    uint64_t camBuffer;
    uint64_t lightBuffer;
    uint32_t camid;
    uint32_t nb_light;
};
#pragma pack(pop)


struct LightGPU{
    glm::vec4 color;
    glm::vec3 pos;
    uint32_t Type;
    glm::vec3 orienation;
    uint32_t offset;
};

class RenderData {
   public:
    DynamicRenderPass* renderPass;
    std::stack<DescriptorSet> descriptorSets;
    Pipeline* default_pipeline;
    Pipeline* binded_pipeline;

    std::vector<VkDrawIndexedIndirectCommand> drawCommands;

    std::map<Mesh::BasicShape, Mesh*> basicMeshes;
    PushConstantStruct pushConstant;
    uint cameraId = 0;
    std::vector<std::shared_ptr<CameraV2>> *cameras;

    uint32_t frameIndex = 0;
    uint32_t swapchainIndex = 0;

   private:
};
}  // namespace TTe