/*
 * ObjetSimuleMSS.cpp : definition des objets anime.
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

/** \file ObjetSimuleMSS.cpp
 \brief Methodes specifiques aux objets a simuler de type MSS.
 */

/** Librairies **/
#include <math.h>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <iostream>
#include <ostream>
#include <vector>

// Fichiers de master_meca_sim
#include "MSS.h"
#include "ObjetSimuleMSS.h"
// #include "Viewer.h"
#include "SolveurExpl.h"
#include "SolveurImpl.h"
#include "scene/mesh.hpp"

// #include "draw.h"

namespace TTe {

/**
 * Constructeur de la class ObjetSimuleMSS.
 */
ObjetSimuleMSS::ObjetSimuleMSS(Device *device, std::string fich_param) : SimulateObj(device, fich_param), _Size(), _SystemeMasseRessort(0) {
    /* Allocation du systeme masse-ressort */
    _SystemeMasseRessort = new MSS();

    /** Recuperation des parametres du systeme masse-ressort mis dans le fichier **/
    Param_mss(fich_param);

    /* Calcul du facteur d amortissement des ressorts */
    _SystemeMasseRessort->_RessOS.SetFactAmorti();
}

/**
 * Initialisation des tableaux et construction du maillage
 a partir des donnees lues dans le fichier.
 */
void ObjetSimuleMSS::initObjetSimule() {
    /* Variables intermediaires pour la lecture des donnees */
    glm::vec3 tmp_coord;
    glm::vec2 tmp_uv;
    float tmp_masse;

    /* Position min de tous les sommets */
    glm::vec3 Pmin(0.0, 0.0, 0.0);

    /* Position max de tous les sommets */
    glm::vec3 Pmax(0.0, 0.0, 0.0);

    /** Lecture des positions des sommets **/

    /// Fichier de donnees des points
    std::ifstream _FichIn_Points(_Fich_Points.c_str());

    /// Test ouverture du fichier
    if (!_FichIn_Points) {
        std::cout << "Erreur d ouverture du fichier de donnees des points : " << _Fich_Points << std::endl;

        // Arret du programme
        exit(1);
    }
    uint32_t   _Nb_Sommets;
    _FichIn_Points >> _Nb_Sommets;
    std::cout << "Nombre de sommets " << _Nb_Sommets <<std::endl;

    /// Lecture du nombre de sommets
    std::vector<glm::vec3> pos;
    /// Lecture des positions
    while (!_FichIn_Points.eof()) {
        _FichIn_Points >> tmp_coord.x;
        _FichIn_Points >> tmp_coord.y;
        _FichIn_Points >> tmp_coord.z;

        /// remplissage de P
        pos.push_back(tmp_coord);
    }
    std::cout << "Positions lues ..." << std::endl;

    /** Lecture de toutes les masses **/

    std::ifstream _FichIn_Masses(_Fich_Masses.c_str());

    /// Test ouverture du fichier
    if (!_FichIn_Masses) {
        std::cout << "Erreur d ouverture du fichier de donnees des masses : " << _Fich_Masses << std::endl;

        // Arret du programme
        exit(1);
    }

    /// Lecture des masses
    while (!_FichIn_Masses.eof()) {
        _FichIn_Masses >> tmp_masse;

        /// Remplissage de M
        M.push_back(tmp_masse);
    }
    std::cout << "Masses lues ..." << std::endl;

    /* Fichier de donnees des textures */
    std::ifstream _FichIn_Texture(_Fich_Texture.c_str());

    
    

    /* Lecture des textures */
    std::vector<glm::vec2> uv;
    /// Lecture des coordonnees de texture
    while (!_FichIn_Texture.eof()) {

        _FichIn_Texture >> tmp_uv.r;
        _FichIn_Texture >> tmp_uv.g;

        /// Remplissage du tableau des textures
        uv.push_back(tmp_uv);
    }

    std::cout << "Textures lues ..." << std::endl;

    /** Calculs intermediaires **/
    /* Calcul de Pmin et Pmax */
    for (int i = 0; i < pos.size(); ++i) {
        Pmin = glm::vec3(std::min(Pmin.x, pos[i].x), std::min(Pmin.y, pos[i].y), std::min(Pmin.z, pos[i].z));

        Pmax = glm::vec3(std::max(Pmax.x, pos[i].x), std::max(Pmax.y, pos[i].y), std::max(Pmax.z, pos[i].z));
    }

    /* Taille du tissu dans chacune des directions x, y, z */
    _Size.x = fabs(Pmin.x - Pmax.x);
    _Size.y = fabs(Pmin.y - Pmax.y);
    _Size.z = fabs(Pmin.z - Pmax.z);
    mesh.verticies.resize(pos.size());

    /*** Initialisation des tableaux pour chacun des sommets ***/
    for (int i = 0; i < pos.size(); ++i) {

        mesh.verticies[i].pos = pos[i];
        mesh.verticies[i].uv = uv[i];
        /** Vecteur nulle pour initialiser les vitesses,
         accel, forces, des normales des sommets **/
        
        V.push_back(glm::vec3(0.0, 0.0, 0.0));
        A.push_back(glm::vec3(0.0, 0.0, 0.0));
        Force.push_back(glm::vec3(0.0, 0.0, 0.0));

        /** Ajout du sommet dans le systeme masses-ressorts **/
        /* Construction d une particule */
        Particule *Part = new Particule();

        /* Attribution de son identificateur */
        Part->SetId(i);

        /* Position de la particule */
        Part->SetPosition(pos[i]);
        // TODO : cette position n est pas mise a jour car on se sert du tableau P
        // TODO : mettre un pointeur sur la valeur contenue dans P[i] pour que la position de la particule soit mise � jour

        /* Masse de la particule */
        Part->SetMass(M[i]);

        /* Ajout effectif de la particule dans le systeme masses-ressorts */
        _SystemeMasseRessort->AddParticule(Part);

    }  // for

    /** Constructions de toutes les facettes du maillage **/

    std::ifstream _FichIn_FaceSet(_Fich_FaceSet.c_str());

    // Test ouverture du fichier
    if (!_FichIn_FaceSet) {
        std::cout << "Erreur d ouverture du fichier de donnees des faceSet : " << _Fich_FaceSet << std::endl;

        // Arret du programme
        exit(1);
    }

    /// Nombre de facets
    int nb_facet = 0;

    /* Construction des facettes */
    while (!_FichIn_FaceSet.eof()) {
        /// Remplissage de allfacets
        // push_back dans le tableau des indices des sommets
        int vertexIds[3];
        _FichIn_FaceSet >> vertexIds[0];
        _FichIn_FaceSet >> vertexIds[1];
        _FichIn_FaceSet >> vertexIds[2];
        
        // _FichIn_FaceSet >> facet.fi;
        // _FichIn_FaceSet >> facet.fj;
        // _FichIn_FaceSet >> facet.fk;

        /* Construction de la facette fi, fj, fk */
        // Creation de la facet en mettant les sommets
        // dans l ordre inverse des aiguilles d une montre
        _SystemeMasseRessort->MakeFace(
            _SystemeMasseRessort->GetParticule(vertexIds[2]), _SystemeMasseRessort->GetParticule(vertexIds[1]),
            _SystemeMasseRessort->GetParticule(vertexIds[0]), &_SystemeMasseRessort->_RessOS);
        if(vertexIds[0] >= pos.size() || vertexIds[1] >= pos.size() || vertexIds[2] >= pos.size()){
            std::cout << "Erreur dans les indices des sommets" << std::endl;
            exit(1);
        }
        // Recopie dans le tableau des indices des sommets
        mesh.indicies.push_back(vertexIds[2]);
        mesh.indicies.push_back(vertexIds[1]);
        mesh.indicies.push_back(vertexIds[0]);
        nb_facet++;
    }


    /** Fermeture des fichiers de donnees **/
    _FichIn_FaceSet.close();
    _FichIn_Masses.close();
    _FichIn_Points.close();
    _FichIn_Texture.close();

    /** Modification des normales **/
    setNormals();

    /** Message pour la fin de la creation du maillage **/
    std::cout << "Systeme masse-ressort build ..." << std::endl;

    // Allocation des structures de donnees dans le cas utilisation solveur implicite
    // X, Y, Df_Dx, Df_Dx_diag, Df_Dv, Df_Dv_diag
    if (_Integration == "implicite") _SolveurImpl->Allocation_Structure(mesh.verticies.size());
}


/**
 * Modification du tableau des normales (lissage des normales).
 */
void ObjetSimuleMSS::setNormals() {
    // Normale d une face
    glm::vec3 normale;

    // Norme d une normale
    double Norme;

    // Indices des sommets d une face
    int a, b, c;

  

    /* Parcours des faces du maillage */
    for (unsigned int i = 0; i < mesh.indicies.size()/3; ++i) {
        // Sommets a, b, c de la face
        a = mesh.indicies[3 * i];
        b = mesh.indicies[(3 * i) + 1];
        c = mesh.indicies[(3 * i) + 2];

        struct Triangle tri = {mesh.verticies[a], mesh.verticies[b], mesh.verticies[c]};
        normale = tri.getNormal();

        

        // Modification des normales des sommets de la face
        // Normale du sommet a
        mesh.verticies[a].normal = mesh.verticies[a].normal + normale;

        // Normale du sommet b
        mesh.verticies[b].normal = mesh.verticies[b].normal + normale;

        // Normale du sommet c
        mesh.verticies[c].normal = mesh.verticies[c].normal + normale;

    }  // for_i

    /* Parcours des normales des sommets */
    float norme;

    for(auto & vertex : mesh.verticies){
        vertex.normal = glm::normalize(vertex.normal);
    }
}

/**
 * \brief Creation du maillage (pour l affichage) de l objet simule de type MSS.
 * Methode invoquee par le graphe de scene.
 */
void ObjetSimuleMSS::initMeshObjet() {
    // std::cout << "------ ObjetSimule::init_Mesh_Object() ----------- " << std::endl;
    mesh.uploadToGPU();
    std::cout << "Maillage du MSS pour affichage build ..." << std::endl;
}

/*
 * Mise a jour du Mesh (pour affichage) de l objet
 * en fonction des nouvelles positions calculees.
 */
void ObjetSimuleMSS::updateVertex() {
    // std::cout << "ObjetSimuleMSS::updateVertex() ..." << std::endl;
    mesh.uploadToGPU();
}

/**
 * Simulation de l objet.
 */
void ObjetSimuleMSS::Simulation(glm::vec3 gravite, float viscosite, int Tps, float dt, float t) {
    /* Calcul des forces dues aux ressorts */
    // std::cout << "Force.... " << std::endl;
    CalculForceSpring();

    /* Calcul des accelerations (avec ajout de la gravite aux forces) */
    // std::cout << "Accel.... " << std::endl;
    if (_Integration == "explicite")
        _SolveurExpl->CalculAccel_ForceGravite(gravite, mesh.verticies.size(), t, A, Force, M);
    else if (_Integration == "implicite")
        _SolveurImpl->CalculAccel_ForceGravite(gravite, mesh.verticies.size(), A, Force, M);

    /* Calcul des vitesses et positions au temps t */
    // std::cout << "Vit.... " << std::endl;
    if (_Integration == "explicite")
        _SolveurExpl->Solve(viscosite, mesh.verticies.size(), dt, A, V, mesh.verticies);
    else if (_Integration == "implicite")
        _SolveurImpl->Solve(viscosite, mesh.verticies.size(), Tps, Force, A, V, mesh.verticies, M, gravite, _SystemeMasseRessort);

    /* ! Gestion des collisions  */
    // Reponse : reste a la position du sol - arret des vitesses
    // Penser au Translate de l objet dans la scene pour trouver plan coherent
    Collision();

    // Affichage des positions
    //  AffichagePos(Tps);

    /** Modification des normales **/
    setNormals();
    updateVertex();
}

}  // namespace TTe