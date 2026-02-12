#include "../include/configShader.h"
#include "../include/screen.h"
// vertex data

int main() {
    Screen screen;
    Config config;
    // VÉRTICES DEL CUBO (8 vértices únicos)
    float vertices[] = {
        0.0f, 0.0f, 0.0f, // 0
        0.5,  0.0f, 0.0f, // 1
        0.5f, 0.5f, 0.0f, // 2
        0.0f, 0.5f, 0.0f, // 3
        0.0f, 0.0f, 0.5f, // 4
        0.5f, 0.0f, 0.5f, // 5
        0.5f, 0.5f, 0.5f, // 6
        0.0f, 0.5f, 0.5f, // 7

    };
    // ÍNDICES (36 para 12 triángulos)
    unsigned int indices[] = {
        4, 5, 6, 4, 6, 7, // Cara frontal
        0, 4, 7, 0, 7, 3, // Cara izquierda
        1, 2, 6, 1, 6, 5, // Cara derecha
        0, 1, 2, 0, 2, 3, // Cara de atras
        0, 1, 5, 0, 5, 4, // Cara de abajo
        3, 2, 6, 3, 6, 7  // Cara de arriba

    };

    config.cargarVertices(vertices, sizeof(vertices));
    config.cargarIndices(indices, sizeof(indices));

    // MATRIZ DE PROYECCIÓN (perspectiva)
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), // FOV
                                            640.0f / 480.0f,     // Aspect ratio
                                            0.1f,                // Near plane
                                            100.0f               // Far plane
    );
    config.setProjectionMatrix(glm::value_ptr(projection));
    glm::mat4 model = glm::mat4(1.0f);
    config.setModelMatrix(glm::value_ptr(model));

    Uint32 lastFrame = SDL_GetTicks();
    while (screen.isRunning()) {
        Uint32 currentFrame = SDL_GetTicks();
        float deltaTime = (currentFrame - lastFrame) / 1000.0f;
        lastFrame = currentFrame;
        screen.poll(deltaTime);
        glm::mat4 view = screen.getCamera().getViewMatrix();
        config.setViewMatrix(glm::value_ptr(view));
        screen.clear();
        config.dibujarBack();
        screen.swap();
    }
    return 0;
}
