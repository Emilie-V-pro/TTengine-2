#pragma once


#include <array>
#include <cstdint>
#include <filesystem>
#include <glm/fwd.hpp>
#include <vector>


#include "GPU_data/buffer.hpp"
#include "GPU_data/image.hpp"
#include "descriptor/descriptorSet.hpp"
#include "device.hpp"
#include "dynamic_renderpass.hpp"
#include "sceneV2/IIndirectRenderable.hpp"
#include "sceneV2/IRenderable.hpp"
#include "sceneV2/Ianimatic.hpp"
#include "sceneV2/Icollider.hpp"
#include "sceneV2/animatic/skeletonObj.hpp"
#include "sceneV2/cameraV2.hpp"
#include "sceneV2/light.hpp"
#include "sceneV2/loader/gltf_loader.hpp"
#include "sceneV2/mesh.hpp"
#include "sceneV2/node.hpp"
#include "shader/pipeline/compute_pipeline.hpp"
#include "shader/pipeline/graphic_pipeline.hpp"
#include "struct.hpp"
#include "utils.hpp"

namespace TTe {
class Scene : public Node {
   public:
    Scene() {};
    Scene(Device *p_device);
    void initSceneData(
        DynamicRenderPass *p_deffered_renderpass, DynamicRenderPass *p_shading_renderpass, std::filesystem::path p_skybox_path = "textures/skybox");
    ~Scene();

    // copy constructor
    Scene(Scene &&other) = default;

    // copy assignment
    Scene &operator=(Scene &&other) = default;

    void Param(std::filesystem::path p_fichier_param);

    void renderDeffered(CommandBuffer &p_cmd, RenderData &p_render_data);
    void renderShading(CommandBuffer &p_cmd, RenderData &p_render_data);

    void updateSim(float p_dt, float p_t, uint32_t p_tick);
    void updateFromInput(Window *p_window, float p_dt);

    uint32_t addNode(uint32_t p_parent_id, std::shared_ptr<Node> p_node);
    void removeNode(uint32_t p_id);

    uint32_t addMaterial(Material p_material);

    void addStaticMesh(Mesh &p_mesh);

    uint32_t addImage(Image &p_image);

    std::shared_ptr<CameraV2> getMainCamera() { return m_main_camera; }

    std::shared_ptr<Node> getNode(uint32_t p_id) { return m_objects[p_id]; }

    std::vector<Material> &getMaterials() { return m_materials; }
    std::vector<std::shared_ptr<Light>> &getLights() { return m_light_objects; }



    void updateCameraBuffer(uint32_t p_frameIndex = 0);
    void updateMaterialBuffer();
    void updateObjectBuffer();
    void updateLightBuffer();
    void updateDescriptorSets();
    void updateRenderPassDescriptorSets();

    uint32_t first_index_available = 0;
    uint32_t first_vertex_available = 0;
    std::map<uint32_t, Mesh> meshes{};
    uint32_t nb_meshes = 0;
    std::vector<Image> images{};

    Buffer index_buffer;
    Buffer vertex_buffer;
    Buffer material_buffer;
    std::array<Buffer, MAX_FRAMES_IN_FLIGHT> camera_buffer;
    DescriptorSet scene_descriptor_set;

   private:
    void createDrawIndirectBuffers();
    void createPipelines();
    void createDescriptorSets();

    std::map<Mesh::BasicShape, Mesh *> m_basic_meshes{};
    std::vector<Material> m_materials{};

    Image m_skybox_image;

    
    std::vector<DescriptorSet> m_deferred_descriptor_set;

    DynamicRenderPass *m_deffered_renderpass;
    DynamicRenderPass *m_shading_renderpass;

    
    
    Buffer m_object_buffer;
    Buffer m_light_buffer;

    std::array<Buffer, MAX_FRAMES_IN_FLIGHT> m_draw_indirect_buffers;
    std::array<Buffer, MAX_FRAMES_IN_FLIGHT> m_count_indirect_buffers;

    Buffer m_mesh_block_buffer;
    uint m_total_mesh_block = 0;

    std::shared_ptr<CameraV2> m_main_camera;
    std::vector<std::shared_ptr<CameraV2>> m_cameras{};
    std::vector<std::shared_ptr<IAnimatic>> m_animatic_objs;
    std::vector<std::shared_ptr<IRenderable>> m_renderables;
    std::vector<std::shared_ptr<IIndirectRenderable>> m_indirect_renderables;
    std::vector<std::shared_ptr<ICollider>> m_collision_objects;
    std::vector<std::shared_ptr<Light>> m_light_objects;
    std::vector<std::shared_ptr<IInputController>> m_controlled_objects;

    std::unordered_map<uint32_t, std::shared_ptr<Node>> m_objects;

    std::vector<uint32_t> m_free_ids;
    int m_next_id = 1;
    uint32_t getNewID();

    GraphicPipeline m_skybox_pipeline;
    ComputePipeline m_shading_pipeline;
    GraphicPipeline m_mesh_pipeline;

    ComputePipeline m_cull_pipeline;

    Device *m_device = nullptr;

    // for physics simulation
    glm::vec3 m_gravity{0.0f, -9.81f, 0.0f};
    int m_nb_iter;
    float m_visco;

    friend class GLTFLoader;
};
}  // namespace TTe