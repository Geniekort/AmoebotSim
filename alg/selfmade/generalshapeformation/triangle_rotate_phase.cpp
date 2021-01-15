#include "../generalshapeformation.h"

void GSFParticle::triangle_rotate_activate(){
    if(_state == State::COORDINATOR){
        triangle_rotate_coordinatorActivate();
    }else{
        triangle_rotate_particleActivate();
    }
    return;
}

void GSFParticle::triangle_rotate_coordinatorActivate(){
    if(hasToken<triangle_rotate_TriggerExpandToken>()){
        auto triggerToken = peekAtToken<triangle_rotate_TriggerExpandToken>();
        if(triggerToken->_initated){
            if(hasToken<triangle_rotate_ConfirmExpandToken>()){
                // Coordinator particle was waiting for confirmation, and received at now. So terminate expand operation
                //  and update triangleDirection
                takeToken<triangle_rotate_ConfirmExpandToken>();
                auto expandToken = takeToken<triangle_rotate_TriggerExpandToken>();
                if(expandToken->_left){
                    _triangleDirection = (_triangleDirection + 5) % 6;
                }else{
                    _triangleDirection = (_triangleDirection + 1) % 6;
                }
            }

        }else{
            triggerToken->_initated = true;

            // Triangle has not yet been initiated, so initiate it.
            int movementDir, dirpassed, confirmationDir;

            // Set token attributes based on whether the triangle has to expand left or right.
            if(triggerToken->_left){
                dirpassed = _triangleDirection;
                movementDir = (_triangleDirection + 4 )% 6;
                confirmationDir = (dirpassed + 2) % 6;
            }else{
                dirpassed = (_triangleDirection + 1) % 6;
                movementDir = (_triangleDirection + 3 )% 6;
                confirmationDir = (movementDir + 2) % 6;
            }

            auto expandToken = std::make_shared<triangle_rotate_ExpandToken>();
            expandToken->_level = 1;
            expandToken->_dirpassed = dirpassed;
            expandToken->_movementdir = movementDir;
            expandToken->_confirmationdir = confirmationDir;
            expandToken->_left = triggerToken->_left;

            if(hasNbrAtLabel(dirpassed)){
                // Pass token to first particle in the chain
                nbrAtLabel(dirpassed).putToken(expandToken);
            }else{
                // If no such particle is present, we are apparently a single-particle triangle, so rotate does not mean anything: "finish"
                takeToken<triangle_rotate_TriggerExpandToken>();
                if(expandToken->_left){
                    _triangleDirection = (_triangleDirection + 5) % 6;
                }else{
                    _triangleDirection = (_triangleDirection + 1) % 6;
                }
            }
        }
    }
}

/**
 * Handle triangle_rotate for a non-coordinator
 * */
void GSFParticle::triangle_rotate_particleActivate(){

    if(hasToken<triangle_rotate_ExpandToken>()){
        std::shared_ptr<triangle_rotate_ExpandToken> expandToken = takeToken<triangle_rotate_ExpandToken>();
        triangle_rotate_createMovementInitToken(expandToken);

        triangle_rotate_forwardExpandToken(expandToken);
    }

    if(hasToken<triangle_rotate_ConfirmExpandToken>()){
        triangle_rotate_handleConfirmExpandToken();
    }
}

/**
 * Forward the confirmation token towards the coordinator, iff you are no longer a chain_coordinator, meaning your chain has finished moving.
 * @brief GSFParticle::triangle_rotate_handleConfirmExpandToken
 * @param confirmToken
 */
void GSFParticle::triangle_rotate_handleConfirmExpandToken(){
    if(_state != State::CHAIN_COORDINATOR && !hasToken<chain_MovementInitToken>()){
        auto confirmToken = peekAtToken<triangle_rotate_ConfirmExpandToken>();
        //  If you are not a chain coordinator anymore, but have the confirm expand token, you are done and can confirm (or pass confirmation).
        int dirpassed = confirmToken->_dirpassed;
        if(hasNbrAtLabel(dirpassed)){
            confirmToken = takeToken<triangle_rotate_ConfirmExpandToken>();
            nbrAtLabel(dirpassed).putToken(confirmToken);
        }
    }
}

/**
 * Create the MovementInitToken to start the movement along the path required for triangle expansion
 * */
void GSFParticle::triangle_rotate_createMovementInitToken(std::shared_ptr<triangle_rotate_ExpandToken> expandToken){
    auto movementToken = std::make_shared<chain_MovementInitToken>();
    movementToken->_contract = true;
    movementToken->_lifetime = 0;
    movementToken->_dirpassed = expandToken->_dirpassed;
    for(int i = 0; i < expandToken->_level; i++){
        movementToken->L.push(expandToken->_movementdir);
    }

    if(expandToken->_left){
        movementToken->_followerDir = (expandToken->_dirpassed + 2) % 6;
    }else{
        movementToken->_followerDir = (expandToken->_dirpassed + 4) % 6;
    }

    putToken(movementToken);
}


void GSFParticle::triangle_rotate_forwardExpandToken(std::shared_ptr<triangle_rotate_ExpandToken> expandToken){
    if(hasNbrAtLabel(expandToken->_dirpassed)){
        expandToken->_level++;
        nbrAtLabel(expandToken->_dirpassed).putToken(expandToken);
    }else{
        // This particle is the last in the triangle edge
        auto confirmToken = std::make_shared<triangle_rotate_ConfirmExpandToken>();
        confirmToken->_dirpassed = expandToken->_confirmationdir;
        putToken(confirmToken);

    }
}
