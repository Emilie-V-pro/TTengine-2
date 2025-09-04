#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <glm/fwd.hpp>
#include <map>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "GPU_data/buffer.hpp"
#include "GPU_data/image.hpp"
#include "descriptor/descriptorSet.hpp"
#include "device.hpp"

#include "dynamic_renderpass.hpp"
#include "sceneV2/IIndirectRenderable.hpp"
#include "sceneV2/Ianimatic.hpp"
#include "sceneV2/Icollider.hpp"
#include "sceneV2/animatic/skeletonObj.hpp"
#include "sceneV2/loader/gltf_loader.hpp"
#include "sceneV2/mesh.hpp"
#include "sceneV2/IRenderable.hpp"
#include "sceneV2/cameraV2.hpp"
#include "sceneV2/node.hpp"
#include "shader/pipeline/compute_pipeline.hpp"
#include "shader/pipeline/graphic_pipeline.hpp"
#include "struct.hpp"
#include "utils.hpp"

namespace TTe {
class Scene : public Node {
   public:
    Scene(){};
    Scene(Device *device);
    void initSceneData(DynamicRenderPass* defferedRenderpass, DynamicRenderPass* shadingRenderPass, std::filesystem::path skyboxPath = "textures/skybox");
    ~Scene();

    //copy constructor
    Scene( Scene &&scene){
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
    Scene &operator=( Scene &&scene){
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

    void Param(std::filesystem::path Fichier_Param);

    void renderDeffered(CommandBuffer &cmd, RenderData &renderData);
    void renderShading(CommandBuffer &cmd, RenderData &renderData);
    

    void updateSim(float dt, float t,  uint32_t tick);
    void updateFromInput(Window *window, float dt);

    uint32_t addNode(uint32_t Parent_id, std::shared_ptr<Node> node);
    void removeNode(uint32_t id);

    uint32_t addMaterial(Material material);

    void addStaticMesh(Mesh &mesh);

    uint32_t addImage(Image &image);


    std::shared_ptr<CameraV2> getMainCamera() { return mainCamera; }

    std::shared_ptr<Node> getNode(uint32_t id) { return objects[id]; }

    std::vector<Material>& getMaterials() { return materials; }

    void updateCameraBuffer(uint32_t frameIndex = 0);
    void updateMaterialBuffer();
    void updateObjectBuffer();
    void updateDescriptorSets();
    void updateRenderPassDescriptorSets();
    
    
    uint32_t firstIndexAvailable = 0;
    uint32_t firstVertexAvailable = 0;
    std::map<uint32_t, Mesh> meshes {};
    uint32_t nb_meshes = 0;
    std::vector<Image> images {};
   private:
   
   
   void createDrawIndirectBuffers();
   void createPipelines();
   void createDescriptorSets();
   
   std::map<Mesh::BasicShape, Mesh*> basicMeshes {};
   std::vector<Material> materials{};
   
   Image skyboxImage;
   
   DescriptorSet sceneDescriptorSet;
   std::vector<DescriptorSet> deferreDescriptorSet;

   DynamicRenderPass* defferedRenderpass;
   DynamicRenderPass* shadingRenderPass;
   
   std::array<Buffer, MAX_FRAMES_IN_FLIGHT> cameraBuffer;
   Buffer materialBuffer;
   Buffer objectBuffer;


   std::array<Buffer, MAX_FRAMES_IN_FLIGHT> drawIndirectBuffers;
    std::array<Buffer, MAX_FRAMES_IN_FLIGHT> countIndirectBuffers;
   
   std::shared_ptr<CameraV2> mainCamera;
   std::vector<std::shared_ptr<CameraV2>> cameras{};
   std::vector<std::shared_ptr<IAnimatic>> animaticObjs;
   std::vector<std::shared_ptr<IRenderable>> renderables;
   std::vector<std::shared_ptr<IIndirectRenderable>> indirectRenderables;
   std::vector<std::shared_ptr<ICollider>> collisionObjects;
   std::vector<std::shared_ptr<IInputController>> controlledObjects;

   
   std::unordered_map<uint32_t, std::shared_ptr<Node>> objects;
   
   std::vector<uint32_t> freeIDs;
   int nextID = 1;
   uint32_t getNewID();
   
   GraphicPipeline skyboxPipeline;
   ComputePipeline shadingPipeline;
   GraphicPipeline meshPipeline;
   
   Device *device = nullptr;

   Buffer indexBuffer;
   Buffer vertexBuffer;


   Buffer testBuffer;
   
   // for physics simulation
   glm::vec3 gravity{0.0f, -9.81f, 0.0f};
   int _nb_iter;
   float _visco;

   friend class GLTFLoader;
};
}  // namespace TTe