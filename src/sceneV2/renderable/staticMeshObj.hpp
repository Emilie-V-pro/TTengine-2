#pragma once


#include "sceneV2/mesh.hpp"
#include "sceneV2/IIndirectRenderable.hpp"
#include "sceneV2/node.hpp"
namespace TTe {
class StaticMeshObj : public IIndirectRenderable, public Node {

   public:
    StaticMeshObj();

    // copy constructor
    StaticMeshObj(const StaticMeshObj &other) : Node(other) {
        this->m_mesh = other.m_mesh;
    }

    // copy assignment
    StaticMeshObj &operator=(const StaticMeshObj &other) {
        if (this != &other) {
            Node::operator=(other);
            this->m_mesh = other.m_mesh;
        }
        return *this;
    }

    // move constructor
    StaticMeshObj(StaticMeshObj &&other) : Node(other) {
        this->m_mesh = other.m_mesh;
    }

    // move assignment
    StaticMeshObj &operator=(StaticMeshObj &&other) {
        if (this != &other) {
            Node::operator=(other);
            this->m_mesh = other.m_mesh;
        }
        return *this;
    }

    ~StaticMeshObj();

    



    // overide hit and compute bounding box
    virtual BoundingBox computeBoundingBox() override;
    virtual SceneHit hit(glm::vec3 &p_ro, glm::vec3 &p_rd) override;
    
    
    void render(CommandBuffer &p_cmd, RenderData &p_render_data) override;
    void setMesh(Mesh* p_mesh) { this->m_mesh = p_mesh; }
    private:
    Mesh* m_mesh;
};
}