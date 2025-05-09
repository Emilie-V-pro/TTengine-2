#pragma once

#include <glm/fwd.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>

#include "struct.hpp"

namespace TTe {

class Scene2;
class Node {
   public:
    Node();
    Node(int id);

    virtual ~Node() = 0;

    // copy constructor
    Node(const Node &other) {
        id = other.id;
        transform = other.transform;
        worldMatrix = other.worldMatrix;
        worldNormalMatrix = other.worldNormalMatrix;
        dirty = other.dirty;
        normalDirty = other.normalDirty;
        parent = other.parent;
        children = other.children;
    };

    // copy assignment operator
    Node &operator=(const Node &other) {
        if (this != &other) {
            id = other.id;
            transform = other.transform;
            worldMatrix = other.worldMatrix;
            worldNormalMatrix = other.worldNormalMatrix;
            dirty = other.dirty;
            normalDirty = other.normalDirty;
            parent = other.parent;
            children = other.children;
        }
        return *this;
    }

    TransformComponent transform;

    bool dirty = true;

    bool normalDirty = true;

    glm::mat4 wMatrix();
    glm::mat3 wNormalMatrix();

    Node *getParent() const;
    void setParent(Node *parent);
    int getId() const;
    void setId(int id);

    std::shared_ptr<Node> getChild(int index) const;
    std::vector<std::shared_ptr<Node>> &getChildren();
    void addChild(std::shared_ptr<Node> child);
    void removeChild(std::shared_ptr<Node> child);

    void setDirty();

    virtual BoundingBox computeBoundingBox() {
        BoundingBox tmp;
        for (auto &child : children) {
            BoundingBox childbb = child->computeBoundingBox();
            bbox.pmin = glm::min(bbox.pmin, tmp.pmin);
            bbox.pmax = glm::max(bbox.pmax, tmp.pmax);
        };
        return bbox;
    }

    BoundingBox getBoundingBox() { return bbox; }


    virtual SceneHit hit(glm::vec3 &ro, glm::vec3 &rd) {
        SceneHit hit;
        hit.t = -1;
        float t_min = bbox.intersect(ro, rd);

        std::multimap<float, std::shared_ptr<Node>> sorted_children;


        if(t_min != -1){
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

                if(it->first > t_min){
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