#pragma once

#include <sys/types.h>

#include <cstdint>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include "../utils.hpp"
#include "GPU_data/buffer.hpp"
#include "device.hpp"
#include "sceneV2/animatic/skeleton/BVH.h"
#include "struct.hpp"
// #include "object.hpp"

namespace TTe {

enum BasicShape { Triangle, Sphere, Cone, Cylinder, Cube, Plane };

struct Triangle {
    Vertex verts[3];
    glm::vec3 getNormal() const;
    
    // glm::vec3 barycentric(const glm::vec3 &p) const;
    // Hit getIntersectData(const Ray& r, const float &dist) const;
    // glm::vec3 getRandomePoint() const;
    float area() const;
};

class Mesh {
   public:
    Mesh() {};
    Mesh(Device *device, Buffer::BufferType type = Buffer::BufferType::DYNAMIC) : device(device), type(type) {};
    Mesh(
        Device *device,
        const std::vector<unsigned int> &indicies,
        const std::vector<Vertex> &verticies,
        Buffer::BufferType type = Buffer::BufferType::DYNAMIC);
    Mesh(Device *device, std::string path, Buffer::BufferType type = Buffer::BufferType::DYNAMIC);

    Mesh(Device *device, const BasicShape &b, uint resolution, Buffer::BufferType type = Buffer::BufferType::DYNAMIC);

    // copy constructor
    Mesh(const Mesh &other) {
        this->device = other.device;
        this->indicies = other.indicies;
        this->verticies = other.verticies;
        this->vertexBuffer = other.vertexBuffer;
        this->indexBuffer = other.indexBuffer;
        this->bvh = other.bvh;
        this->name = other.name;
    }
    Mesh &operator=(const Mesh &other) {
        if (this != &other) {
            this->device = other.device;
            this->indicies = other.indicies;
            this->verticies = other.verticies;
            this->vertexBuffer = other.vertexBuffer;
            this->indexBuffer = other.indexBuffer;
            this->bvh = other.bvh;
            this->name = other.name;
        }
        return *this;
    }

    // move constructor
    Mesh(Mesh &&other) {
        this->device = other.device;
        this->indicies = std::move(other.indicies);
        this->verticies = std::move(other.verticies);
        this->vertexBuffer = std::move(other.vertexBuffer);
        this->indexBuffer = std::move(other.indexBuffer);
        this->bvh = std::move(other.bvh);
        this->name = std::move(other.name);
    }
    Mesh &operator=(Mesh &&other) {
        if (this != &other) {
            this->device = other.device;
            this->indicies = std::move(other.indicies);
            this->verticies = std::move(other.verticies);
            this->vertexBuffer = std::move(other.vertexBuffer);
            this->indexBuffer = std::move(other.indexBuffer);
            this->bvh = std::move(other.bvh);
            this->name = std::move(other.name);
        }
        return *this;
    }

    ~Mesh() {};

    void createBVH();

    void uploadToGPU(CommandBuffer *ext_cmd = nullptr);

    void bindMesh(CommandBuffer &cmd);

    // Hit traceRay(Ray r, const float &distMin, const float &distMax) const;

    int nbTriangle() const { return indicies.size() / 3; };
    int nbVerticies() const { return verticies.size(); };
    int nbIndicies() const { return indicies.size(); };

    Buffer &getVertexBuffer() { return vertexBuffer; }
    Buffer &getIndexBuffer() { return indexBuffer; }
    // Triangle operator[](const int i);
    // Triangle operator[](const int i) const;

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

    std::vector<Vertex> verticies;
    std::vector<uint32_t> indicies;

    BoundingBox getBoundingBox() const {
        return bvh[0].bbox;
    }

    static SceneHit intersectTriangle(glm::vec3 &ro, glm::vec3 &rd, Vertex &v0, Vertex &v1, Vertex &v2);

    static uint32_t leaf_count;
    static uint32_t leaf_without_triangle_count;

   private:
    struct BVH_mesh {
        enum struct SplitAxe { X_SPLIT, Y_SPLIT, Z_SPLIT };

        BoundingBox bbox;

        uint32_t index = 0;  // for child index and indicies index
        uint32_t nbTriangle = 0;
    };

    void split(uint32_t index, uint32_t count, uint32_t bvh_index, uint32_t depth = 0);

    std::string name;

    std::vector<BVH_mesh> bvh;
    // std::unordered_map<uint32_t, ><>

    // bool intersectAABBbox(Ray r) const;

    Buffer vertexBuffer;
    Buffer indexBuffer;

    Buffer::BufferType type;

    Device *device;
};

}  // namespace TTe