#pragma once
#include "../imgui/backends/imgui_impl_opengl3.h"
#include "../imgui/backends/imgui_impl_sdl2.h"
#include "../imgui/imgui.h"
#include "../include/UIShader.h"
#include "camera.h"
#include "chunk.h"
#include "glad.h"
#include <SDL2/SDL.h>
#include <iostream>
using namespace std;
struct elemClickeable {
    int x1;
    int x2;
    int y1;
    int y2;
    bool isClickIn(int x, int y) {
        return (x >= x1 && x <= x2 && y >= y1 && y <= y2);
    }
};
class UIShader;
class Screen {
  private:
    SDL_Window* window;
    SDL_GLContext context;
    bool running;
    Camera camera;
    Camera debugCamera;
    bool debugMode = false;
    bool ceroPressed = false;
    const Uint8* teclado;
    Uint32 mouse;
    int mouseX, mouseY;
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
    vector<int> blocksInHotbar = {2, 4, 3, 5, 6, 7, 8, 17, 18};
    vector<int> itemsInInventory = {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int tabSelected = 1;
    int itemClicked = 0;
    int slotClicked = -1;
    int crosshairSize = 10;
    float crosshairColor[3] = {1.0f, 1.0f, 1.0f};
    UIShader* uiShader;
    // UI interact
    vector<elemClickeable> tabTopItemsClickeables;
    vector<elemClickeable> hotbarItemsClickeables;
    vector<elemClickeable> categoryInvItemsClickeables;
    vector<elemClickeable> invItemsClickeables;
    elemClickeable tabItemClickeable;

  public:
    Screen();
    ~Screen();

    Camera& getCamera();
    bool isRunning();
    void poll(float deltaTime);
    void clear();
    void swap();
    void resize();
    void renderMenu();
    void renderUI();
    int isTabTopClicked(int x, int y);
    int isHotbarItemClicked(int x, int y);
    int isInvItemClicked(int x, int y);
    int isCategoryInvItemClicked(int x, int y);
    void makeClickeableAreas(int width, int height);

    void renderDebugAxes(const glm::mat4& view, const glm::mat4& projection);
    void renderBlockOutline(int x, int y, int z);
    bool wasRightClicked() const {
        return rightClicked;
    }
    int getBlockSelected();
    bool wasLeftClicked() const {
        return leftClicked;
    }
    bool getRegenerate() const {
        return regenerateWorld;
    }
    void clearLeftClick() {
        leftClicked = false;
    }
    void clearRightClick() {
        rightClicked = false;
    }
    void clearRegenerate() {
        regenerateWorld = false;
    }
    Camera& getDebugCamera() {
        return debugCamera;
    }
    bool isDebugMode() const {
        return debugMode;
    }
    void clearDebugMode() {
        debugMode = false;
    }
};
