
#include "animaticObj.hpp"

namespace TTe {

void AnimaticObj::init(const BVH& bvh) {
    for(int i ; i < bvh.getNumberOfJoint(); i++) {
        SkeletonJoint joint;
        auto jointMat = bvh.getJoint(i);
        joint.m_parentId = jointMat.getParentId();
        
    }
}

glm::vec3 AnimaticObj::getJointPosition(int i) const {
    glm::vec3 returnValue;
    return returnValue;
}

int AnimaticObj::getParentId(const int i) const {
    int returnValue;
    return returnValue;
}

void AnimaticObj::setPose(const BVH& bvh, int frameNumber) {
}

}