#include "../include/configShader.h"
#include "../include/screen.h"
#include "../include/world.h"
bool detectBlock(Screen &screen, World &mundo, vec3 &blockSelect,
                 vec3 &blockFace, bool &colocar) {
    vec3 pos = screen.getCamera().getPosition();
    vec3 front = screen.getCamera().getFront();
    float x = 0.0;
    bool encontreBloque = false;
    while (!encontreBloque && x < 4) {
        int prevx = round(pos.x + front.x * x);
        int prevy = ceil(pos.y + front.y * x);
        int prevz = round(pos.z + front.z * x);
        x += 0.1f;
        int px = round(pos.x + front.x * x);
        int py = ceil(pos.y + front.y * x);
        int pz = round(pos.z + front.z * x);

        vec3 posibleBloque = {px, py, pz};
        if (mundo
                .getBlockSafe(posibleBloque.x, posibleBloque.y, posibleBloque.z)
                .active) {
            blockFace = {prevx, prevy, prevz};
            blockSelect = posibleBloque;
            encontreBloque = true;
            if (!mundo.getBlockSafe(blockFace.x, blockFace.y, blockFace.z)
                     .active) {
                colocar = true;
            }
        }
    }
    return encontreBloque;
}
int main() {
    Screen screen;
    World world;
    world.generateWorldWithPerlin(160, 160);
    world.startCreationThread();
    Uint32 lastFrame = SDL_GetTicks();
    int frameCount = 0;
    float fpsTimer = 0.0f;
    while (screen.isRunning()) {
        // Limpiar frame anterior
        screen.clear();
        // FPS y deltatime
        Uint32 currentFrame = SDL_GetTicks();
        float deltaTime = (currentFrame - lastFrame) / 1000.0f;
        lastFrame = currentFrame;
        frameCount++;
        fpsTimer += deltaTime;
        if (fpsTimer >= 1.0f) {
            float FPS = frameCount / fpsTimer;
            std::cout << "FPS: " << FPS << "\r";
            std::cout.flush();
            frameCount = 0;
            fpsTimer = 0;
        }
        // Inputs
        screen.poll(deltaTime);
        vec3 blockPos;
        vec3 blockFace;
        bool colocar = false;
        if (detectBlock(screen, world, blockPos, blockFace, colocar)) {
            //   // Renderizar outline del bloque a mirar
            screen.renderBlockOutline(blockPos.x, blockPos.y, blockPos.z);
            if (screen.wasRightClicked() && colocar) {
                Block block;
                block.active = true;
                block.type = 2;
                world.setBlockSafe(blockFace.x, blockFace.y, blockFace.z,
                                   block);
            }
            if (screen.wasLeftClicked()) {
                Block block;
                block.active = false;
                block.type = 0;
                world.setBlockSafe(blockPos.x, blockPos.y, blockPos.z, block);
            }
        }

        if (screen.getRegenerate()) {
            world.deleteWorld();
            world.generateWorldWithPerlin(160, 160);
        }
        screen.clearRegenerate();
        screen.clearLeftClick();
        screen.clearRightClick();
        // Update logicas
        world.update();

        // Matriz de la camara
        glm::mat4 view = screen.getCamera().getViewMatrix();
        // Dibujar mundo
        if (screen.isDebugMode()) {
            world.render(screen.getCamera().getPosition(), view,
                         screen.getCamera().getProjectionMatrix(),
                         screen.getDebugCamera().getViewMatrix(),
                         screen.getDebugCamera().getProjectionMatrix());
        } else {
            world.render(screen.getCamera().getPosition(), view,
                         screen.getCamera().getProjectionMatrix(),
                         screen.getCamera().getViewMatrix(),
                         screen.getCamera().getProjectionMatrix());
        }
        // Renderizar menu
        screen.renderMenu();
        screen.renderCrosshair();
        // Cambiar los back/front frames
        screen.swap();
    }
    return 0;
}
