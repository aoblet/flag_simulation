#pragma once
#include "PartyKel/AbstractForce.hpp"
#include "PartyKel/ParticleManager2D.hpp"
#include "glm/vec2.hpp"


namespace PartyKel{
    class ConstantForce2D: public AbstractForce{
        glm::vec2 m_force;
    public:
        ConstantForce2D(const glm::vec2& f );
        void apply(ParticleManager2D& pm) override ;
        virtual ~ConstantForce2D(){};
    };
}
