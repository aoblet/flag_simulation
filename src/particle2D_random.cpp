#include <iostream>
#include <cstdlib>

#include <PartyKel/glm.hpp>
#include <PartyKel/WindowManager.hpp>
#include <PartyKel/renderer/Renderer2D.hpp>
#include <PartyKel/atb.hpp>
#include <PartyKel/ParticleManager2D.hpp>
#include <PartyKel/ConstantForce2D.hpp>
#include <PartyKel/LeapfrogSolver2D.hpp>

#include <vector>

static const Uint32 WINDOW_WIDTH = 1024;
static const Uint32 WINDOW_HEIGHT = 1024;

using namespace PartyKel;


int main() {
    WindowManager wm(WINDOW_WIDTH, WINDOW_HEIGHT, "Newton was a Geek");
    wm.setFramerate(30);

    // Initialisation de AntTweakBar (pour la GUI)
    TwInit(TW_OPENGL_CORE, NULL);
    TwWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    Renderer2D renderer;

    uint32_t particleCount = 5;

    // Création des particules
    ParticleManager2D particleManager;
    ConstantForce2D constantForce2D(glm::vec2(0, -0.01));
    LeapfrogSolver2D leapfrogSolver2D;
    particleManager.addRandomParticles(0.5f, particleCount);


    TwBar* gui = TwNewBar("Parametres");
    float randomMoveScale = 0.01f;

    atb::addVarRW(gui, ATB_VAR(randomMoveScale), " step=0.01 ");
    atb::addVarRW(gui, ATB_VAR(particleCount));
    atb::addButton(gui, "reset", [&]() {
        particleManager.clear();
        particleManager.addRandomParticles(0.5f, particleCount);
    });

    // Temps s'écoulant entre chaque frame
    float dt = 0.f;

    bool done = false;
    while(!done) {
        wm.startMainLoop();

        // Rendu
        renderer.clear();
        constantForce2D.apply(particleManager);
        leapfrogSolver2D.solve(particleManager, dt);
        particleManager.drawParticles(renderer);

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
