#include "../include/screen.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_scancode.h>

Screen::Screen() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    window = SDL_CreateWindow("Prueba", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    context = SDL_GL_CreateContext(window);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cout << "GLAD failed\n";
    }
    lineShader = new LineShader();
    // ===== INICIALIZAR IMGUI =====
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Configurar estilo Minecraft (oscuro)
    ImGui::StyleColorsDark();
    ImGui::GetStyle().WindowRounding = 0.0f;
    ImGui::GetStyle().ChildRounding = 0.0f;
    ImGui::GetStyle().FrameRounding = 0.0f;
    ImGui::GetStyle().GrabRounding = 0.0f;
    ImGui::GetStyle().PopupRounding = 0.0f;
    ImGui::GetStyle().ScrollbarRounding = 0.0f;

    // Inicializar backends de ImGui
    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init("#version 130");
    // =============================
    glEnable(GL_DEPTH_TEST);
    teclado = SDL_GetKeyboardState(NULL);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_GetMouseState(&lastMouseX, &lastMouseY);
    running = true;
    openMenu = false;
    resize();
}
void Screen::resize() {
    camera.setAspectRatio(windowWidth, windowHeight);
}
Screen::~Screen() {
    // Limpiar ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

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

void Screen::poll(float deltaTime) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL2_ProcessEvent(&e);
        if (e.type == SDL_QUIT) {
            running = false;
        }
        // Teclas
        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                openMenu = !openMenu;
                SDL_SetRelativeMouseMode(openMenu ? SDL_FALSE : SDL_TRUE);
            }
        }
        // Mouse
        if (e.type == SDL_MOUSEBUTTONDOWN && !openMenu) {
            SDL_SetRelativeMouseMode(SDL_TRUE);

            if (e.button.button == SDL_BUTTON_RIGHT) {
                rightClicked = true;
            }
            if (e.button.button == SDL_BUTTON_LEFT) {
                leftClicked = true;
            }
        }
    }
    int rel_x, rel_y;
    SDL_GetRelativeMouseState(&rel_x, &rel_y);
    if (!openMenu) {
        if (rel_x != 0 || rel_y != 0) {
            camera.processMouse((float)rel_x, (float)-rel_y);
        }
    }
    if (!openMenu) {
        float velocidad = 5.0f * deltaTime;
        if (teclado[SDL_SCANCODE_LCTRL])
            velocidad = 10.0f * deltaTime;
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
        if (teclado[SDL_SCANCODE_LSHIFT])
            camera.moveDown(velocidad);
    }
}

void Screen::clear() {
    glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Screen::swap() {
    SDL_GL_SwapWindow(window);
}
void Screen::renderCrosshair() {
    if (!crosshairVisible) {
        return;
    }
    // Guardar estado del depth test
    GLboolean depthTest;
    glGetBooleanv(GL_DEPTH_TEST, &depthTest);
    glDisable(GL_DEPTH_TEST);
    // Dibujar crosshair usando lineShader
    lineShader->drawCrosshair(windowWidth, windowHeight, crosshairSize, crosshairColor[0], crosshairColor[1], crosshairColor[2]);

    // Restaurar depth test
    if (depthTest)
        glEnable(GL_DEPTH_TEST);
}
void Screen::renderDebugAxes(const glm::mat4& view, const glm::mat4& projection) {
    // Guardar shader activo
    GLint previousProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);

    // Dibujar ejes usando lineShader
    lineShader->drawDebugAxes(view, projection);

    // Restaurar shader anterior
    if (previousProgram != 0 && previousProgram != (GLint)lineShader->getProgram()) {
        glUseProgram(previousProgram);
    }
}
void Screen::renderBlockOutline(int x, int y, int z) {
    // Guardar shader activo
    GLint previousProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);
    // Dibujar outline usando lineShader
    lineShader->drawOutline(x, y, z, camera.getViewMatrix(), camera.getProjectionMatrix());
    // Restaurar shader anterior
    if (previousProgram != 0 && previousProgram != (GLint)lineShader->getProgram()) {
        glUseProgram(previousProgram);
    }
}
void Screen::renderMenu() {
    if (!openMenu)
        return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    int display_w, display_h;
    SDL_GetWindowSize(window, &display_w, &display_h);

    // Configurar ventana del menú (centrada)
    ImGui::SetNextWindowPos(ImVec2(display_w * 0.5f, display_h * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 500));

    // Estilo Minecraft
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));

    ImGui::Begin("PAUSA", &openMenu, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    // Título
    ImGui::SetWindowFontScale(2.0f);
    ImGui::Text("MENU DE PAUSA");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Dummy(ImVec2(0.0f, 30.0f)); // Espaciado

    // Botones
    float anchoBoton = 250.0f;
    float alturaBoton = 50.0f;

    // Centrar botones
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - anchoBoton) * 0.5f);
    if (ImGui::Button("Continuar", ImVec2(anchoBoton, alturaBoton))) {
        openMenu = false;
        // juegoEnPausa = false;
        SDL_SetRelativeMouseMode(SDL_TRUE); // Volver a capturar mouse
    }

    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - anchoBoton) * 0.5f);
    if (ImGui::Button("Configuracion", ImVec2(anchoBoton, alturaBoton))) {
        // Aquí puedes abrir otro menú de configuración
        // Por ahora solo mostraremos un mensaje
        std::cout << "Abrir configuración" << std::endl;
    }

    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - anchoBoton) * 0.5f);
    if (ImGui::Button("Salir al menu principal", ImVec2(anchoBoton, alturaBoton))) {
        // Aquí pondrías la lógica para volver al menú principal
        std::cout << "Volver al menú principal" << std::endl;
    }

    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - anchoBoton) * 0.5f);
    if (ImGui::Button("Salir del juego", ImVec2(anchoBoton, alturaBoton))) {
        running = false; // Cerrar el juego
    }

    // Mostrar información adicional
    ImGui::Dummy(ImVec2(0.0f, 30.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    ImGui::Text("Posicion: %.1f, %.1f, %.1f", camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);

    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    ImGui::End();

    // Restaurar estilos
    ImGui::PopStyleColor(5);

    // Renderizar ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
