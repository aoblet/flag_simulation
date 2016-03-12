#pragma once
#include <vector>
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "PartyKel/renderer/Renderer2D.hpp"

namespace PartyKel{
    class Polygon2D{
        std::vector<glm::vec2> m_points;
        glm::vec3 m_color;
        bool m_isInner;
    public:
        static Polygon2D buildBox(glm::vec3 color, glm::vec2 position, float width, float height, bool isInner = false);
        static Polygon2D buildCircle(glm::vec3 color, glm::vec2 center, float radius, size_t discFactor, bool isInner = false);
        void draw(Renderer2D& renderer, float lineWidth = 1.f) const;
    };
}
