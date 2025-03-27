#include <sys/types.h>

#include <cassert>
#include <glm/fwd.hpp>
#include <unordered_map>
#include <vector>

#include "device.hpp"
#include "mesh.hpp"
#include "utils.hpp"

namespace TTe {


// from https://winter.dev/projects/mesh/icosphere
static const float Z = (1.0f + sqrt(5.0f)) / 2.0f;           // Golden ratio
static const glm::vec2 UV = glm::vec2(1 / 11.0f, 1 / 3.0f);  // The UV coordinates are laid out in a 11x3 grid

static const int IcoVertexCount = 22;
static const int IcoIndexCount = 60;

static const glm::vec3 IcoVerts[] = {
    glm::vec3(0, -1, -Z), glm::vec3(-1, -Z, 0), glm::vec3(Z, 0, -1), glm::vec3(1, -Z, 0),  glm::vec3(1, Z, 0), glm::vec3(-1, -Z, 0),
    glm::vec3(Z, 0, 1),   glm::vec3(0, -1, Z),  glm::vec3(1, Z, 0),  glm::vec3(-1, -Z, 0), glm::vec3(0, 1, Z), glm::vec3(-Z, 0, 1),
    glm::vec3(1, Z, 0),   glm::vec3(-1, -Z, 0), glm::vec3(-1, Z, 0), glm::vec3(-Z, 0, -1), glm::vec3(1, Z, 0), glm::vec3(-1, -Z, 0),
    glm::vec3(0, 1, -Z),  glm::vec3(0, -1, -Z), glm::vec3(1, Z, 0),  glm::vec3(Z, 0, -1)};

static const glm::vec2 IcoUvs[] = {
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

static const int IcoIndex[] = {2, 6,  4,  // Top
                               6, 10, 8, 10, 14, 12, 14, 18, 16, 18, 21, 20,

                               0, 3,  2,  // Middle
                               2, 3,  6, 3,  7,  6,  6,  7,  10, 7,  11, 10, 10, 11, 14, 11, 15, 14, 14, 15, 18, 15, 19, 18, 18, 19, 21,

                               0, 1,  3,  // Bottom
                               3, 5,  7, 7,  9,  11, 11, 13, 15, 15, 17, 19};

Mesh init_sphere(Device *d, uint res) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    const int rn = (int)pow(4, res);
    const int totalIndexCount = IcoIndexCount * rn;
    const int totalVertexCount = IcoVertexCount + IcoIndexCount * (1 - rn) / (1 - 4);

    indices.resize(totalIndexCount);
    vertices.resize(totalVertexCount);

    for (int i = 0; i < IcoVertexCount; i++) {  // Copy in initial mesh
        vertices[i].pos = IcoVerts[i];
        vertices[i].uv = IcoUvs[i];
    }

    for (int i = 0; i < IcoIndexCount; i++) {
        indices[i] = IcoIndex[i];
    }

    int currentIndexCount = IcoIndexCount;
    int currentVertCount = IcoVertexCount;

    for (int r = 0; r < res; r++) {
        // Now split the triangles.
        // This can be done in place, but needs to keep track of the unique triangles
        //
        //     i+2                 i+2
        //    /   \               /  \
		//   /     \    ---->   m2----m1
        //  /       \          /  \  /  \
		// i---------i+1      i----m0----i+1

        std::unordered_map<uint64_t, int> triangleFromEdge;
        int indexCount = currentIndexCount;

        for (int t = 0; t < indexCount; t += 3) {
            int midpoints[3] = {};

            for (int e = 0; e < 3; e++) {
                int first = indices[t + e];
                int second = indices[t + (t + e + 1) % 3];

                if (first > second) {
                    std::swap(first, second);
                }

                uint64_t hash = (uint64_t)first | (uint64_t)second << (sizeof(uint32_t) * 8);

                auto [triangle, wasNewEdge] = triangleFromEdge.insert({hash, currentVertCount});

                if (wasNewEdge) {
                    vertices[currentVertCount].pos = (vertices[first].pos + vertices[second].pos) / 2.0f;
                    vertices[currentVertCount].uv = (vertices[first].uv + vertices[second].uv) / 2.0f;

                    currentVertCount += 1;
                }

                midpoints[e] = triangle->second;
            }

            int mid0 = midpoints[0];
            int mid1 = midpoints[1];
            int mid2 = midpoints[2];

            indices[currentIndexCount++] = indices[t];
            indices[currentIndexCount++] = mid0;
            indices[currentIndexCount++] = mid2;

            indices[currentIndexCount++] = indices[t + 1];
            indices[currentIndexCount++] = mid1;
            indices[currentIndexCount++] = mid0;

            indices[currentIndexCount++] = indices[t + 2];
            indices[currentIndexCount++] = mid2;
            indices[currentIndexCount++] = mid1;

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

    return Mesh(d, indices, vertices);
}

Mesh init_cone(Device *d, uint res) {
    const int div = 25;
    float step = 2.0 * M_PI / div;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Génération des sommets et indices pour le cône
    for (int i = 0; i <= div; ++i) {
        float alpha = i * step;

        glm::vec3 normal = glm::normalize(glm::vec3(cos(alpha), 1.0f, sin(alpha)));

        // Base du cône
        Vertex baseVertex;
        baseVertex.pos = glm::vec3(cos(alpha), 0.0f, sin(alpha));
        baseVertex.normal = normal;
        baseVertex.uv = glm::vec2(float(i) / div, 0.0f);
        vertices.push_back(baseVertex);

        // Sommet du cône
        Vertex topVertex;
        topVertex.pos = glm::vec3(0.0f, 1.0f, 0.0f);
        topVertex.normal = normal;
        topVertex.uv = glm::vec2(float(i) / div, 1.0f);
        vertices.push_back(topVertex);

        // Ajout des indices pour le GL_TRIANGLE_STRIP
        int index1 = i * 2;
        int index2 = i * 2 + 1;
        indices.push_back(index1);
        indices.push_back(index2);
    }

    return Mesh(d, indices, vertices);
}

Mesh init_cylinder(Device *d, uint res) {
    const int div = 25;
    float step = 2.0 * M_PI / div;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Génération des sommets et indices pour le cylindre
    for (int i = 0; i <= div; ++i) {
        float alpha = i * step;

        glm::vec3 normal = glm::normalize(glm::vec3(cos(alpha), 0.0f, sin(alpha)));

        // Base du cylindre
        Vertex baseVertex;
        baseVertex.pos = glm::vec3(cos(alpha), 0.0f, sin(alpha));
        baseVertex.normal = normal;
        baseVertex.uv = glm::vec2(float(i) / div, 0.0f);
        vertices.push_back(baseVertex);

        // Sommet du cylindre
        Vertex topVertex;
        topVertex.pos = glm::vec3(cos(alpha), 1.0f, sin(alpha));
        topVertex.normal = normal;
        topVertex.uv = glm::vec2(float(i) / div, 1.0f);
        vertices.push_back(topVertex);

        // Ajout des indices pour le GL_TRIANGLE_STRIP
        int index1 = i * 2;
        int index2 = i * 2 + 1;
        indices.push_back(index1);
        indices.push_back(index2);
    }

    return Mesh(d, indices, vertices);
}

Mesh init_cube(Device *d, uint res) {
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

    return Mesh(d, indices, vertices);
}

Mesh::Mesh(Device *d, const BasicShape &b, uint res) {
    switch (b) {
        case BasicShape::Sphere:
            *this = init_sphere(d, res);
            break;
        case BasicShape::Cube:
            *this = init_cube(d, res);
            break;
        default:
            assert("Invalid shape");
            break;
    }
}

}  // namespace TTe