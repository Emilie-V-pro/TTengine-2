#pragma once

#include <cstdint>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "../utils.hpp"
#include "GPU_data/buffer.hpp"
#include "device.hpp"
#include "struct.hpp"
// #include "object.hpp"

namespace TTe {

enum BasicShape { Triangle, Sphere, Cone, Cylinder, Cube, Plane };

struct Triangle {
    Vertex verts[3];
    glm::vec3 getNormal() const;
    // Hit intersect(const Ray& r) const;
    // glm::vec3 barycentric(const glm::vec3 &p) const;
    // Hit getIntersectData(const Ray& r, const float &dist) const;
    // glm::vec3 getRandomePoint() const;
    float area() const;
};

class Mesh {
   public:
    Mesh() {};
    Mesh(Device *device) : device(device) {};
    Mesh(Device *device, const std::vector<unsigned int> &indicies, const std::vector<Vertex> &verticies);
    Mesh(Device *device, std::string path);

    Mesh(Device *device, const BasicShape &b, uint resolution);

    // copy constructor
    Mesh(const Mesh &other) {
        this->device = other.device;
        this->indicies = other.indicies;
        this->verticies = other.verticies;
        this->vertexBuffer = other.vertexBuffer;
        this->indexBuffer = other.indexBuffer;
        this->name = other.name;
    }
    Mesh &operator=(const Mesh &other) {
        if (this != &other) {
            this->device = other.device;
            this->indicies = other.indicies;
            this->verticies = other.verticies;
            this->vertexBuffer = other.vertexBuffer;
            this->indexBuffer = other.indexBuffer;
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
        this->name = std::move(other.name);
    }
    Mesh &operator=(Mesh &&other) {
        if (this != &other) {
            this->device = other.device;
            this->indicies = std::move(other.indicies);
            this->verticies = std::move(other.verticies);
            this->vertexBuffer = std::move(other.vertexBuffer);
            this->indexBuffer = std::move(other.indexBuffer);
            this->name = std::move(other.name);
        }
        return *this;
    }

    ~Mesh() {};
    void uploadToGPU();

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
        for (auto v : verticies) {
            v.material_id = i;
        }

        uploadToGPU();
    }

    void applyMaterialOffset(uint offset){
        for (auto &v : verticies) {
            v.material_id += offset;
        }
        uploadToGPU();
    }

 

    std::vector<Vertex> verticies;
    std::vector<uint32_t> indicies;

   private:
    std::string name;
    glm::vec3 min = {FLT_MAX, FLT_MAX, FLT_MAX};
    glm::vec3 max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

    // bool intersectAABBbox(Ray r) const;

    Buffer vertexBuffer;
    Buffer indexBuffer;

    Device *device;
};

}  // namespace TTe