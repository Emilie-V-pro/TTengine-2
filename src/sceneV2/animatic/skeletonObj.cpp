
#include "skeletonObj.hpp"

#include <cstdio>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/ext/scalar_common.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <string>

#include "sceneV2/animatic/skeleton/BVHAxis.h"
#include "sceneV2/animatic/skeleton/BVHChannel.h"
#include "sceneV2/collision/collision_obj.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <filesystem>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <memory>

#include "sceneV2/mesh.hpp"

namespace TTe {

void SkeletonObj::init(BVH bvh) {
    m_bvh[0] = bvh;
    state = 0;
    for (int i = 0; i < bvh.getNumberOfJoint(); i++) {
        std::shared_ptr<SkeletonNode> joint1 = std::make_shared<SkeletonNode>();
        std::shared_ptr<SkeletonNode> joint2 = std::make_shared<SkeletonNode>();
        std::shared_ptr<SkeletonNode> joint3 = std::make_shared<SkeletonNode>();
        auto jointMat = bvh.getJoint(i);
        int parent_id = jointMat.getParentId();

        joint1->setId(i);
        joint2->setId(i);
        joint3->setId(i);
        if (parent_id == -1) {
            this->addChild(joint1);
            this->addChild(joint2);
            this->addChild(joint3);
        } else {
            m_joints_1[parent_id]->addChild(joint1);
            m_joints_2[parent_id]->addChild(joint2);
            m_joints_final[parent_id]->addChild(joint3);
        }

        glm::vec3 pos;
        jointMat.getOffset(pos.x, pos.y, pos.z);
        joint1->transform.pos = pos;
        joint2->transform.pos = pos;
        joint3->transform.pos = pos;

        m_joints_1.push_back(joint1);
        m_joints_2.push_back(joint2);
        m_joints_final.push_back(joint3);
    }
    coliders.resize(m_joints_1.size() - 1);
    this->transform.scale = glm::vec3(0.05f);
    this->transform.pos = glm::vec3(4, -10, 3);
}

void SkeletonObj::init(std::string bvh_folder) {
    for (const auto &entry : std::filesystem::directory_iterator(bvh_folder)) m_bvh[m_bvh.size()] = BVH(entry.path());

    state = 0;
    for (int i = 0; i < m_bvh[0].getNumberOfJoint(); i++) {
        std::shared_ptr<SkeletonNode> joint1 = std::make_shared<SkeletonNode>();
        std::shared_ptr<SkeletonNode> joint2 = std::make_shared<SkeletonNode>();
        std::shared_ptr<SkeletonNode> joint3 = std::make_shared<SkeletonNode>();
        auto jointMat = m_bvh[0].getJoint(i);
        int parent_id = jointMat.getParentId();

        joint1->setId(i);
        joint2->setId(i);
        joint3->setId(i);
        if (parent_id == -1) {
            this->addChild(joint1);
            this->addChild(joint2);
            this->addChild(joint3);
        } else {
            m_joints_1[parent_id]->addChild(joint1);
            m_joints_2[parent_id]->addChild(joint2);
            m_joints_final[parent_id]->addChild(joint3);
        }

        glm::vec3 pos;
        jointMat.getOffset(pos.x, pos.y, pos.z);
        joint1->transform.pos = pos;
        joint2->transform.pos = pos;
        joint3->transform.pos = pos;

        m_joints_1.push_back(joint1);
        m_joints_2.push_back(joint2);
        m_joints_final.push_back(joint3);
    }
    coliders.resize(m_joints_1.size() - 1);
    this->transform.scale = glm::vec3(0.20f);
    this->transform.pos = glm::vec3(4, -10, 3);
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

glm::quat eulerZXYtoQuat(glm::vec3 euler) {
    // Convertir les angles d'Euler en quaternion
    float c1 = cos(euler.x / 2);
    float c2 = cos(euler.y / 2);
    float c3 = cos(euler.z / 2);

    float s1 = sin(euler.x / 2);
    float s2 = sin(euler.y / 2);
    float s3 = sin(euler.z / 2);

    glm::quat q = {c1 * c2 * c3 - s1 * s2 * s3, s1 * c2 * c3 - c1 * s2 * s3, c1 * s2 * c3 + s1 * c2 * s3, c1 * c2 * s3 + s1 * s2 * c3};

    return q;  // Ordre de multiplication ZXY
}

glm::quat eulerZYXtoQuat(glm::vec3 euler) {
    // Convertir les angles d'Euler en quaternion
    float c1 = cos(euler.x / 2);
    float c2 = cos(euler.y / 2);
    float c3 = cos(euler.z / 2);

    float s1 = sin(euler.x / 2);
    float s2 = sin(euler.y / 2);
    float s3 = sin(euler.z / 2);

    glm::quat q = {c1 * c2 * c3 + s1 * s2 * s3, s1 * c2 * c3 - c1 * s2 * s3, c1 * s2 * c3 + s1 * c2 * s3, c1 * c2 * s3 - s1 * s2 * c3};

    return q;  // Ordre de multiplication ZXY
}

glm::vec3 threeaxisrot(double r11, double r12, double r21, double r31, double r32) {
    return glm::vec3(asin(r21), atan2(r31, r32), atan2(r11, r12));
}

glm::vec3 threeaxisrot2(double r11, double r12, double r21, double r31, double r32) {
    return glm::vec3(atan2(r31, r32), asin(r21), atan2(r11, r12));
}

glm::vec3 quatToEulerZXY(glm::quat q) {
    return threeaxisrot(
        -2 * (q.x * q.y - q.w * q.z), q.w * q.w - q.x * q.x + q.y * q.y - q.z * q.z, 2 * (q.y * q.z + q.w * q.x),
        -2 * (q.x * q.z - q.w * q.y), q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z);
}

glm::vec3 quatToEulerZYX(glm::quat q) {
    return threeaxisrot2(
        2 * (q.x * q.y + q.w * q.z), q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z, -2 * (q.x * q.z - q.w * q.y),
        2 * (q.y * q.z + q.w * q.x), q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z);
}

void SkeletonObj::simulation(
    glm::vec3 gravite, float viscosite, uint32_t tick, float dt, float t, std::vector<std::shared_ptr<ICollider>> &collisionObjects) {
    float time = t / 0.083333;

    interpol = time - floor(time);
    // std::cout << "interpol: " << interpol << "\n";
    int frameNB = (int(floor(time)) % m_bvh[state].getNumberOfFrame());
    int frameNB2 = (int(ceil(time)) % m_bvh[state].getNumberOfFrame());

    for (int i = 0; i < m_bvh[state].getNumberOfJoint(); i++) {
        auto jointMat = m_bvh[state].getJoint(i);
        if (lastFrame != frameNB) {
            if (i == m_bvh[state].getNumberOfJoint() - 1) lastFrame = frameNB;
            glm::vec3 pos;
            jointMat.getOffset(pos.x, pos.y, pos.z);

            glm::vec3 rot1 = glm::vec3(0.0f);
            glm::vec3 trans1 = glm::vec3(0.0f);
            glm::vec3 rot2 = glm::vec3(0.0f);
            glm::vec3 trans2 = glm::vec3(0.0f);

            for (int j = 0; j < jointMat.getNumberOfChannel(); ++j) {
                auto channel = jointMat.getChannel(j);

                float data1 = channel.getData(frameNB);
                float data2 = channel.getData(frameNB2);

                if (channel.getType() == BVHChannel::TYPE_TRANSLATION) {
                    switch (channel.getAxis()) {
                        case AXIS_X:
                            trans1.x = data1;
                            trans2.x = data2;
                            break;
                        case AXIS_Y:
                            trans1.y = data1;
                            trans2.y = data2;
                            break;
                        case AXIS_Z:
                            trans1.z = data1;
                            trans2.z = data2;
                            break;
                        case AXIS_W:;
                            break;
                    }
                }
                if (channel.getType() == BVHChannel::TYPE_ROTATION) {
                    switch (channel.getAxis()) {
                        case AXIS_X:
                            rot1.x = glm::radians(data1);
                            rot2.x = glm::radians(data2);
                            break;
                        case AXIS_Y:
                            rot1.y = glm::radians(data1);
                            rot2.y = glm::radians(data2);
                            break;
                        case AXIS_Z:
                            rot1.z = glm::radians(data1);
                            rot2.z = glm::radians(data2);
                            break;
                        case AXIS_W:;
                            break;
                    }
                }
            }

            // CollisionObject c(CollisionObject::Type::sphere);

            m_joints_1[i]->transform.pos = pos + trans1;
            m_joints_1[i]->transform.rot = rot1;

            m_joints_2[i]->transform.pos = pos + trans2;
            m_joints_2[i]->transform.rot = rot2;
        }

        m_joints_final[i]->transform.pos = glm::mix(m_joints_1[i]->transform.pos.value, m_joints_2[i]->transform.pos.value, interpol);
        glm::quat q1 = eulerZXYtoQuat(m_joints_1[i]->transform.rot.value);
        glm::quat q2 = eulerZXYtoQuat(m_joints_2[i]->transform.rot.value);

        glm::quat q = glm::slerp(q1, q2, interpol);
        m_joints_final[i]->transform.rot = quatToEulerZXY(q);

        if (m_joints_final[i]->id != 0) {
            glm::vec3 jpos = m_joints_final[i]->wMatrix()[3];
            glm::vec3 ppos = m_joints_final[i]->parent->wMatrix()[3];
            coliders[i - 1].first = jpos;
            coliders[i - 1].second = ppos;
        }
    }
}

void SkeletonObj::render(CommandBuffer &cmd, GraphicPipeline &pipeline, std::vector<Mesh> &meshes, std::map<BasicShape, Mesh> basicMeshes) {
    // bind sphere
    basicMeshes[Sphere].bindMesh(cmd);

    for (int i = 0; i < m_joints_final.size(); i++) {
        glm::mat4 wMatrix = m_joints_final[i]->wMatrix() * glm::scale(glm::vec3(1.f));
        glm::mat4 wNormalMatrix = m_joints_final[i]->wNormalMatrix();

        // glm::quat quat1 = glm::toQuat(wMatrix);
        // glm::quat quat2 = glm::toQuat(wMatrix2);

        // glm::quat quatNormal = glm::toQuat(wNormalMatrix);
        // glm::quat quatNormal2 = glm::toQuat(wNormalMatrix2);

        // quat1 = glm::slerp(quat1, quat2, interpol);
        // quatNormal = glm::slerp(quatNormal, quatNormal2, interpol);

        // wMatrix = glm::toMat4(quat1);
        // wNormalMatrix = glm::toMat4(quat2);

        // push constant
        vkCmdPushConstants(cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), 0, sizeof(glm::mat4), &wMatrix);
        vkCmdPushConstants(
            cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), sizeof(glm::mat4), sizeof(glm::mat4), &wNormalMatrix);
        vkCmdDrawIndexed(cmd, basicMeshes[Sphere].nbIndicies(), 1, 0, 0, 0);
    }

    basicMeshes[Cube].bindMesh(cmd);
    for (auto &joint : m_joints_final) {
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
            glm::mat4 wMatrix = glm::translate(glm::mat4(1.0f), position) * glm::toMat4(rotation) *
                                glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, length));
            glm::mat4 wNormalMatrix = glm::inverseTranspose(glm::mat3(wMatrix));

            // push constant
            vkCmdPushConstants(cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), 0, sizeof(glm::mat4), &wMatrix);
            vkCmdPushConstants(
                cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), sizeof(glm::mat4), sizeof(glm::mat4), &wNormalMatrix);
            vkCmdDrawIndexed(cmd, basicMeshes[Cube].nbIndicies(), 1, 0, 0, 0);
        }
    }
}

float sdCapsule(glm::vec3 &p, glm::vec3 &a, glm::vec3 &b) {
    glm::vec3 pa = p - a, ba = b - a;
    float h = glm::clamp(glm::dot(pa, ba) / glm::dot(ba, ba), 0.0f, 1.0f);
    return length(pa - ba * h) - 0.25;
}

glm::vec3 closestPointToCapsule(glm::vec3 p, glm::vec3 a, glm::vec3 b, float r) {
    glm::vec3 pa = p - a, ba = b - a;
    float h = glm::clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
    glm::vec3 q = a + ba * h;
    return q + r * normalize(p - q);
}

void SkeletonObj::collisionPos(glm::vec3 &pos, glm::vec3 &vitesse) {
    for (auto &colider : coliders) {
        float dist = sdCapsule(pos, colider.first, colider.second);

        if (dist < 0) {
            pos = closestPointToCapsule(pos, colider.first, colider.second, 0.25f);

            vitesse = glm::vec3(0);
        }
    }
}

void SkeletonObj::updateFromInput(Window *window, float dt) {
    if (glfwGetKey(*window, keys.space) == GLFW_PRESS) {
        if (!keyPressed) {
            state = (state + 1) % m_bvh.size();
            lastFrame = 0;
            keyPressed = true;
        }
    } else {
        keyPressed = false;
    }
}
}  // namespace TTe