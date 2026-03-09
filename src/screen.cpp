#include "../include/screen.h"
#include "../include/blocksRegistry.h"
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
    uiShader = new UIShader();
    makeClickeableAreas(windowWidth, windowHeight);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui::GetStyle().WindowRounding = 0.0f;
    ImGui::GetStyle().ChildRounding = 0.0f;
    ImGui::GetStyle().FrameRounding = 0.0f;
    ImGui::GetStyle().GrabRounding = 0.0f;
    ImGui::GetStyle().PopupRounding = 0.0f;
    ImGui::GetStyle().ScrollbarRounding = 0.0f;
    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init("#version 130");
    glEnable(GL_DEPTH_TEST);
    teclado = SDL_GetKeyboardState(NULL);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    running = true;
    openMenu = false;
    resize();
}
void Screen::resize() {
    camera.setAspectRatio(windowWidth, windowHeight);
}

Camera& Screen::getCamera() {
    return camera;
}

bool Screen::isRunning() {
    return running;
}
void Screen::makeClickeableAreas(int width, int height) {
    // TabItems;
    float itemTabWidth = 194.0f * 2.0f;
    float itemTabHeight = 136.0f * 2.0f;
    float posX = (width - itemTabWidth) / 2.0f;
    float posY = (height - itemTabHeight) / 2.0f;
    float posYTop = posY + itemTabHeight - 8;
    float tabTopHeight = 60;
    float tabTopWidth = 52.0f;
    vector<float> pos = {0.0f,
                         2.0f + tabTopWidth,
                         2 * (2.0f + tabTopWidth),
                         3 * (2.0f + tabTopWidth),
                         4 * (2.0f + tabTopWidth),
                         5 * (2.0f + tabTopWidth) + 12.0f,
                         itemTabWidth - tabTopWidth};
    for (int i = 0; i < 7; i++) {
        elemClickeable n;
        n.x1 = posX + pos[i];
        n.x2 = posX + pos[i] + tabTopWidth;
        n.y1 = posYTop;
        n.y2 = posYTop + tabTopHeight;
        tabTopItemsClickeables.push_back(n);
    }
    float posYBot = posY - tabTopHeight + 2;
    for (int i = 7; i < 14; i++) {
        if (i == 12)
            continue;
        elemClickeable n;
        n.x1 = posX + pos[i - 7];
        n.x2 = posX + pos[i - 7] + tabTopWidth;
        n.y1 = posYBot;
        n.y2 = posYBot + tabTopHeight;
        tabTopItemsClickeables.push_back(n);
    }
    // HotbarItems
    int posXhotbar = posX + 18;
    int posYhotbar = posY + 16;
    for (int i = 0; i < 9; i++) {
        elemClickeable n;
        n.x1 = posXhotbar + 36 * i;
        n.x2 = posXhotbar + 32 + 36 * i;
        n.y1 = posYhotbar;
        n.y2 = posYhotbar + 32;
        hotbarItemsClickeables.push_back(n);
    }
    // tabItem (la cuadricula completa)
    int posYtabItem = posY + 60;
    tabItemClickeable.x1 = posX;
    tabItemClickeable.x2 = posX + 324;
    tabItemClickeable.y1 = posYtabItem;
    tabItemClickeable.y2 = posYtabItem + 178;
    //  tab items (cada item)
    int posYItemInv = posY + 204;
    int posXItemInv = posXhotbar;
    int slotSize = 32;
    int j = 0;
    for (int i = 0; i < 45; i++) {
        elemClickeable n;
        n.x1 = posXItemInv + j * 36;
        n.x2 = n.x1 + slotSize;
        n.y1 = posYItemInv;
        n.y2 = n.y1 + slotSize;
        if (j == 8) {
            j = 0;
            posYItemInv -= 36;
        } else {
            j++;
        }
        invItemsClickeables.push_back(n);
    }
}
int Screen::isInvItemClicked(int x, int y) {
    for (int i = 0; i < invItemsClickeables.size(); i++) {
        if (invItemsClickeables[i].isClickIn(x, y)) {
            return i;
        }
    }
    return -1;
}
int Screen::isTabTopClicked(int x, int y) {
    for (int i = 0; i < tabTopItemsClickeables.size(); i++) {
        if (tabTopItemsClickeables[i].isClickIn(x, y)) {
            if (i != 12)
                return i + 1;
            else
                return i + 2;
        }
    }
    return -1;
}
int Screen::isHotbarItemClicked(int x, int y) {
    for (int i = 0; i < hotbarItemsClickeables.size(); i++) {
        if (hotbarItemsClickeables[i].isClickIn(x, y)) {
            return i;
        }
    }
    return -1;
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
                if (inventoryOpen)
                    inventoryOpen = false;
                else
                    openMenu = !openMenu;
                SDL_SetRelativeMouseMode(openMenu ? SDL_FALSE : SDL_TRUE);
            }
            if (e.key.keysym.sym == SDLK_e) {
                if (!openMenu) {
                    inventoryOpen = !inventoryOpen;
                    SDL_SetRelativeMouseMode(inventoryOpen ? SDL_FALSE : SDL_TRUE);
                }
            }
        }
        // Mouse
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            if (!openMenu && !debugMode && !inventoryOpen) {
                SDL_SetRelativeMouseMode(SDL_TRUE);

                if (e.button.button == SDL_BUTTON_RIGHT) {
                    rightClicked = true;
                }
                if (e.button.button == SDL_BUTTON_LEFT) {
                    leftClicked = true;
                }
            }
            if (inventoryOpen) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    int tabClicked = isTabTopClicked(e.button.x, windowHeight - e.button.y);
                    slotClicked = isHotbarItemClicked(e.button.x, windowHeight - e.button.y);
                    int itemInvClicked = isInvItemClicked(e.button.x, windowHeight - e.button.y);
                    if (slotClicked != -1) {
                        if (itemClicked == 0) {
                            itemClicked = blocksInHotbar[slotClicked];
                            blocksInHotbar[slotClicked] = 0;
                        } else {
                            int swap = itemClicked;
                            itemClicked = blocksInHotbar[slotClicked];
                            blocksInHotbar[slotClicked] = swap;
                        }
                    } else {
                        if (itemClicked != 0) {
                            if (tabItemClickeable.isClickIn(e.button.x, windowHeight - e.button.y)) {
                                itemClicked = 0;
                            }
                        }
                    }
                    if (itemInvClicked != -1) {
                        if (tabSelected != 13) {
                            Category categoria = BlockRegistry::getCategory(tabSelected);
                            vector<int> elems = BlockRegistry::get(categoria);
                            if (itemInvClicked < elems.size()) {
                                itemClicked = elems[itemInvClicked];
                            }
                        }
                    }

                    if (tabClicked != -1)
                        tabSelected = tabClicked;
                }
            }
        }
        if (e.type == SDL_MOUSEWHEEL) {
            if (e.wheel.y > 0) {
                hotbarNumSelected--;
                if (hotbarNumSelected < 1)
                    hotbarNumSelected = 9;
            } else if (e.wheel.y < 0) {
                hotbarNumSelected++;
                if (hotbarNumSelected > 9)
                    hotbarNumSelected = 1;
            }
        }
    }
    if (teclado[SDL_SCANCODE_P]) {
        debugMode = true;
        debugCamera = camera;
    }
    int rel_x, rel_y;
    SDL_GetRelativeMouseState(&rel_x, &rel_y);
    if (!openMenu && !inventoryOpen) {
        if (debugMode) {
            if (rel_x != 0 || rel_y != 0) {
                debugCamera.processMouse((float)rel_x, (float)-rel_y);
            }
            float velocidad = 20.0f * deltaTime;
            if (teclado[SDL_SCANCODE_RCTRL])
                velocidad = 130.0f * deltaTime;

            if (teclado[SDL_SCANCODE_UP])
                debugCamera.moveForward(velocidad);
            if (teclado[SDL_SCANCODE_DOWN])
                debugCamera.moveBackward(velocidad);
            if (teclado[SDL_SCANCODE_LEFT])
                debugCamera.moveLeft(velocidad);
            if (teclado[SDL_SCANCODE_RIGHT])
                debugCamera.moveRight(velocidad);
            if (teclado[SDL_SCANCODE_U])
                debugCamera.moveUp(velocidad);
            if (teclado[SDL_SCANCODE_RSHIFT])
                debugCamera.moveDown(velocidad);
        } else {
            if (rel_x != 0 || rel_y != 0) {
                camera.processMouse((float)rel_x, (float)-rel_y);
            }
        }
        float velocidad = 4.317f * deltaTime;
        if (teclado[SDL_SCANCODE_LCTRL])
            velocidad = 100.0f * deltaTime;
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
        for (int i = SDL_SCANCODE_1; i <= SDL_SCANCODE_9; i++) {
            if (teclado[i]) {
                hotbarNumSelected = i - SDL_SCANCODE_1 + 1;
                break;
            }
        }
    }
    if (inventoryOpen) {
        // Crear atajos para mover items a la hotbar
    }
}

void Screen::clear() {
    glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
Block Screen::getBlockSelected() {
    Block block;
    block.type = blocksInHotbar[hotbarNumSelected - 1];
    block.active = block.type != 0;
    return block;
}
void Screen::swap() {
    SDL_GL_SwapWindow(window);
}
void Screen::renderUI() {
    GLint previousProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);
    if (debugMode) {
        return;
    }
    // Guardar estado del depth test
    GLboolean depthTest;
    glGetBooleanv(GL_DEPTH_TEST, &depthTest);
    glDisable(GL_DEPTH_TEST);
    if (crosshairVisible) {
        // Render Crosshair
        uiShader->drawCrosshair(windowWidth, windowHeight, crosshairSize, crosshairColor[0], crosshairColor[1], crosshairColor[2]);
    }
    if (hotbarVisible) {
        // Render hotbar
        uiShader->drawHotbar(windowWidth, windowHeight, hotbarNumSelected, blocksInHotbar);
    }
    if (inventoryOpen) {
        uiShader->drawCreativeInventory(windowWidth, windowHeight, itemsInInventory, tabSelected, blocksInHotbar);
        if (itemClicked != 0) {
            SDL_GetMouseState(&mouseX, &mouseY);
            uiShader->drawBlockClicked(itemClicked, mouseX, windowHeight - mouseY, windowWidth, windowHeight);
        }
    }
    // Restaurar depth test
    if (depthTest)
        glEnable(GL_DEPTH_TEST);
    if (previousProgram != 0 && previousProgram != (GLint)uiShader->getProgram()) {
        glUseProgram(previousProgram);
    }
}
void Screen::renderDebugAxes(const glm::mat4& view, const glm::mat4& projection) {
    // Guardar shader activo
    GLint previousProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);

    // Dibujar ejes usando UIShader
    uiShader->drawDebugAxes(view, projection);

    // Restaurar shader anterior
    if (previousProgram != 0 && previousProgram != (GLint)uiShader->getProgram()) {
        glUseProgram(previousProgram);
    }
}
void Screen::renderBlockOutline(int x, int y, int z) {
    // Guardar shader activo
    GLint previousProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);
    // Dibujar outline usando UIShader
    uiShader->drawOutline(x, y, z, camera.getViewMatrix(), camera.getProjectionMatrix());
    // Restaurar shader anterior
    if (previousProgram != 0 && previousProgram != (GLint)uiShader->getProgram()) {
        glUseProgram(previousProgram);
    }
}
void Screen::renderMenu() { // Todo era para probar la biblioteca, la tengo que eliminar ya que prefiero implementar de 0 las funciones que me dan imgui.
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

    ImGui::Begin("PAUSA", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

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
        // openMenu = false;
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
Screen::~Screen() {
    delete uiShader;
    // Limpiar ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
