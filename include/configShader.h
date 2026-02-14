#pragma once
#include "SDL2/SDL.h"
#include "glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
class Config {
  private:
    unsigned int VAO, VBO, shader;
    unsigned int modelLoc, viewLoc, projLoc;
    unsigned int EBO;
    int cantIndex = 0;
    int cantVertex = 0;
    // Shaders como strings privados (noentendi esto)
    const char* vertexShaderSrc = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
    
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
    
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";
    const char* fragmentShaderSrc = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(0.3, 0.8, 0.2, 1.0);
        }
    )";

  public:
    Config();
    void cargarIndices(unsigned int*, int);
    void cargarVertices(float*, int);
    void dibujarBack();
    void setModelMatrix(const float*);
    void setViewMatrix(const float*);
    void setProjectionMatrix(const float*);
    ~Config();
};
