#pragma once

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>



#include "sceneV2/Ianimatic.hpp"
#include "sceneV2/Icollider.hpp"
#include "sceneV2/IIndirectRenderable.hpp"
#include "sceneV2/animatic/skeleton/BVH.h"
#include "i_input_controller.hpp"
#include "sceneV2/node.hpp"



namespace TTe {
class SkeletonObj : public Node, public IAnimatic, public IIndirectRenderable, public ICollider, public IInputController {
   public:
    class SkeletonNode : public Node {
    };

    

    //! Cr�er un squelette ayant la m�me structure que d�finit dans le BVH c'est � dire
    //! creer le tableau de SkeletonJoint � la bonne taille, avec les parentId initials� pour chaque case
    void init(BVH bvh);
    void init(std::filesystem::path bvh_folder);

    //! Renvoie la position de l'articulation i en multipliant le m_l2w par le Point(0,0,0)
    glm::vec3 getJointPosition(int i) const;

    //! Renvoie l'identifiant de l'articulation p�re de l'articulation num�ro i
    int getParentId(const int i) const;

    //! Renvoie le nombre d'articulation
    int numberOfJoint() const { return (int)m_joints_1.size(); }

    //! Positionne ce squelette dans la position n du BVH.
    //! Assez proche de la fonction r�cursive (question 1), mais range la matrice (Transform)
    //! dans la case du tableau. Pour obtenir la matrice allant de l'articulation local vers le monde,
    //! il faut multiplier la matrice allant de l'articulation vers son p�re � la matrice du p�re allant de
    //! l'articulation du p�re vers le monde.
    void setPose(const BVH& bvh, int frameNumber);


    void simulation(glm::vec3 gravite, float viscosite, uint32_t tick, float dt, float t, std::vector<std::shared_ptr<ICollider>> &collisionObjects);
    void render(CommandBuffer &cmd, RenderData &renderData);
    void collisionPos(glm::vec3 &pos, glm::vec3 &vitesse);
    void updateFromInput(Window* window, float dt);
    //! Positionne ce squelette entre la position frameNbSrc du BVH Src et la position frameNbDst du bvh Dst
    // void setPoseInterpolation(const BVH& bvhSrc, int frameNbSrc, const BVH& bvhDst, int frameNbDst, float t);

    //! Positionne ce squelette entre la position frameNbSrc du BVH Src et la position frameNbDst du bvh Dst
    //! idem � setPoseInterpolation mais interpole avec des quaternions sur chaque articulations
    // void setPoseInterpolationQ(const BVH& bvhSrc, int frameNbSrc, const BVH& bvhDst, int frameNbDst, float t);

    //! Calcule la distance entre deux poses
    //! precond: les deux squelettes doivent avoir le
    //! m�me nombre d'articulations (m�me structure d'arbre)
    //! ==> Sera utile lors de la construction du graphe d'animation
    // friend float distance(const CASkeleton& a, const CASkeleton& b);

    enum class State {
        IDLE ,
        WALK,
        RUN,
        KICK,
    };


    State state;
    int frameOffset = 0;
    State nextState;

    struct Pose {
        State state;
        int frame;

        bool operator==(const Pose& other) const noexcept {
            return state==other.state && frame==other.frame;
        }

        struct Hasher {
            size_t operator()(const Pose &p) const noexcept {
                return std::hash<State>{}(p.state) ^ (std::hash<int>{}(p.frame) << 4);
            }
        };
    };


    struct StateTransition {
        State state_1;
        State state_2;

        bool operator==(const StateTransition& other) const noexcept {
            return state_1==other.state_1 && state_2==other.state_2;
        }

        struct Hasher {
            size_t operator()(const StateTransition &p) const noexcept {
                return std::hash<State>{}(p.state_1) ^ (std::hash<State>{}(p.state_2) << 4);
            }
        };
    };


    struct FrameTransition {
        int frame_1;
        int frame_2;
    };


    std::string getStrfromState(State state) {
        switch (state) {
            case State::IDLE:
                return "IDLE";
            case State::WALK:
                return "WALK";
            case State::RUN:
                return "RUN";
            case State::KICK:
                return "KICK";
            default:
                return "UNKNOWN";
        }
    }
   private:

    float computePoseDistance(int frame_1, int frame_2, BVH &bvh_1, BVH &bvh_2);

    int lastFrame = 0;
    std::vector<std::pair<glm::vec3, glm::vec3>> coliders;
    std::vector<std::shared_ptr<Node>> m_joints_1;
    std::vector<std::shared_ptr<Node>> m_joints_2;
    std::vector<std::shared_ptr<Node>> m_joints_final;
    std::map<State, BVH> m_bvh;

    
    // transition between states
    std::unordered_map<Pose, std::vector<Pose>, Pose::Hasher> m_graph;
    std::unordered_map<StateTransition, std::vector<FrameTransition>, StateTransition::Hasher> transitions_graph;
    int startTransitionFrame = 0;
    int nextStateFrameOffset = 0;
    bool transition = false;
    bool wantTransition = false;
    bool kickend = true;


    // glm::vec3 forward;
    glm::vec3 orientation = {0.f, 0.f, 0.f};

    bool keyPressed = false;


    
   




    float speed_max = 10.f;
    float speed_max_run = 20.f;

    float accel = 10.f;

    float speed;

    float interpol;

};
}  // namespace TTe