#pragma once

#include <glm/fwd.hpp>
#include <iostream>
#include <map>
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
    bool normalDirty = true;
    bool uploadedToGPU = false;

    glm::mat4 wMatrix();
    glm::mat3 wNormalMatrix();

    void updateOnchangeFunc() {
        transform.pos.on_changed = [this]() { setDirty(); };
        transform.rot.on_changed = [this]() { setDirty(); };
        transform.scale.on_changed = [this]() { setDirty(); };
    };

    Node *getParent() const;
    void setParent(Node *parent);

    int getId() const;
    void setId(int id);

    void setName(const std::string name);
    std::string &getName();

    std::shared_ptr<Node> getChild(int index) const;
    std::vector<std::shared_ptr<Node>> &getChildren();
    void addChild(std::shared_ptr<Node> child);
    void removeChild(std::shared_ptr<Node> child);

    void setDirty();

    virtual BoundingBox computeBoundingBox() {
        BoundingBox tmp;
        // get the pos of the node with the world matrix
        glm::mat4 worldMatrix = wMatrix();
        tmp.pmin = worldMatrix[3];
        tmp.pmax = worldMatrix[3];

        for (auto &child : children) {
            BoundingBox childbb = child->computeBoundingBox();
            tmp.pmin = glm::min(childbb.pmin, tmp.pmin);
            tmp.pmax = glm::max(childbb.pmax, tmp.pmax);
        };
        bbox = tmp;
        if (bbox.pmin.x == bbox.pmax.x) {
            bbox.pmin.x = bbox.pmax.x - 0.000001f;
        }
        if (bbox.pmin.y == bbox.pmax.y) {
            bbox.pmin.y = bbox.pmax.y - 0.000001f;
        }
        if (bbox.pmin.z == bbox.pmax.z) {
            bbox.pmin.z = bbox.pmax.z - 0.000001f;
        }
        return bbox;
    }

    BoundingBox getBoundingBox() { return bbox; }

    virtual SceneHit hit(glm::vec3 &ro, glm::vec3 &rd) {
        SceneHit hit;
        hit.t = -1;
        float t_min = bbox.intersect(ro, rd);

        std::multimap<float, std::shared_ptr<Node>> sorted_children;

        if (t_min != -1) {
            t_min = std::numeric_limits<float>::max();
            for (auto &child : children) {
                float t = child->getBoundingBox().intersect(ro, rd);
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
                SceneHit child_hit = child->hit(ro, rd);
                if (child_hit.t != -1 && child_hit.t < t_min) {
                    t_min = child_hit.t;
                    hit = child_hit;
                }
            }
        }

        return hit;
    }

    // virtual glm::vec3 intersect();

   protected:
    std::string name;
    int id;

    BoundingBox bbox;

    glm::mat4 worldMatrix;
    glm::mat3 worldNormalMatrix;
    std::mutex mtx;
    Node *parent = nullptr;
    std::vector<std::shared_ptr<Node>> children;
    friend class SkeletonObj;
};
}  // namespace TTe