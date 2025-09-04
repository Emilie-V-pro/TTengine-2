#pragma once

#include <sys/types.h>

#include <cstdint>
#include <glm/fwd.hpp>
#include <stack>
#include <vector>

#include "descriptor/descriptorSet.hpp"
#include "dynamic_renderpass.hpp"
#include "sceneV2/mesh.hpp"
#include "shader/pipeline.hpp"

namespace TTe {

#pragma pack(push, 1)
struct PushConstantStruct {
    uint64_t objBuffer;
    uint64_t matBuffer;
    uint64_t camBuffer;
    uint32_t camid;
};
#pragma pack(pop)

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

    uint32_t frameIndex = 0;
    uint32_t swapchainIndex = 0;

   private:
};
}  // namespace TTe