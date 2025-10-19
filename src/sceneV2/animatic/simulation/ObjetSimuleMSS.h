
/** \file ObjetSimuleMSS.h
 \brief Structures de donnes relatives aux objets simules de m_type masses-ressorts.
 */

#ifndef OBJET_SIMULE_MSS_H
#define OBJET_SIMULE_MSS_H



/** Librairies de base **/
#include <cstdint>
#include <glm/fwd.hpp>
#include <memory>


// #include "mesh.h"

// Fichiers de master_meca_sim
#include "MSS.h"
#include "SolveurExpl.h"
#include "SolveurImpl.h"
#include "device.hpp"
#include "sceneV2/Icollider.hpp"
#include "sceneV2/animatic/simulateObj.hpp"



namespace TTe {


/**
 * \brief Structure de donnees pour un systeme masses-ressorts.
 */
class ObjetSimuleMSS: public SimulateObj
{
public:

    ObjetSimuleMSS() {};
    
    /*! Constructeur */
    ObjetSimuleMSS(Device *device, std::filesystem::path fich_param);


    // copy constructor
    ObjetSimuleMSS(const ObjetSimuleMSS &other) : SimulateObj(other) {
        this->_Fich_Masses = other._Fich_Masses;
        this->_Fich_Points = other._Fich_Points;
        this->_Fich_Texture = other._Fich_Texture;
        this->_Fich_FaceSet = other._Fich_FaceSet;
   
      
        this->_Size = other._Size;
        this->_SystemeMasseRessort = other._SystemeMasseRessort;
        this->_Integration = other._Integration;
        this->_SolveurExpl = other._SolveurExpl;
        this->_SolveurImpl = other._SolveurImpl;
    }

    // copy assignment

    ObjetSimuleMSS &operator=(const ObjetSimuleMSS &other) {
        if (this != &other) {

            SimulateObj::operator=(other);
            this->_Fich_Masses = other._Fich_Masses;
            this->_Fich_Points = other._Fich_Points;
            this->_Fich_Texture = other._Fich_Texture;
            this->_Fich_FaceSet = other._Fich_FaceSet;
            this->_Size = other._Size;
            this->_SystemeMasseRessort = other._SystemeMasseRessort;
            this->_Integration = other._Integration;
            this->_SolveurExpl = other._SolveurExpl;
            this->_SolveurImpl = other._SolveurImpl;
        }
        return *this;
    }

    // move constructor
    ObjetSimuleMSS(ObjetSimuleMSS &&other) : SimulateObj(other) {
        this->_Fich_Masses = other._Fich_Masses;
        this->_Fich_Points = other._Fich_Points;
        this->_Fich_Texture = other._Fich_Texture;
        this->_Fich_FaceSet = other._Fich_FaceSet;
        this->_Size = other._Size;
        this->_SystemeMasseRessort = other._SystemeMasseRessort;
        this->_Integration = other._Integration;
        this->_SolveurExpl = other._SolveurExpl;
        this->_SolveurImpl = other._SolveurImpl;
    }

    // move assignment
    ObjetSimuleMSS &operator=(ObjetSimuleMSS &&other) {
        if (this != &other) {
            SimulateObj::operator=(other);
            this->_Fich_Masses = other._Fich_Masses;
            this->_Fich_Points = other._Fich_Points;
            this->_Fich_Texture = other._Fich_Texture;
            this->_Fich_FaceSet = other._Fich_FaceSet;
            this->_Size = other._Size;
            this->_SystemeMasseRessort = other._SystemeMasseRessort;
            this->_Integration = other._Integration;
            this->_SolveurExpl = other._SolveurExpl;
            this->_SolveurImpl = other._SolveurImpl;
        }
        return *this;
    }

    
    /*! Lecture des parametres lies au systeme masses-ressorts */
    void Param_mss(std::string fich_param);
    
    /*! Initialisation des tableaux des sommets
     et construction du systeme masses-ressorts
     a partir du fichier de donnees de l objet */
    void initObjetSimule();
    
    /*! Creation du maillage (pour affichage) de l objet simule */
    void initMeshObjet();
  
    /*! Calcul des forces appliquees sur les sommets dues aux ressorts */
    void CalculForceSpring();

    /*! Simulation de l objet */
    void simulation(glm::vec3 gravite, float viscosite, uint32_t tick, float dt, float t, std::vector<std::shared_ptr<ICollider>> &collisionObjects);
    void render(CommandBuffer &cmd, RenderData &renderData);
    
    void applyForceGravity(float t, glm::vec3 g);
    void solveExplicit(float visco, float deltaT);
    
    /*! Gestion des collisions */
    void Collision(std::vector<std::shared_ptr<ICollider>> &collisionObjects);
    
    /*! Mise a jour du Mesh (pour affichage) de l objet en fonction des nouvelles positions calculees */
    void updateVertex();
    
    /*! Modification du tableau des normales de chaque sommet */
    void setNormals();
    
    /*! Calcul de la normale a une face  definies par les sommets (a, b, c) */
    void NormaleFace(glm::vec3 &normale, int a, int b, int c);
    
    /*! Acces a la taille du tissu */
    inline glm::vec3 GetTissuSize() {return _Size;}

    void attachToNode(uint32_t id, std::shared_ptr<Node> node);

    void setMaterial(uint32_t id);
    
   
    /// Fichier de donnees contenant les textures
    std::string _Fich_Texture;
    
    /// Fichier de donnees contenant les faceSet
    std::string _Fich_FaceSet;
    
    /// Longueur du tissu dans chacune des directions x,y,z
    glm::vec3 _Size;
    
    
    /// Declaration du systeme masse-ressort (maillage point de vue de la simulation)
    MSS * _SystemeMasseRessort;
    
    /// Choix du schema d integration
    std::string _Integration;
    
    /// SolveurExpl : schema d integration semi-implicite
    SolveurExpl *_SolveurExpl;
    
    /// SolveurImpl : schema d integration implicite 
    SolveurImpl *_SolveurImpl;

    std::map<uint32_t, std::shared_ptr<Node>> attachedNodes;
    
};


}
#endif
