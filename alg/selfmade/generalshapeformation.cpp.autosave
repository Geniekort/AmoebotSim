#include "generalshapeformation.h"


GSFParticle::GSFParticle(Node& head, const int globalTailDir,
                         const int orientation, AmoebotSystem& system,
                         const int sideLen, int triangleDirection, const State state,
                         unsigned int level, unsigned int ldrlabel,
                         int depth)
        : AmoebotParticle(head, globalTailDir, orientation, system),
          _initialSideLen(sideLen),
          _triangleDirection(triangleDirection),
          _state(state),
          _level(level),
          _ldrlabel(ldrlabel),
          _depth(depth){

}

void GSFParticle::activate(){
    intermediate_activate();
    chain_activate();
    triangle_shift_activate();
    triangle_rotate_activate();
}

GSFParticle& GSFParticle::nbrAtLabel(int label) const {
  return AmoebotParticle::nbrAtLabel<GSFParticle>(label);
}


int GSFParticle::headMarkColor() const{
    switch(_state){
        case State::COORDINATOR: return 0x00ff00;
        case State::CHAIN_COORDINATOR: return 0x0000ff;
        case State::CHAIN_FOLLOWER: return 0xff9000;
    }
    return -1;
}

int GSFParticle::headMarkDir() const{
    return _ldrlabel;
}

int GSFParticle::tailMarkColor() const{
    return headMarkColor();
}

QString GSFParticle::inspectionText() const{
    QString text;
    text += "Particle State: ";
    text += [this](){
        switch (_state) {
            case State::CHAIN_COORDINATOR: return "chain_coordinator\n";
            case State::COORDINATOR: return "coordinator\n";
            case State::CHAIN_FOLLOWER: return "follower\n";
        }
        return "no state??\n";
    }();
    text += "_ level: " + QString::number(_level) + "\n";
    text += " _depth: " + QString::number(_depth) + "\n";
    text += " _triangleDirection: " + QString::number(_triangleDirection) + "\n";
    text += " _ldrlabel: " + QString::number(_ldrlabel) + "\n";

    if(hasToken<chain_ContractToken>()){
        text+= "has contractToken\n";
    }
    if(hasToken<chain_ChainToken>()){
        auto t = peekAtToken<chain_ChainToken>();
        text+= "has chainToken\n";
        if(t->L.empty()){
            text+= "stack L is empty\n";
        } else {
            text+= "stack L is not empty\n";
        }
    }
    if(hasToken<chain_ConfirmContractToken>()){
        text+= "has chain_ConfirmContractToken\n";
    }
    if (hasToken<triangle_shift_TriggerShiftToken>()){
        text += "has triangle_shift_TriggerShiftToken\n";
    }
    if (hasToken<triangle_shift_CoordinatorToken>()){
        text += "has triangle_shift_CoordinatorToken\n";
    }
    if (hasToken<triangle_shift_ShiftToken>()){
        auto token = peekAtToken<triangle_shift_ShiftToken>();
        text += "has shiftToken\n";
        text += "  _dirpassed: " + QString::number(token->_dirpassed) +"\n";
    }
    if(hasToken<chain_DepthToken>()){
        text+= "has depthToken\n";
    }
    if(hasToken<chain_ResetChainToken>()){
        text+= "has chain_ResetChainToken\n";
    }
    if(hasToken<chain_ConfirmResetChainToken>()){
        text+= "has chain_ConfirmResetChainToken\n";
    }
    if(hasToken<triangle_rotate_TriggerExpandToken>()){
        text+= "has triangle_rotate_TriggerExpandToken\n";
    }
    if(hasToken<triangle_shift_ConfirmShiftToken>()){
        text+= "has triangle_shift_ConfirmShiftToken\n";
    }
    if(hasToken<triangle_rotate_ConfirmExpandToken>()){
        text+= "has triangle_rotate_ConfirmExpandToken\n";
    }
    if(hasToken<triangle_rotate_ExpandToken>()){
        text+= "has a triangle_rotate_ExpandToken\n";
    }
    if(hasToken<chain_MovementInitToken>()){
        auto token = peekAtToken<chain_MovementInitToken>();
        text+= "has a chain_MovementInitToken\n";
        text += "  _dirpassed: " + QString::number(token->_dirpassed) +"\n";
        text += "  _followerDir: " + QString::number(token->_followerDir) +"\n";
    }
    if(hasToken<chain_CutoffToken>()){
        text+= "has a chain_CutoffToken\n";
    }
    if(hasToken<chain_ConfirmCutoffToken>()){
        text+= "has a chain_ConfirmCutoffToken\n";
    }

    if(hasToken<intermediate_TriggerIntermediateToken>()){
        auto token = peekAtToken<intermediate_TriggerIntermediateToken>();
        text+= "has a intermediate_TriggerIntermediateToken\n";
        text+= "    _step: " + token->_step + "\n";
        text+= "    +initiated: " + QString::number(token->_initiated) + "\n";
    }
    return text;
}

GSFSystem::GSFSystem(int sideLen, int l){
    int dir  = 4;
    std::set<Node> occupied;
    Node current(0,0);
    auto coordinator = new GSFParticle(current, -1, 0, *this, sideLen, dir,
                                        GSFParticle::State::COORDINATOR, 0, -1, 0);

    insert(coordinator);

    auto intermediateToken = std::make_shared<GSFParticle::intermediate_TriggerIntermediateToken>();
    intermediateToken->_l = l;

    coordinator->putToken(intermediateToken);

    current = current.nodeInDir(dir%6);
    initializeTriangle(sideLen, current, dir);
}


void GSFSystem::initializeTriangle(int sideLen, Node current, int dir)
{
    for(int i = 1; i < sideLen; i++){
        auto sideP = new GSFParticle(current, -1, 0, *this, sideLen, dir,
                                          GSFParticle::State::CHAIN_FOLLOWER,
                                          i, -1, 0);

        insert(sideP);
        Node temp = current.nodeInDir((dir+2)%6);
        for(int j = 0; j<i; j++){

            auto innerP = new GSFParticle(temp, -1, 0, *this, sideLen, dir,
                                              GSFParticle::State::CHAIN_FOLLOWER,
                                               i, -1, 0);
            insert(innerP);
            temp = temp.nodeInDir((dir+2)%6);
        }
        current = current.nodeInDir(dir%6);
    }
}
