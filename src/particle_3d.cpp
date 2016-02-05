#include <iostream>
#include <cstdlib>

#include <PartyKel/glm.hpp>
#include <PartyKel/WindowManager.hpp>

#include <PartyKel/renderer/Renderer3D.hpp>
#include <PartyKel/renderer/TrackballCamera.hpp>

#include <vector>

static const Uint32 WINDOW_WIDTH = 1024;
static const Uint32 WINDOW_HEIGHT = 768;

using namespace PartyKel;

class StaticParticleManager {
    // Chaque particule d'index i est représentée par sa position m_PositionArray[i], sa masse m_MassArray[i] et sa couleur m_ColorArray[i]
    std::vector<glm::vec3> m_PositionArray;
    std::vector<float> m_MassArray;
    std::vector<glm::vec3> m_ColorArray;

public:
    void addCircleParticles(float radius, uint32_t count) {
        float delta = 2 * 3.14f / count; // 2pi / nombre de particules
        for(size_t i = 0; i < count; ++i) {
            float c = cos(i * delta), s = sin(i * delta);
            addParticle(glm::vec3(radius * c, 0.f, radius * s), 1.f, glm::vec3(c, s, c * s));
        }
    }

    void addParticle(glm::vec3 position, float mass, glm::vec3 color) {
        m_PositionArray.push_back(position);
        m_MassArray.push_back(mass);
        m_ColorArray.push_back(color);
    }

    void drawParticles(Renderer3D& renderer) {
        renderer.drawParticles(m_PositionArray.size(),
                               m_PositionArray.data(),
                               m_MassArray.data(),
                               m_ColorArray.data(),
                               0.05);
    }

    void move(float maxDist) {
        // Déplace aléatoirement les particules
        for(uint32_t i = 0; i < m_PositionArray.size(); ++i) {
            m_PositionArray[i] += glm::sphericalRand(maxDist);
        }
    }
};

int main() {
    WindowManager wm(WINDOW_WIDTH, WINDOW_HEIGHT, "Newton was a Geek");
    wm.setFramerate(30);

    // Création des particules
    StaticParticleManager particleManager;
    particleManager.addParticle(glm::vec3(0), 1, glm::vec3(1, 1, 1));
    particleManager.addCircleParticles(0.5f, 128);

    Renderer3D renderer;
    renderer.setProjMatrix(glm::perspective(70.f, float(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.1f, 100.f));

    TrackballCamera camera;
    int mouseLastX, mouseLastY;

    // Temps s'écoulant entre chaque frame
    float dt = 0.f;

	bool done = false;
    bool wireframe = true;
    while(!done) {
        wm.startMainLoop();

        // Rendu
        renderer.clear();

        renderer.setViewMatrix(camera.getViewMatrix());
        particleManager.drawParticles(renderer);

        // Simulation
        particleManager.move(dt * 0.01f);

        // Gestion des evenements
		SDL_Event e;
        while(wm.pollEvent(e)) {
			switch(e.type) {
				default:
					break;
				case SDL_QUIT:
					done = true;
					break;
                case SDL_KEYDOWN:
                    if(e.key.keysym.sym == SDLK_SPACE) {
                        wireframe = !wireframe;
                    }
                case SDL_MOUSEBUTTONDOWN:
                    if(e.button.button == SDL_BUTTON_WHEELUP) {
                        camera.moveFront(0.1f);
                    } else if(e.button.button == SDL_BUTTON_WHEELDOWN) {
                        camera.moveFront(-0.1f);
                    } else if(e.button.button == SDL_BUTTON_LEFT) {
                        mouseLastX = e.button.x;
                        mouseLastY = e.button.y;
                    }
			}
		}

        int mouseX, mouseY;
        if(SDL_GetMouseState(&mouseX, &mouseY) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            float dX = mouseX - mouseLastX, dY = mouseY - mouseLastY;
            camera.rotateLeft(glm::radians(dX));
            camera.rotateUp(glm::radians(dY));
            mouseLastX = mouseX;
            mouseLastY = mouseY;
        }

        // Mise à jour de la fenêtre
        dt = wm.update();
	}

	return EXIT_SUCCESS;
}
