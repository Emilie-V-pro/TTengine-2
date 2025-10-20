#pragma once

#include <glm/fwd.hpp>
#include <memory>
#include <mutex>
#include <string>

#include "struct.hpp"

namespace TTe {

class Scene;
class Node {
   public:
    Node();

    virtual ~Node() = 0;

    Node(const Node &other);
    Node &operator=(const Node &other);

    TransformComponent transform;

    bool dirty = true;
    bool normal_dirty = true;
    bool uploaded_to_GPU = false;

    glm::mat4 wMatrix();
    glm::mat3 wNormalMatrix();

    void updateOnchangeFunc() {
        transform.pos.on_changed = [this]() { setDirty(); };
        transform.rot.on_changed = [this]() { setDirty(); };
        transform.scale.on_changed = [this]() { setDirty(); };
    };

    Node *getParent() const;
    void setParent(Node *p_parent);

    int getId() const;
    void setId(int p_id);

    void setName(const std::string p_name);
    std::string &getName();

    std::shared_ptr<Node> getChild(int p_index) const;
    std::vector<std::shared_ptr<Node>> &getChildren();
    void addChild(std::shared_ptr<Node> p_child);
    void removeChild(std::shared_ptr<Node> p_child);

    void setDirty();

    virtual BoundingBox computeBoundingBox();

    BoundingBox getBoundingBox() { return m_bbox; }

    virtual SceneHit hit(glm::vec3 &p_ro, glm::vec3 &p_rd);

   protected:
    std::string m_name;
    int m_id;

    BoundingBox m_bbox;

    glm::mat4 m_world_matrix;
    glm::mat3 m_world_normal_matrix;
    std::mutex m_mtx;
    Node *m_parent = nullptr;
    std::vector<std::shared_ptr<Node>> m_children;
    friend class SkeletonObj;
};
}  // namespace TTe