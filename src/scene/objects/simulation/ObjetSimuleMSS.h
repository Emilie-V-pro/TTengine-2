
/** \file ObjetSimuleMSS.h
 \brief Structures de donnes relatives aux objets simules de type masses-ressorts.
 */

#ifndef OBJET_SIMULE_MSS_H
#define OBJET_SIMULE_MSS_H



/** Librairies de base **/
#include <glm/fwd.hpp>


// #include "mesh.h"

// Fichiers de master_meca_sim
#include "MSS.h"
#include "SolveurExpl.h"
#include "SolveurImpl.h"
#include "device.hpp"
#include "scene/objects/simulateObj.hpp"

namespace TTe {


/**
 * \brief Structure de donnees pour un systeme masses-ressorts.
 */
class ObjetSimuleMSS: public SimulateObj
{
public:
    
    /*! Constructeur */
    ObjetSimuleMSS(Device *device, std::string fich_param);
    
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
    void Simulation(glm::vec3 gravite, float viscosite, int Tps);
    
    /*! Gestion des collisions */
    void Collision();
    
    /*! Mise a jour du Mesh (pour affichage) de l objet en fonction des nouvelles positions calculees */
    void updateVertex();
    
    /*! Modification du tableau des normales de chaque sommet */
    void setNormals();
    
    /*! Calcul de la normale a une face  definies par les sommets (a, b, c) */
    void NormaleFace(glm::vec3 &normale, int a, int b, int c);
    
    /*! Acces a la taille du tissu */
    inline glm::vec3 GetTissuSize() {return _Size;}
    
   
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
    
};


}
#endif
