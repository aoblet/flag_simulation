#pragma once

#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#include <vector>
#include "PartyKel/renderer/Renderer2D.hpp"


namespace PartyKel{
    class ParticleManager2D{
    private:
        std::vector<glm::vec2> m_positions;
        std::vector<glm::vec2> m_velocities;
        std::vector<glm::vec2> m_forces;
        std::vector<glm::vec3> m_colors;
        std::vector<float> m_masses;
    public:
        int addParticle(const glm::vec2& pos, const glm::vec2& velo, const glm::vec2& force,const glm::vec3& color, float mass);
        void addRandomParticles(float radius, uint32_t count);
        void drawParticles(Renderer2D& renderer);
        void clear();

        glm::vec2 getPosition(int i);
        glm::vec2 getVelocity(int i);
        glm::vec2 getForce(int i);
        float getMass(int i);


        void addForceToAll(const glm::vec2& f);
        void addForce(const glm::vec2& f, int i);
        void resetForce(int i);

        void setVelocity(const glm::vec2& v, int i);
        void setPosition(const glm::vec2& p, int i);

        int size() const;
        void printParticles() const;
        void printParticle(int index) const;
    };
}
