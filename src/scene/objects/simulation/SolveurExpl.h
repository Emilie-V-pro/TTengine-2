//
//  SolveurExpl.h
//
//
//  Created by fzara on 15/12/2016.
//
//

#ifndef Solveur_Expl_h
#define Solveur_Expl_h

/** Librairies de base **/
#include <glm/fwd.hpp>
#include <vector>
#include "utils.hpp"

// Fichiers de gkit2light
// #include "mesh.h"

// Fichiers de master_meca_sim

namespace TTe {

/*
 * Class pour le schema d integration d'Euler semi-implicite.
 */
class SolveurExpl {
   public:
    /*! Constructeur */
    SolveurExpl() {}

    /*! Calcul des accelerations (avec ajout de la gravite aux forces) */
    void CalculAccel_ForceGravite(glm::vec3 g, int nb_som, std::vector<glm::vec3> &A, std::vector<glm::vec3> &Force, std::vector<float> &M);

    /*! Calcul des vitesses et positions */
    void Solve(float visco, int nb_som, int Tps, std::vector<glm::vec3> &A, std::vector<glm::vec3> &V, std::vector<Vertex> &P);

    /// Pas de temps
    float _delta_t;
};

}  // namespace TTe

#endif
