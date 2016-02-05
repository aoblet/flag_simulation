#include "PartyKel/renderer/Renderer2D.hpp"
#include "PartyKel/renderer/GLtools.hpp"
#include "PartyKel/glm.hpp"

namespace PartyKel {

const GLchar* Renderer2D::VERTEX_SHADER =
"#version 330 core\n"
GL_STRINGIFY(
    layout(location = 0) in vec2 aVertexPosition;

    uniform vec2 uParticlePosition;
    uniform float uParticleScale;

    out vec2 vFragPosition;

    void main() {
        vFragPosition = aVertexPosition;
        gl_Position = vec4(uParticlePosition + uParticleScale * aVertexPosition, 0.f, 1.f);
    }
);

const GLchar* Renderer2D::FRAGMENT_SHADER =
"#version 330 core\n"
GL_STRINGIFY(
    in vec2 vFragPosition;

    out vec4 fFragColor;

    uniform vec3 uParticleColor;

    float computeAttenuation(float distance) {
        return 3.f * exp(-distance * distance * 9.f);
    }

    void main() {
        float distance = length(vFragPosition);
        float attenuation = computeAttenuation(distance);
        fFragColor = vec4(uParticleColor, attenuation);
    }
);

const GLchar* Renderer2D::POLYGON_VERTEX_SHADER =
"#version 330 core\n"
GL_STRINGIFY(
    layout(location = 0) in vec2 aVertexPosition;

    void main() {
        gl_Position = vec4(aVertexPosition, 0.f, 1.f);
    }
);

const GLchar* Renderer2D::POLYGON_FRAGMENT_SHADER =
"#version 330 core\n"
GL_STRINGIFY(
    out vec3 fFragColor;

    uniform vec3 uPolygonColor;

    void main() {
        fFragColor = uPolygonColor;
    }
);

const GLchar* Renderer2D::LINE_VERTEX_SHADER =
"#version 330 core\n"
GL_STRINGIFY(
    layout(location = 0) in vec2 aVertexPosition;
    layout(location = 1) in vec3 aVertexColor;

    out vec3 vFragColor;

    void main() {
        vFragColor = aVertexColor;
        gl_Position = vec4(aVertexPosition, 0.f, 1.f);
    }
);

const GLchar* Renderer2D::LINE_FRAGMENT_SHADER =
"#version 330 core\n"
GL_STRINGIFY(
    out vec3 fFragColor;

    in vec3 vFragColor;

    void main() {
        fFragColor = vFragColor;
    }
);

Renderer2D::Renderer2D():
    m_ProgramID(buildProgram(VERTEX_SHADER, FRAGMENT_SHADER)),
    m_PolygonProgramID(buildProgram(POLYGON_VERTEX_SHADER, POLYGON_FRAGMENT_SHADER)),
    m_LineProgramID(buildProgram(LINE_VERTEX_SHADER, LINE_FRAGMENT_SHADER)) {

    // Récuperation des uniforms
    m_uParticleColor = glGetUniformLocation(m_ProgramID, "uParticleColor");
    m_uParticlePosition = glGetUniformLocation(m_ProgramID, "uParticlePosition");
    m_uParticleScale = glGetUniformLocation(m_ProgramID, "uParticleScale");

    m_uPolygonColor = glGetUniformLocation(m_PolygonProgramID, "uPolygonColor");

    // Création du VBO
    glGenBuffers(1, &m_VBOID);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBOID);

    // Une particule est un carré
    GLfloat positions[] = {
        -1.f, -1.f,
         1.f, -1.f,
         1.f,  1.f,
        -1.f,  1.f
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

    // Création du VAO
    glGenVertexArrays(1, &m_VAOID);
    glBindVertexArray(m_VAOID);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenBuffers(1, &m_PolygonVBOID);
    glBindBuffer(GL_ARRAY_BUFFER, m_PolygonVBOID);
    glGenVertexArrays(1, &m_PolygonVAOID);
    glBindVertexArray(m_PolygonVAOID);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenBuffers(1, &m_LinePositionVBOID);
    glGenBuffers(1, &m_LineColorVBOID);
    glGenBuffers(1, &m_LineIBOID);

    glGenVertexArrays(1, &m_LineVAOID);
    glBindVertexArray(m_LineVAOID);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_LinePositionVBOID);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, m_LineColorVBOID);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Renderer2D::~Renderer2D() {
    glDeleteProgram(m_ProgramID);
    glDeleteProgram(m_PolygonProgramID);
    glDeleteProgram(m_LineProgramID);

    glDeleteBuffers(1, &m_VBOID);
    glDeleteBuffers(1, &m_PolygonVBOID);
    glDeleteVertexArrays(1, &m_VAOID);
    glDeleteVertexArrays(1, &m_PolygonVAOID);
}

void Renderer2D::clear() {
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer2D::drawParticles(
        uint32_t count,
        const glm::vec2* positionArray,
        const float* massArray,
        const glm::vec3* colorArray,
        float massScale) {
    // Active la gestion de la transparence
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST);

    glUseProgram(m_ProgramID);

    glBindVertexArray(m_VAOID);

    // Dessine chacune des particules
    for(uint32_t i = 0; i < count; ++i) {
        glUniform3fv(m_uParticleColor, 1, glm::value_ptr(colorArray[i]));
        glUniform2fv(m_uParticlePosition, 1, glm::value_ptr(positionArray[i]));
        glUniform1f(m_uParticleScale, massScale * massArray[i]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    glBindVertexArray(0);

    glDisable(GL_BLEND);
}

void Renderer2D::drawPolygon(uint32_t count,
                 const glm::vec2* position,
                 const glm::vec3& color,
                 float lineWidth) {
    glBindBuffer(GL_ARRAY_BUFFER, m_PolygonVBOID);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(position[0]), position, GL_DYNAMIC_DRAW);

    glDisable(GL_DEPTH_TEST);

    glLineWidth(lineWidth);

    glUseProgram(m_PolygonProgramID);

    glBindVertexArray(m_PolygonVAOID);

    glUniform3fv(m_uPolygonColor, 1, glm::value_ptr(color));
    glDrawArrays(GL_LINE_LOOP, 0, count);

    glBindVertexArray(0);
}

void Renderer2D::drawLines(
                           uint32_t lineCount,
                           const std::pair<unsigned int, unsigned int>* lines,
                           uint32_t vertexCount,
                           const glm::vec2* positionArray,
                           const glm::vec3* colorArray,
                           float lineWidth) {
    glBindBuffer(GL_ARRAY_BUFFER, m_LinePositionVBOID);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(positionArray[0]), positionArray, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_LineColorVBOID);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(colorArray[0]), colorArray, GL_DYNAMIC_DRAW);

    glDisable(GL_DEPTH_TEST);

    glLineWidth(lineWidth);

    glUseProgram(m_LineProgramID);

    glBindVertexArray(m_LineVAOID);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_LineIBOID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, lineCount * sizeof(lines[0]), lines, GL_DYNAMIC_DRAW);

    glDrawElements(GL_LINES, lineCount * 2, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

}
