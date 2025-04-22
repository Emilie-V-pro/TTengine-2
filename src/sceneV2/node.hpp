#pragma once

#include <iostream>
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

   protected:
    int id;
    glm::mat4 worldMatrix;
    glm::mat3 worldNormalMatrix;
    std::mutex mtx;
    Node *parent = nullptr;
    std::vector<std::shared_ptr<Node>> children;
    friend class SkeletonObj;
};
}  // namespace TTe