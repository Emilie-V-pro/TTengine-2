#pragma once

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <utility>
#include <vector>



#include "sceneV2/Ianimatic.hpp"
#include "sceneV2/Icollider.hpp"
#include "sceneV2/Irenderable.hpp"
#include "sceneV2/animatic/skeleton/BVH.h"
#include "sceneV2/collision/collision_obj.hpp"
#include "sceneV2/node.hpp"
namespace TTe {
class SkeletonObj : public Node, public IAnimatic, public IRenderable, public ICollider {
   public:
    class SkeletonNode : public Node {
    };

    //! Cr�er un squelette ayant la m�me structure que d�finit dans le BVH c'est � dire
    //! creer le tableau de SkeletonJoint � la bonne taille, avec les parentId initials� pour chaque case
    void init(BVH bvh);

    //! Renvoie la position de l'articulation i en multipliant le m_l2w par le Point(0,0,0)
    glm::vec3 getJointPosition(int i) const;

    //! Renvoie l'identifiant de l'articulation p�re de l'articulation num�ro i
    int getParentId(const int i) const;

    //! Renvoie le nombre d'articulation
    int numberOfJoint() const { return (int)m_joints_1.size(); }

    //! Positionne ce squelette dans la position n du BVH.
    //! Assez proche de la fonction r�cursive (question 1), mais range la matrice (Transform)
    //! dans la case du tableau. Pour obtenir la matrice allant de l'articulation local vers le monde,
    //! il faut multiplier la matrice allant de l'articulation vers son p�re � la matrice du p�re allant de
    //! l'articulation du p�re vers le monde.
    void setPose(const BVH& bvh, int frameNumber);


    void simulation(glm::vec3 gravite, float viscosite, uint32_t tick, float dt, float t, std::vector<std::shared_ptr<ICollider>> &collisionObjects);
    void render(CommandBuffer &cmd, GraphicPipeline &pipeline, std::vector<Mesh> &meshes,  std::map<BasicShape, Mesh> basicMeshes);
    void collisionPos(glm::vec3 &pos, glm::vec3 &vitesse);
    //! Positionne ce squelette entre la position frameNbSrc du BVH Src et la position frameNbDst du bvh Dst
    // void setPoseInterpolation(const BVH& bvhSrc, int frameNbSrc, const BVH& bvhDst, int frameNbDst, float t);

    //! Positionne ce squelette entre la position frameNbSrc du BVH Src et la position frameNbDst du bvh Dst
    //! idem � setPoseInterpolation mais interpole avec des quaternions sur chaque articulations
    // void setPoseInterpolationQ(const BVH& bvhSrc, int frameNbSrc, const BVH& bvhDst, int frameNbDst, float t);

    //! Calcule la distance entre deux poses
    //! precond: les deux squelettes doivent avoir le
    //! m�me nombre d'articulations (m�me structure d'arbre)
    //! ==> Sera utile lors de la construction du graphe d'animation
    // friend float distance(const CASkeleton& a, const CASkeleton& b);
   private:

    int lastFrame = 0;
    std::vector<std::pair<glm::vec3, glm::vec3>> coliders;
    std::vector<std::shared_ptr<Node>> m_joints_1;
    std::vector<std::shared_ptr<Node>> m_joints_2;
    std::vector<std::shared_ptr<Node>> m_joints_final;
    BVH m_bvh;

    float interpol;

};
}  // namespace TTe