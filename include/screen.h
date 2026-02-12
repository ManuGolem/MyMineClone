#pragma once
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
    bool mouseCaptured = false;
    int mouseX, mouseY;
    int lastMouseX, lastMouseY; // Pos anterior

  public:
    Screen();
    ~Screen();

    Camera& getCamera();
    bool isRunning();
    void toggleMouse();
    void poll(float deltaTime);
    void clear();
    void swap();
};
