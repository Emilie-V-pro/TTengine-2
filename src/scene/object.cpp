
#include "object.hpp"
#include <cmath>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/matrix.hpp>
#define GLM_FORCE_RADIANS 
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace TTe {

glm::mat4 Object::mat4() const {
    glm::mat4 scaleMatrix = glm::scale(scale);
    glm::mat4 translationMatrix = glm::translate(translation);
    glm::mat4 rotationMatrix = glm::eulerAngleXYZ(rotation.x, rotation.y, rotation.z);

    return translationMatrix * rotationMatrix  * scaleMatrix;
}

glm::mat3 Object::normalMatrix() const {
    return glm::inverseTranspose(glm::mat3(this->mat4()));
}
}