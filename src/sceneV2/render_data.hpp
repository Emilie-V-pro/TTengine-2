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
    uint64_t obj_buffer;
    uint64_t mat_buffer;
    uint64_t cam_buffer;
    uint64_t light_buffer;
    uint32_t camid;
    uint32_t nb_light;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct PushConstantCullStruct {
    uint64_t obj_buffer;
    uint64_t cam_buffer;
    uint64_t mesh_blocks_buffer;
    uint64_t draw_cmds_buffer;
    uint64_t draw_count_buffer;
    uint32_t camid;
    uint32_t numberOfmesh_block;
};
#pragma pack(pop)

struct LightGPU{
    glm::vec4 color;
    glm::vec3 pos;
    uint32_t Type;
    glm::vec3 orientation;
    uint32_t offset;
};

class RenderData {
   public:
    DynamicRenderPass* render_pass;
    std::stack<DescriptorSet> descriptor_sets;
    Pipeline* default_pipeline;
    Pipeline* binded_pipeline;

    std::vector<VkDrawIndexedIndirectCommand> draw_commands;

    std::map<Mesh::BasicShape, Mesh*> basic_meshes;
    PushConstantStruct push_constant;
    uint camera_id = 0;
    std::vector<std::shared_ptr<CameraV2>> *cameras;

    uint32_t frame_index = 0;
    uint32_t swapchain_index = 0;

   private:
};
}  // namespace TTe