#pragma once
#include "../imgui/backends/imgui_impl_opengl3.h"
#include "../imgui/backends/imgui_impl_sdl2.h"
#include "../imgui/imgui.h"
#include "../include/lineShader.h"
#include "camera.h"
#include "chunk.h"
#include "glad.h"
#include <SDL2/SDL.h>
#include <iostream>
using namespace std;
class LineShader;
class Screen {
  private:
    SDL_Window *window;
    SDL_GLContext context;
    bool running;
    Camera camera;
    Camera debugCamera;
    bool debugMode = false;
    bool ceroPressed = false;
    const Uint8 *teclado;
    int mouseX, mouseY;
    int lastMouseX, lastMouseY;
    bool openMenu = false;
    bool gameInPause = false;
    bool rightClicked = false;
    bool leftClicked = false;
    bool regenerateWorld = false;
    int windowWidth = 1280;
    int windowHeight = 720;
    bool crosshairVisible = true;
    bool hotbarVisible = true;
    int hotbarNumSelected = 1;
    bool inventoryOpen = false;
    vector<int> blocksInHotbar = {2, 4, 3, 0, 0, 0, 0, 0, 0};
    vector<int> itemsInInventory = {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int tabTopSelected = 2;
    int crosshairSize = 10;
    float crosshairColor[3] = {1.0f, 1.0f, 1.0f};
    LineShader *lineShader;

  public:
    Screen();
    ~Screen();

    Camera &getCamera();
    bool isRunning();
    void poll(float deltaTime);
    void clear();
    void swap();
    void resize();
    void renderMenu();
    void renderUI();
    void renderDebugAxes(const glm::mat4 &view, const glm::mat4 &projection);
    void renderBlockOutline(int x, int y, int z);
    bool wasRightClicked() const { return rightClicked; }
    Block getBlockSelected();
    bool wasLeftClicked() const { return leftClicked; }
    bool getRegenerate() const { return regenerateWorld; }
    void clearLeftClick() { leftClicked = false; }
    void clearRightClick() { rightClicked = false; }
    void clearRegenerate() { regenerateWorld = false; }
    Camera &getDebugCamera() { return debugCamera; }
    bool isDebugMode() const { return debugMode; }
    void clearDebugMode() { debugMode = false; }
};
