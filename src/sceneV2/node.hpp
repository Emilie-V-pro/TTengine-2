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

    void onChange(Node *node) {
        // std::cout << "me " << this->id << "\n";
        if (dirty != true) {
            std::cout << "mine\n";
            node->dirty = true;
            node->normalDirty = true;
            for (auto &child : node->children) {
                onChange(child.get());
            }
        }
    }

    void setDirty() {
        // std::cout << "mO\n";
        if (dirty != true) {
            dirty = true;
            normalDirty = true;
            for (auto &child : children) {
                onChange(child.get());
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