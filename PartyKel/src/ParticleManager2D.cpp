#include "PartyKel/ParticleManager2D.hpp"
#include <iostream>

namespace PartyKel{
    int ParticleManager2D::addParticle(const glm::vec2& pos, const glm::vec2& velo, const glm::vec2& force,
                                       const glm::vec3& color, float mass){

        m_positions.push_back(pos);
        m_velocities.push_back(velo);
        m_forces.push_back(force);
        m_colors.push_back(color);
        m_masses.push_back(mass);
        return m_masses.size() - 1 ;
    }

    void ParticleManager2D::addRandomParticles(float radius, uint32_t count){

        float delta = 2 * 3.14f / count; // 2pi / nombre de particules
        for(size_t i = 0; i < count; ++i) {
            float c = cos(i * delta), s = sin(i * delta);
            addParticle(glm::vec2(radius * c, radius * s), glm::vec2(0,0), glm::vec2(0, 0), glm::vec3(c, s, c * s), 1.f);
        }
    }

    void ParticleManager2D::drawParticles(Renderer2D& renderer){
        renderer.drawParticles(m_masses.size(), m_positions.data(), m_masses.data(), m_colors.data());
    }

    void ParticleManager2D::clear() {
        m_positions.clear();
        m_velocities.clear();
        m_forces.clear();
        m_colors.clear();
        m_masses.clear();
    }
    void ParticleManager2D::addForceToAll(const glm::vec2& f){
        for(auto& force: m_forces)
            force += f;
    }

    int ParticleManager2D::size() const{
        return m_masses.size();
    }

    void ParticleManager2D::printParticles() const{
        for(int i=0; i<size(); ++i)
            printParticle(i);
    }
    void ParticleManager2D::printParticle(int index) const{
        std::cout << m_forces[index] << std::endl;
    }

    glm::vec2 ParticleManager2D::getPosition(int i){
        return m_positions[i];
    }
    glm::vec2 ParticleManager2D::getVelocity(int i){
        return m_velocities[i];
    }
    glm::vec2 ParticleManager2D::getForce(int i){
        return m_forces[i];
    }
    float ParticleManager2D::getMass(int i){
        return m_masses[i];
    }

    void ParticleManager2D::addForce(const glm::vec2& f, int i){
        m_forces[i] += f;
    }
    void ParticleManager2D::resetForce( int i){
        m_forces[i].x = m_forces[i].y = 0;
    }

    void ParticleManager2D::setVelocity(const glm::vec2& v, int i){
        m_velocities[i] = v;
    }

    void ParticleManager2D::setPosition(const glm::vec2& p, int i){
        m_positions[i] = p;
    }

}