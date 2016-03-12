#include "PartyKel/ConstantForce2D.hpp"

namespace PartyKel{
    ConstantForce2D::ConstantForce2D(const glm::vec2& f): m_force(f){}

    void ConstantForce2D::apply(ParticleManager2D &pm) {
        pm.addForceToAll(m_force);
    }
}