#include <iostream>
#include <cstdlib>

#define GLM_FORCE_RADIANS
#include <PartyKel/glm.hpp>
#include <PartyKel/WindowManager.hpp>
#include <glm/gtc/random.hpp>

#include <PartyKel/renderer/FlagRenderer3D.hpp>
#include <PartyKel/renderer/TrackballCamera.hpp>
#include <PartyKel/renderer/Renderer3D.hpp>
#include <PartyKel/renderer/Sphere.hpp>
#include <PartyKel/atb.hpp>
#include <PartyKel/Octree.h>
#include <glog/logging.h>

#include <vector>

static const Uint32 WINDOW_WIDTH = 1024;
static const Uint32 WINDOW_HEIGHT = 768;

using namespace PartyKel;

// Calcule une force de type ressort de Hook entre deux particules de positions P1 et P2
// K est la résistance du ressort et L sa longueur à vide
inline glm::vec3 hookForce(float K, float L, const glm::vec3& P1, const glm::vec3& P2) {
    static const float epsilon = 0.0001;
    return K * (1-(L/std::max(glm::distance(P1, P2), epsilon))) * (P2 - P1);
}

// Calcule une force de type frein cinétique entre deux particules de vélocités v1 et v2
// V est le paramètre du frein et dt le pas temporel
inline glm::vec3 brakeForce(float V, float dt, const glm::vec3& v1, const glm::vec3& v2) {
    return V * ((v2-v1) / dt);
}

inline glm::vec3 sphereCollisionForce(float distanceToCenter, const glm::vec3& sphereCenter, float sphereRadius, const glm::vec3 particlePosition) {

    glm::vec3 direction = glm::normalize(particlePosition - sphereCenter);

    return direction * (1 / (1 + distanceToCenter));
}

// Structure permettant de simuler un drapeau à l'aide un système masse-ressort
struct Flag {
    int gridWidth, gridHeight; // Dimensions de la grille de points

    // Propriétés physique des points:
    std::vector<glm::vec3> positionArray;
    std::vector<glm::vec3> velocityArray;
    std::vector<float> massArray;
    std::vector<glm::vec3> forceArray;
    int nbParticles;

    // Paramètres des forces interne de simulation
    // Longueurs à vide
    glm::vec2 L0;
    float L1;
    glm::vec2 L2;

    float K0, K1, K2; // Paramètres de résistance
    float V0, V1, V2; // Paramètres de frein

    // Créé un drapeau discretisé sous la forme d'une grille contenant gridWidth * gridHeight
    // points. Chaque point a pour masse mass / (gridWidth * gridHeight).
    // La taille du drapeau en 3D est spécifié par les paramètres width et height
    Flag(float mass, float width, float height, int gridWidth, int gridHeight):
            gridWidth(gridWidth), gridHeight(gridHeight),
            positionArray(gridWidth * gridHeight),
            velocityArray(gridWidth * gridHeight, glm::vec3(0.f)),
            massArray(gridWidth * gridHeight, mass / (gridWidth * gridHeight)),
            forceArray(gridWidth * gridHeight, glm::vec3(0.f)) {


        glm::vec3 origin(-0.5f * width, -0.5f * height, 0.f);
        glm::vec3 scale(width / (gridWidth - 1), height / (gridHeight - 1), 1.f);

        nbParticles = gridWidth * gridHeight;
        for(int j = 0; j < gridHeight; ++j) {
            for(int i = 0; i < gridWidth; ++i) {
                int k = i + j * gridWidth;
                positionArray[k] = origin + glm::vec3(i, j, origin.z) * scale;
                massArray[k] = 1 - ( i / (2*(gridHeight*gridWidth)));
            }
        }

        // Les longueurs à vide sont calculés à partir de la position initiale
        // des points sur le drapeau
        L0.x = scale.x;
        L0.y = scale.y;
        L1 = glm::length(L0);
        L2 = 2.f * L0;

        // Ces paramètres sont à fixer pour avoir un système stable: HAVE FUN !
//        K0 = 1.f;
//        K1 = 1.f;
//        K2 = 1.f;
//
//        V0 = 0.1f;
//        V1 = 0.1f;
//        V2 = 0.1f;

        K0 = 1;
        K1 = 1.3;
        K2 = 0.8;

        V0 = 0.08;
        V1 = 0.02;
        V2 = 0.06;
    }

    // Applique les forces internes sur chaque point du drapeau SAUF les points fixes
    void applyInternalForces(float dt) {
        std::vector<glm::ivec2> neighbors(4);
        for(int i = 1; i<gridWidth; ++i){
            for(int j = 0; j<gridHeight; ++j){
                int currentK = j*gridWidth + i;
//                if(currentK == nbParticles-1 || currentK == nbParticles - gridWidth)
//                    continue;
                // TOPOLOGY 1
                neighbors[0] = glm::ivec2(i+1, j);
                neighbors[1] = glm::ivec2(i-1, j);
                neighbors[2] = glm::ivec2(i, j-1);
                neighbors[3] = glm::ivec2(i, j+1);

                int tmpI = 0;
                for(auto& p : neighbors){
                    if(p.x < 0 || p.y < 0 || p.x >= gridWidth || p.y >= gridHeight)
                        continue;
                    int k = p.y * gridWidth + p.x;
                    forceArray[currentK] += hookForce(K0, tmpI < 2 ? L0.x : L0.y, positionArray[currentK], positionArray[k]);
                    forceArray[currentK] += brakeForce(V0, dt, velocityArray[currentK], velocityArray[k]);
                    ++tmpI;
                }

                // TOPOLOGY 2
                neighbors[0] = glm::ivec2(i-1, j-1);
                neighbors[1] = glm::ivec2(i+1, j-1);
                neighbors[2] = glm::ivec2(i+1, j+1);
                neighbors[3] = glm::ivec2(i-1, j+1);

                for(auto& p : neighbors){
                    if(p.x < 0 || p.y < 0 || p.x >= gridWidth || p.y >= gridHeight)
                        continue;
                    int k = p.y * gridWidth + p.x;
                    forceArray[currentK] += hookForce(K1, L1, positionArray[currentK], positionArray[k]);
                    forceArray[currentK] += brakeForce(V1, dt, velocityArray[currentK], velocityArray[k]);
                }

                // TOPOLOGY 3
                neighbors[0] = glm::ivec2(i-2, j);
                neighbors[1] = glm::ivec2(i+2, j);
                neighbors[2] = glm::ivec2(i, j-2);
                neighbors[3] = glm::ivec2(i, j+2);

                tmpI = 0;
                for(auto& p : neighbors){
                    if(p.x < 0 || p.y < 0 || p.x >= gridWidth || p.y >= gridHeight)
                        continue;
                    int k = p.y * gridWidth + p.x;
                    forceArray[currentK] += hookForce(K2, tmpI < 2 ? L2.x : L2.y, positionArray[currentK], positionArray[k]);
                    forceArray[currentK] += brakeForce(V2, dt, velocityArray[currentK], velocityArray[k]);
                    ++tmpI;
                }

            }
        }
    }

    // Applique une force externe sur chaque point du drapeau SAUF les points fixes
    void applyExternalForce(const glm::vec3& F) {

        for(int i = 0; i < nbParticles; ++i){
            if( (i%gridWidth) == 0)
                continue;
//            if(i<gridWidth)
//                continue;
            forceArray[i] += F;
        }

//        forceArray[nbParticles - gridWidth] = glm::vec3(0);
//        forceArray[nbParticles-1] = glm::vec3(0);

    }

    void applySphereCollision(const SphereHandler& sphereHandler, float multiplier) {

        for(int i = 0; i < nbParticles; ++i){
            if( (i%gridWidth) == 0)
                continue;

            for(size_t j = 0; j < sphereHandler.positions.size(); ++j){
                float dist = glm::distance(sphereHandler.positions[j], positionArray[i]);
                if(dist < sphereHandler.radius[j]){
//                    DLOG(INFO) << "COLLISION";
//                    DLOG(INFO) << dist;
//                    DLOG(INFO) << sphereHandler.radius[j];
                    forceArray[i] += sphereCollisionForce(dist, sphereHandler.positions[j], sphereHandler.radius[j], positionArray[i]) * multiplier;
                }
            }
        }

//        forceArray[nbParticles - gridWidth] = glm::vec3(0);
//        forceArray[nbParticles-1] = glm::vec3(0);

    }

    // Met à jour la vitesse et la position de chaque point du drapeau
    // en utilisant un schema de type Leapfrog
    void update(float dt) {
        for(int i = 0; i<nbParticles ; ++i){
            velocityArray[i] += dt * (forceArray[i]/massArray[i]);
            positionArray[i] += dt * velocityArray[i];
            forceArray[i] = glm::vec3(0);
        }
    }
};

int main() {

    SphereHandler sphereHandler;
    sphereHandler.colors = {glm::vec3(1, 0, 0)};
    sphereHandler.positions = {glm::vec3(0, 0, 0)};
    sphereHandler.radius = {0.5};
    float sphereCollisionMultiplier = 0.01;

    DLOG(INFO) << sphereHandler.positions.size();
    DLOG(INFO) << sphereHandler.radius.size();


    WindowManager wm(WINDOW_WIDTH, WINDOW_HEIGHT, "Newton was a Geek");
    wm.setFramerate(30);

    // Initialisation de AntTweakBar (pour la GUI)
    TwInit(TW_OPENGL, NULL);
    TwWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    Flag flag(4096.f, 2, 1.5, 32, 16); // Création d'un drapeau
    glm::vec3 G(0.f, -0.08, 0.f); // Gravité

    FlagRenderer3D renderer(flag.gridWidth, flag.gridHeight);

    glm::mat4 projection = glm::perspective(70.f, float(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.1f, 100.f);

    renderer.setProjMatrix(projection);
    TwBar* gui = TwNewBar("Parametres");

    float randomMoveScale = 0.01f;

    atb::addVarRW(gui, ATB_VAR(flag.K0), "step=0.01");
    atb::addVarRW(gui, ATB_VAR(flag.K1), "step=0.01");
    atb::addVarRW(gui, ATB_VAR(flag.K2), "step=0.01");

    atb::addVarRW(gui, ATB_VAR(flag.V0), "step=0.01");
    atb::addVarRW(gui, ATB_VAR(flag.V1), "step=0.01");
    atb::addVarRW(gui, ATB_VAR(flag.V2), "step=0.01");
    atb::addVarRW(gui, ATB_VAR(sphereCollisionMultiplier), "step=0.01");
    atb::addVarRW(gui, ATB_VAR(sphereHandler.radius[0]), "step=0.01");
    atb::addVarRW(gui, ATB_VAR(sphereHandler.positions[0].x), "step=0.01");
    atb::addVarRW(gui, ATB_VAR(sphereHandler.positions[0].y), "step=0.01");
    atb::addVarRW(gui, ATB_VAR(sphereHandler.positions[0].z), "step=0.01");

    atb::addButton(gui, "reset", [&]() {
        Flag tmp(4096.f, 2, 1.5, 32, 16); // Création d'un drapeau
        tmp.K0 = flag.K0;
        tmp.K1 = flag.K1;
        tmp.K2 = flag.K2;
        tmp.V0 = flag.V0;
        tmp.V1 = flag.V1;
        tmp.V2 = flag.V2;
        flag = tmp;
    });

    TrackballCamera camera;
    int mouseLastX, mouseLastY;

    // Temps s'écoulant entre chaque frame
    float dt = 0.f;

    Graphics::ShaderProgram debugProgram("../shaders/debug.vert", "", "../shaders/debug.frag");

    Renderer3D renderer3D;
    renderer3D.setProjMatrix(projection);

    bool done = false;
    bool wireframe = true;
    while(!done) {
        wm.startMainLoop();

        // Rendu
        renderer.clear();
        renderer.setViewMatrix(camera.getViewMatrix());
        renderer3D.setViewMatrix(camera.getViewMatrix());
        renderer.drawGrid(flag.positionArray.data(), wireframe);

        debugProgram.updateUniform("MVP", projection * camera.getViewMatrix());

        renderer3D.drawParticles(sphereHandler.positions.size(), sphereHandler.positions.data(), sphereHandler.radius.data(), sphereHandler.colors.data(), 1);
//        octree.draw(debugProgram);
//        octree.drawRecursive(debugProgram);
//        glBindVertexArray(0);
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//        glBindBuffer(GL_ARRAY_BUFFER, 0);

//         Simulation
        if(dt > 0.f) {
            flag.applyExternalForce(G); // Applique la gravité
            flag.applyExternalForce(glm::sphericalRand(0.04f)); // Applique un "vent" de direction aléatoire et de force 0.1 Newtons
            flag.applyInternalForces(dt); // Applique les forces internes
            flag.applySphereCollision(sphereHandler, sphereCollisionMultiplier);
            flag.update(dt); // Mise à jour du système à partir des forces appliquées
        }

        TwDraw();

        // Gestion des evenements
        SDL_Event e;
        while(wm.pollEvent(e)) {
            int handled = TwEventSDL(&e, SDL_MAJOR_VERSION, SDL_MINOR_VERSION);

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
                    if(e.key.keysym.sym == SDLK_ESCAPE) {
                        done = true;
                        break;
                    }
                case SDL_MOUSEBUTTONDOWN:
                    if(e.button.button == SDL_BUTTON_WHEELUP) {
                        camera.moveFront(0.1f);
                    } else if(e.button.button == SDL_BUTTON_WHEELDOWN) {
                        camera.moveFront(-0.1f);
                    }
                    else if(e.button.button == SDL_BUTTON_LEFT) {
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
