#include "../generalshapeformation.h"
#include <QDebug>
#include <iostream>

// All methods required to perform a Chain movement
void GSFParticle::chain_activate(){
    // used for ensure that a chain has descending depth
    // depth i means that some particle is the ith particle
    // in a chain
    if(hasToken<chain_DepthToken>()){
        chain_handleDepthToken();
    }

    //if follower is contracted, send confirmation token allong the chain
    if(hasToken<chain_ConfirmContractToken>()){
        chain_handleConfirmContractToken();
    }

    if(hasToken<chain_ChainToken>()){
        chain_handleChainToken();
    }

    //Token to initialize a certian movement pattern. This is just used for debugging.
    //Should be replaced by a token of a specific primitive for a certian result.
    //Can give an idea of how to setup a movement primitive however.
    if(hasToken<chain_MovementInitToken>()){
        chain_handleMovementInitToken();
    }

    if(hasToken<chain_ResetChainToken>() || hasToken<chain_ConfirmResetChainToken>()){
        chain_handleResetChainTokens();
    }




    //If a contract token is receive a particle in the chain wil try to contract
    if(hasToken<chain_ContractToken>()){
        chain_handleContractToken();
    }

    if(hasToken<chain_CutoffToken>()){
        chain_handleCutoffToken();
    }

    if(hasToken<chain_ConfirmCutoffToken>()){
        chain_handleConfirmCutoffToken();
    }
}

void GSFParticle::chain_handleCutoffToken(){
    auto token = peekAtToken<chain_CutoffToken>();

    if(token->_handled){
        return;
    }

    if(hasNbrAtLabel(token->_dirpassed)){
        // copy cutoff token to neighbor if not already done
        if(!nbrAtLabel(token->_dirpassed).hasToken<chain_CutoffToken>()){
            auto cutoffToken = std::make_shared<GSFParticle::chain_CutoffToken>();
            cutoffToken->_dirpassed = token->_dirpassed;
            nbrAtLabel(token->_dirpassed).putToken(cutoffToken);
        }
    }else{
        auto confirmCutoffToken = std::make_shared<GSFParticle::chain_ConfirmCutoffToken>();
        confirmCutoffToken->_dirpassed = (token->_dirpassed + 3) %6;
        putToken(confirmCutoffToken);
    }

    token->_handled = true;
}

void GSFParticle::chain_handleConfirmCutoffToken(){
    auto token = peekAtToken<chain_ConfirmCutoffToken>();
    if(hasNbrAtLabel(token->_dirpassed)){
        // forward token to neighbor
        auto confirmToken = takeToken<chain_ConfirmCutoffToken>();
        nbrAtLabel(token->_dirpassed).putToken(confirmToken);
    }
}

void GSFParticle::chain_handleContractToken()
{
    Q_ASSERT(_state == State::CHAIN_FOLLOWER || (_state == State::COORDINATOR && !hasToken<triangle_shift_TriggerShiftToken>()));
    auto token = peekAtToken<chain_ContractToken>();

    int followerLabel = -1;


    if(isExpanded()){
        //check if has follower
        for(int label : tailLabels()){
            if(hasNbrAtLabel(label)){
                if(nbrAtLabel(label).hasToken<chain_DepthToken>()){
                    followerLabel = -2;
                    break;
                }
                if(nbrAtLabel(label)._ldrlabel>-1 &&
                        pointsAtMe(nbrAtLabel(label), nbrAtLabel(label).dirToHeadLabel(nbrAtLabel(label)._ldrlabel))){
                    followerLabel = label;
                    break;
                }
            }
        }
        if(followerLabel!= -2){ // check if setup is still in progress
            if(followerLabel>-1){
                auto follower = nbrAtLabel(followerLabel);
                if(token->_final){
                    if(nbrAtLabel(followerLabel).isExpanded()){
                        // Only pass if nbr has not already ContractToken
                        if(!nbrAtLabel(followerLabel).hasToken<chain_ContractToken>()){
                            nbrAtLabel(followerLabel).putToken(token);
                        }
                    } else {
                        nbrAtLabel(followerLabel)._ldrlabel =
                                    dirToNbrDir(nbrAtLabel(followerLabel), (tailDir() + 3) % 6);
                        pull(followerLabel);
                        token = takeToken<chain_ContractToken>();
                    }
                } else {
                    auto nbr = nbrAtLabel(followerLabel);
                    if(nbr.isExpanded()){
                        if(!_sent_pull){
                            // Only pass if nbr has not already ContractToken
                            if(!nbrAtLabel(followerLabel).hasToken<chain_ContractToken>()){
                                nbrAtLabel(followerLabel).putToken(token);
                            }
                            _sent_pull = true;
                        }

                    } else {
                        nbrAtLabel(followerLabel)._ldrlabel =
                                    dirToNbrDir(nbrAtLabel(followerLabel), (tailDir() + 3) % 6);
                        pull(followerLabel);
                        token = takeToken<chain_ContractToken>();
                        _sent_pull = false;
                    }
                }
            } else {
                token = takeToken<chain_ContractToken>();
                contractTail();                
            }
        }
    } else {
        for(int label : headLabels()){
            if(hasNbrAtLabel(label)){
                auto nbr = nbrAtLabel(label);
                if(nbrAtLabel(label).hasToken<chain_DepthToken>()){
                    followerLabel = -2;
                    break;
                }
                if(nbrAtLabel(label)._ldrlabel>-1 &&
                        pointsAtMe(nbrAtLabel(label),
                                   nbrAtLabel(label).dirToHeadLabel(nbrAtLabel(label)._ldrlabel))){
                    followerLabel = label;
                    break;
                }
            }
        }
        if(followerLabel != -2){
            if(followerLabel>-1){
                token = takeToken<chain_ContractToken>();
                // Only pass if nbr has not already ContractToken
                if(!nbrAtLabel(followerLabel).hasToken<chain_ContractToken>()){
                    nbrAtLabel(followerLabel).putToken(token);
                }
            } else {
                token = takeToken<chain_ContractToken>();
                if(token->_final){
                    if(isContracted()){
                        Q_ASSERT(hasNbrAtLabel(_ldrlabel));
                        auto t = std::make_shared<chain_ConfirmContractToken>();
                        nbrAtLabel(_ldrlabel).putToken(t);
                    }
                }
            }
        }
    }
}

// FOR DEBUG PURPOSES ONLY
void GSFParticle::chain_handleMovementInitToken()
{
    _ldrlabel = -1;
    auto initToken = takeToken<chain_MovementInitToken>();

    //pass token to next particle along the side of the triangle if necissary
    //and put chaintoken on this particle
    if(hasNbrAtLabel(initToken->_dirpassed) && initToken->_lifetime>0){
        initToken->_lifetime--;
        int dir = initToken->_dirpassed;
        nbrAtLabel(dir).putToken(initToken);
    }

    // Make a chain token, which initiates the chain movement
    auto chainToken = std::make_shared<chain_ChainToken>();
    chainToken->_contract = initToken->_contract;
    chainToken->L = initToken->L;


    putToken(chainToken);

    //send a token to followers with their respective depth value
    if(_state != State::COORDINATOR){
        _state = State::CHAIN_COORDINATOR;

        // TODO: FIX DIRECTION TO NOT DEPEND ON hasNbr
        int followerDir = initToken->_followerDir;
        Q_ASSERT(hasNbrAtLabel(followerDir));

        auto depthToken = std::make_shared<chain_DepthToken>();
        depthToken->_passeddir = followerDir;
        nbrAtLabel(followerDir).putToken(depthToken);
    }
}

void GSFParticle::chain_handleDepthToken()
{
    Q_ASSERT(_state == State::CHAIN_FOLLOWER);
    auto token = takeToken<chain_DepthToken>();
    _ldrlabel = (token->_passeddir+3)%6;
    if(hasToken<chain_CutoffToken>()){
        takeToken<chain_CutoffToken>();
    }else if(hasNbrAtLabel(token->_passeddir)){
        nbrAtLabel(token->_passeddir).putToken(token);
    }
}

int GSFParticle::findFollower()
{
    for(int label : headLabels()){
        if(hasNbrAtLabel(label)){
            auto nbr = nbrAtLabel(label);
            if(nbr._ldrlabel>-1 && pointsAtMe(nbr, nbr.dirToHeadLabel(nbr._ldrlabel))){
                return label;
            }
        }
    }
    return -1;
}

void GSFParticle::chain_handleConfirmContractToken()
{
    auto token = takeToken<chain_ConfirmContractToken>();

    if(hasToken<chain_ContractToken>()){
        auto contract_token = takeToken<chain_ContractToken>();
    }

    if(_state == State::CHAIN_FOLLOWER){
        if(isContracted()){
            if(_ldrlabel>-1){
                Q_ASSERT(hasNbrAtLabel(_ldrlabel));
                nbrAtLabel(_ldrlabel).putToken(token);
            }
        }
    }else if(isContracted() && (_state == State::CHAIN_COORDINATOR || _state == State::COORDINATOR)){

        if(hasToken<chain_ConfirmResetChainToken>()){
            auto _ = takeToken<chain_ConfirmResetChainToken>();
            auto token = takeToken<chain_ChainToken>();
            _state = State::CHAIN_FOLLOWER;
            _ldrlabel = -1;
        }else{
            int followerLabel = findFollower();
            if(followerLabel > -1 && !nbrAtLabel(followerLabel).hasToken<chain_ResetChainToken>()){
                auto resetToken = std::make_shared<chain_ResetChainToken>();
                nbrAtLabel(followerLabel).putToken(resetToken);
            }
        }

    }
}

void GSFParticle::chain_handleResetChainTokens(){
    if(_state == State::CHAIN_FOLLOWER){
        if(hasToken<chain_ResetChainToken>()){
            int followerLabel = findFollower();
            if(followerLabel > -1 && !nbrAtLabel(followerLabel).hasToken<chain_ResetChainToken>()){
                auto resetToken = takeToken<chain_ResetChainToken>();
                nbrAtLabel(followerLabel).putToken(resetToken);
            }else if(followerLabel == -1){
                auto _ = takeToken<chain_ResetChainToken>();
                auto resetToken = std::make_shared<chain_ConfirmResetChainToken>();
                putToken(resetToken);
            }
        }

        if(hasToken<chain_ConfirmResetChainToken>()){
            auto confirmToken = takeToken<chain_ConfirmResetChainToken>();
            if(hasNbrAtLabel(_ldrlabel)){
                    nbrAtLabel(_ldrlabel).putToken(confirmToken);
            }
            // Reset particle completely from chain movement
            _ldrlabel = -1;
        }
    }
}


void GSFParticle::chain_handleChainToken(){
    Q_ASSERT(_state == State::CHAIN_COORDINATOR || _state == State::COORDINATOR);

    auto token = peekAtToken<chain_ChainToken>();
    int followerLabel = -1;

    //expand to space if the chain coordinator is contracted
    if(isContracted()){
        //find follower
        for(int label : headLabels()){
            if(hasNbrAtLabel(label)){
                auto nbr = nbrAtLabel(label);
                if(nbr.hasToken<chain_DepthToken>()){
                    followerLabel = -2;
                    break;
                }
                if(nbr._ldrlabel>-1 && pointsAtMe(nbr, nbr.dirToHeadLabel(nbr._ldrlabel))){
                    followerLabel = label;
                    break;
                }
            }
        }

        //take top direction of token
        if(!token->L.empty()){
            int dir = token->L.top();
            token->L.pop();
            if(canExpand(dir)){
                expand(dir);
            }
        } else {
            if(token->_contract){
                if(_state == State::COORDINATOR){
                    auto token = takeToken<chain_ChainToken>();
                } else {
                    auto t = std::make_shared<chain_ContractToken>();
                    t->_final = true;
                    // Only pass if nbr has not already ContractToken
                    if(!nbrAtLabel(followerLabel).hasToken<chain_ContractToken>()){
                        nbrAtLabel(followerLabel).putToken(t);
                    }
                }
            }
        }
    //otherwise pull next node in the chain
    } else {
        for(int label : tailLabels()){
            if(hasNbrAtLabel(label)){
                auto nbr = nbrAtLabel(label);
                if(nbr.hasToken<chain_DepthToken>()){
                    followerLabel = -2;
                    break;
                }
                if(nbr._ldrlabel>-1 && pointsAtMe(nbr, nbr.dirToHeadLabel(nbr._ldrlabel))){
                    followerLabel = label;
                    break;
                }
            }
        }

        if(!token->L.empty()){
            if(followerLabel>-1){
                if(canPull(followerLabel)){
                    auto part = nbrAtLabel(followerLabel);
                    nbrAtLabel(followerLabel)._ldrlabel =
                            dirToNbrDir(nbrAtLabel(followerLabel), (tailDir() + 3) % 6);
                    pull(followerLabel);
                    _sent_pull = false;
                } else {
                    if(!_sent_pull){
                        _sent_pull = true;
                        auto t = std::make_shared<chain_ContractToken>();
                        t->_final = false;
                        nbrAtLabel(followerLabel).putToken(t);
                    }
                }
            }
        } else {
            //Contract the entire chain if so desired
            if (token->_contract){
                if(_state == State::COORDINATOR){
                    contractTail();
                    auto token = takeToken<chain_ChainToken>();
                } else {
                    if(followerLabel>-1){
                        if(canPull(followerLabel)){
                            auto t = std::make_shared<chain_ContractToken>();
                            t->_final = true;
                            nbrAtLabel(followerLabel).putToken(t);

                            nbrAtLabel(followerLabel)._ldrlabel =
                                        dirToNbrDir(nbrAtLabel(followerLabel), (tailDir() + 3) % 6);
                            pull(followerLabel);
                        } else {
                            auto t = std::make_shared<chain_ContractToken>();
                            t->_final = true;
                            nbrAtLabel(followerLabel).putToken(t);
                        }
                    }
                }
            //otherwise stop chain execution
            } else {
                if(_state != State::COORDINATOR){
                    auto token = takeToken<chain_ChainToken>();
                    _state = State::CHAIN_FOLLOWER;
                    _ldrlabel = -1;
                } else {
                    if(followerLabel>=-1){
                        auto token = takeToken<chain_ChainToken>();
                        _ldrlabel = -1;
                    }
                }
            }
        }
    }

}
