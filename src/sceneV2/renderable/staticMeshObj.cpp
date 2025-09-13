
#include "staticMeshObj.hpp"

#include <cstdio>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

namespace TTe {

StaticMeshObj::StaticMeshObj() {}

StaticMeshObj::~StaticMeshObj() {}

bool checkBlockVisibility(glm::vec3 frustrumBoxPoints[8], glm::vec3 pmin, glm::vec3 pmax, glm::mat4 VPMatrix) {
    int compteur[6] = {0};
    for (int i = 0; i < 8; i++) {
        if (frustrumBoxPoints[i].x < pmin.x) {
            compteur[0]++;
        }
        if (frustrumBoxPoints[i].x > pmax.x) {
            compteur[1]++;
        }
        if (frustrumBoxPoints[i].y < pmin.y) {
            compteur[2]++;
        }
        if (frustrumBoxPoints[i].y > pmax.y) {
            compteur[3]++;
        }
        if (frustrumBoxPoints[i].z < pmin.z) {
            compteur[4]++;
        }
        if (frustrumBoxPoints[i].z > pmax.z) {
            compteur[5]++;
        }
    }
    for (int i = 0; i < 6; i++)
        if (compteur[i] == 8) return false;

    glm::vec4 boxPoint[8] = {
        {pmin.x, pmin.y, pmin.z, 1.0}, {pmin.x, pmin.y, pmax.z, 1.0}, {pmin.x, pmax.y, pmin.z, 1.0}, {pmin.x, pmax.y, pmax.z, 1.0},
        {pmax.x, pmin.y, pmin.z, 1.0}, {pmax.x, pmin.y, pmax.z, 1.0}, {pmax.x, pmax.y, pmin.z, 1.0}, {pmax.x, pmax.y, pmax.z, 1.0},
    };
    for (int i = 0; i < 8; i++) boxPoint[i] = VPMatrix * boxPoint[i];

    int n[6] = {0};
    for (int i = 0; i < 8; i++) {
        if (boxPoint[i].x < -boxPoint[i].w) n[0]++;  // trop a gauche
        if (boxPoint[i].x > boxPoint[i].w) n[1]++;   // a droite

        if (boxPoint[i].y < -boxPoint[i].w) n[2]++;  // en bas
        if (boxPoint[i].y > boxPoint[i].w) n[3]++;   // en haut

        if (boxPoint[i].z < -boxPoint[i].w) n[4]++;  // derriere
        if (boxPoint[i].z > boxPoint[i].w) n[5]++;   // devant
    }
    for (int i = 0; i < 6; i++)
        if (n[i] == 8) return false;

    return true;
}

void matrixTOPminAndPmax(glm::vec3 &pmin, glm::vec3 &pmax, glm::mat4 matrix) {
    glm::vec3 BoxPoints[8];
    BoxPoints[0] = glm::vec3(matrix * glm::vec4(pmin.x, pmin.y, pmin.z, 1));
    BoxPoints[1] = glm::vec3(matrix * glm::vec4(pmin.x, pmin.y, pmax.z, 1));
    BoxPoints[2] = glm::vec3(matrix * glm::vec4(pmin.x, pmax.y, pmin.z, 1));
    BoxPoints[3] = glm::vec3(matrix * glm::vec4(pmin.x, pmax.y, pmax.z, 1));
    BoxPoints[4] = glm::vec3(matrix * glm::vec4(pmax.x, pmin.y, pmin.z, 1));
    BoxPoints[5] = glm::vec3(matrix * glm::vec4(pmax.x, pmin.y, pmax.z, 1));
    BoxPoints[6] = glm::vec3(matrix * glm::vec4(pmax.x, pmax.y, pmin.z, 1));
    BoxPoints[7] = glm::vec3(matrix * glm::vec4(pmax.x, pmax.y, pmax.z, 1));
    pmin = {FLT_MAX, FLT_MAX, FLT_MAX};
    pmax = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
    for (int i = 1; i < 8; i++) {
        pmin = min(pmin, BoxPoints[i]);
        pmax = max(pmax, BoxPoints[i]);
    }
}

void StaticMeshObj::render(CommandBuffer &cmd, RenderData &renderData) {
    glm::mat4 IVPmatrix = glm::inverse(renderData.cameras->at(0)->getProjectionMatrix() * renderData.cameras->at(0)->getViewMatrix());
    glm::mat4 VPmatrix = renderData.cameras->at(0)->getProjectionMatrix() * renderData.cameras->at(0)->getViewMatrix();

    glm::vec4 frustrumBoxPointW[8];
    frustrumBoxPointW[0] = (IVPmatrix * glm::vec4(1, 1, 1, 1));
    frustrumBoxPointW[1] = (IVPmatrix * glm::vec4(1, 1, -1, 1));
    frustrumBoxPointW[2] = (IVPmatrix * glm::vec4(1, -1, 1, 1));
    frustrumBoxPointW[3] = (IVPmatrix * glm::vec4(1, -1, -1, 1));
    frustrumBoxPointW[4] = (IVPmatrix * glm::vec4(-1, 1, 1, 1));
    frustrumBoxPointW[5] = (IVPmatrix * glm::vec4(-1, 1, -1, 1));
    frustrumBoxPointW[6] = (IVPmatrix * glm::vec4(-1, -1, 1, 1));
    frustrumBoxPointW[7] = (IVPmatrix * glm::vec4(-1, -1, -1, 1));
    glm::vec3 frustrumBoxPoint[8];
    for (int i = 0; i < 8; i++) {
        frustrumBoxPoint[i] = glm::vec3(frustrumBoxPointW[i].x, frustrumBoxPointW[i].y, frustrumBoxPointW[i].z) / frustrumBoxPointW[i].w;
    }

    glm::vec3 pmin = this->mesh->getBoundingBox().pmin;
    glm::vec3 pmax = this->mesh->getBoundingBox().pmax;
    matrixTOPminAndPmax(pmin, pmax, wMatrix());

    std::stack<uint32_t> bvh_stack;
    bvh_stack.push(0);
    int truc = renderData.drawCommands.size();
    // inspired from https://github.com/SebLague/Ray-Tracing/
    while (!bvh_stack.empty()) {
        uint32_t index = bvh_stack.top();
        bvh_stack.pop();

        // if leaf
        if (mesh->bvh[index].nbTriangleToDraw <= 1000) {
            glm::vec3 pmin = mesh->bvh[index].bbox.pmin;
            glm::vec3 pmax = mesh->bvh[index].bbox.pmax;
            matrixTOPminAndPmax(pmin, pmax, wMatrix());
            if (checkBlockVisibility(frustrumBoxPoint, pmin, pmax, VPmatrix)) {
                VkDrawIndexedIndirectCommand drawCmd;
                drawCmd.firstIndex = mesh->getFirstIndex() + mesh->bvh[index].indicies_index;
                drawCmd.vertexOffset = mesh->getFirstVertex();
                drawCmd.indexCount = mesh->bvh[index].nbTriangleToDraw * 3;
                drawCmd.instanceCount = 1;
                drawCmd.firstInstance = this->id;

                renderData.drawCommands.push_back(drawCmd);
            }

        } else {
  
            bvh_stack.push( mesh->bvh[index].index);
            bvh_stack.push(mesh->bvh[index].index + 1);
        }
    }
    

    // if (checkBlockVisibility(frustrumBoxPoint, pmin, pmax, VPmatrix)) {
    //     VkDrawIndexedIndirectCommand drawCmd;
    //     drawCmd.firstIndex = mesh->getFirstIndex();
    //     drawCmd.vertexOffset = mesh->getFirstVertex();
    //     drawCmd.indexCount = mesh->nbIndicies();
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
    bbox = mesh->getBoundingBox();

    // apply transformation to bounding box
    tmp.pmin = glm::vec3(wMatrix() * glm::vec4(bbox.pmin, 1.0f));
    tmp.pmax = glm::vec3(wMatrix() * glm::vec4(bbox.pmax, 1.0f));
    bbox.pmin = glm::min(tmp.pmin, tmp.pmax);
    bbox.pmax = glm::max(tmp.pmin, tmp.pmax);

    for (auto &child : children) {
        BoundingBox childbb = child->computeBoundingBox();
        bbox.pmin = glm::min(bbox.pmin, tmp.pmin);
        bbox.pmax = glm::max(bbox.pmax, tmp.pmax);
    };

    if (bbox.pmin.x == bbox.pmax.x) {
        bbox.pmin.x = bbox.pmax.x - 0.000001f;
    }
    if (bbox.pmin.y == bbox.pmax.y) {
        bbox.pmin.y = bbox.pmax.y - 0.000001f;
    }
    if (bbox.pmin.z == bbox.pmax.z) {
        bbox.pmin.z = bbox.pmax.z - 0.000001f;
    }
    return bbox;
}

SceneHit StaticMeshObj::hit(glm::vec3 &ro, glm::vec3 &rd) {
    // transform ray to local space
    glm::vec3 localRo = glm::inverse(wMatrix()) * glm::vec4(ro, 1.0f);
    glm::vec3 localRd = glm::normalize(glm::inverse(wMatrix()) * glm::vec4(rd, 0.0f));

    return mesh->hit(localRo, localRd);
}
}  // namespace TTe