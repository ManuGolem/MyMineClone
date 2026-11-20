#include "../include/screen.h"
#include "../include/world.h"
#include <chrono>
#include <cstdlib>
#include <sys/types.h>
bool detectBlock(Screen& screen, World& mundo, vec3& blockSelect, vec3& blockFace, bool& colocar) {
    vec3 pos = screen.getCamera().getPosition();
    vec3 front = screen.getCamera().getFront();
    float x = 0.0;
    bool encontreBloque = false;
    while (!encontreBloque && x < 4) {
        int prevx = round(pos.x + front.x * x);
        int prevy = ceil(pos.y + front.y * x);
        int prevz = round(pos.z + front.z * x);
        x += 0.2f;
        int px = round(pos.x + front.x * x);
        int py = ceil(pos.y + front.y * x);
        int pz = round(pos.z + front.z * x);

        vec3 posibleBloque = {px, py, pz};
        if (mundo.getBlockSafe(posibleBloque.x, posibleBloque.y, posibleBloque.z) != 0) {
            blockFace = {prevx, prevy, prevz};
            blockSelect = posibleBloque;
            encontreBloque = true;
            if (mundo.getBlockSafe(blockFace.x, blockFace.y, blockFace.z) == 0) {
                colocar = true;
            }
        }
    }
    return encontreBloque;
}
int main() {
    srand(time(NULL));
    Screen screen;
    World world;
    int seed = rand();
    world.generateWorldWithPerlin(seed);
    world.startCreationThread();
    Uint32 lastFrame = SDL_GetTicks();
    // Distintos medidores, los dejo en Us o sea mircosegundos.
    double timeInputUs = 0.0;
    double timeDetectUs = 0.0;
    double timeUpdateUs = 0.0; // Todavia no uso esto
    double timeRenderUs = 0.0;
    double timeRenderUIUs = 0.0;
    double timeSwapUs = 0.0;
    double timeClearUs = 0.0;
    int frameCount = 0;
    float fpsTimer = 0.0f;
    while (screen.isRunning()) {
        // Limpiar frame anterior
        auto iClear = std::chrono::high_resolution_clock::now();
        screen.clear();
        auto fClear = std::chrono::high_resolution_clock::now();
        timeClearUs = std::chrono::duration_cast<std::chrono::microseconds>(fClear - iClear).count();
        // FPS y deltatime
        Uint32 currentFrame = SDL_GetTicks();
        float deltaTime = (currentFrame - lastFrame) / 1000.0f;
        lastFrame = currentFrame;
        frameCount++;
        fpsTimer += deltaTime;
        if (fpsTimer >= 1.0f) { // Esto sacar a interfaz despues
            float FPS = frameCount / fpsTimer;
            std::cout << "FPS reales: " << FPS << " | Render: " << (timeRenderUs / 1000.0) << "ms"
                      << " | Input: " << (timeInputUs / 1000.0) << "ms"
                      << " | Detect: " << (timeDetectUs / 1000.0) << "ms"
                      << " | Update: " << (timeUpdateUs / 1000.0) << "ms"
                      << " | UI: " << (timeRenderUIUs / 1000.0) << "ms"
                      << " | Swap: " << (timeSwapUs / 1000.0) << "ms"
                      << " | Clear: " << (timeClearUs / 1000.0) << "ms"
                      << "        \r";
            std::cout.flush();
            frameCount = 0;
            fpsTimer = 0;
        }
        // Inputs

        auto iInput = std::chrono::high_resolution_clock::now();
        screen.poll(deltaTime);
        auto fInput = std::chrono::high_resolution_clock::now();
        timeInputUs = std::chrono::duration_cast<std::chrono::microseconds>(fInput - iInput).count();

        // Detectar el bloque
        auto iDetect = std::chrono::high_resolution_clock::now();
        vec3 blockPos;
        vec3 blockFace;
        bool colocar = false;
        if (detectBlock(screen, world, blockPos, blockFace, colocar)) {
            // Renderizar outline del bloque a mirar
            screen.renderBlockOutline(blockPos.x, blockPos.y, blockPos.z);
            if (screen.wasRightClicked() && colocar) {
                int block = screen.getBlockSelected();
                int blockBase = world.getBlockSafe(blockPos.x, blockPos.y, blockPos.z);
                if (block < 256 && (block != 71 || (block == 71 && (blockBase == 19 || blockBase == 71) && blockFace.y > blockPos.y))) {
                    world.setBlockSafe(blockFace.x, blockFace.y, blockFace.z, block);
                }
            }
            if (screen.wasLeftClicked()) {
                world.setBlockSafe(blockPos.x, blockPos.y, blockPos.z, 0);
            }
        }
        auto fDetect = std::chrono::high_resolution_clock::now();
        timeDetectUs = std::chrono::duration_cast<std::chrono::microseconds>(fDetect - iDetect).count();

        // screen.clearRegenerate();
        screen.clearLeftClick();
        screen.clearRightClick();
        // Update logicas
        // world.update();

        // Render Mundo
        auto iRender = std::chrono::high_resolution_clock::now();
        glm::mat4 view = screen.getCamera().getViewMatrix();
        if (screen.isDebugCamera()) {
            world.render(screen.getCamera().getPosition(), view, screen.getCamera().getProjectionMatrix(), screen.getDebugCamera().getViewMatrix(),
                         screen.getDebugCamera().getProjectionMatrix());
        } else {
            world.render(screen.getCamera().getPosition(), view, screen.getCamera().getProjectionMatrix(), screen.getCamera().getViewMatrix(),
                         screen.getCamera().getProjectionMatrix());
        }
        auto fRender = std::chrono::high_resolution_clock::now();
        timeRenderUs = std::chrono::duration_cast<std::chrono::microseconds>(fRender - iRender).count();

        // Render UI
        auto iUI = std::chrono::high_resolution_clock::now();
        screen.renderUI();
        auto fUI = std::chrono::high_resolution_clock::now();
        timeRenderUIUs = std::chrono::duration_cast<std::chrono::microseconds>(fUI - iUI).count();
        // Cambiar los back/front frames
        auto iSwap = std::chrono::high_resolution_clock::now();
        screen.swap();
        auto fSwap = std::chrono::high_resolution_clock::now();
        timeSwapUs = std::chrono::duration_cast<std::chrono::microseconds>(fSwap - iSwap).count();
    }
    return 0;
}
