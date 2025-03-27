#pragma once

#include <iostream>
#include <memory>

#include "struct.hpp"

namespace TTe {

class Scene2;
class Node {
   public:
    Node();
    Node(int id);

    virtual ~Node() = 0;

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


    void setDirty() {

        if (dirty != true) {
            dirty = true;
            normalDirty = true;
            for (auto &child : children) {
                child->setDirty();
            }
        }
    }

   protected:
    int id;
    glm::mat4 worldMatrix;
    glm::mat3 worldNormalMatrix;

    Node *parent = nullptr;
    std::vector<std::shared_ptr<Node>> children;
};
}  // namespace TTe