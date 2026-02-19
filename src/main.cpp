#include "../include/configShader.h"
#include "../include/screen.h"
#include "../include/world.h"
#include <climits>
int main() {
    Screen screen;
    World world;
    world.generateFlatWorld(16, 16);

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
        if (screen.wasRightClicked()) {
            vec3 pos = screen.getCamera().getPosition();
            vec3 front = screen.getCamera().getFront();
            float x = 0.0;
            bool encontreBloque = false;
            while (!encontreBloque) {
                vec3 posibleBloque = pos + front * x;
                if (world.getBlock(posibleBloque.x, posibleBloque.y, posibleBloque.z).active) {
                    encontreBloque = true;
                    ivec2 posChunk = world.getChunkPos(posibleBloque);
                    Chunk* chunk = world.getChunk(posChunk.x, posChunk.y);
                    Block block;
                    block.active = true;
                    block.type = 2;
                    int offsetx = chunk->getNroChunkX() * 16;
                    int offsetz = chunk->getNroChunkZ() * 16;
                    chunk->setBlock(posibleBloque.x - offsetx, posibleBloque.y + 1, posibleBloque.z - offsetz, block);
                } else {
                    x += 1.0f;
                }
            }
            screen.clearRightClick();
        }
        // Update logicas
        world.update();

        // Matriz de la camara
        glm::mat4 view = screen.getCamera().getViewMatrix();
        // Dibujar mundo
        world.render(screen.getCamera().getPosition(), view);
        // Renderizar menu
        screen.renderMenu();
        screen.renderCrosshair();
        // Cambiar los back/front frames
        screen.swap();
    }
    return 0;
}
