
#include "mesh.hpp"

#include <vulkan/vulkan_core.h>

#include <tuple>
#include <vector>

#include "GPU_data/buffer.hpp"
#include "commandBuffer/commandPool_handler.hpp"
#include "commandBuffer/command_buffer.hpp"
#include "device.hpp"
#include "mesh_io.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/normal.hpp>

namespace TTe {

float Triangle::area() const {
    float returnValue;
    return returnValue;
}

Mesh::Mesh(Device* device, const std::vector<unsigned int>& indicies, const std::vector<Vertex>& verticies, Buffer::BufferType type)
    : device(device), indicies(indicies), verticies(verticies), type(type) {
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
