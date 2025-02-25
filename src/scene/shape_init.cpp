#include <sys/types.h>
#include <glm/fwd.hpp>
#include <vector>
#include "device.hpp"
#include "mesh.hpp"
#include "utils.hpp"

namespace TTe {

Mesh init_sphere(Device *d, uint res) {
    const int divBeta = 26;
    const int divAlpha = divBeta / 2;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (int i = 0; i < divAlpha; ++i)
    {
        float alpha = -0.5f * M_PI + float(i) * M_PI / divAlpha;
        float alpha2 = -0.5f * M_PI + float(i + 1) * M_PI / divAlpha;

        for (int j = 0; j < divBeta; ++j)
        {
            float beta = float(j) * 2.f * M_PI / (divBeta - 1);

            // Premier sommet
            Vertex v1;
            v1.pos = glm::vec3(cos(alpha) * cos(beta), sin(alpha), cos(alpha) * sin(beta));
            v1.normal = glm::normalize(v1.pos);
            v1.uv = glm::vec2(beta / (2.0f * M_PI), 0.5f + alpha / M_PI);
            vertices.push_back(v1);

            // Deuxième sommet
            Vertex v2;
            v2.pos = glm::vec3(cos(alpha2) * cos(beta), sin(alpha2), cos(alpha2) * sin(beta));
            v2.normal = glm::normalize(v2.pos);
            v2.uv = glm::vec2(beta / (2.0f * M_PI), 0.5f + alpha2 / M_PI);
            vertices.push_back(v2);

            // Ajout des indices pour former des bandes triangulaires
            int index1 = i * divBeta + j;
            int index2 = (i + 1) * divBeta + j;
            indices.push_back(index1);
            indices.push_back(index2);
        }
    }

    return Mesh(d, indices, vertices);
}

Mesh init_cone(Device *d, uint res){
    const int div = 25;
    float step = 2.0 * M_PI / div;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Génération des sommets et indices pour le cône
    for (int i = 0; i <= div; ++i)
    {
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

Mesh init_cylinder(Device *d, uint res){
    const int div = 25;
    float step = 2.0 * M_PI / div;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Génération des sommets et indices pour le cylindre
    for (int i = 0; i <= div; ++i)
    {
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

Mesh init_cube(Device *d, uint res){
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
    indices.push_back( 2);
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


Mesh::Mesh(Device* d, const BasicShape& b, uint res) {}

}  // namespace TTe