#pragma once
#include "../imgui/backends/imgui_impl_opengl3.h"
#include "../imgui/backends/imgui_impl_sdl2.h"
#include "../imgui/imgui.h"
#include "../include/lineShader.h"
#include "camera.h"
#include "glad.h"
#include <SDL2/SDL.h>
#include <iostream>
using namespace std;
class LineShader;
class Screen {
  private:
    SDL_Window* window;
    SDL_GLContext context;
    bool running;
    Camera camera;
    const Uint8* teclado;
    int mouseX, mouseY;
    int lastMouseX, lastMouseY;
    bool openMenu = false;
    bool gameInPause = false;
    bool rightClicked = false;
    bool leftClicked = false;
    int windowWidth = 1280;
    int windowHeight = 720;
    bool crosshairVisible = true;
    int crosshairSize = 10;
    float crosshairColor[3] = {1.0f, 1.0f, 1.0f};

    LineShader* lineShader;

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
    void renderCrosshair();
    void renderDebugAxes(const glm::mat4& view, const glm::mat4& projection);
    void renderBlockOutline(int x, int y, int z);
    bool wasRightClicked() const {
        return rightClicked;
    }
    bool wasLeftClicked() const {
        return leftClicked;
    }
    void clearLeftClick() {
        leftClicked = false;
    }
    void clearRightClick() {
        rightClicked = false;
    }
};
