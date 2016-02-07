//
// Created by mehdi on 05/02/16.
//

#ifndef IMAC3_MOTEURSPHYSIQUES_PARTYKEL_LEAPFROGSOLVER2D_HPP
#define IMAC3_MOTEURSPHYSIQUES_PARTYKEL_LEAPFROGSOLVER2D_HPP

#include "PartyKel/ParticleManager2D.hpp"
namespace PartyKel
{

    class LeapFrogSolver2D
    {
    public:
        void solve(ParticleManager2D& particleManager, float dt);
    };
}


#endif //IMAC3_MOTEURSPHYSIQUES_PARTYKEL_LEAPFROGSOLVER2D_HPP
