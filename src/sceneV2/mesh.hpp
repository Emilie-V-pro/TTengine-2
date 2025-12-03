#pragma once

#include <sys/types.h>


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
    Mesh(Device *p_device, Buffer::BufferType p_type = Buffer::BufferType::DYNAMIC) : m_type(p_type), m_device(p_device) {};
    Mesh(
        Device *p_device,
        const std::vector<unsigned int> &p_indicies,
        const std::vector<Vertex> &p_verticies,
        Buffer::BufferType p_type = Buffer::BufferType::DYNAMIC);

    Mesh(
        Device *p_device,
        const std::vector<uint32_t> &p_indicies,
        const std::vector<Vertex> &p_verticies,
        uint p_first_index,
        uint p_first_vertex,
        Buffer p_index_buffer,
        Buffer p_vertex_buffer);

    Mesh(Device *p_device, const BasicShape &p_shape, uint p_resolution, Buffer::BufferType p_type);

    Mesh(Device *p_device, const BasicShape &p_shape, uint p_resolution);

    Mesh(const Mesh &other) = default;
    Mesh &operator=(const Mesh &other) = default;
    Mesh(Mesh &&other) = default;
    Mesh &operator=(Mesh &&other) = default;

    ~Mesh() {};

    void createBVH();

    void uploadToGPU(CommandBuffer *p_ext_cmd = nullptr);

    void bindMesh(CommandBuffer &p_cmd);

    void setVertexAndIndexBuffer(uint p_first_index, uint p_first_vertex, Buffer p_index_buffer, Buffer p_vertex_buffer);
    void setVertexAndIndexBuffer(Buffer p_index_buffer, Buffer p_vertex_buffer);

    int nbTriangle() const { return indicies.size() / 3; };
    int nbVerticies() const { return verticies.size(); };
    int nbIndicies() const { return indicies.size(); };

    uint32_t getFirstIndex() const { return m_first_index; }
    uint32_t getFirstVertex() const { return m_first_vertex; }

    Buffer &getVertexBuffer() { return m_vertex_buffer; }
    Buffer &getIndexBuffer() { return m_index_buffer; }

    void setMaterial(uint p_i) {
        for (auto &v : verticies) {
            v.material_id = p_i;
        }

        uploadToGPU();
    }

    void applyMaterialOffset(uint p_offset) {
        for (auto &v : verticies) {
            v.material_id += p_offset;
        }
        uploadToGPU();
    }

    std::vector<MeshBlock> getMeshBlock(uint32_t p_nb_max_triangle);
    
    

    SceneHit hit(glm::vec3 &p_ro, glm::vec3 &p_rd);

    BoundingBox getBoundingBox() const { return bvh[0].bbox; }

    static SceneHit intersectTriangle(glm::vec3 &p_ro, glm::vec3 &p_rd, Vertex &p_v0, Vertex &p_v1, Vertex &p_v2);

    std::vector<Vertex> verticies;
    std::vector<uint32_t> indicies;

    struct BVH_mesh {

        BoundingBox bbox;

        uint32_t index = 0;  // for child index and indicies index
        uint32_t nb_triangle = 0;

        uint32_t indicies_index = 0;
        uint32_t nb_triangle_to_draw = 0;
    };

    std::vector<BVH_mesh> bvh;
    std::string name = "";
   private:
    

    // BVH

    void split(uint32_t p_index, uint32_t p_count, uint32_t p_bvh_index, uint32_t p_depth = 0);

    // Storage data
    Buffer m_vertex_buffer;
    Buffer m_index_buffer;
    Buffer::BufferType m_type = Buffer::BufferType::GPU_ONLY;

    // Draw data
    uint32_t m_first_index = 0;
    uint32_t m_first_vertex = 0;

    Device *m_device;
};

}  // namespace TTe