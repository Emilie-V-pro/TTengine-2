#pragma once

#include <glm/fwd.hpp>
namespace TTe {
class ICollider {
   public:
    virtual void hit(glm::vec3 &ro, glm::vec3 &rd) = 0;

   private:
   protected:
};
}  // namespace TTe