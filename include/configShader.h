#pragma once
#include "SDL2/SDL.h"
#include "glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
class Shader
{
private:
  unsigned int modelLoc, viewLoc, projLoc;
  unsigned int shaderProgram;
  unsigned int textureID;
  // Shaders como strings privados (noentendi esto)
  const char *vertexShaderSrc = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aColor;        
        layout (location = 2) in vec2 aTexCoord;
        layout (location = 3) in vec2 aTexCoordOffset;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        out vec3 vertexColor;        
        out vec2 textureCoord;
        out vec2 textureCoordOffset;

        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
            vertexColor = aColor;                    
            textureCoord = aTexCoord;
            textureCoordOffset = aTexCoordOffset;
        }
    )";
  const char *fragmentShaderSrc = R"(
        #version 330 core
        out vec4 FragColor;

        in vec3 vertexColor;         
        in vec2 textureCoord;
        in vec2 textureCoordOffset;

        uniform sampler2D textureBlock;
        uniform float textureSize;

        void main() {
            vec2 tiledCoord = fract(textureCoord);
            vec2 scaledCoord = tiledCoord * textureSize;
            vec2 finalCoord = scaledCoord + textureCoordOffset;
            
            vec4 texColor = texture(textureBlock, finalCoord);
            FragColor = texColor * vec4(vertexColor, 1.0);  // <-- Multiplicar por color
        }
    )";

public:
  Shader();
  ~Shader();
  void use();
  void setModelMatrix(const float *);
  void setViewMatrix(const float *);
  void setProjectionMatrix(const float *);
  unsigned int getTextureID() const
  {
    return textureID;
  }
};
class ChunkBuffer
{
private:
  unsigned int VAO, VBO, EBO;
  int indexCount;

public:
  ChunkBuffer();
  ~ChunkBuffer();
  void render();
  void uploadData(const std::vector<float> &vertices, const std::vector<unsigned int> &indices);
};
