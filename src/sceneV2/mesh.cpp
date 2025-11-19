
#include "mesh.hpp"

#include <cstdint>
#include <cstring>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <iostream>
#include <stack>
#include <vector>

#include "GPU_data/buffer.hpp"
#include "commandBuffer/commandPool_handler.hpp"
#include "commandBuffer/command_buffer.hpp"
#include "device.hpp"
#include "struct.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/normal.hpp>

namespace TTe {

Mesh::Mesh(Device* p_device, const std::vector<uint32_t>& p_indicies, const std::vector<Vertex>& p_verticies, Buffer::BufferType p_type)
    : verticies(p_verticies), indicies(p_indicies), m_type(p_type), m_device(p_device) {
    createBVH();
    uploadToGPU();
}

Mesh::Mesh(
    Device* p_device,
    const std::vector<uint32_t>& p_indicies,
    const std::vector<Vertex>& p_verticies,
    uint p_first_index,
    uint p_first_vertex,
    Buffer p_index_buffer,
    Buffer p_vertex_buffer)
    : verticies(p_verticies),
      indicies(p_indicies),
      m_vertex_buffer(p_vertex_buffer),
      m_index_buffer(p_index_buffer),
      m_type(p_vertex_buffer.getType()),
      m_first_index(p_first_index),
      m_first_vertex(p_first_vertex),
      m_device(p_device) {
    createBVH();
    uploadToGPU();
}




void Mesh::split(uint32_t p_index, uint32_t p_count, uint32_t p_bvh_index, uint32_t p_depth) {
    static uint32_t max_depth = 0;
    // compute bounding box
    glm::vec3 pmin = verticies[indicies[p_index] ].pos;
    glm::vec3 pmax = verticies[indicies[p_index] ].pos;
    for (uint32_t i = 0; i < p_count; i++) {
        pmin = glm::min(pmin, verticies[indicies[p_index + i] ].pos);
        pmax = glm::max(pmax, verticies[indicies[p_index + i] ].pos);
    }
    if(pmin.x == pmax.x){
        pmax.x = pmin.x + 0.000001f;
    }
    if(pmin.y == pmax.y){
        pmax.y = pmin.y + 0.000001f;
    }
    if(pmin.z == pmax.z){
        pmax.z = pmin.z + 0.000001f;
    }

    bvh[p_bvh_index].bbox.pmin = pmin;
    bvh[p_bvh_index].bbox.pmax = pmax;
    bvh[p_bvh_index].indicies_index = p_index;
    bvh[p_bvh_index].nb_triangle_to_draw = p_count/3;
    // leaf
    if (p_count < (30 * 3)) {
        if(p_depth > max_depth){
            max_depth = p_depth;
            std::cout << "max depth: " << max_depth << std::endl;
        }
        bvh[p_bvh_index].nb_triangle = p_count;
        bvh[p_bvh_index].index = p_index;
        

        return;
    }
    // split
    else {

        // find the longest axis

        // find the split center based on bounding box of barycenter of triangles

        glm::vec3 bary_min = bvh[p_bvh_index].bbox.pmax;
        glm::vec3 bary_max = bvh[p_bvh_index].bbox.pmin;

        for (uint32_t i = 0; i < p_count; i += 3) {
            glm::vec3 center = (verticies[indicies[p_index + i] ].pos + verticies[indicies[p_index + i + 1] ].pos +
                                verticies[indicies[p_index + i + 2] ].pos) /
                               3.0f;

            bary_min = {std::min(bary_min.x, center.x), std::min(bary_min.y, center.y), std::min(bary_min.z, center.z)};
            bary_max = {std::max(bary_max.x, center.x), std::max(bary_max.y, center.y), std::max(bary_max.z, center.z)};
            ;
        }

        glm::vec3 size = bary_max - bary_min;
        int split_axis = 0;
        if (size.y > size.x) {
            split_axis = 1;
        }
        if (size.z > size[split_axis]) {
            split_axis = 2;
        }

        float split_center = (bary_max[split_axis] + bary_min[split_axis]) / 2.0f;

        std::vector<uint32_t>* left_indicies = new std::vector<uint32_t>();
        std::vector<uint32_t>* right_indicies = new std::vector<uint32_t>();

        for (uint32_t i = 0; i < p_count; i += 3) {
            glm::vec3 center = (verticies[indicies[p_index + i] ].pos + verticies[indicies[p_index + i + 1] ].pos +
                                verticies[indicies[p_index + i + 2] ].pos) /
                               3.0f;
            float value = center[split_axis];

            if (value < split_center) {
                left_indicies->push_back(indicies[p_index + i]);
                left_indicies->push_back(indicies[p_index + i + 1]);
                left_indicies->push_back(indicies[p_index + i + 2]);
            } else {
                right_indicies->push_back(indicies[p_index + i]);
                right_indicies->push_back(indicies[p_index + i + 1]);
                right_indicies->push_back(indicies[p_index + i + 2]);
            }
        }
        if (left_indicies->size() == 0 || right_indicies->size() == 0) {
            bvh[p_bvh_index].nb_triangle = p_count;
            bvh[p_bvh_index].index = p_index;

            delete left_indicies;
            delete right_indicies;
            return;
        }

        memcpy(indicies.data() + p_index, left_indicies->data(), left_indicies->size() * sizeof(uint32_t));

        memcpy(indicies.data() + (p_index + left_indicies->size()), right_indicies->data(), right_indicies->size() * sizeof(uint32_t));
        // // create the left and right children
        bvh.push_back(BVH_mesh());
        bvh[p_bvh_index].index = bvh.size() - 1;
        bvh.push_back(BVH_mesh());

        uint32_t left_indicies_size = left_indicies->size();
        uint32_t right_indicies_size = right_indicies->size();

        delete left_indicies;
        delete right_indicies;

        split(p_index, left_indicies_size, bvh[p_bvh_index].index, p_depth + 1);
        split(p_index + left_indicies_size, right_indicies_size, bvh[p_bvh_index].index + 1, p_depth + 1);
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


SceneHit Mesh::hit(glm::vec3& p_ro, glm::vec3& p_rd) {
 
    std::stack<uint32_t> bvh_stack;
    std::stack<float> dist_stack;
    bvh_stack.push(0);

    SceneHit hit_min;
    hit_min.t = std::numeric_limits<float>::max();
    // inspired from https://github.com/SebLague/Ray-Tracing/
    while (!bvh_stack.empty()) {
        uint32_t index = bvh_stack.top();
        bvh_stack.pop();
        

        // if leaf
        if (bvh[index].nb_triangle != 0) {
            for (uint32_t i = 0; i < bvh[index].nb_triangle; i += 3) {
                SceneHit hit = intersectTriangle(
                    p_ro, p_rd, verticies[indicies[bvh[index].index + i]], verticies[indicies[bvh[index].index + i + 1]],
                    verticies[indicies[bvh[index].index + i + 2]]);

                if (hit.t > 0 && hit.t < hit_min.t) {
                    hit_min = hit;
                }
            }
        } else {
            // check if ray intersect the bounding box of children
            uint32_t left_child = bvh[index].index;
            uint32_t right_child = bvh[index].index + 1;

            float left_t = bvh[left_child].bbox.intersect(p_ro, p_rd);
            float right_t = bvh[right_child].bbox.intersect(p_ro, p_rd);

            bool is_nearest_A = left_t <= right_t;
            float dstNear = is_nearest_A ? left_t : right_t;
            float dstFar = is_nearest_A ? right_t : left_t;
            uint32_t child_index_near = is_nearest_A ? left_child : right_child;
            uint32_t child_index_far = is_nearest_A ? right_child : left_child;

            if (dstFar < hit_min.t && dstFar != -1) bvh_stack.push(child_index_far);
            if (dstNear < hit_min.t && dstNear != -1) bvh_stack.push(child_index_near);
        }
    }
    return hit_min;
}

void Mesh::uploadToGPU(CommandBuffer* p_ext_cmd) {
    if ((m_vertex_buffer == VK_NULL_HANDLE || m_index_buffer == VK_NULL_HANDLE) ||
        (m_vertex_buffer.getInstancesCount() < verticies.size() || m_index_buffer.getInstancesCount() < indicies.size())) {
        m_vertex_buffer =
            Buffer(m_device, sizeof(Vertex), verticies.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, m_type);
        m_index_buffer =
            Buffer(m_device, sizeof(uint32_t), indicies.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, m_type);
    }

    if (m_type == Buffer::BufferType::DYNAMIC) {
        m_vertex_buffer.writeToBuffer(verticies.data(), verticies.size() * sizeof(Vertex), m_first_vertex * sizeof(Vertex));
        m_index_buffer.writeToBuffer(indicies.data(), indicies.size() * sizeof(uint32_t), m_first_index * sizeof(uint32_t));
    } else {
        CommandBuffer* cmd_buffer = p_ext_cmd;
        if (cmd_buffer == nullptr) {
            cmd_buffer = new CommandBuffer(
                std::move(CommandPoolHandler::getCommandPool(m_device, m_device->getTransferQueue())->createCommandBuffer(1)[0]));
            cmd_buffer->beginCommandBuffer();
        }

        // create staging buffer
        Buffer* staging_vertex_buffer =
            new Buffer(m_device, sizeof(Vertex), verticies.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Buffer::BufferType::STAGING,0);
        Buffer* staging_index_buffer =
            new Buffer(m_device, sizeof(uint32_t), indicies.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Buffer::BufferType::STAGING,0);

        staging_vertex_buffer->writeToBuffer(verticies.data(), verticies.size() * sizeof(Vertex));
        staging_index_buffer->writeToBuffer(indicies.data(), indicies.size() * sizeof(uint32_t));

        // copy to device local buffer
        Buffer::copyBuffer(
            m_device, *staging_vertex_buffer, m_vertex_buffer, cmd_buffer, verticies.size() * sizeof(Vertex), 0, m_first_vertex * sizeof(Vertex));
        Buffer::copyBuffer(
            m_device, *staging_index_buffer, m_index_buffer, cmd_buffer, indicies.size() * sizeof(uint32_t), 0, m_first_index * sizeof(uint32_t));

        // add staging buffer to destroy list
        cmd_buffer->addRessourceToDestroy(staging_vertex_buffer);
        cmd_buffer->addRessourceToDestroy(staging_index_buffer);

        if (p_ext_cmd == nullptr) {
            cmd_buffer->endCommandBuffer();
            cmd_buffer->addRessourceToDestroy(cmd_buffer);
            cmd_buffer->submitCommandBuffer({}, {}, nullptr, true);
        }
    }
}

void Mesh::bindMesh(CommandBuffer& p_cmd) {
    VkBuffer vbuffers[] = {m_vertex_buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(p_cmd, 0, 1, vbuffers, offsets);
    vkCmdBindIndexBuffer(p_cmd, m_index_buffer, 0, VK_INDEX_TYPE_UINT32);
}

void Mesh::setVertexAndIndexBuffer(uint p_first_index, uint p_first_vertex, Buffer p_index_buffer, Buffer p_vertex_buffer) {


    this->m_first_index = p_first_index;
    this->m_first_vertex = p_first_vertex;
    this->m_vertex_buffer = p_vertex_buffer;
    this->m_index_buffer = p_index_buffer;
}

void Mesh::setVertexAndIndexBuffer(Buffer p_index_buffer, Buffer p_vertex_buffer) {
    this->m_vertex_buffer = p_vertex_buffer;
    this->m_index_buffer = p_index_buffer;
}


// namespace TTe

// https://iquilezles.org/articles/intersectors/
SceneHit Mesh::intersectTriangle(glm::vec3& p_ro, glm::vec3& p_rd, Vertex& p_v0, Vertex& p_v1, Vertex& p_v2) {
    glm::vec3 v1v0 = p_v1.pos - p_v0.pos;
    glm::vec3 v2v0 = p_v2.pos - p_v0.pos;
    glm::vec3 rov0 = p_ro - p_v0.pos;

    glm::vec3 n = glm::cross(v1v0, v2v0);
    glm::vec3 q = glm::cross(rov0, p_rd);

    float d = 1.0f / glm::dot(p_rd, n);
    float u = d * glm::dot(-q, v2v0);
    float v = d * glm::dot(q, v1v0);
    float t = d * glm::dot(-n, rov0);
    if (u < 0.0 || v < 0.0 || (u + v) > 1.0) t = -1.0;

    SceneHit hit;
    hit.t = t;
    hit.normal = glm::normalize(n);
    hit.node_index = -1;
    hit.material_id = p_v0.material_id;
    return hit;
}




}  // namespace TTe