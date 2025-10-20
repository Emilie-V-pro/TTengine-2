
#include "node.hpp"


#include <map>
#include <memory>
#include <string>



#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

namespace TTe {

Node::Node() {
    transform.pos.on_changed = [this]() { setDirty(); };
    transform.rot.on_changed = [this]() { setDirty(); };
    transform.scale.on_changed = [this]() { setDirty(); };
}

Node::~Node() {
    for (auto &child : m_children) {
        child->setParent(nullptr);
    }
}

Node::Node(const Node &other) {
    transform.pos.on_changed = [this]() { setDirty(); };
    transform.rot.on_changed = [this]() { setDirty(); };
    transform.scale.on_changed = [this]() { setDirty(); };
    m_id = other.m_id;
    m_name = other.m_name;
    transform = other.transform;
    m_world_matrix = other.m_world_matrix;
    m_world_normal_matrix = other.m_world_normal_matrix;
    dirty = other.dirty;
    normal_dirty = other.normal_dirty;
    m_parent = other.m_parent;
    m_children = other.m_children;
};

Node &Node::operator=(const Node &other) {
    if (this != &other) {
        transform.pos.on_changed = [this]() { setDirty(); };
        transform.rot.on_changed = [this]() { setDirty(); };
        transform.scale.on_changed = [this]() { setDirty(); };
        m_id = other.m_id;
        m_name = other.m_name;
        transform = other.transform;
        m_world_matrix = other.m_world_matrix;
        m_world_normal_matrix = other.m_world_normal_matrix;
        dirty = other.dirty;
        normal_dirty = other.normal_dirty;
        m_parent = other.m_parent;
        m_children = other.m_children;
    }
    return *this;
}

glm::mat4 Node::wMatrix() {
    // mtx.lock();
    if (dirty) {
        glm::mat4 scale_matrix = glm::scale(transform.scale.value);
        glm::mat4 translation_matrix = glm::translate(transform.pos.value);
        glm::mat4 rotation_matrix = glm::eulerAngleZXY(transform.rot.value.z, transform.rot.value.x, transform.rot.value.y);
        m_world_matrix = translation_matrix * rotation_matrix * scale_matrix;

        m_world_matrix = (m_parent) ? m_parent->wMatrix() * m_world_matrix : m_world_matrix;
        dirty = false;
        // mtx.unlock();
    }
    // mtx.unlock();
    return m_world_matrix;
}

glm::mat3 Node::wNormalMatrix() {
    if (normal_dirty) {
        m_world_normal_matrix = glm::inverseTranspose(glm::mat3(this->wMatrix()));
        normal_dirty = false;
        // worldNormalMatrix = m_parent ? m_parent->wNormalMatrix() * worldNormalMatrix : worldNormalMatrix;
    }
    return m_world_normal_matrix;
}

void Node::setDirty() {
    // if (mtx.try_lock()) {
    dirty = true;
    normal_dirty = true;
    uploaded_to_GPU = false;
    for (auto &child : m_children) {
        child->setDirty();
    }
    //     mtx.unlock();
    // }
}


BoundingBox Node::computeBoundingBox() {
        BoundingBox tmp;
        // get the pos of the node with the world matrix
        glm::mat4 world_matrix = wMatrix();
        tmp.pmin = world_matrix[3];
        tmp.pmax = world_matrix[3];

        for (auto &child : m_children) {
            BoundingBox childbb = child->computeBoundingBox();
            tmp.pmin = glm::min(childbb.pmin, tmp.pmin);
            tmp.pmax = glm::max(childbb.pmax, tmp.pmax);
        };
        m_bbox = tmp;
        if (m_bbox.pmin.x == m_bbox.pmax.x) {
            m_bbox.pmin.x = m_bbox.pmax.x - 0.000001f;
        }
        if (m_bbox.pmin.y == m_bbox.pmax.y) {
            m_bbox.pmin.y = m_bbox.pmax.y - 0.000001f;
        }
        if (m_bbox.pmin.z == m_bbox.pmax.z) {
            m_bbox.pmin.z = m_bbox.pmax.z - 0.000001f;
        }
        return m_bbox;
    }

SceneHit Node::hit(glm::vec3 &p_ro, glm::vec3 &p_rd)  {
        SceneHit hit;
        hit.t = -1;
        float t_min = m_bbox.intersect(p_ro, p_rd);

        std::multimap<float, std::shared_ptr<Node>> sorted_children;

        if (t_min != -1) {
            t_min = std::numeric_limits<float>::max();
            for (auto &child : m_children) {
                float t = child->getBoundingBox().intersect(p_ro, p_rd);
                if (t != -1) {
                    sorted_children.insert(std::make_pair(t, child));
                }
            }

            while (sorted_children.size() > 0) {
                auto it = sorted_children.begin();
                std::shared_ptr<Node> child = it->second;
                sorted_children.erase(it);

                if (it->first > t_min) {
                    continue;
                }
                SceneHit child_hit = child->hit(p_ro, p_rd);
                if (child_hit.t != -1 && child_hit.t < t_min) {
                    t_min = child_hit.t;
                    hit = child_hit;
                }
            }
        }

        return hit;
    }

Node *Node::getParent() const { return m_parent; }

void Node::setParent(Node *p_parent) { this->m_parent = p_parent; }

int Node::getId() const { return m_id; }

void Node::setId(int p_id) { this->m_id = p_id; }

void Node::setName(const std::string p_name) { this->m_name = p_name; }

std::string &Node::getName() { return this->m_name; }

std::shared_ptr<Node> Node::getChild(int p_index) const { return m_children[p_index]; }

std::vector<std::shared_ptr<Node>> &Node::getChildren() { return m_children; }

void Node::addChild(std::shared_ptr<Node> p_child) {
    m_children.push_back(p_child);
    p_child->setParent(this);
}

void Node::removeChild(std::shared_ptr<Node> p_child) {
    auto it = std::find(m_children.begin(), m_children.end(), p_child);
    if (it != m_children.end()) {
        m_children.erase(it);
        p_child->setParent(nullptr);
    }
}

}  // namespace TTe