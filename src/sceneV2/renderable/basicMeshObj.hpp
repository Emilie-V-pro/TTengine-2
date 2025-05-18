#pragma once

#include "sceneV2/mesh.hpp"
#include "sceneV2/Irenderable.hpp"
#include "sceneV2/node.hpp"
namespace TTe {
class BasicMeshObj : public IRenderable, public Node {

   public:
    BasicMeshObj();

    // copy constructor
    BasicMeshObj(const BasicMeshObj &other) : Node(other) {
        this->shape = other.shape;
    }

    // copy assignment
    BasicMeshObj &operator=(const BasicMeshObj &other) {
        if (this != &other) {
            Node::operator=(other);
            this->shape = other.shape;
        }
        return *this;
    }

    // move constructor
    BasicMeshObj(BasicMeshObj &&other) : Node(other) {
        this->shape = other.shape;
    }

    // move assignment
    BasicMeshObj &operator=(BasicMeshObj &&other) {
        if (this != &other) {
            Node::operator=(other);
            this->shape = other.shape;
        }
        return *this;
    }

    ~BasicMeshObj();
    
    void render(CommandBuffer &cmd, RenderData &renderData);
    void setShape(BasicShape shape) { this->shape = shape; }
    private:
    BasicShape shape;
};
}