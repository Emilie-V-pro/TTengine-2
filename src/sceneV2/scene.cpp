
#include "scene.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/matrix.hpp>

#include "sceneV2/Icollider.hpp"
#include "sceneV2/Irenderable.hpp"
#include "sceneV2/animatic/skeletonObj.hpp"
#include "sceneV2/cameraV2.hpp"
#include "sceneV2/mesh.hpp"
#include "sceneV2/renderable/staticMeshObj.hpp"
#include "struct.hpp"

namespace TTe {

Scene::Scene(Device *device) : device(device) {
    basicMeshes[Mesh::Sphere] = Mesh(device, Mesh::Sphere, 4);
    basicMeshes[Mesh::Cube] = Mesh(device, Mesh::Cube, 1);
    basicMeshes[Mesh::Plane] = Mesh(device, Mesh::Plane, 1);

    ImageCreateInfo skyboxImageCreateInfo;
    skyboxImageCreateInfo.filename.push_back("textures/posx.jpg");
    skyboxImageCreateInfo.filename.push_back("textures/negx.jpg");
    skyboxImageCreateInfo.filename.push_back("textures/posy.jpg");
    skyboxImageCreateInfo.filename.push_back("textures/negy.jpg");
    skyboxImageCreateInfo.filename.push_back("textures/posz.jpg");
    skyboxImageCreateInfo.filename.push_back("textures/negz.jpg");
    skyboxImageCreateInfo.usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    skyboxImageCreateInfo.isCubeTexture = true;
    skyboxImageCreateInfo.enableMipMap = true;
    skyboxImageCreateInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    skyboxImage = Image(device, skyboxImageCreateInfo);

    uint32_t whitePixel = 0xFFFFFFFF;
    ImageCreateInfo imageCreateInfo;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageCreateInfo.width = 1;
    imageCreateInfo.height = 1;
    imageCreateInfo.usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageCreateInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageCreateInfo.datas.push_back(&whitePixel);
    
    images.push_back(Image(device, imageCreateInfo));

    vkDeviceWaitIdle(*device);

    addNode(-1, std::make_shared<CameraV2>());

    createPipelines();
    updateCameraBuffer();
    updateMaterialBuffer();
    createDescriptorSets();
    updateDescriptorSets();
}

Scene::~Scene() {}

void Scene::render(CommandBuffer &cmd, RenderData &renderData) {
    skyboxPipeline.bindPipeline(cmd);
    std::vector<DescriptorSet *> descriptorSets = {&sceneDescriptorSet};
    DescriptorSet::bindDescriptorSet(cmd, descriptorSets, skyboxPipeline.getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS);

    basicMeshes[Mesh::Cube].bindMesh(cmd);
    renderData.renderPass->setDepthAndStencil(cmd, false);

    //set push_constant for cam_id
    vkCmdPushConstants(cmd, skyboxPipeline.getPipelineLayout(), skyboxPipeline.getPushConstantStage(), 0, sizeof(uint32_t), &renderData.cameraId);

    vkCmdDrawIndexed(cmd, basicMeshes[Mesh::Cube].nbIndicies(), 1, 0, 0, 0);
    renderData.renderPass->setDepthAndStencil(cmd, true);
    descriptorSets = {&sceneDescriptorSet};

    meshPipeline.bindPipeline(cmd);
    DescriptorSet::bindDescriptorSet(cmd, descriptorSets, meshPipeline.getPipelineLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS);

    renderData.basicMeshes = &basicMeshes;
    renderData.meshes = &meshes;
    renderData.default_pipeline = &meshPipeline;
    renderData.binded_pipeline = &meshPipeline;
    renderData.binded_mesh = &basicMeshes[Mesh::Cube];
    renderData.descriptorSets.push(sceneDescriptorSet);

    for (auto &renderable : renderables) {
        renderable->render(cmd, renderData);
    }
}

uint32_t Scene::getNewID() {
    if (!freeIDs.empty()) {
        uint32_t id = freeIDs.back();
        freeIDs.pop_back();
        return id;
    }
    return nextID++;

    Device *device = nullptr;
};

uint32_t Scene::addNode(uint32_t Parent_id, std::shared_ptr<Node> node) {
    if (dynamic_cast<IRenderable *>(node.get())) {
        renderables.push_back(std::dynamic_pointer_cast<IRenderable>(node));
        // check if node is a static mesh
        if (dynamic_cast<StaticMeshObj *>(node.get())) {
            std::shared_ptr<StaticMeshObj> staticMesh = std::dynamic_pointer_cast<StaticMeshObj>(node);
            staticMesh->setMeshList(&meshes);
        }
    }
    if (dynamic_cast<CameraV2 *>(node.get())) {
        cameras.push_back(std::dynamic_pointer_cast<CameraV2>(node));
        if (cameras.size() == 1) {
            mainCamera = cameras[0];
        }
    }

    if (dynamic_cast<IAnimatic *>(node.get())) {
        animaticObjs.push_back(std::dynamic_pointer_cast<IAnimatic>(node));
    }

    if (dynamic_cast<ICollider *>(node.get())) {
        collisionObjects.push_back(std::dynamic_pointer_cast<ICollider>(node));
    }

    if (dynamic_cast<IInputController *>(node.get())) {
        controlledObjects.push_back(std::dynamic_pointer_cast<IInputController>(node));
    }

    if (Parent_id == -1) {
        this->addChild(node);
    } else {
        objects[Parent_id]->addChild(node);
    }
    node->setId(getNewID());
    objects[node->getId()] = node;
    return node->getId();
}

void Scene::removeNode(uint32_t id) {}

uint32_t Scene::addMaterial(Material material) {
    materials.push_back(material);
    return materials.size() - 1;
}

void Scene::addMesh(Mesh mesh) { meshes.push_back(mesh); }

void Scene::addObjectFileData(ObjectFileData &data) {
    for (auto &mesh : data.meshes) {
        mesh.applyMaterialOffset(materials.size());
    }
    for (auto &mesh : data.meshes) {
        addMesh(mesh);
    }
    for (auto &material : data.materials) {
        material.applyTextureOffset(images.size());
    }
    for (auto &material : data.materials) {
        addMaterial(material);
    }
    for (auto &image : data.images) {
        addImage(image);
    }
}

uint32_t Scene::addImage(Image image) {
    images.push_back(image);
    return images.size() - 1;
}

void Scene::updateSim(float dt, float t, uint32_t tick) {
    for (auto &animaticObj : animaticObjs) {
        animaticObj->simulation(glm::vec3(0, -9.81, 0), 0.995, tick, dt, t, collisionObjects);
    }
}

void Scene::updateFromInput(Window *window, float dt) {
    for (auto &controlledObject : controlledObjects) {
        controlledObject->updateFromInput(window, dt);
    }
}

void Scene::updateCameraBuffer() {
    if (cameraBuffer.getInstancesCount() == 0) {
        cameraBuffer = Buffer(device, sizeof(Ubo), 20, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    }
    Ubo ubo;
    ubo.projection = mainCamera->getProjectionMatrix();
    ubo.view = mainCamera->getViewMatrix();
    ubo.invView = glm::inverse(mainCamera->getViewMatrix());
    cameraBuffer.writeToBuffer(&ubo, sizeof(Ubo));
}

inline float sign(float a)
{
    if (a > 0.0F) return (1.0F);
    if (a < 0.0F) return (-1.0F);
    return (0.0F);
}

glm::mat4 oblic_projection(const glm::mat4 &proj, glm::vec4 &clip_plane) {
    // 1) Inversion de la matrice de projection
    glm::mat4 invProj = glm::inverse(proj);

    // 2) Calcul du point q dans l'espace caméra
 
    glm::vec4 q = glm::vec4(
        (sign(clip_plane.x) + proj[2][0]) / proj[0][0],
        (sign(clip_plane.y) + proj[2][1]) / proj[1][1],
        -1.0f,
        (1.0f + proj[2][2]) / proj[3][2]
    );

    glm::vec4 clip_plane4 = glm::vec4(clip_plane.x, clip_plane.y, clip_plane.z, clip_plane.w);

    // 3) Mise à l'échelle du plan de coupe
    glm::vec4 c = clip_plane4 * (2.0f / glm::dot(clip_plane4, q));

    // 4) Construction de la nouvelle matrice
    glm::mat4 result = proj;
    // dans glm : result[col][row]
    result[0][2] = c.x - proj[0][3];
    result[1][2] = c.y - proj[1][3];
    result[2][2] = c.z - proj[2][3];
    result[3][2] = c.w - proj[3][3];

    return result;
}

void Scene::updateCameraBuffer(float near, float x_rot) {
    if (cameraBuffer.getInstancesCount() == 0) {
        cameraBuffer = Buffer(device, sizeof(Ubo), 20, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    }
    glm::vec3 normal = glm::normalize(glm::vec3{x_rot, 0, M_PI - x_rot});

    glm::vec4 plane_world;
    plane_world.x = 0;
    plane_world.y = 0;
    plane_world.z = -1;
    plane_world.w = near;

    // glm::vec4 clipPlane_cameraSpace = glm::transpose(glm::inverse(mainCamera->getViewMatrix())) * plane_world;

    Ubo ubo;
    ubo.projection = oblic_projection(mainCamera->getProjectionMatrix(), plane_world);
    ubo.view = mainCamera->getViewMatrix();
    ubo.invView = glm::inverse(mainCamera->getViewMatrix());
    cameraBuffer.writeToBuffer(&ubo, sizeof(Ubo));
}

void Scene::updateCameraBuffer(std::vector<Ubo> camData) {
    if (cameraBuffer.getInstancesCount() == 0) {
        cameraBuffer = Buffer(device, sizeof(Ubo), 20, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    }

    cameraBuffer.writeToBuffer(&camData[0], sizeof(Ubo) * camData.size(), 0);
}

void Scene::updateMaterialBuffer() {
    if (materialBuffer.getInstancesCount() < materials.size() || materials.size() == 0) {
        if (materials.size() == 0) {
            Material mat;
            mat.color = glm::vec4(0.8, 0, 0, 1);
            mat.metallic = 0.8f;
            mat.roughness = 0.9f;
            materials.push_back(mat);
        }
        materialBuffer =
            Buffer(device, sizeof(MaterialGPU), materials.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    }
    std::vector<MaterialGPU> materialsGPU;

    for (auto &material : materials) {
        materialsGPU.push_back(
            {material.color, material.metallic, material.roughness, material.albedo_tex_id, material.metallic_roughness_tex_id,
             material.normal_tex_id});

        if (material.albedo_tex_id == -1) {
            materialsGPU.back().albedo_tex_id = 0;
        }
        if (material.metallic_roughness_tex_id == -1) {
            materialsGPU.back().metallic_roughness_tex_id = 0;
        }
    }
    materialBuffer.writeToBuffer(materialsGPU.data(), sizeof(MaterialGPU) * materialsGPU.size(), 0);
}

void Scene::createDescriptorSets() { sceneDescriptorSet = DescriptorSet(device, meshPipeline.getDescriptorSetLayout(0)); }

void Scene::updateDescriptorSets() {
    sceneDescriptorSet.writeBufferDescriptor(0, cameraBuffer);

    sceneDescriptorSet.writeBufferDescriptor(1, materialBuffer);

    if (images.size() == 0) {
        ImageCreateInfo imageCreateInfo;
        imageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageCreateInfo.width = 1;
        imageCreateInfo.height = 1;
        imageCreateInfo.usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageCreateInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        images.push_back(Image(device, imageCreateInfo));
    }
    std::vector<VkDescriptorImageInfo> imageInfos;
    for (auto &texture : images) {
        imageInfos.push_back(texture.getDescriptorImageInfo(samplerType::LINEAR));
    }
    sceneDescriptorSet.writeImagesDescriptor(2, imageInfos);
    sceneDescriptorSet.writeImageDescriptor(3, skyboxImage.getDescriptorImageInfo(samplerType::LINEAR));
}

void Scene::createPipelines() {
    GraphicPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.fragmentShaderFile = "hello_scene.frag";
    pipelineCreateInfo.vexterShaderFile = "hello_scene.vert";
    meshPipeline = GraphicPipeline(device, pipelineCreateInfo);

    pipelineCreateInfo.fragmentShaderFile = "bgV2.frag";
    pipelineCreateInfo.vexterShaderFile = "bgV2.vert";
    skyboxPipeline = GraphicPipeline(device, pipelineCreateInfo);
}

}  // namespace TTe