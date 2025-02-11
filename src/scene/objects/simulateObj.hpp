#pragma once
#include <string>

#include "scene/mesh.hpp"
#include "scene/object.hpp"
namespace TTe {
class SimulateObj : public Object {
   public:
    // SimulateObj(){}
    SimulateObj(std::string &paramFile){
        Param_mesh(paramFile);
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
    virtual void Collision() = 0;

    /*! Simulation de l objet */
    virtual void Simulation(glm::vec3 gravite, float viscosite, int Tps) = 0;

    /*! Interaction avec l utilisateur */
    void Interaction(glm::ivec2 MousePos);

    /*! Affichage des positions de chaque sommet */
    void AffichagePos(int tps);

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
    Mesh mesh;
};
}  // namespace TTe