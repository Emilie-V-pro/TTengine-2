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
#include <glm/ext/quaternion_common.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <iostream>
#include <memory>


// #include "ObjetSimule.h"
#include "ObjetSimuleMSS.h"
#include "MSS.h"
#include "sceneV2/Icollider.hpp"
// #include "Viewer.h"

using namespace std;




namespace TTe {


/**
* Calcul des forces appliquees sur les particules du systeme masses-ressorts.
 */
void ObjetSimuleMSS::CalculForceSpring()
{	
	// #pragma omp parallel for schedule(dynamic, 1)
	for(auto& ressort : _SystemeMasseRessort->GetRessortList()){
		Particule *particule1 = ressort->GetParticuleA();
		Particule *particule2 = ressort->GetParticuleB();

		glm::vec3 direction = mesh.verticies[particule2->GetId()].pos - mesh.verticies[particule1->GetId()].pos;
		
		glm::vec3 direction_norm = glm::normalize(direction);
		glm::vec3 Fe = ressort->GetRaideur() * (glm::length(direction) - (ressort->GetLrepos())) * direction_norm;
		glm::vec3 Fv = ressort->GetAmortissement() * glm::dot((V[particule1->GetId()]  -V[particule2->GetId()] ), direction_norm) * direction_norm;
		this->Force[particule1->GetId()] += Fe + Fv;
		this->Force[particule2->GetId()] -= Fe + Fv;
	}
	
	/// f = somme_i (ki * (l(i,j)-l_0(i,j)) * uij ) + (nuij * (vi - vj) * uij) + (m*g) + force_ext
	
	/// Rq : Les forces dues a la gravite et au vent sont ajoutees lors du calcul de l acceleration
    
		
}//void

void ObjetSimuleMSS::applyForceGravity(float t, glm::vec3 g)
{
	// set gravity to object space
	g = glm::inverse(wNormalMatrix()) * glm::vec4(g, 0.0);
	glm::vec3 wind  = glm::inverse(wNormalMatrix()) * glm::normalize(glm::vec3(1.0, 0.0, 1.0)) * 5.f * glm::sin(t);
	for (int i = 0; i < mesh.verticies.size(); ++i) {
        if (M[i] == 0.0) {
            A[i] = glm::vec3(0.0, 0.0, 0.0);
            continue;
        }
        A[i] = Force[i] / M[i] + g + wind;
        Force[i] = glm::vec3(0.0, 0.0, 0.0);
    }
}

void ObjetSimuleMSS::solveExplicit(float visco, float deltaT)
{
    // deltaT = max(deltaT, 0.00005f);

    // #pragma omp parallel for schedule(dynamic, 1)
	deltaT = 0.001;
    for (int i = 0; i < mesh.verticies.size(); ++i) {
        V[i] = (V[i] + deltaT * A[i]) * visco;
        mesh.verticies[i].pos = mesh.verticies[i].pos + deltaT * V[i];
    }
}

/**
 * Gestion des collisions avec le sol.
 */
void ObjetSimuleMSS::Collision(std::vector<std::shared_ptr<ICollider>> &collisionObjects)
{

	for (int i = 0; i < mesh.verticies.size(); ++i) {
		for (auto &collisionObject : collisionObjects) {
				mesh.verticies[i].pos = this->wMatrix() * glm::vec4(mesh.verticies[i].pos, 1);
				collisionObject->collisionPos(mesh.verticies[i].pos, V[i]);
				mesh.verticies[i].pos = glm::inverse(this->wMatrix()) * glm::vec4(mesh.verticies[i].pos, 1);
			
		}
	}
    /// Arret de la vitesse quand touche le plan
   
    
}// void




}