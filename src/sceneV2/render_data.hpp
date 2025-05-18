#pragma once

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
        std::map<BasicShape, Mesh> *basicMeshes;

        private:
    };
}