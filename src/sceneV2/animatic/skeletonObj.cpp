
#include "skeletonObj.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <memory>
#include "sceneV2/mesh.hpp"

namespace TTe {

void SkeletonObj::init(BVH bvh) {
    for(int i = 0 ; i < bvh.getNumberOfJoint(); i++) {
        std::shared_ptr<SkeletonNode> joint = std::make_shared<SkeletonNode>();
        auto jointMat = bvh.getJoint(i);
        int parent_id = jointMat.getParentId();

        joint->setId(i);
        if(parent_id == -1) {
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

void SkeletonObj::setPose(const BVH& bvh, int frameNumber) {
}

void SkeletonObj::simulation(glm::vec3 gravite, float viscosite, int Tps, float dt, float t, std::vector<std::shared_ptr<ICollider>> &collisionObjects) {
}

void SkeletonObj::render(CommandBuffer &cmd, GraphicPipeline &pipeline, std::vector<Mesh> &meshes,  std::map<BasicShape, Mesh> basicMeshes) {
    //bind sphere
    basicMeshes[Sphere].bindMesh(cmd);
    
    for (auto &joint : m_joints) {
        glm::mat4 wMatrix = joint->wMatrix()* glm::scale(glm::vec3(4.f));
        glm::mat4 wNormalMatrix = joint->wNormalMatrix();

        //push constant
        vkCmdPushConstants(cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), 0, sizeof(glm::mat4), &wMatrix);
        vkCmdPushConstants(cmd, pipeline.getPipelineLayout(), pipeline.getPushConstantStage(), sizeof(glm::mat4), sizeof(glm::mat4), &wNormalMatrix);
        vkCmdDrawIndexed(cmd, basicMeshes[Sphere].nbIndicies(), 1, 0, 0, 0);
    }
}

}