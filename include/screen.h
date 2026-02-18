#pragma once
#include "../imgui/backends/imgui_impl_opengl3.h"
#include "../imgui/backends/imgui_impl_sdl2.h"
#include "../imgui/imgui.h"

#include "camera.h"
#include "glad.h"
#include <SDL2/SDL.h>
#include <iostream>

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

  public:
    Screen();
    ~Screen();

    Camera& getCamera();
    bool isRunning();
    void poll(float deltaTime);
    void clear();
    void swap();
    void renderMenu();
    bool wasRightClicked() const {
        return rightClicked;
    }
    void clearRightClick() {
        rightClicked = false;
    }
};
