#pragma once
#include "SDL2/SDL.h"
#include "glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
class LineShader {
  private:
    unsigned int shaderProgram;
    unsigned int modelLoc, viewLoc, projLoc;
    unsigned int colorLoc;
    static unsigned int axesVAO;
    static unsigned int axesVBO;

  public:
    LineShader();
    void use() {
        glUseProgram(shaderProgram);
    }

    void setModelMatrix(const float* matrix) {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, matrix);
    }

    void setViewMatrix(const float* matrix) {
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, matrix);
    }

    void setProjectionMatrix(const float* matrix) {
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, matrix);
    }
    void drawDebugAxes(const glm::mat4& view, const glm::mat4& projection);
    void drawOutline(int x, int y, int z);
    void drawCrosshair(int screenWidth, int screenHeight, int size = 10, 
                       float r = 1.0f, float g = 1.0f, float b = 1.0f);
    void setColor(float r, float g, float b) {
        glUniform3f(colorLoc, r, g, b);
    }
    unsigned int getProgram() const { return shaderProgram; }
    ~LineShader();
};
