#pragma once



#include "struct.hpp"

namespace TTe {

    class Scene2;
class Node {
   public:
    Node();
    Node(int id);

    // delete copy and move constructors
    Node(Node const&) = delete;             // Copy construct
    Node(Node&&) = delete;                  // Move construct
    Node& operator=(Node const&) = delete;  // Copy assign
    Node& operator=(Node &&) = delete;      // Move assign

    ~Node();

    TransformComponent transform;

    bool dirty = true;

    bool normalDirty = true;

    glm::mat4 wMatrix();
    glm::mat3 wNormalMatrix();

    Node *getParent() const;
    void setParent(Node *parent);

    Node *getChild(int index) const;
    std::vector<Node *> &getChildren();
    void addChild(Node child);
    void removeChild(Node *child);

   private:
    int id;
    glm::mat4 worldMatrix;
    glm::mat3 worldNormalMatrix;

    Scene2 *scene;
    Node *parent;
    std::vector<Node *> children;
};
}  // namespace TTe