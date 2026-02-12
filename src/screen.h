#pragma once
#include "../include/glad.h"
#include "camera.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_stdinc.h>
#include <iostream>

class Screen {
private:
  SDL_Window *window;
  SDL_GLContext context;
  bool running;
  Camera camera;
  const Uint8 *teclado;
  bool mouseCaptured = false;
  int mouseX, mouseY;
  int lastMouseX, lastMouseY; // Pos anterior

public:
  Screen() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    window = SDL_CreateWindow("Prueba", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, 640, 480,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    context = SDL_GL_CreateContext(window);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
      std::cout << "GLAD failed\n";
    }

    glEnable(GL_DEPTH_TEST);
    teclado = SDL_GetKeyboardState(NULL);
    toggleMouse();
    SDL_GetMouseState(&lastMouseX, &lastMouseY);
    running = true;
  }
  Camera &getCamera() { return camera; }
  bool isRunning() { return running; }
  void toggleMouse() {
    mouseCaptured = !mouseCaptured;
    SDL_SetRelativeMouseMode(mouseCaptured ? SDL_TRUE : SDL_FALSE);
  }
  void poll(float deltaTime) {
    // Aca deberia expandir esto para manejar todos los inputs
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        running = false;
      }
      if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_ESCAPE) {
          if (mouseCaptured) {
            toggleMouse();
          } else {
            running = false; // Salir con escape
          }
        }
      }
      if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (!mouseCaptured) {
          toggleMouse(); // Captura el mouse con click
        }
      }
    }
    if (mouseCaptured) {
      SDL_GetMouseState(&mouseX, &mouseY);
      // Calcular offset desde la ultima Pos
      float xoffset = (float)(mouseX - lastMouseX);
      float yoffset = (float)(-mouseY + lastMouseY); // Invertir esto maybe
      if (xoffset != 0 || yoffset != 0) {
        camera.processMouse(xoffset, yoffset);
      }
      lastMouseX = mouseX;
      lastMouseY = mouseY;
    }
    float velocidad = 5.0f * deltaTime;
    if (teclado[SDL_SCANCODE_W])
      camera.moveForward(velocidad);
    if (teclado[SDL_SCANCODE_S])
      camera.moveBackward(velocidad);
    if (teclado[SDL_SCANCODE_A])
      camera.moveLeft(velocidad);
    if (teclado[SDL_SCANCODE_D])
      camera.moveRight(velocidad);
    if (teclado[SDL_SCANCODE_SPACE])
      camera.moveUp(velocidad);
    if (teclado[SDL_SCANCODE_LCTRL])
      camera.moveDown(velocidad);
  }

  void clear() {
    //"Limpia" el back frame, borra todo lo que tenga dibujado.
    glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void swap() {
    // Cambio entre el front frame y el back frame
    SDL_GL_SwapWindow(window);
  }

  ~Screen() {
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
  }
};
