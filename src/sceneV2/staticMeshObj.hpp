#pragma once

#include "scene/mesh.hpp"
#include "sceneV2/Irenderable.hpp"
#include "sceneV2/node.hpp"
namespace TTe {
class StaticMeshObj : public IRenderable, public Node {

   public:
    StaticMeshObj();

    // copy constructor
    StaticMeshObj(const StaticMeshObj &other) : Node(other) {
        this->meshId = other.meshId;
    }

    // copy assignment
    StaticMeshObj &operator=(const StaticMeshObj &other) {
        if (this != &other) {
            Node::operator=(other);
            this->meshId = other.meshId;
        }
        return *this;
    }

    // move constructor
    StaticMeshObj(StaticMeshObj &&other) : Node(other) {
        this->meshId = other.meshId;
    }

    // move assignment
    StaticMeshObj &operator=(StaticMeshObj &&other) {
        if (this != &other) {
            Node::operator=(other);
            this->meshId = other.meshId;
        }
        return *this;
    }

    ~StaticMeshObj();
    
    void render(CommandBuffer &cmd, GraphicPipeline &pipeline, std::vector<Mesh> &meshes, std::map<BasicShape, Mesh> basicMeshes);
    void setMeshId(int id) { meshId = id; }
    private:
    int meshId;
};
}