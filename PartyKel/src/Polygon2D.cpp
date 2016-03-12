#include "PartyKel/Polygon2D.hpp"


namespace PartyKel{
    Polygon2D Polygon2D::buildBox(glm::vec3 color, glm::vec2 position, float width, float height, bool isInner) {
        Polygon2D polygon2D;
        polygon2D.m_color = color;
        polygon2D.m_isInner = isInner;
        polygon2D.m_points.push_back(position);
        polygon2D.m_points.push_back(position + glm::vec2(width, 0));
        polygon2D.m_points.push_back(position + glm::vec2(width, height));
        polygon2D.m_points.push_back(position + glm::vec2(0, height));

        return polygon2D;
    }

    Polygon2D Polygon2D::buildCircle(glm::vec3 color, glm::vec2 center, float radius, size_t discFactor, bool isInner){
        Polygon2D polygon2D;
        polygon2D.m_color = color;
        for(int i=0; i < discFactor; ++i){
            float angle = 2*M_PI/discFactor * i;
            polygon2D.m_points.push_back(glm::vec2(std::cos(angle), std::sin(angle) ) * radius);
        }
        polygon2D.m_isInner = isInner;
        return polygon2D;
    }

    void Polygon2D::draw(Renderer2D& renderer, float lineWidth) const{
        renderer.drawPolygon(m_points.size(), m_points.data(), m_color, lineWidth);
    }
}
