
#include "node.hpp"

#include <algorithm>
#include <memory>

#include "sceneV2/cameraV2.hpp"
#include "sceneV2/scene.hpp"

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

namespace TTe {

Node::Node() {
    transform.pos.onChanged = [this]() { setDirty(); };
    transform.rot.onChanged = [this]() { setDirty(); };
    transform.scale.onChanged = [this]() { setDirty(); };
}

Node::Node(int id) : id(id) {}

Node::~Node() {
    for (auto &child : children) {
        child->setParent(nullptr);
    }
}

glm::mat4 Node::wMatrix() {
    mtx.lock();
    if (dirty) {
        glm::mat4 scaleMatrix = glm::scale(transform.scale.value);
        glm::mat4 translationMatrix = glm::translate(transform.pos.value);
        glm::mat4 rotationMatrix = glm::eulerAngleZXY(transform.rot.value.z, transform.rot.value.x, transform.rot.value.y);
        worldMatrix = translationMatrix * rotationMatrix * scaleMatrix;

        worldMatrix = (parent) ? parent->wMatrix() * worldMatrix : worldMatrix;
        dirty = false;
        // mtx.unlock();
    }
    mtx.unlock();
    return worldMatrix;
}

glm::mat3 Node::wNormalMatrix() {
    if (normalDirty) {
        worldNormalMatrix = glm::inverseTranspose(glm::mat3(this->wMatrix()));
        normalDirty = false;
        // worldNormalMatrix = parent ? parent->wNormalMatrix() * worldNormalMatrix : worldNormalMatrix;
    }
    return worldNormalMatrix;
}

void Node::setDirty() {
    if (mtx.try_lock()) {
        dirty = true;
        normalDirty = true;
        for (auto &child : children) {
            child->setDirty();
        }
        mtx.unlock();
    }
}

Node *Node::getParent() const { return parent; }

void Node::setParent(Node *parent) { this->parent = parent; }

int Node::getId() const { return id; }

void Node::setId(int id) { this->id = id; }

std::shared_ptr<Node> Node::getChild(int index) const { return children[index]; }

std::vector<std::shared_ptr<Node>> &Node::getChildren() { return children; }

void Node::addChild(std::shared_ptr<Node> child) {
    children.push_back(child);
    child->setParent(this);
}

void Node::removeChild(std::shared_ptr<Node> child) {
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        children.erase(it);
        child->setParent(nullptr);
    }
}

}  // namespace TTe