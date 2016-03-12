#pragma once
#include "PartyKel/ParticleManager2D.hpp"

namespace PartyKel{
    class LeapfrogSolver2D{
    public:
        void solve(ParticleManager2D& pm, float dt);
    };
}
