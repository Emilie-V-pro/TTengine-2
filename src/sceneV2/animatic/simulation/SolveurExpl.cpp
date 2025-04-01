/*
 * SolveurExpl.cpp : Application schemas semi-implicite sur les objets.
 * Copyright (C) 2016 Florence Zara, LIRIS
 *               florence.zara@liris.univ-lyon1.fr
 *               http://liris.cnrs.fr/florence.zara/
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/** \file Calculs.cpp
 Fonctions de calculs communes aux objets simules.
 \brief Fonctions de calculs communes aux objets simules.
 */

#include <math.h>

#include <cstdio>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <vector>

#include "struct.hpp"
#include "utils.hpp"

// #include "vec.h"
// #include "Viewer.h"
#include "SolveurExpl.h"

using namespace std;

/**
 * Calcul de l acceleration des particules
 * avec ajout de la gravite aux forces des particules
 * et ajout de la force due au vent sur une des particules du maillage
 * et reinitialisation des forces.
 */
namespace TTe {

void SolveurExpl::CalculAccel_ForceGravite(
    glm::vec3 g, int nb_som, float t, std::vector<glm::vec3> &A, std::vector<glm::vec3> &Force, std::vector<float> &M) {
    // #pragma omp parallel for schedule(dynamic, 1)

    for (int i = 0; i < nb_som; ++i) {
        if (M[i] == 0.0) {
            A[i] = glm::vec3(0.0, 0.0, 0.0);
            continue;
        }
        A[i] = Force[i] / M[i] + g + glm::normalize(glm::vec3(1.0, 0.0, 1.0)) * 5.f * glm::sin(t);
        Force[i] = glm::vec3(0.0, 0.0, 0.0);
    }

    //// Cas SPH
    // On a calcule dans Force[i] : fij / rho_i
    // Il ne reste qu a ajoute le vecteur g
    // a_i = fij / rho_i + g

}  // void

/*! Calcul des vitesses et positions :
 *  Formule d Euler semi-implicite :
 *  x'(t+dt) = x'(t) + dt x"(t)
 *  x(t+dt) = x(t) + dt x'(t+dt)
 */
void SolveurExpl::Solve(
    float visco, int nb_som, float deltaT, std::vector<glm::vec3> &A, std::vector<glm::vec3> &V, std::vector<Vertex> &P) {
    // deltaT = min(deltaT, 0.0005f);

    // #pragma omp parallel for schedule(dynamic, 1)
    for (int i = 0; i < nb_som; ++i) {
        V[i] = (V[i] + deltaT * A[i]) * visco;
        P[i].pos = P[i].pos + deltaT * V[i];
    }
}  // void
}  // namespace TTe