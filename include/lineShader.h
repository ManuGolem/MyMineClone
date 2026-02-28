#pragma once
#include "SDL2/SDL.h"
#include "glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
using namespace std;
class LineShader {
  private:
    // Shader para lineas
    unsigned int shaderProgram;
    unsigned int modelLoc, viewLoc, projLoc;
    unsigned int colorLoc;
    static unsigned int axesVAO;
    static unsigned int axesVBO;
    static unsigned int outlinesVAO;
    static unsigned int outlinesVBO;
    static unsigned int crosshairVAO;
    static unsigned int crosshairVBO;
    // Shader para UI
    unsigned int uiShaderProgram;
    unsigned int uiModelLoc, uiViewLoc, uiProjLoc;
    unsigned int uiTextureLoc, uiColorLoc;
    unsigned int uiVAO, uiVBO, uiEBO;
    // Textura de la hotbar
    unsigned int hotbarTextureID;
    unsigned int selectorTextureID;
    // Texturas Icono
    unordered_map<int, unsigned int> iconTexturesID;
    // Textura creativeInventory
    unsigned int tabItemsTextureID;
    unsigned int tabTopUnselectedTextureID;
    unsigned int tabTopSelectedRightTextureID;
    unsigned int tabTopSelectedLeftTextureID;
    unsigned int tabTopSelectedMidTextureID;
    unsigned int scrollerTextureID;
    unsigned int scrollerDisabledTextureID;

  public:
    LineShader();

    void setModelMatrix(const float *matrix) {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, matrix);
    }

    void setViewMatrix(const float *matrix) {
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, matrix);
    }

    void setProjectionMatrix(const float *matrix) {
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, matrix);
    }
    void drawDebugAxes(const glm::mat4 &view, const glm::mat4 &projection);
    void drawOutline(int x, int y, int z, const glm::mat4 &view,
                     const glm::mat4 &projection);
    void drawCrosshair(int screenWidth, int screenHeight, int size = 10,
                       float r = 1.0f, float g = 1.0f, float b = 1.0f);
    void drawHotbar(int windowWidth, int windowHeight, int hotbarNumSelected,
                    vector<int> blockTypes);
    void drawCreativeInventory(int windowWidth, int windowHeight,
                               vector<int> itemsInInventory,
                               int tabTopSelected);
    void loadHotbarTexture(const char *path, unsigned int &textureID);
    void setColor(float r, float g, float b) { glUniform3f(colorLoc, r, g, b); }
    unsigned int getProgram() const { return shaderProgram; }
    ~LineShader();
};
