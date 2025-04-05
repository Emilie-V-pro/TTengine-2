
#include "skeletonObj.hpp"

#include <cstdio>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/geometric.hpp>

#include "sceneV2/animatic/skeleton/BVHAxis.h"
#include "sceneV2/animatic/skeleton/BVHChannel.h"
#include "sceneV2/collision/collision_obj.hpp"

#define GLM_ENABLE_EXPERIMENTAL
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
    m_bvh = bvh;
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
    this->transform.scale = glm::vec3(0.05f);
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

glm::vec3 quatToEulerZXY(const glm::quat &q) {
    // Convertir le quaternion en matrice 4x4
    glm::mat4 mat = glm::mat4_cast(q);

    // Extraction des angles selon l'ordre ZXY
    float x, y, z;

    // Éviter la singularité (Gimbal Lock)
    if (std::abs(mat[1][2]) < 0.9999f) {
        x = std::asin(-mat[1][2]);             // Rotation autour de X
        y = std::atan2(mat[0][2], mat[2][2]);  // Rotation autour de Y
        z = std::atan2(mat[1][0], mat[1][1]);  // Rotation autour de Z
    } else {
        // Cas proche du Gimbal Lock
        x = (mat[1][2] > 0) ? -glm::half_pi<float>() : glm::half_pi<float>();
        y = std::atan2(-mat[0][1], mat[0][0]);
        z = 0.0f;
    }

    return glm::vec3(x, y, z);
}

void SkeletonObj::simulation(
    glm::vec3 gravite, float viscosite, uint32_t tick, float dt, float t, std::vector<std::shared_ptr<ICollider>> &collisionObjects) {
    coliders.clear();
    for (int i = 0; i < m_bvh.getNumberOfJoint(); i++) {
        auto jointMat = m_bvh.getJoint(i);

        glm::vec3 pos;
        jointMat.getOffset(pos.x, pos.y, pos.z);

        glm::vec3 rot1 = glm::vec3(0.0f);
        glm::vec3 trans1 = glm::vec3(0.0f);
        glm::vec3 rot2 = glm::vec3(0.0f);
        glm::vec3 trans2 = glm::vec3(0.0f);

        interpol = fmod((t / 0.0333333 /2.0), double(m_bvh.getNumberOfFrame())) - fmod(round(t / 0.0333333/2.0), double(m_bvh.getNumberOfFrame()));
        int frameNB = (int(round(t / 0.0333333/2.0)) % m_bvh.getNumberOfFrame());
        int frameNB2 = (int(floor(t / 0.0333333/2.0)) % m_bvh.getNumberOfFrame());

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

        CollisionObject c(CollisionObject::Type::sphere);

        m_joints_1[i]->transform.pos = pos + trans1;
        m_joints_1[i]->transform.rot = rot1;

        m_joints_2[i]->transform.pos = pos + trans2;
        m_joints_2[i]->transform.rot = rot2;

        c.transform.pos = m_joints_1[i]->transform.pos;
        c.transform.rot = m_joints_1[i]->transform.rot;
        c.transform.scale = glm::vec3(0.205f);

        // coliders.push_back(c);

        // m_joints_final[i]->transform.pos = glm::mix(m_joints_1[i]->transform.pos.value, m_joints_2[i]->transform.pos.value, interpol);

        // auto mat1 = glm::eulerAngleZXY(
        //     m_joints_1[i]->transform.rot.value.z, m_joints_1[i]->transform.rot.value.x, m_joints_1[i]->transform.rot.value.y);
        // auto mat2 = glm::eulerAngleZXY(
        //     m_joints_2[i]->transform.rot.value.z, m_joints_2[i]->transform.rot.value.x, m_joints_2[i]->transform.rot.value.y);

        // glm::quat q1 = glm::quat_cast(mat1);
        // glm::quat q2 = glm::quat_cast(mat2);

        // q1 = glm::slerp(q1, q2, interpol);

        // auto res = quatToEulerZXY(q1);
        // m_joints_final[i]->transform.rot = glm::vec3(res.x, res.y, res.z);
    }
    // for (auto &joint : m_joints_1) {
    //     // draw cube as line between joints
    //     glm::mat4 wPoint = joint->wMatrix();

    //     for (auto &child : joint->getChildren()) {
    //         glm::mat4 wPointChild = child->wMatrix();

    //         glm::vec3 pos = wPoint[3];
    //         glm::vec3 childPos = wPointChild[3];

    //         glm::vec3 position = (pos + childPos) * 0.5f;

    //         glm::vec3 dir = glm::normalize(childPos - pos);

    //         float length = glm::length(childPos - pos);
    //         glm::vec3 baseDir = glm::vec3(0.0f, 0.0f, 1.0f);
    //         // Direction de base du cube
    //         if (glm::length(glm::cross(baseDir, dir)) < 0.0001f) {
    //             // Si dir est parallèle à baseDir, éviter NaN (cas particulier)
    //             baseDir = glm::vec3(0.0f, 1.0f, 0.0f);
    //         }

    //         glm::vec3 axis = glm::normalize(glm::cross(baseDir, dir));
    //         float angle = acos(glm::dot(baseDir, dir));

    //         glm::quat rotation = glm::angleAxis(angle, axis);

    //         // Calculer la matrice de transformation
    //         glm::mat4 wMatrix = glm::translate(glm::mat4(1.0f), position) * glm::toMat4(rotation) *
    //                             glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, length / 2));

    //         CollisionObject c(CollisionObject::Type::cube);
    //         c.dirty = false;
    //         c.worldMatrix = wMatrix;
    //         coliders.push_back(c);
    //     }
    // }
}

void SkeletonObj::render(CommandBuffer &cmd, GraphicPipeline &pipeline, std::vector<Mesh> &meshes, std::map<BasicShape, Mesh> basicMeshes) {
    // bind sphere
    basicMeshes[Sphere].bindMesh(cmd);

    for (int i = 0; i < m_joints_1.size(); i++) {
        glm::mat4 wMatrix = m_joints_1[i]->wMatrix() * glm::scale(glm::vec3(4.f));
        glm::mat4 wNormalMatrix = m_joints_1[i]->wNormalMatrix();

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
    for (auto &joint : m_joints_1) {
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

void SkeletonObj::collisionPos(glm::vec3 &pos, glm::vec3 &vitesse) {
    for (auto &colider : coliders) {
        colider.collisionPos(pos, vitesse);
    }
}

}  // namespace TTe