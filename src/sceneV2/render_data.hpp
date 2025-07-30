#pragma once

#include <sys/types.h>
#include <cstdint>
#include <glm/fwd.hpp>
#include <stack>
#include "descriptor/descriptorSet.hpp"
#include "dynamic_renderpass.hpp"
#include "sceneV2/mesh.hpp"
#include "shader/pipeline.hpp"
namespace TTe {
    class RenderData{
        public:
        DynamicRenderPass *renderPass;
        std::stack<DescriptorSet> descriptorSets;
        Pipeline* default_pipeline;
        Pipeline* binded_pipeline;
        Mesh* binded_mesh;
        std::vector<Mesh> *meshes; 
        std::map<Mesh::BasicShape, Mesh> *basicMeshes;

        uint recursionLevel = 0;
        uint cameraId = 0;

        uint32_t frameIndex = 0;

        glm::vec3 portal_pos;
        glm::vec3 portal_normal;

        private:
    };
}