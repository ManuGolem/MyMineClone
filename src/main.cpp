#include "configShader.h"
#include "screen.h"
// vertex data

int main() {
  Screen screen;
  Config config;
  // VÉRTICES DEL CUBO (8 vértices únicos)
  float vertices[] = {
      // Posiciones
      -0.5f, -0.5f, 0.5f,  // 0
      0.5f,  -0.5f, 0.5f,  // 1
      0.5f,  0.5f,  0.5f,  // 2
      -0.5f, 0.5f,  0.5f,  // 3
      -0.5f, -0.5f, -0.5f, // 4
      0.5f,  -0.5f, -0.5f, // 5
      0.5f,  0.5f,  -0.5f, // 6
      -0.5f, 0.5f,  -0.5f  // 7
  };

  // ÍNDICES (36 para 12 triángulos)
  unsigned int indices[] = {
      0, 1, 2, 0, 2, 3, // frontal
      4, 6, 5, 4, 7, 6, // trasera
      4, 0, 3, 4, 3, 7, // izquierda
      1, 5, 6, 1, 6, 2, // derecha
      3, 2, 6, 3, 6, 7, // superior
      4, 5, 1, 4, 1, 0  // inferior
  };

  config.cargarVertices(vertices, sizeof(vertices));
  config.cargarIndices(indices,
                       sizeof(indices)); // Necesitas agregar este método

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
