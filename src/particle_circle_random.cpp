#include <iostream>
#include <cstdlib>

#include <PartyKel/glm.hpp>
#include <PartyKel/WindowManager.hpp>
#include <PartyKel/renderer/Renderer2D.hpp>
#include <PartyKel/atb.hpp>

#include <vector>

static const Uint32 WINDOW_WIDTH = 1024;
static const Uint32 WINDOW_HEIGHT = 1024;

using namespace PartyKel;

class StaticParticleManager {
    // Chaque particule d'index i est représentée par sa position m_PositionArray[i], sa masse m_MassArray[i] et sa couleur m_ColorArray[i]
    std::vector<glm::vec2> m_PositionArray;
    std::vector<float> m_MassArray;
    std::vector<glm::vec3> m_ColorArray;

public:
    void clear() {
        m_PositionArray.clear();
        m_MassArray.clear();
        m_ColorArray.clear();
    }

    void addCircleParticles(float radius, uint32_t count) {
        float delta = 2 * 3.14f / count; // 2pi / nombre de particules
        for(size_t i = 0; i < count; ++i) {
            float c = cos(i * delta), s = sin(i * delta);
            addParticle(glm::vec2(radius * c, radius * s), 1.f, glm::vec3(c, s, c * s));
        }
    }

    void addParticle(glm::vec2 position, float mass, glm::vec3 color) {
        m_PositionArray.push_back(position);
        m_MassArray.push_back(mass);
        m_ColorArray.push_back(color);
    }

    void drawParticles(Renderer2D& renderer) {
        renderer.drawParticles(m_PositionArray.size(),
                               m_PositionArray.data(),
                               m_MassArray.data(),
                               m_ColorArray.data());
    }

    void move(float maxDist) {
        // Déplace aléatoirement les particules
        for(uint32_t i = 0; i < m_PositionArray.size(); ++i) {
            m_PositionArray[i] += glm::diskRand(maxDist);
        }
    }
};

int main() {
    WindowManager wm(WINDOW_WIDTH, WINDOW_HEIGHT, "Newton was a Geek");
    wm.setFramerate(30);

    // Initialisation de AntTweakBar (pour la GUI)
    TwInit(TW_OPENGL_CORE, NULL);
    TwWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    Renderer2D renderer;

    uint32_t particleCount = 128;

    // Création des particules
    StaticParticleManager particleManager;
    particleManager.addParticle(glm::vec2(0, 0), 1, glm::vec3(1, 1, 1));
    particleManager.addCircleParticles(0.5f, particleCount);

    TwBar* gui = TwNewBar("Parametres");
    float randomMoveScale = 0.01f;

    atb::addVarRW(gui, ATB_VAR(randomMoveScale), " step=0.01 ");
    atb::addVarRW(gui, ATB_VAR(particleCount));
    atb::addButton(gui, "reset", [&]() {
        particleManager.clear();
        particleManager.addParticle(glm::vec2(0, 0), 1, glm::vec3(1, 1, 1));
        particleManager.addCircleParticles(0.5f, particleCount);
    });

    // Temps s'écoulant entre chaque frame
    float dt = 0.f;

	bool done = false;
    while(!done) {
        wm.startMainLoop();

        // Rendu
        renderer.clear();
        particleManager.drawParticles(renderer);

        // Simulation
        particleManager.move(dt * randomMoveScale);

        TwDraw();

        // Gestion des evenements
        SDL_Event e;
        while(wm.pollEvent(e)) {
            int handled = TwEventSDL(&e, SDL_MAJOR_VERSION, SDL_MINOR_VERSION);

            if(!handled) {
                switch(e.type) {
                    default:
                        break;
                    case SDL_QUIT:
                        done = true;
                        break;
                }
            }
		}

        // Mise à jour de la fenêtre
        dt = wm.update();
	}

	return EXIT_SUCCESS;
}
