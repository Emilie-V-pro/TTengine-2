
#include "mesh.hpp"

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <cstring>
#include <glm/fwd.hpp>
#include <map>
#include <tuple>
#include <vector>

#include "GPU_data/buffer.hpp"
#include "commandBuffer/commandPool_handler.hpp"
#include "commandBuffer/command_buffer.hpp"
#include "device.hpp"
#include "mesh_io.h"
#include "struct.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/normal.hpp>

namespace TTe {


uint32_t Mesh::leaf_count = 0;
uint32_t Mesh::leaf_without_triangle_count = 0;

float Triangle::area() const {
    float returnValue;
    return returnValue;
}

Mesh::Mesh(Device* device, const std::vector<unsigned int>& indicies, const std::vector<Vertex>& verticies, Buffer::BufferType type)
    : device(device), indicies(indicies), verticies(verticies), type(type) {
    createBVH();
    uploadToGPU();
}

struct hash_tuple {
    template <class T1, class T2, class T3>

    size_t operator()(const std::tuple<T1, T2, T3>& x) const {
        return get<0>(x) ^ get<1>(x) ^ get<2>(x);
    }
};

Mesh::Mesh(Device* device, std::string path, Buffer::BufferType type) : device(device), type(type) {
    MeshIOData data;
    read_meshio_data(path.c_str(), data);
    verticies = data.vertices;
    indicies = data.indices;

    uploadToGPU();
}

void Mesh::split(uint32_t index, uint32_t count, uint32_t bvh_index, uint32_t depth) {
    // compute bounding box
    glm::vec3 pmin = verticies[indicies[index]].pos;
    glm::vec3 pmax = verticies[indicies[index]].pos;
    for (int i = 0; i < count; i++) {
        pmin = glm::min(pmin, verticies[indicies[index + i]].pos);
        pmax = glm::max(pmax, verticies[indicies[index + i]].pos);
    }
    bvh[bvh_index].pmin = pmin;
    bvh[bvh_index].pmax = pmax;

    // leaf
    if (count < (10 * 3) || depth > 32) {
        bvh[bvh_index].nbTriangle = count;
        bvh[bvh_index].index = index;
        std::cout << "Leaf " << bvh_index << " : " << count / 3 << " triangles" << std::endl;
        if (count == 0) {
            leaf_without_triangle_count++;
        }
        leaf_count++;
        
        return;
    }
    // split
    else {
        // find the longest axis
        glm::vec3 size = bvh[bvh_index].pmax - bvh[bvh_index].pmin;
        int split_axis = 0;
        if (size.y > size.x) {
            split_axis = 1;
        }
        if (size.z > size[split_axis]) {
            split_axis = 2;
        }

        // find the split center based on bounding box of barycenter of triangles

        float bary_min = bvh[bvh_index].pmax[split_axis];
        float bary_max = bvh[bvh_index].pmin[split_axis];
        
        for(int i = 0; i < count; i += 3) {
            glm::vec3 center =
                (verticies[indicies[index + i]].pos + verticies[indicies[index + i + 1]].pos + verticies[indicies[index + i + 2]].pos) /
                3.0f;
                bary_min = std::min(bary_min, center[split_axis]);
                bary_max = std::max(bary_max, center[split_axis]);
        }
        float split_center = (bary_min + bary_max) / 2.0f;


        // split the indices into two groups
        std::vector<uint32_t> *left_indicies = new std::vector<uint32_t>();
        std::vector<uint32_t> *right_indicies = new std::vector<uint32_t>();

        // reorder the indices of triangle based on center of the triangle
        do{
        for (uint32_t i = 0; i < count; i += 3) {
            glm::vec3 center =
                (verticies[indicies[index + i]].pos + verticies[indicies[index + i + 1]].pos + verticies[indicies[index + i + 2]].pos) /
                3.0f;
            float value = center[split_axis];
            if (value < split_center) {
                left_indicies->push_back(indicies[index + i]);
                left_indicies->push_back(indicies[index + i + 1]);
                left_indicies->push_back(indicies[index + i + 2]);
            } else {
                right_indicies->push_back(indicies[index + i]);
                right_indicies->push_back(indicies[index + i + 1]);
                right_indicies->push_back(indicies[index + i + 2]);
            }
            
        }}
        while(left_indicies->size() == 0 || right_indicies->size() == 0);

        // memcpy the indicies to the indicies array
        memcpy(indicies.data() + index, left_indicies->data(), left_indicies->size() * sizeof(uint32_t));

        memcpy(indicies.data() + (index + left_indicies->size()), right_indicies->data(),
               right_indicies->size() * sizeof(uint32_t));
        // // create the left and right children
        bvh.push_back(BVH_mesh());
        bvh[bvh_index].index = bvh.size() - 1;
        bvh.push_back(BVH_mesh());

        // // set the left and right children

        // // update the indicies
        // for (int i = 0; i < left_indicies->size(); i++) {
        //     indicies[index + i] = left_indicies[i];
        // }
        // for (int i = 0; i < right_indicies->size(); i++) {
        //     indicies[index + left_indicies->size() + i] = right_indicies[i];
        // }

        // call split on the left and right children

        uint32_t left_indicies_size = left_indicies->size();
        uint32_t right_indicies_size = right_indicies->size();
        // delete the left and right indicies
        delete left_indicies;
        delete right_indicies;

        split(index, left_indicies_size, bvh[bvh_index].index, depth + 1);
        split(index + left_indicies_size, right_indicies_size, bvh[bvh_index].index + 1, depth + 1);
    }
}

void Mesh::createBVH() {
    // get bounding box
    bvh.clear();
    bvh.reserve(indicies.size() / 3);
    bvh.push_back(BVH_mesh());
    split(0, indicies.size(), 0);
    // create bvh
}

void Mesh::uploadToGPU(CommandBuffer* ext_cmd) {
    if ((vertexBuffer == VK_NULL_HANDLE || indexBuffer == VK_NULL_HANDLE) ||
        (vertexBuffer.getInstancesCount() != verticies.size() || indexBuffer.getInstancesCount() != indicies.size())) {
        vertexBuffer =
            Buffer(device, sizeof(Vertex), verticies.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, type);
        indexBuffer = Buffer(
            device, sizeof(unsigned int), indicies.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, type);
    }

    if (type == Buffer::BufferType::DYNAMIC) {
        vertexBuffer.writeToBuffer(verticies.data(), verticies.size() * sizeof(Vertex));
        indexBuffer.writeToBuffer(indicies.data(), indicies.size() * sizeof(unsigned int));
    } else {
        CommandBuffer* cmd_buffer = ext_cmd;
        if (cmd_buffer == nullptr) {
            cmd_buffer = new CommandBuffer(
                std::move(CommandPoolHandler::getCommandPool(device, device->getTransferQueue())->createCommandBuffer(1)[0]));
            cmd_buffer->beginCommandBuffer();
        }

        // create staging buffer
        Buffer* stagingVertexBuffer =
            new Buffer(device, sizeof(Vertex), verticies.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Buffer::BufferType::STAGING);
        Buffer* stagingIndexBuffer =
            new Buffer(device, sizeof(unsigned int), indicies.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Buffer::BufferType::STAGING);

        stagingVertexBuffer->writeToBuffer(verticies.data(), verticies.size() * sizeof(Vertex));
        stagingIndexBuffer->writeToBuffer(indicies.data(), indicies.size() * sizeof(unsigned int));

        // copy to device local buffer
        Buffer::copyBuffer(device, *stagingIndexBuffer, indexBuffer, cmd_buffer);
        Buffer::copyBuffer(device, *stagingVertexBuffer, vertexBuffer, cmd_buffer);

        // add staging buffer to destroy list
        cmd_buffer->addRessourceToDestroy(stagingVertexBuffer);
        cmd_buffer->addRessourceToDestroy(stagingIndexBuffer);

        if (ext_cmd == nullptr) {
            cmd_buffer->endCommandBuffer();
            cmd_buffer->addRessourceToDestroy(cmd_buffer);
            cmd_buffer->submitCommandBuffer({}, {}, nullptr, true);
        }
    }
}

void Mesh::bindMesh(CommandBuffer& cmd) {
    VkBuffer vbuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, vbuffers, offsets);
    vkCmdBindIndexBuffer(cmd, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
}

glm::vec3 Triangle::getNormal() const { return glm::triangleNormal(verts[0].pos, verts[1].pos, verts[2].pos); };
}  // namespace TTe
