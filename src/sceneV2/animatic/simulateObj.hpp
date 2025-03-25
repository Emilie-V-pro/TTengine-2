#pragma once
#include <glm/fwd.hpp>
#include <memory>
#include <string>

#include "device.hpp"
#include "sceneV2/Icollider.hpp"
#include "sceneV2/Irenderable.hpp"
#include "sceneV2/mesh.hpp"
#include "sceneV2/node.hpp"
namespace TTe {
class SimulateObj {
   public:
    // SimulateObj(){}
    SimulateObj(Device *device, std::string &paramFile) : device(device) {
        Param_mesh(paramFile);
        mesh = Mesh(device);
    }
    // ~SimulateObj();

    /*! Lecture des parametres lies au maillage */
    void Param_mesh(std::string fich_param);

    /*! Initialisation des tableaux des sommets a partir du fichier de donnees de l objet */
    virtual void initObjetSimule() = 0;

    /*! Mise a jour du Mesh (pour affichage) de l objet en fonction des nouvelles positions calculees */
    virtual void updateVertex() = 0;

    /*! Operation de creation du maillage (servant a l affichage) */
    virtual void initMeshObjet() = 0;

    /*! Gestion des collisions */
    virtual void Collision(std::vector<std::shared_ptr<ICollider>> &collisionObjects) = 0;

    virtual void applyForceGravity(float t, glm::vec3 g) = 0;
    virtual void solveExplicit(float visco, float deltaT) = 0;

    /*! Interaction avec l utilisateur */
    void Interaction(glm::ivec2 MousePos);

    /*! Affichage des positions de chaque sommet */
    void AffichagePos(int tps);
    Mesh mesh;

    protected:
    /// Fichier de donnees contenant les points
    std::string _Fich_Points;

    /// Fichier de donnees contenant les masses
    std::string _Fich_Masses;

    /// Interaction avec l utilisateur ou non
    std::string _Interaction;

    /// valeur d'absorption de la vitesse en cas de collision:
    /// 1=la particule repart aussi vite, 0=elle s'arrete
    float _Friction = 1.0f;

    /// Declaration du tableau des vitesses
    std::vector<glm::vec3> V;

    /// Declaration du tableau des accelerations
    std::vector<glm::vec3> A;

    /// Declaration du tableau des forces
    std::vector<glm::vec3> Force;

    /// Declaration du tableau des masses
    std::vector<float> M;
    
    Device *device;
};
}  // namespace TTe