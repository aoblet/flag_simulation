#pragma once
#include "PartyKel/ParticleManager2D.hpp"

namespace PartyKel{
    class AbstractForce{
    public:
        virtual void apply(ParticleManager2D& pm) = 0;
        virtual ~AbstractForce() = 0;
    };
}
