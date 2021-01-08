#include "../generalshapeformation.h"
#include <QDebug>

// All methods required to perform the transformation to an intermediate structure
void GSFParticle::intermediate_activate(){
    if(!hasToken<intermediate_TriggerIntermediateToken>()){
        return;
    }

    auto token = peekAtToken<intermediate_TriggerIntermediateToken>();

    auto step = token->_step;
    if(step == "1a"){
        intermediate_activate_1a(token);
    }else if(step == "1b"){
        intermediate_activate_1b(token);
    }else if(step == "2a"){
        intermediate_activate_2a(token);
    }else if(step == "2b"){
        intermediate_activate_2b(token);
    }else if(step == "3"){
        intermediate_activate_3(token);
    }else if(step == "4a"){
        intermediate_activate_4a(token);
    }else if(step == "4b"){
        intermediate_activate_4b(token);
    }else if(step == "5"){
        intermediate_activate_5(token);
    }else if(step == "6a"){
        intermediate_activate_6a(token);
    }else if(step == "6b"){
        intermediate_activate_6b(token);
    }else if(step == "7"){
        intermediate_activate_7(token);
    }
}


void GSFParticle::intermediate_activate_1a(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken){
    int transferDir;
    if(triggerToken->_up){
        // Transfer on right side of triangle
        transferDir = (_triangleDirection + 1)%6;
    }else{
        // Transfer on left side of triangle
        transferDir = _triangleDirection;
    }

    if(hasNbrAtLabel(transferDir)){
        // Forward leadership
        forwardLeadership(triggerToken, transferDir);
    }else{
        // We are at the end of the triangle side
        triggerToken->_step = "1b";
        triggerToken->_running_l = triggerToken->_l;
    }
}


void GSFParticle::intermediate_activate_1b(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken){
    if(triggerToken->_running_l == 0){
        // Reached the desired node at height l
        if(triggerToken->_up){
            _triangleDirection = (_triangleDirection + 4) % 6;
        }else{
            _triangleDirection = (_triangleDirection + 2) % 6;
        }
        triggerToken->_step = "2a";
    }else{
        // Move the leadership back
        int transferDir;
        if(triggerToken->_up){
            // Transfer on right side of triangle
            transferDir = (_triangleDirection + 4)%6;
        }else{
            // Transfer on left side of triangle
            transferDir = (_triangleDirection + 3)%6;
        }
        triggerToken->_running_l--;

        if(hasNbrAtLabel(transferDir)){
            forwardLeadership(triggerToken, transferDir);
        }else{
            // The edge of the triangle left has less than l particles, so we are DONE!
            takeToken<intermediate_TriggerIntermediateToken>();
            // FINISHED!!
        }
    }
}

void GSFParticle::intermediate_activate_2a(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken){
    rotateA("2b", triggerToken);
}

void GSFParticle::intermediate_activate_2b(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken){
    rotateB("3", triggerToken);
}

void GSFParticle::intermediate_activate_3(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken){
    if(!triggerToken->_initiated){
        // Initialize rotation
        auto shiftToken = std::make_shared<GSFParticle::triangle_shift_TriggerShiftToken>();
        shiftToken->_dir = 0;
        putToken(shiftToken);
        triggerToken->_initiated = true;
    }else if(!hasToken<triangle_shift_TriggerShiftToken>()){
        triggerToken->_step = "4a";
        triggerToken->_initiated = false;
    }
}

void GSFParticle::intermediate_activate_4a(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken){
    rotateA("4b", triggerToken);
}

void GSFParticle::intermediate_activate_4b(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken){
    rotateB("5", triggerToken);
}



void GSFParticle::intermediate_activate_5(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken){
    if(!triggerToken->_initiated){
        // Initialize rotation
        auto shiftToken = std::make_shared<GSFParticle::triangle_shift_TriggerShiftToken>();
        if(triggerToken->_up){
            shiftToken->_dir = 5;
        }else{
            shiftToken->_dir = 1;
        }
        putToken(shiftToken);
        triggerToken->_initiated = true;
    }else if(!hasToken<triangle_shift_TriggerShiftToken>()){
        triggerToken->_step = "6a";
        triggerToken->_initiated = false;
    }
}

void GSFParticle::intermediate_activate_6a(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken){
    rotateA("6b", triggerToken);
}

void GSFParticle::intermediate_activate_6b(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken){
    rotateB("7", triggerToken);
}

void GSFParticle::intermediate_activate_7(std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken){
    int transferDir;
    if(triggerToken->_up){
        // Transfer on left side of triangle
        transferDir = _triangleDirection;
    }else{
        // Transfer on right side of triangle
        transferDir = (_triangleDirection + 1)%6;
    }

    if(hasNbrAtLabel(transferDir)){
        // Forward leadership
        forwardLeadership(triggerToken, transferDir);
    }else{
        // We are at the end of the triangle side
        if(triggerToken->_up){
            _triangleDirection = (_triangleDirection + 2)%6;
        }else{
            _triangleDirection = (_triangleDirection + 4)%6;
        }
        triggerToken->_step = "1a";
        triggerToken->_up = !triggerToken->_up;
    }
}




// Forward the leadership by removing TriggerIntermediateToken, and forwarding it to a neighbor
//  and also updating the required variables for a leader.
void GSFParticle::forwardLeadership(std::shared_ptr<intermediate_TriggerIntermediateToken> token, int dir){
    takeToken<intermediate_TriggerIntermediateToken>();
    _state = State::CHAIN_FOLLOWER;

    nbrAtLabel(dir).putToken(token);
    nbrAtLabel(dir)._triangleDirection = _triangleDirection;
    nbrAtLabel(dir)._state = State::COORDINATOR;
}

// Helper method for rotation steps, part a
void GSFParticle::rotateA(QString nextStep, std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken){
    if(hasToken<chain_ConfirmCutoffToken>()){
        auto _ = takeToken<chain_ConfirmCutoffToken>();
        triggerToken->_step = nextStep;
        triggerToken->_initiated = false;
    }else{
        int cutoffDir;
        if(triggerToken->_up){
            cutoffDir = (_triangleDirection + 1)%6;
        }else{
            cutoffDir = _triangleDirection;
        }

        // Start cutoff if not yet started
        if(!nbrAtLabel(cutoffDir).hasToken<chain_CutoffToken>()){
            auto cutoffToken = std::make_shared<GSFParticle::chain_CutoffToken>();
            cutoffToken->_dirpassed = cutoffDir;
            qDebug() << "Passing cutoff token to ";
            qDebug() << cutoffDir;
            nbrAtLabel(cutoffDir).putToken(cutoffToken);
        }
    }
}

// Helper method for rotation steps, part b
void GSFParticle::rotateB(QString nextStep, std::shared_ptr<intermediate_TriggerIntermediateToken> triggerToken){
    if(!triggerToken->_initiated){
        // Initialize rotation
        auto expandToken = std::make_shared<GSFParticle::triangle_expand_TriggerExpandToken>();
        // Move left if up=true, or else right
        expandToken->_left = triggerToken->_up;
        putToken(expandToken);
        triggerToken->_initiated = true;
    }else if(!hasToken<triangle_expand_TriggerExpandToken>()){
        // Rotation is done
        triggerToken->_step = nextStep;
        triggerToken->_initiated = false;
    }
}
