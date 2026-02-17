#include "../include/configShader.h"
#include "../include/screen.h"
#include "../include/world.h"
// vertex data

int main() {
    Screen screen;
    World world;

    world.generateFlatWorld(160, 160);

    Uint32 lastFrame = SDL_GetTicks();
    DebugAxes debugAxes;
    debugAxes.inicializar();
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

        world.render(screen.getCamera().getPosition(), view);

        // a borrar
        Chunk::sharedShader->use();
        Chunk::sharedShader->setUseTexture(false);

        // Matriz modelo identidad (ejes en el origen)
        glm::mat4 model = glm::mat4(1.0f);
        Chunk::sharedShader->setModelMatrix(glm::value_ptr(model));
        Chunk::sharedShader->setViewMatrix(glm::value_ptr(view));
        // La proyección ya está seteada en el shader

        // Desactivar culling para ver las líneas desde cualquier lado
        glDisable(GL_CULL_FACE);

        debugAxes.renderizar();

        // Restaurar
        glEnable(GL_CULL_FACE);
        Chunk::sharedShader->setUseTexture(true);

        screen.swap();
    }
    return 0;
}
