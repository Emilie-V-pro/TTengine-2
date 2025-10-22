#pragma once

#include "sceneV2/mesh.hpp"
#include "sceneV2/IIndirectRenderable.hpp"
#include "sceneV2/node.hpp"
namespace TTe {
class BasicMeshObj : public IIndirectRenderable, public Node {

   public:
    BasicMeshObj();

    // copy constructor
    BasicMeshObj(const BasicMeshObj &other) : Node(other) {
        this->m_shape = other.m_shape;
    }

    // copy assignment
    BasicMeshObj &operator=(const BasicMeshObj &other) {
        if (this != &other) {
            Node::operator=(other);
            this->m_shape = other.m_shape;
        }
        return *this;
    }

    // move constructor
    BasicMeshObj(BasicMeshObj &&other) : Node(other) {
        this->m_shape = other.m_shape;
    }

    // move assignment
    BasicMeshObj &operator=(BasicMeshObj &&other) {
        if (this != &other) {
            Node::operator=(other);
            this->m_shape = other.m_shape;
        }
        return *this;
    }

    ~BasicMeshObj();
    
    void render(CommandBuffer &p_cmd, RenderData &p_render_data);
    void setShape(Mesh::BasicShape shape) { this->m_shape = shape; }
    private:
    Mesh::BasicShape m_shape;
};
}