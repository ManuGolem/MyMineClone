#pragma once
#include "SDL2/SDL.h"
#include "glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
class Shader {
  private:
    unsigned int modelLoc, viewLoc, projLoc;
    unsigned int shaderProgram;
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
    Shader();
    ~Shader();
    void use();
    void setModelMatrix(const float*);
    void setViewMatrix(const float*);
    void setProjectionMatrix(const float*);
};
class ChunkBuffer {
  private:
    unsigned int VAO, VBO, EBO;
    int indexCount;

  public:
    ChunkBuffer();
    ~ChunkBuffer();
    void render();
    void uploadData(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);
};
