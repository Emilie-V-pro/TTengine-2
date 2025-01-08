#pragma once

#include <volk.h>
namespace TTe {
class Destroyable {
   public:
    virtual ~Destroyable() = default;
    virtual void destroy() = 0;
};
}