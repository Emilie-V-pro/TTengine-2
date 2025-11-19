
#include "staticMeshObj.hpp"

#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

#include "struct.hpp"

namespace TTe {

StaticMeshObj::StaticMeshObj() {}

StaticMeshObj::~StaticMeshObj() {}

bool checkBlockVisibility(glm::vec3 p_frustrum_box_points[8], glm::vec3 p_pmin, glm::vec3 p_pmax, glm::mat4 p_VP_matrix) {
    int compteur[6] = {0};
    for (int i = 0; i < 8; i++) {
        if (p_frustrum_box_points[i].x < p_pmin.x) {
            compteur[0]++;
        }
        if (p_frustrum_box_points[i].x > p_pmax.x) {
            compteur[1]++;
        }
        if (p_frustrum_box_points[i].y < p_pmin.y) {
            compteur[2]++;
        }
        if (p_frustrum_box_points[i].y > p_pmax.y) {
            compteur[3]++;
        }
        if (p_frustrum_box_points[i].z < p_pmin.z) {
            compteur[4]++;
        }
        if (p_frustrum_box_points[i].z > p_pmax.z) {
            compteur[5]++;
        }
    }
    for (int i = 0; i < 6; i++)
        if (compteur[i] == 8) return false;

    glm::vec4 box_point[8] = {
        {p_pmin.x, p_pmin.y, p_pmin.z, 1.0}, {p_pmin.x, p_pmin.y, p_pmax.z, 1.0}, {p_pmin.x, p_pmax.y, p_pmin.z, 1.0},
        {p_pmin.x, p_pmax.y, p_pmax.z, 1.0}, {p_pmax.x, p_pmin.y, p_pmin.z, 1.0}, {p_pmax.x, p_pmin.y, p_pmax.z, 1.0},
        {p_pmax.x, p_pmax.y, p_pmin.z, 1.0}, {p_pmax.x, p_pmax.y, p_pmax.z, 1.0},
    };
    for (int i = 0; i < 8; i++) box_point[i] = p_VP_matrix * box_point[i];

    int n[6] = {0};
    for (int i = 0; i < 8; i++) {
        if (box_point[i].x < -box_point[i].w) n[0]++;  // trop a gauche
        if (box_point[i].x > box_point[i].w) n[1]++;   // a droite

        if (box_point[i].y < -box_point[i].w) n[2]++;  // en bas
        if (box_point[i].y > box_point[i].w) n[3]++;   // en haut

        if (box_point[i].z < -box_point[i].w) n[4]++;  // derriere
        if (box_point[i].z > box_point[i].w) n[5]++;   // devant
    }
    for (int i = 0; i < 6; i++)
        if (n[i] == 8) return false;

    return true;
}

void matrixTOPminAndPmax(glm::vec3& p_pmin, glm::vec3& p_pmax, glm::mat4 p_matrix) {
    glm::vec3 p_box_points[8];
    p_box_points[0] = glm::vec3(p_matrix * glm::vec4(p_pmin.x, p_pmin.y, p_pmin.z, 1));
    p_box_points[1] = glm::vec3(p_matrix * glm::vec4(p_pmin.x, p_pmin.y, p_pmax.z, 1));
    p_box_points[2] = glm::vec3(p_matrix * glm::vec4(p_pmin.x, p_pmax.y, p_pmin.z, 1));
    p_box_points[3] = glm::vec3(p_matrix * glm::vec4(p_pmin.x, p_pmax.y, p_pmax.z, 1));
    p_box_points[4] = glm::vec3(p_matrix * glm::vec4(p_pmax.x, p_pmin.y, p_pmin.z, 1));
    p_box_points[5] = glm::vec3(p_matrix * glm::vec4(p_pmax.x, p_pmin.y, p_pmax.z, 1));
    p_box_points[6] = glm::vec3(p_matrix * glm::vec4(p_pmax.x, p_pmax.y, p_pmin.z, 1));
    p_box_points[7] = glm::vec3(p_matrix * glm::vec4(p_pmax.x, p_pmax.y, p_pmax.z, 1));
    p_pmin = {FLT_MAX, FLT_MAX, FLT_MAX};
    p_pmax = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
    for (int i = 1; i < 8; i++) {
        p_pmin = min(p_pmin, p_box_points[i]);
        p_pmax = max(p_pmax, p_box_points[i]);
    }
}

std::vector<MeshBlock> StaticMeshObj::getMeshBlock(uint32_t p_nb_max_triangle) {
    std::vector<MeshBlock> returnValue;
    std::stack<uint32_t> bvh_stack;
    bvh_stack.push(0);
    while (!bvh_stack.empty()) {
        uint32_t index = bvh_stack.top();
        bvh_stack.pop();

        // if leaf
        if (m_mesh->bvh[index].nb_triangle_to_draw <= p_nb_max_triangle) {
            glm::vec3 pmin = m_mesh->bvh[index].bbox.pmin;
            glm::vec3 pmax = m_mesh->bvh[index].bbox.pmax;
            MeshBlock mesh_block;

            mesh_block.indexOffset = m_mesh->getFirstIndex() + m_mesh->bvh[index].indicies_index;
            mesh_block.vertexOffset = m_mesh->getFirstVertex();
            mesh_block.indexSize = m_mesh->bvh[index].nb_triangle_to_draw * 3;
            mesh_block.pmin = pmin;
            mesh_block.pmax = pmax;
            mesh_block.instancesID = this->m_id;
            returnValue.push_back(mesh_block);
        } else {
            bvh_stack.push(m_mesh->bvh[index].index);
            bvh_stack.push(m_mesh->bvh[index].index + 1);
        }
    }
    return returnValue;
}

void StaticMeshObj::render(CommandBuffer& p_cmd, RenderData& p_render_data) {
    glm::mat4 IVP_matrix =
        glm::inverse(p_render_data.cameras->at(0)->getProjectionMatrix() * p_render_data.cameras->at(0)->getViewMatrix());
    glm::mat4 VP_matrix = p_render_data.cameras->at(0)->getProjectionMatrix() * p_render_data.cameras->at(0)->getViewMatrix();

    glm::vec4 frustrum_box_point_w[8];
    frustrum_box_point_w[0] = (IVP_matrix * glm::vec4(1, 1, 1, 1));
    frustrum_box_point_w[1] = (IVP_matrix * glm::vec4(1, 1, -1, 1));
    frustrum_box_point_w[2] = (IVP_matrix * glm::vec4(1, -1, 1, 1));
    frustrum_box_point_w[3] = (IVP_matrix * glm::vec4(1, -1, -1, 1));
    frustrum_box_point_w[4] = (IVP_matrix * glm::vec4(-1, 1, 1, 1));
    frustrum_box_point_w[5] = (IVP_matrix * glm::vec4(-1, 1, -1, 1));
    frustrum_box_point_w[6] = (IVP_matrix * glm::vec4(-1, -1, 1, 1));
    frustrum_box_point_w[7] = (IVP_matrix * glm::vec4(-1, -1, -1, 1));
    glm::vec3 frustrum_box_point[8];
    for (int i = 0; i < 8; i++) {
        frustrum_box_point[i] =
            glm::vec3(frustrum_box_point_w[i].x, frustrum_box_point_w[i].y, frustrum_box_point_w[i].z) / frustrum_box_point_w[i].w;
    }

    glm::vec3 pmin = this->m_mesh->getBoundingBox().pmin;
    glm::vec3 pmax = this->m_mesh->getBoundingBox().pmax;
    matrixTOPminAndPmax(pmin, pmax, wMatrix());

    std::stack<uint32_t> bvh_stack;
    bvh_stack.push(0);

    // inspired from https://github.com/SebLague/Ray-Tracing/
    while (!bvh_stack.empty()) {
        uint32_t index = bvh_stack.top();
        bvh_stack.pop();

        // if leaf
        if (m_mesh->bvh[index].nb_triangle_to_draw <= 1000) {
            glm::vec3 pmin = m_mesh->bvh[index].bbox.pmin;
            glm::vec3 pmax = m_mesh->bvh[index].bbox.pmax;
            matrixTOPminAndPmax(pmin, pmax, wMatrix());
            if (checkBlockVisibility(frustrum_box_point, pmin, pmax, VP_matrix)) {
                VkDrawIndexedIndirectCommand draw_cmd;
                draw_cmd.firstIndex = m_mesh->getFirstIndex() + m_mesh->bvh[index].indicies_index;
                draw_cmd.vertexOffset = m_mesh->getFirstVertex();
                draw_cmd.indexCount = m_mesh->bvh[index].nb_triangle_to_draw * 3;
                draw_cmd.instanceCount = 1;
                draw_cmd.firstInstance = this->m_id;

                p_render_data.draw_commands.push_back(draw_cmd);
            }

        } else {
            bvh_stack.push(m_mesh->bvh[index].index);
            bvh_stack.push(m_mesh->bvh[index].index + 1);
        }
    }

    // if (checkBlockVisibility(frustrumBoxPoint, pmin, pmax, VPmatrix)) {
    //     VkDrawIndexedIndirectCommand drawCmd;
    //     drawCmd.firstIndex = m_mesh->getFirstIndex();
    //     drawCmd.vertexOffset = m_mesh->getFirstVertex();
    //     drawCmd.indexCount = m_mesh->nbIndicies();
    //     drawCmd.instanceCount = 1;
    //     drawCmd.firstInstance = this->id;

    //     renderData.drawCommands.push_back(drawCmd);
    //     printf("AAAAAAAAAAAAAAAAAALLLLLLLLLLLLLLLLLLER\n");
    // } else {
    //     printf("ya quedal\n");
    // }
}

BoundingBox StaticMeshObj::computeBoundingBox() {
    BoundingBox tmp;
    m_bbox = m_mesh->getBoundingBox();

    // apply transformation to bounding box
    tmp.pmin = glm::vec3(wMatrix() * glm::vec4(m_bbox.pmin, 1.0f));
    tmp.pmax = glm::vec3(wMatrix() * glm::vec4(m_bbox.pmax, 1.0f));
    m_bbox.pmin = glm::min(tmp.pmin, tmp.pmax);
    m_bbox.pmax = glm::max(tmp.pmin, tmp.pmax);

    for (auto& child : m_children) {
        BoundingBox childbb = child->computeBoundingBox();
        m_bbox.pmin = glm::min(childbb.pmin, tmp.pmin);
        m_bbox.pmax = glm::max(childbb.pmax, tmp.pmax);
    };

    if (m_bbox.pmin.x == m_bbox.pmax.x) {
        m_bbox.pmin.x = m_bbox.pmax.x - 0.000001f;
    }

    if (m_bbox.pmin.y == m_bbox.pmax.y) {
        m_bbox.pmin.y = m_bbox.pmax.y - 0.000001f;
    }

    if (m_bbox.pmin.z == m_bbox.pmax.z) {
        m_bbox.pmin.z = m_bbox.pmax.z - 0.000001f;
    }

    return m_bbox;
}

SceneHit StaticMeshObj::hit(glm::vec3& p_ro, glm::vec3& p_rd) {
    // transform ray to local space
    glm::vec3 local_ro = glm::inverse(wMatrix()) * glm::vec4(p_ro, 1.0f);
    glm::vec3 local_rd = glm::normalize(glm::inverse(wMatrix()) * glm::vec4(p_rd, 0.0f));

    return m_mesh->hit(local_ro, local_rd);
}
}  // namespace TTe