
#include "mesh.hpp"

#include <tuple>
#include <vector>

#include "GPU_data/buffer.hpp"
#include "device.hpp"
#include "mesh_io.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/normal.hpp>


namespace TTe {

float Triangle::area() const {
    float returnValue;
    return returnValue;
}

Mesh::Mesh(Device* device, const std::vector<unsigned int>& indicies, const std::vector<Vertex>& verticies)
    : device(device), indicies(indicies), verticies(verticies) {
    vertexBuffer = Buffer(device, sizeof(Vertex), verticies.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    indexBuffer = Buffer(device, sizeof(unsigned int), indicies.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    uploadToGPU();
}

struct hash_tuple {
    template <class T1, class T2, class T3>

    size_t operator()(const std::tuple<T1, T2, T3>& x) const {
        return get<0>(x) ^ get<1>(x) ^ get<2>(x);
    }
};

Mesh::Mesh(Device* device, std::string path) : device(device) {
    MeshIOData data;
    read_meshio_data(path.c_str(), data);
    verticies = data.vertices;
    indicies = data.indices;


    vertexBuffer = Buffer(device, sizeof(Vertex), verticies.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    indexBuffer = Buffer(device, sizeof(unsigned int), indicies.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, Buffer::BufferType::DYNAMIC);
    uploadToGPU();
}

void Mesh::uploadToGPU() {
    vertexBuffer.writeToBuffer(verticies.data(), verticies.size() * sizeof(Vertex));
    indexBuffer.writeToBuffer(indicies.data(), indicies.size() * sizeof(unsigned int));
}

glm::vec3 Triangle::getNormal() const { return glm::triangleNormal(verts[0].pos, verts[1].pos, verts[2].pos); };
}  // namespace TTe
