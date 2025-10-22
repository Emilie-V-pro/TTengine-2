#include <sys/types.h>

#include <cassert>
#include <cstdint>
#include <glm/fwd.hpp>
#include <unordered_map>
#include <vector>


#include "device.hpp"
#include "mesh.hpp"
#include "struct.hpp"

namespace TTe {

struct IndexVertex {
    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;
};

// from https://winter.dev/projects/mesh/icosphere
static const float Z = (1.0f + sqrt(5.0f)) / 2.0f;           // Golden ratio
static const glm::vec2 UV = glm::vec2(1 / 11.0f, 1 / 3.0f);  // The UV coordinates are laid out in a 11x3 grid

static const int ico_vertex_count = 22;
static const int ico_index_count = 60;

static const glm::vec3 ico_verts[] = {
    glm::vec3(0, -1, -Z), glm::vec3(-1, -Z, 0), glm::vec3(Z, 0, -1), glm::vec3(1, -Z, 0),  glm::vec3(1, Z, 0), glm::vec3(-1, -Z, 0),
    glm::vec3(Z, 0, 1),   glm::vec3(0, -1, Z),  glm::vec3(1, Z, 0),  glm::vec3(-1, -Z, 0), glm::vec3(0, 1, Z), glm::vec3(-Z, 0, 1),
    glm::vec3(1, Z, 0),   glm::vec3(-1, -Z, 0), glm::vec3(-1, Z, 0), glm::vec3(-Z, 0, -1), glm::vec3(1, Z, 0), glm::vec3(-1, -Z, 0),
    glm::vec3(0, 1, -Z),  glm::vec3(0, -1, -Z), glm::vec3(1, Z, 0),  glm::vec3(Z, 0, -1)};

static const glm::vec2 ico_uvs[] = {
    UV * glm::vec2(0, 1),  //  0
    UV *glm::vec2(1, 0),   //  1
    UV *glm::vec2(1, 2),   //  2  //
    UV *glm::vec2(2, 1),   //  3  // Vertices & UVs are ordered by U then V coordinates,
    UV *glm::vec2(2, 3),   //  4  //
    UV *glm::vec2(3, 0),   //  5  //        4     8    12    16    20
    UV *glm::vec2(3, 2),   //  6  //        /  \  /  \  /  \  /  \  /  \  /
    UV *glm::vec2(4, 1),   //  7  //     2---- 6----10----14----18----21
    UV *glm::vec2(4, 3),   //  8  //   /  \  /  \  /  \  /  \  /  \  /
    UV *glm::vec2(5, 0),   //  9  //  0---- 3---- 7----11----15----19
    UV *glm::vec2(5, 2),   // 10  //   \  /  \  /  \  /  \  /  \  /
    UV *glm::vec2(6, 1),   // 11  //     1     5     9    13    17
    UV *glm::vec2(6, 3),   // 12  //
    UV *glm::vec2(7, 0),   // 13  // [4, 8, 12, 16, 20] have the same position
    UV *glm::vec2(7, 2),   // 14  // [1, 5, 9, 13, 17]  have the same position
    UV *glm::vec2(8, 1),   // 15  // [0, 19]            have the same position
    UV *glm::vec2(8, 3),   // 16  // [2, 21]            have the same position
    UV *glm::vec2(9, 0),   // 17  //
    UV *glm::vec2(9, 2),   // 18
    UV *glm::vec2(10, 1),  // 19
    UV *glm::vec2(10, 3),  // 20
    UV *glm::vec2(11, 2)   // 21
};

static const int ico_index[] = {2, 6,  4,  // Top
                               6, 10, 8, 10, 14, 12, 14, 18, 16, 18, 21, 20,

                               0, 3,  2,  // Middle
                               2, 3,  6, 3,  7,  6,  6,  7,  10, 7,  11, 10, 10, 11, 14, 11, 15, 14, 14, 15, 18, 15, 19, 18, 18, 19, 21,

                               0, 1,  3,  // Bottom
                               3, 5,  7, 7,  9,  11, 11, 13, 15, 15, 17, 19};

IndexVertex init_sphere(uint p_res) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    const int rn = (int)pow(4, p_res);
    const int total_index_count = ico_index_count * rn;
    const int total_vertex_count = ico_vertex_count + ico_index_count * (1 - rn) / (1 - 4);

    indices.resize(total_index_count);
    vertices.resize(total_vertex_count);

    for (int i = 0; i < ico_vertex_count; i++) {  // Copy in initial mesh
        vertices[i].pos = ico_verts[i];
        vertices[i].uv = ico_uvs[i];
    }

    for (int i = 0; i < ico_index_count; i++) {
        indices[i] = ico_index[i];
    }

    int current_index_count = ico_index_count;
    int current_vert_count = ico_vertex_count;

    for (uint32_t r = 0; r < p_res; r++) {
        // Now split the triangles.
        // This can be done in place, but needs to keep track of the unique triangles
        //
        //     i+2                 i+2
        //    /   \               /  \
		//   /     \    ---->   m2----m1
        //  /       \          /  \  /  \
		// i---------i+1      i----m0----i+1

        std::unordered_map<uint64_t, int> triangle_from_edge;
        int index_count = current_index_count;

        for (int t = 0; t < index_count; t += 3) {
            int midpoints[3] = {};

            for (int e = 0; e < 3; e++) {
                int first = indices[t + e];
                int second = indices[t + (t + e + 1) % 3];

                if (first > second) {
                    std::swap(first, second);
                }

                uint64_t hash = (uint64_t)first | (uint64_t)second << (sizeof(uint32_t) * 8);

                auto [triangle, wasNewEdge] = triangle_from_edge.insert({hash, current_vert_count});

                if (wasNewEdge) {
                    vertices[current_vert_count].pos = (vertices[first].pos + vertices[second].pos) / 2.0f;
                    vertices[current_vert_count].uv = (vertices[first].uv + vertices[second].uv) / 2.0f;

                    current_vert_count += 1;
                }

                midpoints[e] = triangle->second;
            }

            int mid0 = midpoints[0];
            int mid1 = midpoints[1];
            int mid2 = midpoints[2];

            indices[current_index_count++] = indices[t];
            indices[current_index_count++] = mid0;
            indices[current_index_count++] = mid2;

            indices[current_index_count++] = indices[t + 1];
            indices[current_index_count++] = mid1;
            indices[current_index_count++] = mid0;

            indices[current_index_count++] = indices[t + 2];
            indices[current_index_count++] = mid2;
            indices[current_index_count++] = mid1;

            indices[t] = mid0;  // Overwrite the original triangle with the 4th new triangle
            indices[t + 1] = mid1;
            indices[t + 2] = mid2;
        }
    }

    // Normalize all the positions to create the sphere

    for (auto &vert : vertices) {
        vert.pos = -normalize(vert.pos);
        vert.normal = vert.pos;
    }

    return {indices, vertices};
}

IndexVertex init_cone(uint p_res) {
    const int div = p_res;
    float step = 2.0 * M_PI / div;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Génération des sommets et indices pour le cône
    for (int i = 0; i <= div; ++i) {
        float alpha = i * step;

        glm::vec3 normal = glm::normalize(glm::vec3(cos(alpha), 1.0f, sin(alpha)));

        // Base du cône
        Vertex base_vertex;
        base_vertex.pos = glm::vec3(cos(alpha), 0.0f, sin(alpha));
        base_vertex.normal = normal;
        base_vertex.uv = glm::vec2(float(i) / div, 0.0f);
        vertices.push_back(base_vertex);

        // Sommet du cône
        Vertex top_vertex;
        top_vertex.pos = glm::vec3(0.0f, 1.0f, 0.0f);
        top_vertex.normal = normal;
        top_vertex.uv = glm::vec2(float(i) / div, 1.0f);
        vertices.push_back(top_vertex);

        // Ajout des indices pour le GL_TRIANGLE_STRIP
        int index1 = i * 2;
        int index2 = i * 2 + 1;
        indices.push_back(index1);
        indices.push_back(index2);
    }

    return {indices, vertices};
}

IndexVertex init_cylinder(uint p_res) {
    const int div = p_res;
    float step = 2.0 * M_PI / div;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Génération des sommets et indices pour le cylindre
    for (int i = 0; i <= div; ++i) {
        float alpha = i * step;

        glm::vec3 normal = glm::normalize(glm::vec3(cos(alpha), 0.0f, sin(alpha)));

        // Base du cylindre
        Vertex base_vertex;
        base_vertex.pos = glm::vec3(cos(alpha), 0.0f, sin(alpha));
        base_vertex.normal = normal;
        base_vertex.uv = glm::vec2(float(i) / div, 0.0f);
        vertices.push_back(base_vertex);

        // Sommet du cylindre
        Vertex top_vertex;
        top_vertex.pos = glm::vec3(cos(alpha), 1.0f, sin(alpha));
        top_vertex.normal = normal;
        top_vertex.uv = glm::vec2(float(i) / div, 1.0f);
        vertices.push_back(top_vertex);

        // Ajout des indices pour le GL_TRIANGLE_STRIP
        int index1 = i * 2;
        int index2 = i * 2 + 1;
        indices.push_back(index1);
        indices.push_back(index2);
    }

    return {indices, vertices};
}

IndexVertex init_cube() {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // face avant
    vertices.push_back({glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)});
    vertices.push_back({glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)});
    vertices.push_back({glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)});
    vertices.push_back({glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)});

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(2);
    indices.push_back(3);
    indices.push_back(0);

    // face arrière
    vertices.push_back({glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)});
    vertices.push_back({glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)});
    vertices.push_back({glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)});
    vertices.push_back({glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)});

    indices.push_back(4);
    indices.push_back(5);
    indices.push_back(6);
    indices.push_back(6);
    indices.push_back(7);
    indices.push_back(4);

    // face droite
    vertices.push_back({glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)});
    vertices.push_back({glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)});
    vertices.push_back({glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)});
    vertices.push_back({glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)});

    indices.push_back(8);
    indices.push_back(9);
    indices.push_back(10);
    indices.push_back(10);
    indices.push_back(11);
    indices.push_back(8);

    // face gauche
    vertices.push_back({glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)});
    vertices.push_back({glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)});
    vertices.push_back({glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)});
    vertices.push_back({glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)});

    indices.push_back(12);
    indices.push_back(13);
    indices.push_back(14);
    indices.push_back(14);
    indices.push_back(15);
    indices.push_back(12);

    // face haut
    vertices.push_back({glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)});
    vertices.push_back({glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)});
    vertices.push_back({glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)});
    vertices.push_back({glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)});

    indices.push_back(16);
    indices.push_back(17);
    indices.push_back(18);
    indices.push_back(18);
    indices.push_back(19);
    indices.push_back(16);

    // face bas
    vertices.push_back({glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)});
    vertices.push_back({glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)});
    vertices.push_back({glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)});
    vertices.push_back({glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)});

    indices.push_back(20);
    indices.push_back(21);
    indices.push_back(22);
    indices.push_back(22);
    indices.push_back(23);
    indices.push_back(20);

    // Génération des sommets et indices pour le cube

    return {indices, vertices};
}

IndexVertex init_plane() {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // face avant
    vertices.push_back({glm::vec3(-0.5f, 0.0f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)});
    vertices.push_back({glm::vec3(0.5f, 0.0f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)});
    vertices.push_back({glm::vec3(0.5f, 0.0f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)});
    vertices.push_back({glm::vec3(-0.5f, 0.0f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)});

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);

    return {indices, vertices};
}

Mesh::Mesh(Device *p_device, const BasicShape &p_b, uint p_resolution, Buffer::BufferType p_type) : m_type(p_type), m_device(p_device) {
    IndexVertex IV;
    switch (p_b) {
        case BasicShape::Sphere:
            IV = init_sphere(p_resolution);
            break;
        case BasicShape::Cube:
            IV = init_cube();
            break;
        case BasicShape::Plane:
            IV = init_plane();
            break;
        default:
            assert("Invalid shape");
            break;
    }
    *this = Mesh(p_device, IV.indices, IV.vertices, p_type);
}

Mesh::Mesh(
    Device *p_device, const BasicShape &p_b, uint p_resolution) {
    IndexVertex IV;
    switch (p_b) {
        case BasicShape::Sphere:
            IV = init_sphere(p_resolution);
            break;
        case BasicShape::Cube:
            IV = init_cube();
            break;
        case BasicShape::Plane:
            IV = init_plane();
            break;
        default:
            assert("Invalid shape");
            break;
    }
    this->verticies = IV.vertices;
    this->indicies = IV.indices;

    this->m_device = p_device;
    this->createBVH();
}

}  // namespace TTe