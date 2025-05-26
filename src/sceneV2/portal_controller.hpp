#pragma once

#include <GLFW/glfw3.h>

#include "device.hpp"
#include "sceneV2/renderable/portalObj.hpp"
#include "sceneV2/scene.hpp"
#include "window.hpp"

namespace TTe {
class PortalController {
   public:
    PortalController(){};

    void init(Device *device, Scene2 *scene){
        this->device = device;
        this->scene = scene;




        // s2->setMaterialOffset(scene->addMaterial(mat));

        int id = scene->addNode(-1, std::make_shared<PortalObj>());
        portalObjA = std::dynamic_pointer_cast<PortalObj>(scene->getNode(id)).get();

        id = scene->addNode(-1, std::make_shared<PortalObj>());
        portalObjB = std::dynamic_pointer_cast<PortalObj>(scene->getNode(id)).get();


        portalObjA->transform.pos = glm::vec3(-5, 0, 0);
        portalObjB->transform.pos = glm::vec3(5, 0, 0);

        portalObjA->transform.rot = glm::vec3(M_PI / 2, M_PI / 2, 0);
        portalObjB->transform.rot = glm::vec3(M_PI / 2, M_PI / 2, 0);

        portalObjA->portalColor = glm::vec3(0, 0, 1);
        portalObjB->portalColor = glm::vec3(1, 0, 0);

        portalObjB->portalId = 1;


    }

    void setCursors(Window* window) {
        glfwSetCursorPosCallback(*window, mouseMoveCallback);
        glfwSetMouseButtonCallback(*window, mouseButtonCallback);
    }
    struct KeyMappings {
        int moveLeft = GLFW_KEY_A;
        int moveRight = GLFW_KEY_D;
        int moveForward = GLFW_KEY_W;
        int moveBackward = GLFW_KEY_S;
        int moveUp = GLFW_KEY_E;
        int moveDown = GLFW_KEY_Q;
        int lookLeft = GLFW_KEY_LEFT;
        int lookRight = GLFW_KEY_RIGHT;
        int lookUp = GLFW_KEY_UP;
        int lookDown = GLFW_KEY_DOWN;
        int space = GLFW_KEY_SPACE;
        int alt = GLFW_KEY_LEFT_ALT;
        int ct = GLFW_KEY_LEFT_CONTROL;
        int esc = GLFW_KEY_ESCAPE;
    };

    void moveInPlaneXZ(Window* window, float dt);
    KeyMappings keys{};

    static void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    bool waspressed = false;
    float moveSpeed{3.f};
    float lookSpeed{1.5f};


    bool focus = false;
    bool escPressed = false;


    bool leftClick = false;
    bool rightClick = false;


    Device* device;
    Scene2* scene;
    PortalObj* portalObjA;
    PortalObj* portalObjB;
};
}  // namespace TTe