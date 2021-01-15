#ifndef GENERALSHAPEFORMATION_H
#define GENERALSHAPEFORMATION_H


#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"
#include <stack>


class GSFParticle:public AmoebotParticle{
    public:
        //states that a particle can be in
        enum class State{
            COORDINATOR,
            CHAIN_COORDINATOR,
            CHAIN_FOLLOWER
        };

        //constructor for TriangleParticle
        GSFParticle(Node& head, const int globalTailDir,
                    const int orientation, AmoebotSystem& system,
                    const int sideLen, int triangleDirection, State state, unsigned int level,
                    unsigned int ldrlabel, int depth);

        //what to be done during activation
        void activate() override;

        // Chain Primitive Phase
        // Allows a chain of particles to move along a certain path
        void chain_activate();
        void chain_handleChainToken();

        //set color of the particle
        int headMarkColor() const override;
        int headMarkDir() const override;
        int tailMarkColor() const override;

        QString inspectionText() const override;

        GSFParticle& nbrAtLabel(int label) const;

        int findFollower();

protected:
        //private vars
        const int _initialSideLen;
        int _triangleDirection; // Direction of the left leg of the triangle as seen from the leader
        State _state;

        unsigned int _level = 0;
        int _ldrlabel = -1;
        int _depth = -1;

        //used for chain movement
        bool _sent_pull = false;

        // Used to initialize a triangle expansions from a coordinator
        // _left => expand the triangle at the left leg, looking at the triangle with the leader on top.
        // !_left => expand the triangle at the right leg, looking at the triangle with the leader on top.
        // _initiated <=> the triangle expansion has been initiated
        struct triangle_rotate_TriggerExpandToken :public Token {bool _left; bool _initated = false;};

        // Used to inform all particles at a side of the triangle of the initiated expansion
        struct triangle_rotate_ExpandToken :public Token {int _level; int _movementdir; int _dirpassed; int _confirmationdir; bool _left;};

        // Used to confirm the coordinator of a succesful triangle expansion, from the last particlein the triangle edge
        struct triangle_rotate_ConfirmExpandToken :public Token {int _dirpassed;};

        // Token to initialize a movement from a chain.
        //  _followerDir => The direction in which chain coordinators are to seek followers
        struct chain_MovementInitToken :public Token {std::stack<int> L; int _lifetime; int _dirpassed;
                                               bool _contract; int _level; int _followerDir;};
        //used to initiate chain movement
        //L: the path that the chain should follow
        //_contract: whether or not the chain should be fully contracted at the end
        struct chain_ChainToken : public Token {std::stack<int> L; bool _contract;};

        //_final: whether the final chain should be contracted (set by chain)
        struct chain_ContractToken : public Token{bool _final;};

        //Sets the depth of all tokens in the chain
        //_passeddir: the direction from which the token was passed. Used to
        // set what particle should be followed
        //_depth: location of the particle in the chain
        struct chain_DepthToken : public Token{int _passeddir; int _depth; int _level;};

        //used for confirming whether the chain is contracted when _final = true
        // in chain token
        struct chain_ConfirmContractToken : public Token{};

        struct chain_ResetChainToken : public Token{};
        struct chain_ConfirmResetChainToken : public Token{};

        // Used to indicate the end of a chain, when the final particle might still have a neighbor, but we do not want to include it in the chain.
        // _handled indicates whether this token was just put here or already processed by the node
        struct chain_CutoffToken : public Token{int _dirpassed; bool _handled = false;};
        struct chain_ConfirmCutoffToken : public Token{int _dirpassed;};

        // Tokens used to perform the shift movement
        struct triangle_shift_TriggerShiftToken: public Token{int _dir; bool _initiated=false;};
        struct triangle_shift_ShiftToken: public Token{int _left; int _dirpassed;};
        struct triangle_shift_CoordinatorToken: public Token{int _dirpassed; int _relShiftdir; bool _shiftDone=false;};
        struct triangle_shift_ConfirmShiftToken: public Token{int _dirpassed;};

        // _l: the desired height of the intermediate structure. _step: in which phase the process is currently (as documented in the report
        // _step: the current step that the intermediate structure formation is in.
        // _up: whether we are in the phase where the triangle was initially pointing upwards.
        // _running_l: used in step 1b, to keep track of how many steps are taken up already
        // _initiated: to indicate whether a step is already initiated (and particle is waiting), or should still be initiated. Only used in some specific steps.
        struct intermediate_TriggerIntermediateToken: public Token{int _l; QString _step="1a"; bool _up=true; int _running_l; bool _initiated = false;};


    private:
        friend class GSFSystem;

        // Helper methods to perform the chain Movement primitive
        void chain_handleContractToken();
        void chain_handleMovementInitToken();
        void chain_handleDepthToken();
        void chain_handleConfirmContractToken();
        void chain_handleCutoffToken();
        void chain_handleConfirmCutoffToken();

        // Helper methods to perform the shift Movement primitive
        void triangle_shift_activate();
        void triangle_shift_coordinatorActivate();
        void triangle_shift_particleActivate();
        void triangle_shift_passCoordinatorToken(std::shared_ptr<triangle_shift_TriggerShiftToken> triggerToken);
        void triangle_shift_createMovementInitToken(std::shared_ptr<triangle_shift_ShiftToken> shiftToken);

        // Helper methods to perform the triangle rotation Movement primitive
        void triangle_rotate_activate();
        void triangle_rotate_coordinatorActivate();
        void triangle_rotate_particleActivate();
        void triangle_rotate_createMovementInitToken(std::shared_ptr<triangle_rotate_ExpandToken> expandToken);
        void triangle_rotate_forwardExpandToken(std::shared_ptr<triangle_rotate_ExpandToken> expandToken);
        void triangle_rotate_handleTriggerExpandToken();
        void triangle_rotate_handleConfirmExpandToken();

        // Helper methods to perform the procedure to form the intermediate structure using movement primitives.
        void intermediate_activate();
        void intermediate_activate_1a(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken);
        void intermediate_activate_1b(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken);
        void forwardLeadership(std::shared_ptr<intermediate_TriggerIntermediateToken> token, int dir);
        void intermediate_activate_2a(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken);
        void intermediate_activate_2b(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken);
        void intermediate_activate_3(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken);
        void intermediate_activate_4a(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken);
        void intermediate_activate_4b(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken);
        void intermediate_activate_5(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken);
        void intermediate_activate_6a(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken);
        void intermediate_activate_6b(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken);
        void intermediate_activate_7(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken);
        void rotateA(QString nextStep, std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken);
        void rotateB(QString nextStep, std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken);
        void chain_handleResetChainTokens();
};

class GSFSystem:public AmoebotSystem{
    public:
        GSFSystem(int sideLen = 6, int l = 2);


private:
        // Create a triangle of side length sideLen
        void initializeTriangle(int sideLen, Node current, int dir);
};


#endif // GENERALSHAPEFORMATION_H
