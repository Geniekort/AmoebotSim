#ifndef SYSTEM_H
#define SYSTEM_H

#include <array>
#include <deque>
#include <map>
#include <random>
#include <set>

#include "alg/algorithm.h"
#include "sim/node.h"
#include "sim/particle.h"

class System
{
public:
    enum class SystemState {
        Valid,
        Disconnected,
        Terminated,
        Deadlocked
    };

public:
    System();
    System(const System& other);

    void insertParticle(const Particle &p);
    void insertParticleAt(const Node &n);
    const Particle& at(int index) const;
    int getNumParticles() const;
    int getNumNonStaticParticles() const;

    SystemState round();
    SystemState runUntilNotValid();
    SystemState roundForParticle(const Node node);

    SystemState getSystemState() const;
    Node getDisconnectionNode() const;
    int getNumMovements() const;
    int getNumRounds() const;

protected:
    std::array<const Flag*, 10> assembleFlags(Particle& p);
    bool handleExpansion(Particle& p, int label);
    bool handleContraction(Particle& p, int label, bool isHandoverContraction);
    bool isConnected() const;
    void updateNumRounds(Particle* p);

public:
    // FIXME: Not the nice way of doing it.
    // But until we actually appraoch algorithms for disconnected systems it should do just fine.
    static bool checkConnectivity;

protected:
    std::mt19937 rng;

    std::deque<Particle> particles;
    std::map<Node, Particle*> particleMap;
    std::set<Particle*> activatedParticles;
    std::deque<Particle*> shuffledParticles;

    SystemState systemState;
    Node disconnectionNode;

    unsigned int numNonStaticParticles;
    int numMovements;
    int numRounds;
};

#endif // SYSTEM_H
