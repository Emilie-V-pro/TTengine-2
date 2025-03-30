#pragma once
#include <glm/fwd.hpp>
#include <memory>
#include <string>

#include "device.hpp"
#include "sceneV2/Ianimatic.hpp"
#include "sceneV2/Icollider.hpp"
#include "sceneV2/Irenderable.hpp"
#include "sceneV2/mesh.hpp"
#include "sceneV2/node.hpp"
namespace TTe {
class SimulateObj : public Node, public IRenderable, public IAnimatic {
   public:



    SimulateObj(){}
    SimulateObj(Device *device, std::string &paramFile) : device(device) {
        Param_mesh(paramFile);
        mesh = Mesh(device);
    }

    // copy move
    SimulateObj(const SimulateObj &other) : Node(other), IRenderable(other), IAnimatic(other) {
        this->_Fich_Masses = other._Fich_Masses;
        this->_Fich_Points = other._Fich_Points;
        this->_Interaction = other._Interaction;
        this->_Friction = other._Friction;
        this->V = other.V;
        this->A = other.A;
        this->Force = other.Force;
        this->M = other.M;
        this->mesh = other.mesh;
        this->device = other.device;
    }

    SimulateObj &operator=(const SimulateObj &other) {
        if (this != &other) {
            Node::operator=(other);
            IRenderable::operator=(other);
            IAnimatic::operator=(other);
            this->_Fich_Masses = other._Fich_Masses;
            this->_Fich_Points = other._Fich_Points;
            this->_Interaction = other._Interaction;
            this->_Friction = other._Friction;
            this->V = other.V;
            this->A = other.A;
            this->Force = other.Force;
            this->M = other.M;
            this->mesh = other.mesh;
            this->device = other.device;
        }
        return *this;
    }

    SimulateObj &operator=(SimulateObj &&other) {
        if (this != &other) {
            Node::operator=(other);
            IRenderable::operator=(other);
            IAnimatic::operator=(other);
            this->_Fich_Masses = other._Fich_Masses;
            this->_Fich_Points = other._Fich_Points;
            this->_Interaction = other._Interaction;
            this->_Friction = other._Friction;
            this->V = other.V;
            this->A = other.A;
            this->Force = other.Force;
            this->M = other.M;
            this->mesh = other.mesh;
            this->device = other.device;
        }
        return *this;
    }

    SimulateObj(SimulateObj &&other) : Node(other), IRenderable(other), IAnimatic(other) {
        this->_Fich_Masses = other._Fich_Masses;
        this->_Fich_Points = other._Fich_Points;
        this->_Interaction = other._Interaction;
        this->_Friction = other._Friction;
        this->V = other.V;
        this->A = other.A;
        this->Force = other.Force;
        this->M = other.M;
        this->mesh = other.mesh;
        this->device = other.device;
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

    // virtual void render(CommandBuffer &cmd, GraphicPipeline &pipeline, std::vector<Mesh> &meshes,  std::map<BasicShape, Mesh> basicMeshes) = 0;
    // virtual void simulation(glm::vec3 gravite, float viscosite, int Tps, float dt, float t, std::vector<std::shared_ptr<ICollider>> &collisionObjects);
    
    Device *device;
};
}  // namespace TTe