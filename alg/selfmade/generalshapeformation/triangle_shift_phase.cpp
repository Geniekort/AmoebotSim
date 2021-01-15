#include "../generalshapeformation.h"
#include <iostream>


void GSFParticle::triangle_shift_activate(){
    if (_state == State::COORDINATOR){
        triangle_shift_coordinatorActivate();
    }
    else {
        triangle_shift_particleActivate();
    }
}


void GSFParticle::triangle_shift_coordinatorActivate(){
    if (hasToken<triangle_shift_TriggerShiftToken>()){
        auto triggerToken = peekAtToken<triangle_shift_TriggerShiftToken>();

        if (triggerToken->_initiated){
            // restore original coordinator
            if (hasToken<triangle_shift_ConfirmShiftToken>() && hasToken<triangle_shift_CoordinatorToken>()){
                /*
                * do this if shift movement is done with temporary coordinator.
                * temporary coordinator is identified by having a coordtoken, triggershifttoken,
                * and eventually confirmshifttoken
                */
                auto coordToken = takeToken<triangle_shift_CoordinatorToken>();
                coordToken->_dirpassed = (coordToken->_dirpassed + 3) % 6;
                coordToken->_shiftDone = true;

                Q_ASSERT(hasNbrAtLabel(coordToken->_dirpassed));

                nbrAtLabel(coordToken->_dirpassed).putToken(coordToken);
                _state = State::CHAIN_FOLLOWER;
                takeToken<triangle_shift_ConfirmShiftToken>();
                takeToken<triangle_shift_TriggerShiftToken>();
            }else if(hasToken<triangle_shift_ConfirmShiftToken>()){
                takeToken<triangle_shift_TriggerShiftToken>();
                takeToken<triangle_shift_ConfirmShiftToken>();
            }
        }
        else {
            triggerToken->_initiated = true;
            triangle_shift_passCoordinatorToken(triggerToken);
        }
    }
}


void GSFParticle::triangle_shift_particleActivate(){
    if (hasToken<triangle_shift_ShiftToken>()) {
        // Pass shift token
        auto shiftToken = takeToken<triangle_shift_ShiftToken>();
        int passTo = shiftToken->_dirpassed;
        if (hasNbrAtLabel(passTo)){
            nbrAtLabel(passTo).putToken(shiftToken);
        }
        else {
            auto confirmToken = std::make_shared<triangle_shift_ConfirmShiftToken>();
            confirmToken->_dirpassed = (passTo + 3) % 6;
            putToken(confirmToken);
        }
        _state = State::CHAIN_COORDINATOR;

        // move the chain with movement init token
        triangle_shift_createMovementInitToken(shiftToken);
    }

    if (hasToken<triangle_shift_CoordinatorToken>()) {
        auto coordToken = peekAtToken<triangle_shift_CoordinatorToken>();
        if (hasNbrAtLabel(coordToken->_dirpassed)){
            // remove token and pass it over to the next particle
            takeToken<triangle_shift_CoordinatorToken>();
            nbrAtLabel(coordToken->_dirpassed).putToken(coordToken);
        }
        else {
            if (coordToken->_shiftDone){
                takeToken<triangle_shift_CoordinatorToken>();
                takeToken<triangle_shift_TriggerShiftToken>();
                _state = State::COORDINATOR;
            }
            else {
                // make node coordinator if there is no node in direction dirpassed
                auto triggerToken = std::make_shared<triangle_shift_TriggerShiftToken>();
                triggerToken->_dir = (coordToken->_dirpassed + coordToken->_relShiftdir) % 6;
                triggerToken->_initiated = false;
                putToken(triggerToken);
                _state = State::COORDINATOR;
            }
        }
    }

    if (hasToken<triangle_shift_ConfirmShiftToken>()){
        auto confirmToken = peekAtToken<triangle_shift_ConfirmShiftToken>();
        if (_state == State::CHAIN_FOLLOWER && hasNbrAtLabel(confirmToken->_dirpassed)){
            nbrAtLabel(confirmToken->_dirpassed).putToken(confirmToken);
            takeToken<triangle_shift_ConfirmShiftToken>();
        }
    }
}

void GSFParticle::triangle_shift_passCoordinatorToken(std::shared_ptr<triangle_shift_TriggerShiftToken> triggerToken){
    int moveDir = triggerToken->_dir;

    if ((_triangleDirection + 5) % 6 == moveDir || (_triangleDirection + 2) % 6 == moveDir) {
        // if triangle has to move left or right from perspective of coordinator

        auto shiftToken = std::make_shared<triangle_shift_ShiftToken>();
        shiftToken->_left = (_triangleDirection + 5) % 6 == moveDir;

        // pass token on the left side of the triangle, else right side
        int passTo = (shiftToken->_left) ? _triangleDirection : (_triangleDirection + 1) % 6;
        shiftToken->_dirpassed = passTo;
        if(hasNbrAtLabel(passTo)){
            nbrAtLabel(passTo).putToken(shiftToken);
        }else{
            // If we are shifting just one particle, already put the confirm.
            auto confirmToken = std::make_shared<triangle_shift_ConfirmShiftToken>();
            confirmToken->_dirpassed = (passTo + 3) % 6;
            putToken(confirmToken);
        }

        // Move the chain with movement init token
        triangle_shift_createMovementInitToken(shiftToken);

    }else{
        // if a shift_coordinator, distinct from the coordinator has to be appointed
        // WARN: Temporarily removed, since implementation was conflicting with the formation of intermediate structure. Can be implemented again if necessary.
    }
}


void GSFParticle::triangle_shift_createMovementInitToken(std::shared_ptr<triangle_shift_ShiftToken> shiftToken){
    auto movementToken = std::make_shared<chain_MovementInitToken>();
    movementToken->_contract = true;
    movementToken->_lifetime = 0;
    movementToken->_dirpassed = shiftToken->_dirpassed;
    int dir = (shiftToken->_left) ? (shiftToken->_dirpassed + 5) % 6 : (shiftToken->_dirpassed + 1) % 6;
    movementToken->L.push(dir);

    movementToken->_followerDir = (dir + 3) % 6;

    putToken(movementToken);
}
