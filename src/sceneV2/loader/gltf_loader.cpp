
#include "gltf_loader.hpp"

#include <cstddef>
#include <cstdint>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <iterator>
#include <memory>
#include <stack>
#include <vector>

#include "GPU_data/image.hpp"
#include "commandBuffer/commandPool_handler.hpp"
#include "commandBuffer/command_buffer.hpp"
#include "math/quaternion_convertor.hpp"
#include "math/fov.hpp"
#include "sceneV2/cameraV2.hpp"
#include "sceneV2/container.hpp"
#include "sceneV2/light.hpp"
#include "sceneV2/mesh.hpp"
#include "sceneV2/node.hpp"
#include "sceneV2/renderable/staticMeshObj.hpp"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#include "stb_image.h"

#define DATA_PATH "../data/"
namespace TTe {

void GLTFLoader::load(const std::filesystem::path& filePath) {
    dataPath = filePath;
    cgltf_options options = {};
    cgltf_data* data = nullptr;
    cgltf_result result = cgltf_parse_file(&options, (DATA_PATH / filePath).c_str(), &data);
    if (result == cgltf_result_success) {
        result = cgltf_load_buffers(&options, data, (DATA_PATH / filePath).c_str());
        if (result == cgltf_result_success) {
            scene = new Scene(device);
            loadMesh(data);
            loadMaterial(data);
            // loadTexture(data);
            loadNode(data);

            for (int i = 0; i < data->scenes[0].nodes_count; i++) {
                cgltf_node* node = data->scenes[0].nodes[i];
                std::cout << "Node name: " << (node->name ? node->name : "Unnamed") << std::endl;
            }
            cgltf_free(data);
        }
    }
}

void GLTFLoader::loadMesh(cgltf_data* data) {
    // get total size of the mesh to allocate
    uint32_t total_index_size = 0;
    uint32_t total_vertex_size = 0;
    uint32_t previous_max_index = 0;
    std::vector<std::vector<uint32_t>> previous_max_indices;
    std::vector<uint32_t> global_Indices_Indices;
    std::vector<uint32_t> global_Vertex_Indices;

    for (int i = 0; i < data->meshes_count; i++) {
        global_Indices_Indices.push_back(total_index_size);
        global_Vertex_Indices.push_back(total_vertex_size);

        cgltf_mesh* mesh = &data->meshes[i];
        std::cout << "Mesh name: " << (mesh->name ? mesh->name : "Unnamed") << std::endl;
        std::vector<uint32_t> previous_max_index;
        for (int j = 0; j < mesh->primitives_count; j++) {
            cgltf_primitive* primitive = &mesh->primitives[j];

            if (primitive->indices) {
                total_index_size += primitive->indices->count;
            }

            previous_max_index.push_back(total_vertex_size);
            for (int k = 0; k < primitive->attributes_count; k++) {
                cgltf_attribute* attribute = &primitive->attributes[k];
                if (attribute->data && attribute->type == cgltf_attribute_type_position) {
                    total_vertex_size += attribute->data->count;

                    break;
                }
            }
        }
        previous_max_indices.push_back(previous_max_index);
    }

    scene->indexBuffer = Buffer(
        device, sizeof(uint32_t), total_index_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        Buffer::BufferType::GPU_ONLY);

    scene->vertexBuffer = Buffer(
        device, sizeof(Vertex), total_vertex_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        Buffer::BufferType::GPU_ONLY);

    std::cout << "Total index size: " << total_index_size << std::endl;
    std::cout << "Total vertex size: " << total_vertex_size << std::endl;

    // measure time
    auto start = std::chrono::high_resolution_clock::now();
    std::mutex addMeshMutex;
#pragma omp parallel for schedule(dynamic, 1)
    for (int i = 0; i < data->meshes_count; i++) {
        cgltf_mesh* mesh = &data->meshes[i];

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        for (int j = 0; j < mesh->primitives_count; j++) {
            cgltf_primitive* primitive = &mesh->primitives[j];

            if (primitive->indices) {
                for (unsigned k = 0; k < primitive->indices->count; k++) {
                    indices.push_back((cgltf_accessor_read_index(primitive->indices, k) + previous_max_indices[i][j]));
                }
            }

            int material = -1;
            // p.material_index = -1;
            if (primitive->material) material = std::distance(data->materials, primitive->material);

            std::vector<float> pos_buffer;
            std::vector<float> normal_buffer;
            std::vector<float> uv_buffer;

            for (int k = 0; k < primitive->attributes_count; k++) {
                cgltf_attribute* attribute = &primitive->attributes[k];
                if (attribute->type == cgltf_attribute_type_position) {
                    assert(attribute->data->type == cgltf_type_vec3);

                    pos_buffer.resize(cgltf_accessor_unpack_floats(attribute->data, nullptr, 0));
                    cgltf_accessor_unpack_floats(attribute->data, pos_buffer.data(), pos_buffer.size());
                }

                if (attribute->type == cgltf_attribute_type_normal) {
                    assert(attribute->data->type == cgltf_type_vec3);

                    normal_buffer.resize(cgltf_accessor_unpack_floats(attribute->data, nullptr, 0));
                    cgltf_accessor_unpack_floats(attribute->data, normal_buffer.data(), normal_buffer.size());
                }

                if (attribute->type == cgltf_attribute_type_texcoord) {
                    assert(attribute->data->type == cgltf_type_vec2);

                    uv_buffer.resize(cgltf_accessor_unpack_floats(attribute->data, nullptr, 0));
                    cgltf_accessor_unpack_floats(attribute->data, uv_buffer.data(), uv_buffer.size());
                }
            }
            // Process attributes, indices, etc.
            // This is where you would handle the mesh data
            for (int k = 0; k < pos_buffer.size() / 3; k++) {
                Vertex vertex;
                vertex.pos = glm::vec3(pos_buffer[k * 3], pos_buffer[k * 3 + 1], pos_buffer[k * 3 + 2]);
                if (!normal_buffer.empty()) {
                    vertex.normal = glm::vec3(normal_buffer[k * 3], normal_buffer[k * 3 + 1], normal_buffer[k * 3 + 2]);
                }
                if (!uv_buffer.empty()) {
                    vertex.uv = glm::vec2(uv_buffer[k * 2], uv_buffer[k * 2 + 1]);
                }
                vertex.material_id = material;
                vertices.push_back(vertex);
            }

            previous_max_index += (pos_buffer.size() / 3);
        }

        Mesh m =
            Mesh(device, indices, vertices, global_Indices_Indices[i], global_Vertex_Indices[i], scene->indexBuffer, scene->vertexBuffer);
        addMeshMutex.lock();
        scene->addMesh(m);
        addMeshMutex.unlock();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Mesh loading took: " << elapsed.count() << " seconds" << std::endl;
}

void GLTFLoader::loadMaterial(cgltf_data* data) {
    isAlbedoTex.resize(data->images_count, true);
    for (int i = 0; i < data->materials_count; i++) {
        cgltf_material* material = &data->materials[i];
        std::cout << "Material name: " << (material->name ? material->name : "Unnamed") << std::endl;

        Material mat;
        mat.name = material->name ? material->name : "Unnamed";
        if (material->has_pbr_metallic_roughness) {
            cgltf_pbr_metallic_roughness* pbr = &material->pbr_metallic_roughness;

            mat.color =
                glm::vec4(pbr->base_color_factor[0], pbr->base_color_factor[1], pbr->base_color_factor[2], pbr->base_color_factor[3]);
            if (pbr->base_color_texture.texture && pbr->base_color_texture.texture->image)
                mat.albedo_tex_id = int(std::distance(data->images, pbr->base_color_texture.texture->image));

            mat.metallic = pbr->metallic_factor;

            mat.roughness = pbr->roughness_factor;
            if (pbr->metallic_roughness_texture.texture && pbr->metallic_roughness_texture.texture->image) {
                mat.metallic_roughness_tex_id = int(std::distance(data->images, pbr->metallic_roughness_texture.texture->image));
                isAlbedoTex[mat.metallic_roughness_tex_id] = false;
            }
        }

        if (material->normal_texture.texture && material->normal_texture.texture->image) {
            mat.normal_tex_id = int(std::distance(data->images, material->normal_texture.texture->image));
            isAlbedoTex[mat.normal_tex_id] = false;
        }
        scene->addMaterial(mat);
    }
}

void GLTFLoader::loadTexture(cgltf_data* data) {
    auto start = std::chrono::high_resolution_clock::now();

#pragma omp parallel for schedule(dynamic, 1)
    for (int i = 0; i < data->images_count; i++) {
        cgltf_image* image = &data->images[i];
        // std::cout << dataPath.parent_path() / image->uri << std::endl;
        std::cout << "Image name: " << (image->name ? image->name : "Unnamed") << std::endl;

        ImageCreateInfo imageCreateInfo;
        imageCreateInfo.format = isAlbedoTex[i] ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
        imageCreateInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageCreateInfo.usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageCreateInfo.enableMipMap = false;
        if (image->uri) {
            imageCreateInfo.filename.push_back(dataPath.parent_path() / image->uri);
        } else {
            // put data of bufferview into datas ptr

            const uint8_t* data = static_cast<const uint8_t*>(image->buffer_view->buffer->data) + image->buffer_view->offset;
            size_t size = image->buffer_view->size;
            int width, height, nbOfchannel;
            unsigned char* pixels = stbi_load_from_memory(data, size, &width, &height, &nbOfchannel, 4);
            if (pixels) {
                imageCreateInfo.width = width;
                imageCreateInfo.height = height;
                imageCreateInfo.datas.push_back(pixels);
            } else {
                std::cerr << "Failed to load image from buffer view: " << stbi_failure_reason() << std::endl;
            }
        }

        Image imageObj(device, imageCreateInfo);
        if (imageCreateInfo.datas.size() > 0) {
            stbi_image_free(imageCreateInfo.datas[0]);
        }
        scene->addImage(imageObj);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Mesh loading took: " << elapsed.count() << " seconds" << std::endl;

}  // namespace TTec

void GLTFLoader::loadNode(cgltf_data* data) {
    std::map<cgltf_node*, std::shared_ptr<Node>> nodeMap;
    std::stack<cgltf_node*> nodeStack;

    for (int i = data->scene->nodes_count - 1; i >= 0; i--) {
        nodeStack.push(data->scene->nodes[i]);
    }

    nodeMap[nullptr] = nullptr; // Handle the root node case

    while (!nodeStack.empty()) {
        cgltf_node* node = nodeStack.top();
        nodeStack.pop();
        std::shared_ptr<Node> engin_node;

        if (node->children_count > 0 || node->mesh || node->camera || node->light) {
            if (node->mesh) {
                std::shared_ptr<StaticMeshObj> mesh_node = std::make_shared<StaticMeshObj>();
                mesh_node->setMeshId(std::distance(data->meshes, node->mesh));
                engin_node = mesh_node;
            }

            if (node->camera) {
                std::shared_ptr<CameraV2> cam_node = std::make_shared<CameraV2>();
                cam_node->fov = yFOV_to_FOV(glm::degrees(node->camera->data.perspective.yfov), node->camera->data.perspective.aspect_ratio);
                cam_node->near = node->camera->data.perspective.znear;
                cam_node->far = node->camera->data.perspective.zfar;
                engin_node = cam_node;
            }

            if(node->light){
                std::shared_ptr<Light> light_node = std::make_shared<Light>();
                light_node->color = glm::vec3(node->light->color[0], node->light->color[1], node->light->color[2]);
                light_node->intensity = node->light->intensity;
                if(node->light->type == cgltf_light_type_directional) {
                    light_node->type = Light::LightType::DIRECTIONAL;
                    engin_node = light_node;
                } else if (node->light->type == cgltf_light_type_point) {
                    light_node->type = Light::LightType::POINT;
                    engin_node = light_node;
                }
                
            }


            if(engin_node == nullptr){
                engin_node = std::make_shared<Container>();
            }

            if (node->has_rotation) {
                engin_node->transform.rot = quatToEulerZXY({node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3]});
            }
            if (node->has_translation) {
                engin_node->transform.pos = glm::vec3(node->translation[0], node->translation[1], node->translation[2]);
            }
            if (node->has_scale) {
                engin_node->transform.scale = glm::vec3(node->scale[0], node->scale[1], node->scale[2]);
            }

            engin_node->setName(node->name ? node->name : "Unnamed");
            nodeMap[node] = engin_node;



            if(node->parent) {
                scene->addNode(nodeMap[node->parent]->getId(), engin_node);
            } else {
                scene->addNode(-1, engin_node );
            }

            for (int i = node->children_count - 1; i >= 0; i--) {
                nodeStack.push(node->children[i]);
            }
        }
    }
}

}  // namespace TTe