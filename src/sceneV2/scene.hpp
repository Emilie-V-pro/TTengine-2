#pragma once



#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>
#include "GPU_data/image.hpp"
#include "scene/mesh.hpp"
#include "sceneV2/node.hpp"

namespace TTe {
    class Scene2 : Node {
    public:
        Scene2();
        ~Scene2();

        void addNode(uint32_t Parent_id, Node node);
        void removeNode(uint32_t id);
    private:
        std::vector<Mesh> meshes;
        std::vector<Image> images;
        std::vector<Material> materials;

        
        std::unordered_map<uint32_t, std::shared_ptr<Node>> objects;
    };
}