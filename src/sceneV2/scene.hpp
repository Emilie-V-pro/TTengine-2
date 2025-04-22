#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "GPU_data/buffer.hpp"
#include "GPU_data/image.hpp"
#include "descriptor/descriptorSet.hpp"
#include "device.hpp"

#include "sceneV2/Ianimatic.hpp"
#include "sceneV2/Icollider.hpp"
#include "sceneV2/animatic/skeletonObj.hpp"
#include "sceneV2/mesh.hpp"
#include "sceneV2/Irenderable.hpp"
#include "sceneV2/cameraV2.hpp"
#include "sceneV2/i_object_file_loader.hpp"
#include "sceneV2/node.hpp"
#include "shader/pipeline/graphic_pipeline.hpp"
#include "struct.hpp"

namespace TTe {
class Scene2 : public Node {
   public:
    Scene2(){};
    Scene2(Device *device);
    ~Scene2();

    //copy constructor
    Scene2( Scene2 &&scene){
        this->device = scene.device;
        this->meshes = scene.meshes;
        this->basicMeshes = scene.basicMeshes;
        this->images = scene.images;
        this->materials = scene.materials;
        this->skyboxImage = scene.skyboxImage;
         this->sceneDescriptorSet = scene.sceneDescriptorSet;
        this->cameraBuffer = scene.cameraBuffer;
        this->materialBuffer = scene.materialBuffer;
        this->mainCamera = scene.mainCamera;
        this->cameras = scene.cameras;
        this->animaticObjs = scene.animaticObjs;
        this->renderables = scene.renderables;
        this->objects = scene.objects;
        this->freeIDs = scene.freeIDs;
        this->nextID = scene.nextID;
        this->skyboxPipeline = std::move(scene.skyboxPipeline);
        this->meshPipeline = std::move(scene.meshPipeline);
    };

    //copy assignment
    Scene2 &operator=( Scene2 &&scene){
        this->device = scene.device;
        this->meshes = scene.meshes;
        this->basicMeshes = scene.basicMeshes;
        this->images = scene.images;
        this->materials = scene.materials;
        this->skyboxImage = scene.skyboxImage;
        this->sceneDescriptorSet = scene.sceneDescriptorSet;
        this->cameraBuffer = scene.cameraBuffer;
        this->materialBuffer = scene.materialBuffer;
        this->mainCamera = scene.mainCamera;
        this->cameras = scene.cameras;
        this->animaticObjs = scene.animaticObjs;
        this->renderables = scene.renderables;
        this->objects = scene.objects;
        this->freeIDs = scene.freeIDs;
        this->nextID = scene.nextID;
        this->skyboxPipeline = std::move(scene.skyboxPipeline);
        this->meshPipeline = std::move(scene.meshPipeline);
        return *this;
    };

    void Param(std::string Fichier_Param);

    void render(CommandBuffer &cmd);
    void renderSkybox(CommandBuffer &cmd);

    void updateSim(float dt, float t,  uint32_t tick);
    void updateFromInput(Window *window, float dt);

    uint32_t addNode(uint32_t Parent_id, std::shared_ptr<Node> node);
    void removeNode(uint32_t id);

    uint32_t addMaterial(Material material);

    void addMesh(Mesh mesh);

    uint32_t addImage(Image image);

    void addObjectFileData(ObjectFileData &data);

    std::shared_ptr<CameraV2> getMainCamera() { return mainCamera; }

    std::shared_ptr<Node> getNode(uint32_t id) { return objects[id]; }

    std::vector<Material>& getMaterials() { return materials; }

    void updateCameraBuffer();
    void updateMaterialBuffer();
    void updateDescriptorSets();
   private:
    glm::vec3 gravity{0.0f, -9.81f, 0.0f};
    float _visco;

    

    void createPipelines();
    void createDescriptorSets();

    std::vector<Mesh> meshes;
    std::map<BasicShape, Mesh> basicMeshes;
    std::vector<Image> images;
    std::vector<Material> materials;

    Image skyboxImage;

    DescriptorSet sceneDescriptorSet;

    Buffer cameraBuffer;
    Buffer materialBuffer;

    std::shared_ptr<CameraV2> mainCamera;
    std::vector<std::shared_ptr<CameraV2>> cameras;
    std::vector<std::shared_ptr<IAnimatic>> animaticObjs;
    std::vector<std::shared_ptr<IRenderable>> renderables;
    std::vector<std::shared_ptr<ICollider>> collisionObjects;
    std::vector<std::shared_ptr<IInputController>> controlledObjects;
    std::unordered_map<uint32_t, std::shared_ptr<Node>> objects;

    std::vector<uint32_t> freeIDs;
    int nextID = 1;
    uint32_t getNewID();

    GraphicPipeline skyboxPipeline;
    GraphicPipeline meshPipeline;

    Device *device = nullptr;
    int _nb_iter;
};
}  // namespace TTe