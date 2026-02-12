#include "../include/screen.h"

Screen::Screen() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    window = SDL_CreateWindow("Prueba", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
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

Screen::~Screen() {
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

Camera& Screen::getCamera() {
    return camera;
}

bool Screen::isRunning() {
    return running;
}

void Screen::toggleMouse() {
    mouseCaptured = !mouseCaptured;
    SDL_SetRelativeMouseMode(mouseCaptured ? SDL_TRUE : SDL_FALSE);
}

void Screen::poll(float deltaTime) {
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
                    running = false;
                }
            }
        }
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            if (!mouseCaptured) {
                toggleMouse();
            }
        }
    }

    if (mouseCaptured) {
        SDL_GetMouseState(&mouseX, &mouseY);
        float xoffset = (float)(mouseX - lastMouseX);
        float yoffset = (float)(-mouseY + lastMouseY);
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

void Screen::clear() {
    glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Screen::swap() {
    SDL_GL_SwapWindow(window);
}
