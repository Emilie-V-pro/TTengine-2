#pragma once

#include <sys/types.h>

#include <array>
#include <cstdint>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "GPU_data/buffer.hpp"
#include "device.hpp"
#include "struct.hpp"
// #include "object.hpp"

namespace TTe {

class Mesh {
   public:
    enum BasicShape { Sphere, Cone, Cylinder, Cube, Plane };

    Mesh() {};
    Mesh(Device *device, Buffer::BufferType type = Buffer::BufferType::DYNAMIC) : device(device), type(type) {};
    Mesh(
        Device *device,
        const std::vector<unsigned int> &indicies,
        const std::vector<Vertex> &verticies,
        Buffer::BufferType type = Buffer::BufferType::DYNAMIC);

    Mesh(
        Device *device,
        const std::vector<uint32_t> &indicies,
        const std::vector<Vertex> &verticies,
        uint first_index,
        uint first_vertex,
        Buffer indexBuffer,
        Buffer vertexBuffer);

    Mesh(Device *device, const BasicShape &b, uint resolution, Buffer::BufferType type);

    Mesh(Device *device, const BasicShape &b, uint resolution);

    Mesh(const Mesh &other) = default;
    Mesh &operator=(const Mesh &other) = default;
    Mesh(Mesh &&other) = default;
    Mesh &operator=(Mesh &&other) = default;

    ~Mesh() {};

    void createBVH();

    void uploadToGPU(CommandBuffer *ext_cmd = nullptr);

    void bindMesh(CommandBuffer &cmd);

    void setVertexAndIndexBuffer(uint first_index, uint first_vertex, Buffer indexBuffer, Buffer vertexBuffer);
    void setVertexAndIndexBuffer(Buffer indexBuffer, Buffer vertexBuffer);

    int nbTriangle() const { return indicies.size() / 3; };
    int nbVerticies() const { return verticies.size(); };
    int nbIndicies() const { return indicies.size(); };

    uint32_t getFirstIndex() const { return first_index; }
    uint32_t getFirstVertex() const { return first_vertex; }

    Buffer &getVertexBuffer() { return vertexBuffer; }
    Buffer &getIndexBuffer() { return indexBuffer; }

    void setMaterial(uint i) {
        for (auto &v : verticies) {
            v.material_id = i;
        }

        uploadToGPU();
    }

    void applyMaterialOffset(uint offset) {
        for (auto &v : verticies) {
            v.material_id += offset;
        }
        uploadToGPU();
    }

    SceneHit hit(glm::vec3 &ro, glm::vec3 &rd);

    BoundingBox getBoundingBox() const { return bvh[0].bbox; }

    static SceneHit intersectTriangle(glm::vec3 &ro, glm::vec3 &rd, Vertex &v0, Vertex &v1, Vertex &v2);

    std::vector<Vertex> verticies;
    std::vector<uint32_t> indicies;

    struct BVH_mesh {
        enum struct SplitAxe { X_SPLIT, Y_SPLIT, Z_SPLIT };

        BoundingBox bbox;

        uint32_t index = 0;  // for child index and indicies index
        uint32_t nbTriangle = 0;

        uint32_t indicies_index = 0;
        uint32_t nbTriangleToDraw = 0;
    };

    std::vector<BVH_mesh> bvh;

   private:
    std::string name = "";

    // BVH

    void split(uint32_t index, uint32_t count, uint32_t bvh_index, uint32_t depth = 0);

    // Storage data
    Buffer vertexBuffer;
    Buffer indexBuffer;
    Buffer::BufferType type = Buffer::BufferType::GPU_ONLY;

    // Draw data
    uint32_t first_index = 0;
    uint32_t first_vertex = 0;

    Device *device;
};

}  // namespace TTe