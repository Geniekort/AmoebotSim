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

    if (!(hasNbrAtLabel(moveDir)) && !(hasNbrAtLabel((moveDir + 3) % 6))) {
        // do this only if triangle has to move left or right from perspective of coordinator
        auto shiftToken = std::make_shared<triangle_shift_ShiftToken>();
        shiftToken->_left = hasNbrAtLabel((moveDir + 1) % 6) ? true : false;

        // pass token on the left side of the triangle, else right side
        int passTo = (shiftToken->_left) ? (moveDir + 1) % 6: (moveDir + 5) % 6;
        shiftToken->_dirpassed = passTo;
        nbrAtLabel(passTo).putToken(shiftToken);

        // Move the chain with movement init token
        triangle_shift_createMovementInitToken(shiftToken);

    }
    else {
        // if shift direction is not left or right, pass the coordinator token to the
        // correct triangle vertex

        // direction to pass the coordinator token to
        int passCoordTo = -1;
        // shift direction relative to passCoordTo i.e. in terms of passCoordTo
        int relShiftdir = -1;

        /*
         * Determine the direction to which the coordinator should be passed
         */

        // move coordinator state to left bottom vertex
        if (!hasNbrAtLabel(moveDir) && hasNbrAtLabel((moveDir + 3) % 6) && hasNbrAtLabel((moveDir + 2) % 6)) {
        // if pointing left upwards
            passCoordTo = (moveDir + 2) % 6;
            relShiftdir = 4;
        }
        else if (hasNbrAtLabel(moveDir) && hasNbrAtLabel((moveDir + 5) % 6) && !hasNbrAtLabel((moveDir + 3) % 6)) {
        // if pointing right downwards
            passCoordTo = (moveDir + 5) % 6;
            relShiftdir = 1;
        }
        // move coordinator state to right bottom vertex
        else if (!hasNbrAtLabel(moveDir) && hasNbrAtLabel((moveDir + 3) % 6) && hasNbrAtLabel((moveDir + 4) % 6)) {
        // if pointing right upwards
            passCoordTo = (moveDir + 4) % 6;
            relShiftdir = 2;
        }
        else if (hasNbrAtLabel(moveDir) && hasNbrAtLabel((moveDir + 1) % 6) && !hasNbrAtLabel((moveDir + 3) % 6)) {
        // if pointing left downwards
            passCoordTo = (moveDir + 1) % 6;
            relShiftdir = 5;
        }
        Q_ASSERT(passCoordTo >= 0 && relShiftdir >= 0);

        auto coordToken = std::make_shared<triangle_shift_CoordinatorToken>();
        coordToken->_dirpassed = passCoordTo;
        coordToken->_relShiftdir = relShiftdir;
        nbrAtLabel(passCoordTo).putToken(coordToken);
        _state = State::CHAIN_FOLLOWER;
    }
}


void GSFParticle::triangle_shift_createMovementInitToken(std::shared_ptr<triangle_shift_ShiftToken> shiftToken){
    auto movementToken = std::make_shared<chain_MovementInitToken>();
    movementToken->_contract = true;
    movementToken->_lifetime = 0;
    movementToken->_dirpassed = shiftToken->_dirpassed;
    int dir = (shiftToken->_left) ? (shiftToken->_dirpassed + 5) % 6 : (shiftToken->_dirpassed + 1) % 6;
    movementToken->L.push(dir);
    putToken(movementToken);
}
