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

class Mesh {
   public:
    enum BasicShape { Triangle, Sphere, Cone, Cylinder, Cube, Plane };

    Mesh() {};
    Mesh(Device *device, Buffer::BufferType type = Buffer::BufferType::DYNAMIC) : device(device), type(type) {};
    Mesh(
        Device *device,
        const std::vector<unsigned int> &indicies,
        const std::vector<Vertex> &verticies,
        Buffer::BufferType type = Buffer::BufferType::DYNAMIC);
    
    Mesh(Device *device, const std::vector<uint32_t> &indicies, const std::vector<Vertex> &verticies, 
         uint first_index, uint first_vertex, Buffer indexBuffer, Buffer vertexBuffer);
 

    Mesh(Device *device, const BasicShape &b, uint resolution, Buffer::BufferType type = Buffer::BufferType::DYNAMIC);


    Mesh(const Mesh &other);
    
    Mesh &operator=(const Mesh &other);

    Mesh(Mesh &&other) ;
    Mesh &operator=(Mesh &&other);

    ~Mesh() {};

    void createBVH();

    void uploadToGPU(CommandBuffer *ext_cmd = nullptr);

    void bindMesh(CommandBuffer &cmd);

    int nbTriangle() const { return indicies.size() / 3; };
    int nbVerticies() const { return verticies.size(); };
    int nbIndicies() const { return indicies.size(); };

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

   private:
    struct BVH_mesh {
        enum struct SplitAxe { X_SPLIT, Y_SPLIT, Z_SPLIT };

        BoundingBox bbox;

        uint32_t index = 0;  // for child index and indicies index
        uint32_t nbTriangle = 0;
    };

    void split(uint32_t index, uint32_t count, uint32_t bvh_index, uint32_t depth = 0);

    std::string name = "";

    std::vector<BVH_mesh> bvh;

    Buffer vertexBuffer;
    Buffer indexBuffer;
    Buffer::BufferType type = Buffer::BufferType::GPU_ONLY;

    uint32_t first_index = 0;
    uint32_t first_vertex = 0;

    Device *device;
};

}  // namespace TTe