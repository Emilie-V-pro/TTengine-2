#include <GLFW/glfw3.h>
#include "window.hpp"

namespace TTe {
class IInputController {
    public:
    struct KeyMappings {
        int move_left = GLFW_KEY_A;
        int move_right = GLFW_KEY_D;
        int move_forward = GLFW_KEY_W;
        int move_backward = GLFW_KEY_S;
        int move_up = GLFW_KEY_E;
        int move_down = GLFW_KEY_Q;
        int look_left = GLFW_KEY_LEFT;
        int look_right = GLFW_KEY_RIGHT;
        int look_up = GLFW_KEY_UP;
        int look_down = GLFW_KEY_DOWN;
        int space = GLFW_KEY_SPACE;
        int alt = GLFW_KEY_LEFT_ALT;
        int shift = GLFW_KEY_LEFT_SHIFT;
    };

    virtual void updateFromInput(Window* window, float dt) = 0;

    KeyMappings keys{};
};
}  // namespace TTe