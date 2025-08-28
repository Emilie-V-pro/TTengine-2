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
     




    class RenderData{
        public:
        DynamicRenderPass *renderPass;
        std::stack<DescriptorSet> descriptorSets;
        Pipeline* default_pipeline;
        Pipeline* binded_pipeline;

        std::vector<VkDrawIndexedIndirectCommand> drawCommands;
        
     
        std::map<Mesh::BasicShape, Mesh*>  basicMeshes;


        uint cameraId = 0;

        uint32_t frameIndex = 0;
        uint32_t swapchainIndex = 0;

        private:
    };
}