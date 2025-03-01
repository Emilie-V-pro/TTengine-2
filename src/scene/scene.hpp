#pragma once

#include <glm/fwd.hpp>
#include <utility>
#include <vector>

#include "../utils.hpp"
#include "GPU_data/buffer.hpp"
#include "GPU_data/image.hpp"
#include "camera.hpp"
#include "commandBuffer/command_buffer.hpp"
#include "descriptor/descriptorSet.hpp"
#include "mesh.hpp"
#include "scene/object.hpp"
#include "scene/objects/animatic/BVH.h"
#include "shader/pipeline/graphic_pipeline.hpp"

namespace TTe {

class Scene {
   public:
    Scene() {};
    Scene(Device *device);
    Camera camera;

    void render(CommandBuffer &cmd);
    void updateBuffer();
    void updateCameraBuffer();

    void addBVH(BVH& bvh);

    std::vector<Object> objects;

    std::vector<std::pair<std::vector<Object>, BVH>> animaticOBJ; 
    
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::vector<Image> textures;
    void createDescriptorSets();
    
   private:

    glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f);

    GraphicPipeline pipeline;
    GraphicPipeline backgroundPipeline;


    DescriptorSet sceneDescriptorSet;

    DescriptorSet backgroundDescriptorSet;

    Image cubeTexture;

    Mesh sphere;

    Buffer CameraBuffer;
    Buffer ObjectBuffer;
    Buffer MaterialBuffer;

    

    Device *device = nullptr;
    
    // std::vector<Light> lights;
  
};

}