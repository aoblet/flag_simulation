#include "PartyKel/LeapfrogSolver2D.hpp"

namespace PartyKel{
    void LeapfrogSolver2D::solve(ParticleManager2D &pm, float dt) {
        for(int i=0; i<pm.size(); ++i){
            pm.setVelocity(pm.getVelocity(i) + (dt*(pm.getForce(i)/pm.getMass(i))), i);
            pm.setPosition(pm.getPosition(i) + (dt*(pm.getVelocity(i))), i);
            pm.resetForce(i);
        }
    }
}