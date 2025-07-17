
#include "skeletonObj.hpp"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <string>

#include "math/quaternion_convertor.hpp"
#include "sceneV2/Irenderable.hpp"
#include "sceneV2/animatic/skeleton/BVHAxis.h"
#include "sceneV2/animatic/skeleton/BVHChannel.h"

#include <filesystem>

#include <memory>

#include "sceneV2/mesh.hpp"

namespace TTe {

void SkeletonObj::init(BVH bvh) {
    m_bvh[State::IDLE] = bvh;
    state = State::IDLE;
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
    coliders.resize(m_joints_1.size() - 6);
    this->transform.scale = glm::vec3(0.05f);
    this->transform.pos = glm::vec3(4, -10, 3);
}

void SkeletonObj::init(std::string bvh_folder) {
    for (const auto &entry : std::filesystem::directory_iterator(bvh_folder)) {
        if (entry.path().filename().string().find("talk") != std::string::npos) {
            m_bvh[State::IDLE] = BVH(entry.path(), true);
        }
        if (entry.path().filename().string().find("walk") != std::string::npos) {
            m_bvh[State::WALK] = BVH(entry.path(), true);
        }
        if (entry.path().filename().string().find("run") != std::string::npos) {
            m_bvh[State::RUN] = BVH(entry.path(), true);
        }
        if (entry.path().filename().string().find("kick") != std::string::npos) {
            m_bvh[State::KICK] = BVH(entry.path(), true);
        }

        std::cout << entry.path() << std::endl;
    }

    state = State::IDLE;
    nextState = State::IDLE;
    for (int i = 0; i < m_bvh[State::IDLE].getNumberOfJoint(); i++) {
        std::shared_ptr<SkeletonNode> joint1 = std::make_shared<SkeletonNode>();
        std::shared_ptr<SkeletonNode> joint2 = std::make_shared<SkeletonNode>();
        std::shared_ptr<SkeletonNode> joint3 = std::make_shared<SkeletonNode>();
        auto jointMat = m_bvh[State::IDLE].getJoint(i);

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
    coliders.resize(m_joints_1.size() - 6);
    this->transform.scale = glm::vec3(0.10);
    this->transform.pos = glm::vec3(4, -10, 3);

    // create graph

    for (auto &bvh : m_bvh) {
        for (int i = 0; i < bvh.second.getNumberOfFrame(); i++) {
            for (auto &other_bvh : m_bvh) {
                if (bvh.first != other_bvh.first) {
                    FrameTransition frameTransition = {-1, -1};
                    float minDistance = 25.f;
                    for (int j = 0; j < other_bvh.second.getNumberOfFrame(); j++) {
                        float dist = computePoseDistance(i, j, bvh.second, other_bvh.second);
                        if (dist < minDistance) {
                            minDistance = dist;
                            frameTransition.frame_1 = i;
                            frameTransition.frame_2 = j;
                        }
                    }
                    if (frameTransition.frame_1 != -1) {
                        if (bvh.first == State::KICK) {
                            if (i == bvh.second.getNumberOfFrame() - 1) {
                                frameTransition.frame_1 = 0;
                                transitions_graph[{bvh.first, other_bvh.first}].push_back(frameTransition);
                            }
                        } else {
                            transitions_graph[{bvh.first, other_bvh.first}].push_back(frameTransition);
                        }
                    }
                }
            }
        }
    }

    int test = 0;
}

float SkeletonObj::computePoseDistance(int frame_1, int frame_2, BVH &bvh_1, BVH &bvh_2) {
    float returnValue = 0;
    for (int i = 0; i < m_bvh[state].getNumberOfJoint(); i++) {
        auto jointMat1 = bvh_1.getJoint(i);
        auto jointMat2 = bvh_2.getJoint(i);

        glm::vec3 pos;
        jointMat1.getOffset(pos.x, pos.y, pos.z);

        glm::vec3 rot1 = glm::vec3(0.0f);
        glm::vec3 trans1 = glm::vec3(0.0f);
        glm::vec3 rot2 = glm::vec3(0.0f);
        glm::vec3 trans2 = glm::vec3(0.0f);

        for (int j = 0; j < jointMat1.getNumberOfChannel(); ++j) {
            auto channel_1 = jointMat1.getChannel(j);

            float data1 = channel_1.getData(frame_1);

            if (channel_1.getType() == BVHChannel::TYPE_TRANSLATION) {
                switch (channel_1.getAxis()) {
                    case AXIS_X:
                        trans1.x = data1;
                        break;
                    case AXIS_Y:
                        trans1.y = data1;
                        break;
                    case AXIS_Z:
                        trans1.z = data1;
                        break;
                    case AXIS_W:;
                        break;
                }
            }
            if (channel_1.getType() == BVHChannel::TYPE_ROTATION) {
                switch (channel_1.getAxis()) {
                    case AXIS_X:
                        rot1.x = glm::radians(data1);
                        break;
                    case AXIS_Y:
                        rot1.y = glm::radians(data1);
                        break;
                    case AXIS_Z:
                        rot1.z = glm::radians(data1);
                        break;
                    case AXIS_W:;
                        break;
                }
            }
        }

        for (int j = 0; j < jointMat1.getNumberOfChannel(); ++j) {
            auto channel_2 = jointMat2.getChannel(j);

            float data2 = channel_2.getData(frame_2);

            if (channel_2.getType() == BVHChannel::TYPE_TRANSLATION) {
                switch (channel_2.getAxis()) {
                    case AXIS_X:
                        trans2.x = data2;
                        break;
                    case AXIS_Y:
                        trans2.y = data2;
                        break;
                    case AXIS_Z:
                        trans2.z = data2;
                        break;
                    case AXIS_W:;
                        break;
                }
            }
            if (channel_2.getType() == BVHChannel::TYPE_ROTATION) {
                switch (channel_2.getAxis()) {
                    case AXIS_X:
                        rot2.x = glm::radians(data2);
                        break;
                    case AXIS_Y:
                        rot2.y = glm::radians(data2);
                        break;
                    case AXIS_Z:
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

        glm::vec3 pos_1 = m_joints_1[i]->wMatrix()[3];
        glm::vec3 pos_2 = m_joints_2[i]->wMatrix()[3];
        returnValue += glm::length(pos_1 - pos_2);
    }
    return returnValue;
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
    float time = t / 0.081667;

    interpol = time - floor(time);
    // std::cout << "interpol: " << interpol << "\n";
    int frameNB = (int(floor(time) + frameOffset) % m_bvh[state].getNumberOfFrame());
    int frameNB2;
    State secon_State;
    if (transition && frameNB == startTransitionFrame + 1) {
        if (state == State::KICK) {
            kickend = true;
        }
        transition = false;
        wantTransition = false;
        state = nextState;
        frameOffset = nextStateFrameOffset;
        startTransitionFrame = -1;
    }

    if (frameNB == startTransitionFrame) {
        transition = true;
        if (nextState == State::KICK) {
            kickend = false;
        }
    }

    if (transition) {
        frameNB2 = (int(ceil(time) + nextStateFrameOffset) % m_bvh[nextState].getNumberOfFrame());
        secon_State = nextState;
    } else {
        frameNB2 = (int(ceil(time) + frameOffset) % m_bvh[state].getNumberOfFrame());
        secon_State = state;
    }

    int collider_iter = 0;

    for (int i = 0; i < m_bvh[state].getNumberOfJoint(); i++) {
        if (lastFrame != frameNB) {
            if (i == m_bvh[state].getNumberOfJoint() - 1) lastFrame = frameNB;

            auto jointMat1 = m_bvh[state].getJoint(i);
            auto jointMat2 = m_bvh[secon_State].getJoint(i);

            glm::vec3 pos;
            jointMat1.getOffset(pos.x, pos.y, pos.z);

            glm::vec3 rot1 = glm::vec3(0.0f);
            glm::vec3 trans1 = glm::vec3(0.0f);
            glm::vec3 rot2 = glm::vec3(0.0f);
            glm::vec3 trans2 = glm::vec3(0.0f);

            for (int j = 0; j < jointMat1.getNumberOfChannel(); ++j) {
                auto channel_1 = jointMat1.getChannel(j);

                float data1 = channel_1.getData(frameNB);

                if (channel_1.getType() == BVHChannel::TYPE_TRANSLATION) {
                    switch (channel_1.getAxis()) {
                        case AXIS_X:
                            trans1.x = data1;
                            break;
                        case AXIS_Y:
                            trans1.y = data1;
                            break;
                        case AXIS_Z:
                            trans1.z = data1;
                            break;
                        case AXIS_W:;
                            break;
                    }
                }
                if (channel_1.getType() == BVHChannel::TYPE_ROTATION) {
                    switch (channel_1.getAxis()) {
                        case AXIS_X:
                            rot1.x = glm::radians(data1);
                            break;
                        case AXIS_Y:
                            rot1.y = glm::radians(data1);
                            break;
                        case AXIS_Z:
                            rot1.z = glm::radians(data1);
                            break;
                        case AXIS_W:;
                            break;
                    }
                }
            }

            for (int j = 0; j < jointMat1.getNumberOfChannel(); ++j) {
                auto channel_2 = jointMat2.getChannel(j);

                float data2 = channel_2.getData(frameNB2);

                if (channel_2.getType() == BVHChannel::TYPE_TRANSLATION) {
                    switch (channel_2.getAxis()) {
                        case AXIS_X:
                            trans2.x = data2;
                            break;
                        case AXIS_Y:
                            trans2.y = data2;
                            break;
                        case AXIS_Z:
                            trans2.z = data2;
                            break;
                        case AXIS_W:;
                            break;
                    }
                }
                if (channel_2.getType() == BVHChannel::TYPE_ROTATION) {
                    switch (channel_2.getAxis()) {
                        case AXIS_X:
                            rot2.x = glm::radians(data2);
                            break;
                        case AXIS_Y:
                            rot2.y = glm::radians(data2);
                            break;
                        case AXIS_Z:
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
        // m_joints_final[i]->transform.rot = m_joints_1[i]->transform.rot;
        // m_joints_final[i]->transform.pos = m_joints_1[i]->transform.pos;

        if (m_joints_final[i]->id != 0) {
            if (m_joints_final[i]->children.size() == 0) {
                continue;
            }
            glm::vec3 jpos = m_joints_final[i]->wMatrix()[3];
            glm::vec3 ppos = m_joints_final[i]->parent->wMatrix()[3];
            coliders[collider_iter].first = jpos;
            coliders[collider_iter].second = ppos;
            collider_iter++;
        }
    }
}

void SkeletonObj::render(CommandBuffer &cmd, RenderData &renderData) {
    // bind sphere
    renderData.basicMeshes->at(Sphere).bindMesh(cmd);
    if (renderData.binded_pipeline != renderData.default_pipeline) {
        renderData.binded_pipeline->bindPipeline(cmd);
        renderData.binded_pipeline = renderData.default_pipeline;
    }
    for (int i = 0; i < m_joints_final.size(); i++) {
        PushConstantData pc = {m_joints_final[i]->wMatrix() * glm::scale(glm::vec3(1.3f)), m_joints_final[i]->wNormalMatrix(), renderData.portal_pos, renderData.cameraId, renderData.portal_normal};

        vkCmdPushConstants(cmd, renderData.binded_pipeline->getPipelineLayout(), renderData.binded_pipeline->getPushConstantStage(), 0, sizeof(PushConstantData), &pc);

        vkCmdDrawIndexed(cmd, renderData.basicMeshes->at(Sphere).nbIndicies(), 1, 0, 0, 0);
    }

    renderData.basicMeshes->at(Cube).bindMesh(cmd);
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
                                glm::scale(glm::mat4(1.0f), glm::vec3(0.14f, 0.14f, length));
            glm::mat4 wNormalMatrix = glm::inverseTranspose(glm::mat3(wMatrix));
            PushConstantData pc = {wMatrix, wNormalMatrix, renderData.portal_pos, renderData.cameraId, renderData.portal_normal};
            // push constant
            vkCmdPushConstants(cmd, renderData.binded_pipeline->getPipelineLayout(), renderData.binded_pipeline->getPushConstantStage(), 0, sizeof(PushConstantData), &pc);
            vkCmdDrawIndexed(cmd, renderData.basicMeshes->at(Cube).nbIndicies(), 1, 0, 0, 0);
        }
    }

    
}

float sdCapsule(glm::vec3 &p, glm::vec3 &a, glm::vec3 &b) {
    glm::vec3 pa = p - a, ba = b - a;
    float h = glm::clamp(glm::dot(pa, ba) / glm::dot(ba, ba), 0.0f, 1.0f);
    return length(pa - ba * h) - 0.19f;
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
            pos = closestPointToCapsule(pos, colider.first, colider.second, 0.19f);
            vitesse = glm::vec3(0.f);

            // vitesse = (closestPointToCapsule(pos, colider.first, colider.second, 0.1f) - pos) * 1000.f;
        }
    }
}

void SkeletonObj::updateFromInput(Window *window, float dt) {
    static bool stateChanged = false;
    if (glfwGetKey(*window, keys.space) == GLFW_PRESS) {
        if (!keyPressed) {
            // state = (state + 1) % m_bvh.size();
            // lastFrame = 0;
            keyPressed = true;
        }
    } else {
        keyPressed = false;
    }
    State wantedState = State::IDLE;

    if (glfwGetKey(*window, keys.lookUp) == GLFW_PRESS) {
        if (glfwGetKey(*window, keys.shift) == GLFW_PRESS) {
            speed = std::min(speed + accel * dt * 4, speed_max_run);
            wantedState = State::RUN;

        } else {
            speed = std::min(speed + accel * dt * 4, speed_max);
            wantedState = State::WALK;
        }

    } else {
        speed = std::max(speed - accel * dt * 10, 0.f);
        wantedState = State::IDLE;
    }

    if (glfwGetKey(*window, keys.lookLeft) == GLFW_PRESS) {
        transform.rot->y += dt * 4;
    }
    if (glfwGetKey(*window, keys.lookRight) == GLFW_PRESS) {
        transform.rot->y -= dt * 4;
    }

    if (glfwGetKey(*window, keys.space) == GLFW_PRESS) {
        wantedState = State::KICK;
        speed = 0.f;
    }

    if (!kickend) {
        speed = 0.f;
    }

    if (wantedState != state && (!wantTransition || wantedState != nextState) && (kickend || wantedState != State::KICK)) {
        wantTransition = true;

        nextState = wantedState;
        bool found = false;
        // find the closest frame to the last frame
        for (auto &frame : transitions_graph[{state, nextState}]) {
            if (frame.frame_1 > lastFrame) {
                startTransitionFrame = frame.frame_1;
                nextStateFrameOffset = frame.frame_2;
                found = true;
                break;
            }
        }
        if (!found) {
            startTransitionFrame = transitions_graph[{state, nextState}][0].frame_1;
            nextStateFrameOffset = transitions_graph[{state, nextState}][0].frame_2;
        }

    } else if (state == wantedState) {
        wantTransition = false;
        transition = false;
        nextState = wantedState;
        startTransitionFrame = -1;
    }

    const float yaw = transform.rot->y;
    // transform.rot.
    const float pitch = transform.rot->x;

    orientation = {std::sin(yaw) * std::cos(pitch), std::sin(pitch), std::cos(yaw) * std::cos(pitch)};

    transform.pos = transform.pos + orientation * speed * dt;

    // Position cible en fonction de la direction
}
}  // namespace TTe