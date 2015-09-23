#ifndef AMOEBOTSYSTEM_H
#define AMOEBOTSYSTEM_H

#include <deque>
#include <map>
#include <set>

#include "alg/amoebotparticle.h"
#include "helper/randomnumbergenerator.h"
#include "sim/system.h"

class AmoebotSystem : public System, public RandomNumberGenerator
{
public:
    AmoebotSystem();
    ~AmoebotSystem();

    virtual void activate();

    virtual unsigned int size() const;
    virtual const Particle& at(const int i) const;

protected:
    std::deque<AmoebotParticle*> particles;
    std::map<Node, AmoebotParticle*> particleMap;
    std::deque<AmoebotParticle*> shuffledParticles;
    std::set<AmoebotParticle*> activatedParticles;
};

#endif // AMOEBOTSYSTEM_H