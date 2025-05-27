/*
 * CalculsMSS.cpp :
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

/** \file CalculsMSS.cpp
Programme calculant pour chaque particule i d un MSS son etat au pas de temps suivant
 (methode d 'Euler semi-implicite) : principales fonctions de calculs.
\brief Fonctions de calculs de la methode semi-implicite sur un systeme masses-ressorts.
*/

#include <math.h>

#include <chrono>
#include <glm/ext/quaternion_common.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/matrix.hpp>
#include <iostream>
#include <memory>

// #include "ObjetSimule.h"
#include "MSS.h"
#include "ObjetSimuleMSS.h"
#include "sceneV2/Icollider.hpp"
// #include "Viewer.h"

using namespace std;

namespace TTe {

/**
 * Calcul des forces appliquees sur les particules du systeme masses-ressorts.
 */
void ObjetSimuleMSS::CalculForceSpring() {
    // #pragma omp parallel for schedule(dynamic, 1)
    // #pragma omp parallel for
    for (auto &ressort : _SystemeMasseRessort->GetRessortList()) {
        Particule *particule1 = ressort->GetParticuleA();
        Particule *particule2 = ressort->GetParticuleB();

        glm::vec3 direction = mesh.verticies[particule2->GetId()].pos - mesh.verticies[particule1->GetId()].pos;

        glm::vec3 direction_norm = glm::normalize(direction);
        glm::vec3 Fe = ressort->GetRaideur() * (glm::length(direction) - (ressort->GetLrepos() * 0.75f)) * direction_norm;
        glm::vec3 Fv =
            ressort->GetAmortissement() * glm::dot((V[particule1->GetId()] - V[particule2->GetId()]), direction_norm) * direction_norm;
        this->Force[particule1->GetId()] += Fe + Fv;
        this->Force[particule2->GetId()] -= Fe + Fv;
    }

    /// f = somme_i (ki * (l(i,j)-l_0(i,j)) * uij ) + (nuij * (vi - vj) * uij) + (m*g) + force_ext

    /// Rq : Les forces dues a la gravite et au vent sont ajoutees lors du calcul de l acceleration

}  // void

void ObjetSimuleMSS::applyForceGravity(float t, glm::vec3 g) {
    // set gravity to object space
    g = glm::inverse(wNormalMatrix()) * glm::vec4(g, 0.0);
    glm::vec3 wind = glm::inverse(wNormalMatrix()) * glm::normalize(glm::vec3(1.0, 0.0, 1.0)) * 5.f * glm::sin(t);
    // #pragma omp parallel for
    for (int i = 0; i < mesh.verticies.size(); ++i) {
        if (attachedNodes.find(i) != attachedNodes.end()) {
            glm::vec3 pos = attachedNodes[i]->wMatrix()[3];
            mesh.verticies[i].pos = pos;
        }
        if (M[i] == 0.0) {
            A[i] = glm::vec3(0.0, 0.0, 0.0);

            continue;
        }
        A[i] = (Force[i] + g + wind) / M[i] ;
        Force[i] = glm::vec3(0.0, 0.0, 0.0);
    }
}

void ObjetSimuleMSS::solveExplicit(float visco, float deltaT) {
    deltaT = min(deltaT, 0.001f);

    // #pragma omp parallel for schedule(dynamic, 1)
    // #pragma omp parallel for
    for (int i = 0; i < mesh.verticies.size(); ++i) {
        V[i] = (V[i] + deltaT * A[i]) * visco;
        mesh.verticies[i].pos = mesh.verticies[i].pos + deltaT * V[i];
    }
}

/**
 * Gestion des collisions avec le sol.
 */
void ObjetSimuleMSS::Collision(std::vector<std::shared_ptr<ICollider>> &collisionObjects) {
    glm::mat4 wMatrix = this->wMatrix();
    glm::mat4 wInvMatrix = glm::inverse(this->wMatrix());

	// time the execution
	// auto start = std::chrono::high_resolution_clock::now();

	

#pragma omp parallel for
    for (int i = 0; i < mesh.verticies.size(); ++i) {
        if (M[i] == 0 || i % 2 == 0) continue;

        for (auto &collisionObject : collisionObjects) {
            // mesh.verticies[i].pos = wMatrix * glm::vec4(mesh.verticies[i].pos, 1);
            collisionObject->collisionPos(mesh.verticies[i].pos, V[i]);
            // mesh.verticies[i].pos = wInvMatrix * glm::vec4(mesh.verticies[i].pos, 1);
        }
    }
	auto end = std::chrono::high_resolution_clock::now();
	// auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	// std::cout << "Collision time: " << duration.count() << "\n";
    /// Arret de la vitesse quand touche le plan

}  // void

}  // namespace TTe