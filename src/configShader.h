#pragma once
#include "../include/glad.h"
#include "SDL2/SDL.h"
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
    // Shaders como strings privados
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
    Config() {
        // VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        // VBO
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Configurar shaders
        //  shaders
        unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vertexShaderSrc, NULL);
        glCompileShader(vs);

        unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fragmentShaderSrc, NULL);
        glCompileShader(fs);

        // Crea el shader
        shader = glCreateProgram();
        glAttachShader(shader, vs);
        glAttachShader(shader, fs);
        glLinkProgram(shader);

        glDeleteShader(vs);
        glDeleteShader(fs);
        modelLoc = glGetUniformLocation(shader, "model");
        viewLoc = glGetUniformLocation(shader, "view");
        projLoc = glGetUniformLocation(shader, "projection");
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    }
    void cargarIndices(unsigned int* indices, int size) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
        cantIndex = size / sizeof(unsigned int);
    }
    void cargarVertices(float* vertices, int size) {
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_DYNAMIC_DRAW);
        cantVertex = size / (3 * sizeof(float));
    }
    void dibujarBack() {
        glUseProgram(shader);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, cantIndex, GL_UNSIGNED_INT, 0);
    }
    void setModelMatrix(float* matrix) {
        glUseProgram(shader);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, matrix);
    }

    void setViewMatrix(float* matrix) {
        glUseProgram(shader);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, matrix);
    }

    void setProjectionMatrix(float* matrix) {
        glUseProgram(shader);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, matrix);
    }
    ~Config() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteProgram(shader);
        glDeleteBuffers(1, &EBO);
    }
};
