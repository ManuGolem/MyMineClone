#include "../include/configShader.h"
#include "../include/screen.h"
#include "../include/world.h"
// vertex data

int main() {
    Screen screen;
    Config shader;
    World world;

    world.generateFlatWorld(32, 1);

    Uint32 lastFrame = SDL_GetTicks();

    int frameCount = 0;
    float fpsTimer = 0.0f;
    while (screen.isRunning()) {
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
        screen.poll(deltaTime);
        screen.clear();
        world.update();
        // Matriz de la camara
        glm::mat4 view = screen.getCamera().getViewMatrix();
        shader.setViewMatrix(glm::value_ptr(view));
        world.render(screen.getCamera().getPosition(), view);
        screen.swap();
    }
    return 0;
}
