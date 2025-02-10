//
//  SolveurImpl.h
//
//
//  Created by fzara on 15/12/2016.
//
//

#ifndef Solveur_Impl_h
#define Solveur_Impl_h


/** Librairies de base **/
#include <stdio.h>
#include <glm/fwd.hpp>
#include <vector>
#include <string.h>
#include <fstream>


// Fichiers de master_meca_sim
#include "MSS.h"



namespace TTe {


class SolveurImpl
{
public:
    
    /*! Constructeur */
    SolveurImpl();
    
    
    /*! Calcul des accelerations (avec ajout de la gravite aux forces) */
    void CalculAccel_ForceGravite(glm::vec3 g,
                                  int nb_som,
                                  std::vector<glm::vec3> &A,
                                  std::vector<glm::vec3> &Force,
                                  std::vector<float> &M);
    
    
    /*! Calcul des vitesses et positions */
    void Solve(float visco,
               int nb_som,
               int Tps,
               std::vector<glm::vec3> &Force,
               std::vector<glm::vec3> &A,
               std::vector<glm::vec3> &V,
               std::vector<glm::vec3> &P,
               std::vector<float> &M,
               glm::vec3 gravite,
               MSS * _SystemeMasseRessort);
    
    
    /*! Calcul des vitesses et reinitialisation des vecteurs forces, contribX, contribV, Y*/
    void CalculVitesse(float visco,
                       int nb_som,
                       int Tps,
                       std::vector<glm::vec3> &A,
                       std::vector<glm::vec3> &V);
    
    
    /*! Calcul des positions */
    void CalculPosition(int nb_som,
                        std::vector<glm::vec3> &V,
                        std::vector<glm::vec3> &P);
    
    
    /* Allocation des structures de donnees -
     X, Y, PP, W, W1, W2, Df_Dx, Df_Dx_diag, Df_Dv, Df_Dv_diag */
    void Allocation_Structure(int nb_som);
    
    /*! Initialisation des structures -
     X, Y, PP, W, W1, W2, Df_Dx, Df_Dx_diag, Df_Dv, Df_Dv_diag */
    void Init(int nb_som, MSS * _SystemeMasseRessort);
    
    /*! Remplissage matrices df/dx et df/dv -
     Utilisation formulation Volino ou Baraff */
    void Remplissage_df_dx_dv(int nb_som,
                              MSS * _SystemeMasseRessort,
                              std::vector<glm::vec3> &P);
    
    /* Re-initialisation : Df_Dx_diag, Df_Dv_diag, Y */
    void Initialisation(int nb_som, std::vector<glm::vec3> &Force);
    
    /*! Fonction construisant le vecteur Y du systeme HX = Y
     avec Y = dt f(t) + dt^2 df/dx v(t)*/
    void Remplissage_Y(int nb_som,
                       std::vector<glm::vec3> &V,
                       std::vector<glm::vec3> &Force,
                       std::vector<float> &M,
                       glm::vec3 gravite,
                       MSS * _SystemeMasseRessort);
    
    /*! Fonction resolvant HX = Y par la methode du gradient conjugue */
    void Resolution(int nb_som,
                    std::vector<float> &M,
                    MSS * _SystemeMasseRessort);
    
    /*! Calcul du produit matrice * vecteur :
     W = H * PP = (M - dt df/dv - dt^2 df/dx) * PP */
    void CalculProdMatVect(int nb_som,
                           std::vector<float> &M,
                           MSS *_SystemeMasseRessort);
    
    /*! Calcul de la direction PP = Y + (_alpha / _beta) * PP  */
    void CalculDirection(int nb_som);
    
    /*! Calcul de _beta = PP^T W */
    void CalculBeta(int nb_som);
    
    /*! Calcul de la solution X = X + (_alpha / _beta) * PP
     et calcul du residu Y = Y - (_alpha / _beta) * W */
    void CalculSolution_Residu(int nb_som);
    
    /*! Calcul de _alpha = Y^T Y */
    void CalculNorme(int nb_som);
    
    
    /// Nombre d iterations pour le calcul des vitesses en implicite
    int _Nb_Iter_VitImpl;
    
    /// Declaration du vecteur Y du systeme HX = Y
    std::vector<glm::vec3> Y;
    
    /// Declaration du vecteur X du systeme HX = Y
    std::vector<glm::vec3> X;
    
    /// Matrice des contributions des forces df/dx et df/dv :
    /// Chacun des elements et une matrice 3 x 3
    /// par symetrie peut etre reduit a 6 valeurs
    // [0 1 2]      [0 1 2]
    // [3 4 5] -->  [  3 4]
    // [6 7 8]      [    5]
    
    /// elements diagonaux et hors diagonale
    std::vector<std::vector<float>> Df_Dx_diag;
    std::vector<std::vector<std::vector<float>>> Df_Dx;
    
    /// Matrice des contributions des forces df/dv :
    /// elements diagonaux et hors diagonale
    std::vector<std::vector<float>> Df_Dv_diag;
    std::vector<std::vector<std::vector<float>>> Df_Dv;
    
    /// Vecteur PP de direction
    std::vector<glm::vec3> PP;
    
    /// Vecteur W pour la resolution du systeme
    /// W = H PP = (M - dt df/dv - dt^2 df/dx) * PP
    std::vector<glm::vec3> W;
    
    /// Vecteur W1 = df/dv * PP
    std::vector<glm::vec3> W1;
    
    /// Vecteur W2 = df/dx * PP
    std::vector<glm::vec3> W2;
    
    /// Vecteur R de residu : R = Y - HX
    std::vector<glm::vec3> R;
    
    
    /// Facteur d erreur
    float _beta;
    
    /// Taille du pas du GC
    float _alpha;
    
    /// Pas de temps
    float _delta_t;
    
};


}
#endif
