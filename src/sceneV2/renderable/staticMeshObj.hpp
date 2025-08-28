#pragma once

#include <cstdint>
#include <vector>
#include "sceneV2/mesh.hpp"
#include "sceneV2/Irenderable.hpp"
#include "sceneV2/node.hpp"
namespace TTe {
class StaticMeshObj : public IRenderable, public Node {

   public:
    StaticMeshObj();

    // copy constructor
    StaticMeshObj(const StaticMeshObj &other) : Node(other) {
        this->mesh = other.mesh;
    }

    // copy assignment
    StaticMeshObj &operator=(const StaticMeshObj &other) {
        if (this != &other) {
            Node::operator=(other);
            this->mesh = other.mesh;
        }
        return *this;
    }

    // move constructor
    StaticMeshObj(StaticMeshObj &&other) : Node(other) {
        this->mesh = other.mesh;
    }

    // move assignment
    StaticMeshObj &operator=(StaticMeshObj &&other) {
        if (this != &other) {
            Node::operator=(other);
            this->mesh = other.mesh;
        }
        return *this;
    }

    ~StaticMeshObj();

    



    // overide hit and compute bounding box
    virtual BoundingBox computeBoundingBox() override;
    virtual SceneHit hit(glm::vec3 &ro, glm::vec3 &rd) override;
    
    
    void render(CommandBuffer &cmd, RenderData &renderData) override;
    void setMesh(Mesh* mesh) { this->mesh = mesh; }
    private:
    Mesh* mesh;
};
}