
#include "node.hpp"
#include <algorithm>
#include "sceneV2/scene.hpp"

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

namespace TTe {


Node::Node() {}

Node::Node(int id) : id(id) {}

Node::~Node() {
    for (auto &child : children) {
        scene->removeNode(child->id);    
    }
    scene->removeNode(id);
}

glm::mat4 Node::wMatrix() {
    if (dirty) {
        transform.scale += glm::vec3(0.0001);
        glm::mat4 scaleMatrix = glm::scale(transform.scale.value);
        glm::mat4 translationMatrix = glm::translate(transform.pos.value);
        glm::mat4 rotationMatrix = glm::eulerAngleXYZ(transform.rot.value.x, transform.rot.value.y, transform.rot.value.z);
        worldMatrix = translationMatrix * rotationMatrix * scaleMatrix;
        dirty = false;
    }
    return worldMatrix;
}

glm::mat3 Node::wNormalMatrix() {
    if (normalDirty) {
        worldNormalMatrix = glm::inverseTranspose(glm::mat3(this->wMatrix()));
        normalDirty = false;
    }
    return worldNormalMatrix;
}

Node *Node::getParent() const { return parent; }

void Node::setParent(Node *parent) { this->parent = parent; }

Node *Node::getChild(int index) const { return children[index]; }

std::vector<Node *> &Node::getChildren() { return children; }

void Node::addChild(Node child) {
    children.push_back(child);
    scene->addNode(child->id);
    child->setParent(this);
}

void Node::removeChild(Node *child) {
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        scene->removeNode(child->id);
    }
}

}  // namespace TTe