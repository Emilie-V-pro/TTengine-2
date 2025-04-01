
#include "skeletonObj.hpp"

#include <cstdio>
#include <glm/geometric.hpp>

#include "sceneV2/animatic/skeleton/BVHAxis.h"
#include "sceneV2/animatic/skeleton/BVHChannel.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <glm/fwd.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <memory>

#include "sceneV2/mesh.hpp"

namespace TTe {

void SkeletonObj::init(BVH bvh) {
    m_bvh = bvh;
    for (int i = 0; i < bvh.getNumberOfJoint(); i++) {
        std::shared_ptr<SkeletonNode> joint = std::make_shared<SkeletonNode>();
        auto jointMat = bvh.getJoint(i);
        int parent_id = jointMat.getParentId();

        joint->setId(i);
        if (parent_id == -1) {
            this->addChild(joint);
        } else {
            m_joints[parent_id]->addChild(joint);
        }

        glm::vec3 pos;
        jointMat.getOffset(pos.x, pos.y, pos.z);
        joint->transform.pos = pos;

        m_joints.push_back(joint);
    }
    this->transform.scale = glm::vec3(0.1f);
    this->transform.pos = glm::vec3(0, 0, 15);
}

glm::vec3 SkeletonObj::getJointPosition(int i) const {
    glm::vec3 returnValue;
    return returnValue;
}

int SkeletonObj::getParentId(const int i) const {
    int returnValue;
    return returnValue;
}

void SkeletonObj::setPose(const BVH &bvh, int frameNumber) {}

void SkeletonObj::simulation(
    glm::vec3 gravite, float viscosite, uint32_t tick, float dt, float t, std::vector<std::shared_ptr<ICollider>> &collisionObjects) {


    for (int i = 0; i < m_bvh.getNumberOfJoint(); i++) {
        auto jointMat = m_bvh.getJoint(i);

        glm::vec3 pos;
        jointMat.getOffset(pos.x, pos.y, pos.z);

        glm::vec3 rot = glm::vec3(0.0f);
        glm::vec3 trans = glm::vec3(0.0f);
        for (int j = 0; j < jointMat.getNumberOfChannel(); ++j) {
            auto channel = jointMat.getChannel(j);
            
            float data = channel.getData(static_cast<int>(int(t / 0.0666666) % m_bvh.getNumberOfFrame()));

            if (channel.getType() == BVHChannel::TYPE_TRANSLATION) {
                switch (channel.getAxis()) {
                    case AXIS_X:
                        trans.x = data;
                        break;
                    case AXIS_Y:
                        trans.y = data;
                        break;
                    case AXIS_Z:
                        trans.z = data;
                        break;
                    case AXIS_W:;
                        break;
                }
            }
            if (channel.getType() == BVHChannel::TYPE_ROTATION) {
                switch (channel.getAxis()) {
                    case AXIS_X:
                        rot.x = glm::radians(data);
                        break;
                    case AXIS_Y:
                        rot.y = glm::radians(data);
                        break;
                    case AXIS_Z:
                        rot.z = glm::radians(data);
                        break;
                    case AXIS_W:;
                        break;
                }
            }
        }
        m_joints[i]->transform.pos = pos + trans;
        m_joints[i]->transform.rot = rot;
    }
}

void SkeletonObj::render(CommandBuffer &cmd, GraphicPipeline &pipeline, std::vector<Mesh> &meshes, std::map<BasicShape, Mesh> basicMeshes) {
    // bind sphere
    basicMeshes[Sphere].bindMesh(cmd);

    for (auto &joint : m_joints) {
        glm::mat4 wMatrix = joint->wMatrix() * glm::scale(glm::vec3(4.f));
        glm::mat4 wNormalMatrix = joint->wNormalMatrix();

        // push constant
        vkCmdPushConstants(cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), 0, sizeof(glm::mat4), &wMatrix);
        vkCmdPushConstants(
            cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), sizeof(glm::mat4), sizeof(glm::mat4), &wNormalMatrix);
        vkCmdDrawIndexed(cmd, basicMeshes[Sphere].nbIndicies(), 1, 0, 0, 0);
    }

    basicMeshes[Cube].bindMesh(cmd);
    for (auto &joint : m_joints) {
        // draw cube as line between joints
        glm::mat4 wPoint = joint->wMatrix();

        for (auto &child : joint->getChildren()) {
            glm::mat4 wPointChild = child->wMatrix();

            glm::vec3 pos = wPoint[3];
            glm::vec3 childPos = wPointChild[3];

            glm::vec3 position = (pos + childPos) * 0.5f;

            glm::vec3 dir = glm::normalize(childPos - pos);

            float length = glm::length(childPos - pos);
            glm::vec3 baseDir = glm::vec3(0.0f, 0.0f, 1.0f);  
            // Direction de base du cube
            if (glm::length(glm::cross(baseDir, dir)) < 0.0001f) { 
                // Si dir est parallèle à baseDir, éviter NaN (cas particulier)
                baseDir = glm::vec3(0.0f, 1.0f, 0.0f);
            }
            
            glm::vec3 axis = glm::normalize(glm::cross(baseDir, dir));
            float angle = acos(glm::dot(baseDir, dir));
        
            glm::quat rotation = glm::angleAxis(angle, axis);
        
            // Calculer la matrice de transformation
            glm::mat4 wMatrix = glm::translate(glm::mat4(1.0f), position) *
                                  glm::toMat4(rotation) *
                                  glm::scale(glm::mat4(1.0f), glm::vec3(0.6f, 0.6f, length));
            glm::mat4 wNormalMatrix = glm::inverseTranspose(glm::mat3(wMatrix));

            // push constant
            vkCmdPushConstants(cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), 0, sizeof(glm::mat4), &wMatrix);
            vkCmdPushConstants(
                cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), sizeof(glm::mat4), sizeof(glm::mat4), &wNormalMatrix);
            vkCmdDrawIndexed(cmd, basicMeshes[Cube].nbIndicies(), 1, 0, 0, 0);
        }
    }
}

}  // namespace TTe