
#include "scene.hpp"

#include <cstddef>
#include <cstdint>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/matrix.hpp>
#include <memory>
#include <vector>

#include "GPU_data/buffer.hpp"
#include "GPU_data/image.hpp"
#include "commandBuffer/command_buffer.hpp"
#include "descriptor/descriptorSet.hpp"
#include "device.hpp"
#include "sceneV2/IIndirectRenderable.hpp"
#include "sceneV2/IRenderable.hpp"
#include "sceneV2/Icollider.hpp"
#include "sceneV2/animatic/skeletonObj.hpp"
#include "sceneV2/cameraV2.hpp"
#include "sceneV2/light.hpp"
#include "sceneV2/mesh.hpp"
#include "sceneV2/render_data.hpp"
#include "shader/pipeline/compute_pipeline.hpp"
#include "struct.hpp"

namespace TTe {

Scene::Scene(Device* p_device) : m_device(p_device) {
    createPipelines();
    createDescriptorSets();
}

Scene::~Scene() {}

void Scene::initSceneData(
    DynamicRenderPass* p_deffered_renderpass, DynamicRenderPass* p_shading_renderpass, std::filesystem::path p_skybox_path) {
    this->m_deffered_renderpass = p_deffered_renderpass;
    this->m_shading_renderpass = p_shading_renderpass;
    ImageCreateInfo skybox_image_create_info;
    skybox_image_create_info.filename.push_back(p_skybox_path / "posx.jpg");
    skybox_image_create_info.filename.push_back(p_skybox_path / "negx.jpg");
    skybox_image_create_info.filename.push_back(p_skybox_path / "posy.jpg");
    skybox_image_create_info.filename.push_back(p_skybox_path / "negy.jpg");
    skybox_image_create_info.filename.push_back(p_skybox_path / "posz.jpg");
    skybox_image_create_info.filename.push_back(p_skybox_path / "negz.jpg");
    skybox_image_create_info.usage_flags = VK_IMAGE_USAGE_SAMPLED_BIT;
    skybox_image_create_info.is_cube_texture = true;
    skybox_image_create_info.enable_mipmap = true;
    skybox_image_create_info.image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    m_skybox_image = Image(m_device, skybox_image_create_info);

    Mesh cube_mesh(m_device, Mesh::BasicShape::Cube, 1);
    addStaticMesh(cube_mesh);
    m_basic_meshes[Mesh::BasicShape::Cube] = &meshes.at(nb_meshes - 1);

    // Mesh sphereMesh(m_device, Mesh::BasicShape::Sphere, 1);
    // addStaticMesh(sphereMesh);
    // m_basic_meshes[Mesh::BasicShape::Sphere] = &meshes.at(nb_meshes - 1);

    // Mesh planeMesh(m_device, Mesh::BasicShape::Plane, 1);
    // addStaticMesh(planeMesh);
    // m_basic_meshes[Mesh::BasicShape::Plane] = &meshes.back();

    vkDeviceWaitIdle(*m_device);

    if (m_cameras.size() == 0) {
        addNode(-1, std::make_shared<CameraV2>());
    }
    createDrawIndirectBuffers();
    updateCameraBuffer();
    updateObjectBuffer();
    updateMaterialBuffer();
    updateLightBuffer();
    updateDescriptorSets();
    updateRenderPassDescriptorSets();
}

void Scene::renderDeffered(CommandBuffer& p_cmd, RenderData& p_render_data) {
    p_render_data.basic_meshes = m_basic_meshes;
    p_render_data.cameras = &m_cameras;
    m_skybox_pipeline.bindPipeline(p_cmd);
    std::vector<DescriptorSet*> descriptor_sets = {&scene_descriptor_set};

    DescriptorSet::bindDescriptorSet(p_cmd, descriptor_sets, m_skybox_pipeline.getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS);

    m_basic_meshes[Mesh::Cube]->bindMesh(p_cmd);
    p_render_data.render_pass->setDepthAndStencil(p_cmd, false);

    PushConstantStruct tp{
        m_object_buffer.getBufferDeviceAddress(),
        material_buffer.getBufferDeviceAddress(),
        camera_buffer[p_render_data.frame_index].getBufferDeviceAddress(),
        m_light_buffer.getBufferDeviceAddress(),
        0,
        static_cast<uint32_t>(m_light_objects.size())};
    p_render_data.push_constant = tp;
    // // set push_constant for cam_id
    vkCmdPushConstants(
        p_cmd, m_skybox_pipeline.getPipelineLayout(), m_skybox_pipeline.getPushConstantStage(), 0, sizeof(PushConstantStruct), &tp);

    vkCmdSetCullMode(p_cmd, VK_CULL_MODE_NONE);
    vkCmdDrawIndexed(
        p_cmd, m_basic_meshes[Mesh::Cube]->nbIndicies(), 1, m_basic_meshes[Mesh::Cube]->getFirstIndex(),
        m_basic_meshes[Mesh::Cube]->getFirstVertex(), 0);
    vkCmdSetCullMode(p_cmd, VK_CULL_MODE_BACK_BIT);
    p_render_data.render_pass->setDepthAndStencil(p_cmd, true);
    ;

    m_mesh_pipeline.bindPipeline(p_cmd);
    DescriptorSet::bindDescriptorSet(p_cmd, descriptor_sets, m_mesh_pipeline.getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS);

    // renderData.m_basic_meshes = &m_basic_meshes;
    // renderData.meshes = &meshes;
    // renderData.default_pipeline = &m_mesh_pipeline;
    // renderData.binded_pipeline = &m_mesh_pipeline;
    // renderData.binded_mesh = &m_basic_meshes[Mesh::Cube];
    // renderData.descriptorSets.push(scene_descriptor_set);
    if (true) {
        for (auto& renderable : m_indirect_renderables) {
            renderable->render(p_cmd, p_render_data);
        }

        m_draw_indirect_buffers[p_render_data.frame_index].writeToBuffer(
            p_render_data.draw_commands.data(), p_render_data.draw_commands.size() * sizeof(VkDrawIndexedIndirectCommand), 0);

        uint32_t drawcount = p_render_data.draw_commands.size();
        m_count_indirect_buffers[p_render_data.frame_index].writeToBuffer(&drawcount, sizeof(uint32_t), 0);
    } else {
        PushConstantCullStruct pc_cull;
        // m_object_buffer.getBufferDeviceAddress(),
        // material_buffer.getBufferDeviceAddress(),
        // camera_buffer[p_render_data.frame_index].getBufferDeviceAddress(),
        // m_light_buffer.getBufferDeviceAddress(),
        pc_cull.cam_buffer = camera_buffer[p_render_data.frame_index].getBufferDeviceAddress();
        pc_cull.obj_buffer = m_object_buffer.getBufferDeviceAddress();
        pc_cull.draw_cmds_buffer = m_draw_indirect_buffers[p_render_data.frame_index].getBufferDeviceAddress();
        pc_cull.numberOfmesh_block = m_count_indirect_buffers[p_render_data.frame_index].getBufferDeviceAddress();
        pc_cull.camid = 0;
        pc_cull.numberOfmesh_block = 0;
    }

    vkCmdDrawIndexedIndirectCount(
        p_cmd, m_draw_indirect_buffers[p_render_data.frame_index], 0, m_count_indirect_buffers[p_render_data.frame_index], 0, 1000000,
        sizeof(VkDrawIndexedIndirectCommand));

    for (auto& renderable : m_renderables) {
        renderable->render(p_cmd, p_render_data);
    }
}

void Scene::renderShading(CommandBuffer& p_cmd, RenderData& p_renderData) {
    p_renderData.basic_meshes = m_basic_meshes;
    p_renderData.cameras = &m_cameras;
    m_deffered_renderpass->transitionAttachment(p_renderData.swapchain_index, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, p_cmd);
    m_shading_renderpass->transitionColorAttachment(p_renderData.swapchain_index, VK_IMAGE_LAYOUT_GENERAL, p_cmd);

    m_shading_pipeline.bindPipeline(p_cmd);

    std::vector<DescriptorSet*> descriptor_sets = {&m_deferred_descriptor_set[p_renderData.swapchain_index]};
    DescriptorSet::bindDescriptorSet(p_cmd, descriptor_sets, m_shading_pipeline.getPipelineLayout(), VK_PIPELINE_BIND_POINT_COMPUTE);
    PushConstantStruct tp{
        m_object_buffer.getBufferDeviceAddress(),
        material_buffer.getBufferDeviceAddress(),
        camera_buffer[p_renderData.frame_index].getBufferDeviceAddress(),
        m_light_buffer.getBufferDeviceAddress(),
        0,
        static_cast<uint32_t>(m_light_objects.size())};
    p_renderData.push_constant = tp;
    // // set push_constant for cam_id
    vkCmdPushConstants(
        p_cmd, m_shading_pipeline.getPipelineLayout(), m_shading_pipeline.getPushConstantStage(), 0, sizeof(PushConstantStruct), &tp);

    m_shading_pipeline.dispatch(p_cmd, p_renderData.render_pass->getFrameSize().width, p_renderData.render_pass->getFrameSize().height);

    m_deffered_renderpass->transitionColorAttachment(p_renderData.swapchain_index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, p_cmd);
    m_deffered_renderpass->transitionDepthAttachment(p_renderData.swapchain_index, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, p_cmd);
    m_shading_renderpass->transitionColorAttachment(p_renderData.swapchain_index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, p_cmd);

    m_shading_renderpass->setClearEnable(false);
    m_shading_renderpass->beginRenderPass(p_cmd, p_renderData.swapchain_index);
    for (auto& renderable : m_renderables) {
        renderable->render(p_cmd, p_renderData);
    }
    m_shading_renderpass->endRenderPass(p_cmd);
    m_shading_renderpass->setClearEnable(true);
}

uint32_t Scene::getNewID() {
    if (!m_free_ids.empty()) {
        uint32_t id = m_free_ids.back();
        m_free_ids.pop_back();
        return id;
    }
    return m_next_id++;
};

uint32_t Scene::addNode(uint32_t p_parent_id, std::shared_ptr<Node> p_node) {
    if (dynamic_cast<IRenderable*>(p_node.get())) {
        m_renderables.push_back(std::dynamic_pointer_cast<IRenderable>(p_node));
    }
    if (dynamic_cast<IIndirectRenderable*>(p_node.get())) {
        m_indirect_renderables.push_back(std::dynamic_pointer_cast<IIndirectRenderable>(p_node));
    }

    if (dynamic_cast<CameraV2*>(p_node.get())) {
        m_cameras.push_back(std::dynamic_pointer_cast<CameraV2>(p_node));
        if (m_cameras.size() == 1) {
            m_main_camera = m_cameras[0];
        }
    }

    if (dynamic_cast<IAnimatic*>(p_node.get())) {
        m_animatic_objs.push_back(std::dynamic_pointer_cast<IAnimatic>(p_node));
    }

    if (dynamic_cast<ICollider*>(p_node.get())) {
        m_collision_objects.push_back(std::dynamic_pointer_cast<ICollider>(p_node));
    }

    if (dynamic_cast<IInputController*>(p_node.get())) {
        m_controlled_objects.push_back(std::dynamic_pointer_cast<IInputController>(p_node));
    }

    if (dynamic_cast<Light*>(p_node.get())) {
        m_light_objects.push_back(std::dynamic_pointer_cast<Light>(p_node));
    }

    if (p_parent_id == uint32_t(-1)) {
        this->addChild(p_node);
    } else {
        m_objects[p_parent_id]->addChild(p_node);
    }
    p_node->setId(getNewID());
    m_objects[p_node->getId()] = p_node;
    return p_node->getId();
}

void Scene::removeNode(uint32_t p_id) {}

uint32_t Scene::addMaterial(Material p_material) {
    m_materials.push_back(p_material);
    return m_materials.size() - 1;
}

void Scene::addStaticMesh(Mesh& p_mesh) {
    bool need_GPU_upload = false;
    if (p_mesh.indicies.size() + first_index_available > index_buffer.getInstancesCount()) {
        need_GPU_upload = true;
        index_buffer = Buffer(
            m_device, sizeof(uint32_t), (p_mesh.indicies.size() + index_buffer.getInstancesCount()) * 1.5,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Buffer::BufferType::GPU_ONLY);
    }

    if (p_mesh.verticies.size() + first_vertex_available > vertex_buffer.getInstancesCount()) {
        need_GPU_upload = true;
        vertex_buffer = Buffer(
            m_device, sizeof(Vertex), (p_mesh.verticies.size() + vertex_buffer.getInstancesCount()) * 1.5,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Buffer::BufferType::GPU_ONLY);
    }

    if (need_GPU_upload) {
        for (auto& scene_mesh : meshes) {
            if (scene_mesh.second.indicies.size() == 0 || scene_mesh.second.verticies.size() == 0) continue;
            scene_mesh.second.setVertexAndIndexBuffer(index_buffer, vertex_buffer);
            scene_mesh.second.uploadToGPU();
        }
    }

    p_mesh.setVertexAndIndexBuffer(first_index_available, first_vertex_available, index_buffer, vertex_buffer);

    p_mesh.uploadToGPU();
    first_index_available += p_mesh.indicies.size();
    first_vertex_available += p_mesh.verticies.size();

    meshes[nb_meshes] = p_mesh;
    nb_meshes++;
}

uint32_t Scene::addImage(Image& p_image) {
    images.push_back(p_image);
    return images.size() - 1;
}

void Scene::createDrawIndirectBuffers() {
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_draw_indirect_buffers[i] = Buffer(
            m_device, sizeof(VkDrawIndexedIndirectCommand), 100000,
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, Buffer::BufferType::DYNAMIC);

        m_count_indirect_buffers[i] = Buffer(
            m_device, sizeof(uint32_t), 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
            Buffer::BufferType::DYNAMIC);
    }
}

void Scene::updateSim(float p_dt, float p_t, uint32_t p_tick) {
    for (auto& animatic_obj : m_animatic_objs) {
        animatic_obj->simulation(glm::vec3(0, -9.81, 0), 0.995, p_tick, p_dt, p_t, m_collision_objects);
    }
}

void Scene::updateFromInput(Window* p_window, float p_dt) {
    for (auto& controlled_obj : m_controlled_objects) {
        controlled_obj->updateFromInput(p_window, p_dt);
    }
}

void Scene::updateCameraBuffer(uint32_t p_frame_index) {
    if (m_cameras.size() == 0) return;

    if (camera_buffer[p_frame_index].getInstancesCount() < m_cameras.size()) {
        camera_buffer[p_frame_index] = Buffer(m_device, sizeof(Ubo), 20, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    }
    std::vector<Ubo> ubos;

    for (auto cam : m_cameras) {
        Ubo ubo;
        ubo.projection = m_main_camera->getProjectionMatrix();
        ubo.view = m_main_camera->getViewMatrix();
        ubo.invView = glm::inverse(m_main_camera->getViewMatrix());
        ubos.push_back(ubo);
    }

    camera_buffer[p_frame_index].writeToBuffer(ubos.data(), sizeof(Ubo) * ubos.size());
}

struct Object_data {
    glm::mat4 world_matrix;
    glm::mat4 normal_matrix;
    glm::vec3 padding{0};
    uint32_t material_offset = 0;
};

void Scene::updateObjectBuffer() {
    if (m_object_buffer.getInstancesCount() < m_objects.size()) {
        m_object_buffer = Buffer(
            m_device, sizeof(Object_data), m_objects.size() * 2, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, Buffer::BufferType::DYNAMIC);
        for (auto& obj : m_objects) {
            obj.second->uploaded_to_GPU = false;
        }

        m_mesh_block_buffer =
            Buffer(m_device, sizeof(MeshBlock), 100000, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, Buffer::BufferType::DYNAMIC);
    }

    uint32_t total_mesh_block = 0;
    for (auto& obj : m_objects) {
        if (!obj.second->uploaded_to_GPU) {
            Object_data data;
            data.world_matrix = obj.second->wMatrix();
            data.normal_matrix = obj.second->wNormalMatrix();
            data.material_offset = 0;
            m_object_buffer.writeToBuffer(&data, sizeof(Object_data), sizeof(Object_data) * obj.second->getId());
            obj.second->uploaded_to_GPU = true;

            auto renderable_obj = std::dynamic_pointer_cast<IIndirectRenderable>(obj.second);

            if (renderable_obj) {
                auto mesh_blocks = renderable_obj->getMeshBlock(10000);
                m_mesh_block_buffer.writeToBuffer(
                    mesh_blocks.data(), sizeof(MeshBlock) * mesh_blocks.size(), sizeof(MeshBlock) * total_mesh_block);
                total_mesh_block += mesh_blocks.size();
            }
        }
    }
}

void Scene::updateLightBuffer() {
    if (m_light_objects.size() == 0) {
        Light l;
        l.color = glm::vec3(0);
        l.intensity = 0;
        l.m_type = Light::POINT;
        addNode(-1, std::make_shared<Light>(l));
    }

    if (m_light_buffer.getInstancesCount() < m_light_objects.size()) {
        m_light_buffer = Buffer(
            m_device, sizeof(LightGPU), m_light_objects.size() * 2, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, Buffer::BufferType::DYNAMIC);

        for (auto& light : m_light_objects) {
            light->uploaded_to_GPU = false;
        }
    }
    int i = 0;
    for (auto& light : m_light_objects) {
        if (!light->uploaded_to_GPU) {
            LightGPU l;
            l.color = glm::vec4(light->color, light->intensity);
            l.pos = light->wMatrix() * glm::vec4(0, 0, 0, 1);
            l.orientation = light->getParent()->wNormalMatrix() * light->transform.rot.value;
            l.Type = light->m_type;
            light->uploaded_to_GPU = true;
            m_light_buffer.writeToBuffer(&l, sizeof(LightGPU), sizeof(LightGPU) * i);
        }
        i++;
    }
}

void Scene::updateMaterialBuffer() {
    if (material_buffer.getInstancesCount() < m_materials.size() || m_materials.size() == 0) {
        if (m_materials.size() == 0) {
            Material mat;
            mat.color = glm::vec4(0.8, 0, 0, 1);
            mat.metallic = 0.8f;
            mat.roughness = 0.9f;
            m_materials.push_back(mat);
        }
        material_buffer =
            Buffer(m_device, sizeof(MaterialGPU), m_materials.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    }
    std::vector<MaterialGPU> materials_GPU;

    for (auto& material : m_materials) {
        materials_GPU.push_back(
            {material.color, material.metallic, material.roughness, material.albedo_tex_id, material.metallic_roughness_tex_id,
             material.normal_tex_id});
    }
    material_buffer.writeToBuffer(materials_GPU.data(), sizeof(MaterialGPU) * materials_GPU.size(), 0);
}

void Scene::createDescriptorSets() { scene_descriptor_set = DescriptorSet(m_device, m_mesh_pipeline.getDescriptorSetLayout(0)); }

void Scene::updateDescriptorSets() {
    if (images.size() == 0) {
        glm::vec4* default_pixel = new glm::vec4(1, 1, 1, 1);
        // create a default texture
        ImageCreateInfo default_image_create_info;
        default_image_create_info.width = 1;
        default_image_create_info.height = 1;
        default_image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        default_image_create_info.usage_flags = VK_IMAGE_USAGE_SAMPLED_BIT;
        default_image_create_info.image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        default_image_create_info.datas.push_back(default_pixel);
        Image default_image = Image(m_device, default_image_create_info);
        images.push_back(default_image);
    }
    std::vector<VkDescriptorImageInfo> image_infos;
    for (auto& texture : images) {
        image_infos.push_back(texture.getDescriptorImageInfo(samplerType::LINEAR));
    }
    scene_descriptor_set.writeImagesDescriptor(0, image_infos);
    scene_descriptor_set.writeImageDescriptor(1, m_skybox_image.getDescriptorImageInfo(samplerType::LINEAR));
}

void Scene::updateRenderPassDescriptorSets() {
    if (m_deferred_descriptor_set.size() == 0) {
        m_deferred_descriptor_set.reserve(m_deffered_renderpass->getimageAttachement().size());
        for (size_t i = 0; i < m_deffered_renderpass->getimageAttachement().size(); i++) {
            m_deferred_descriptor_set.push_back(DescriptorSet(m_device, m_shading_pipeline.getDescriptorsSetLayout()[0]));
        }
    }
    for (size_t i = 0; i < m_deffered_renderpass->getimageAttachement().size(); i++) {
        // color metal
        VkDescriptorImageInfo image_info = m_deffered_renderpass->getimageAttachement()[i][0].getDescriptorImageInfo(samplerType::LINEAR);
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        m_deferred_descriptor_set[i].writeImageDescriptor(0, image_info);

        // normal roughness
        image_info = m_deffered_renderpass->getimageAttachement()[i][1].getDescriptorImageInfo(samplerType::LINEAR);
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        m_deferred_descriptor_set[i].writeImageDescriptor(1, image_info);

        // depth
        image_info = m_deffered_renderpass->getDepthAndStencilAttachement()[i].getDescriptorImageInfo(samplerType::LINEAR);
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        m_deferred_descriptor_set[i].writeImageDescriptor(2, image_info);

        // swapchain
        image_info = m_shading_renderpass->getimageAttachement()[i][0].getDescriptorImageInfo(samplerType::LINEAR);
        image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        m_deferred_descriptor_set[i].writeImageDescriptor(3, image_info);
    }
}

void Scene::createPipelines() {
    GraphicPipelineCreateInfo pipeline_create_info;
#ifdef DEFAULT_APP_PATH
    pipeline_create_info.fragment_shader_file = "shaders/deffered.frag";
    pipeline_create_info.vexter_shader_file = "shaders/deffered.vert";
#else
    pipeline_create_info.fragment_shader_file = "TTengine-2/shaders/deffered.frag";
    pipeline_create_info.vexter_shader_file = "TTengine-2/shaders/deffered.vert";
#endif
    m_mesh_pipeline = GraphicPipeline(m_device, pipeline_create_info);

#ifdef DEFAULT_APP_PATH
    pipeline_create_info.fragment_shader_file = "shaders/bgV2.frag";
    pipeline_create_info.vexter_shader_file = "shaders/bgV2.vert";
#else
    pipeline_create_info.fragment_shader_file = "TTengine-2/shaders/bgV2.frag";
    pipeline_create_info.vexter_shader_file = "TTengine-2/shaders/bgV2.vert";
#endif
    m_skybox_pipeline = GraphicPipeline(m_device, pipeline_create_info);

#ifdef DEFAULT_APP_PATH
    m_shading_pipeline = ComputePipeline(m_device, "shaders/shading.comp");
#else
    m_shading_pipeline = ComputePipeline(m_device, "TTengine-2/shaders/shading.comp");
#endif

#ifdef DEFAULT_APP_PATH
    m_cull_pipeline = ComputePipeline(m_device, "shaders/cull.comp");
#else
    m_cull_pipeline = ComputePipeline(m_device, "TTengine-2/shaders/cull.comp");
#endif
}

}  // namespace TTe