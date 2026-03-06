#include "../include/UIShader.h"
#include "../include/stb_image.h"
unsigned int UIShader::axesVAO = 0;
unsigned int UIShader::axesVBO = 0;
unsigned int UIShader::outlinesVAO = 0;
unsigned int UIShader::outlinesVBO = 0;
unsigned int UIShader::crosshairVAO = 0;
unsigned int UIShader::crosshairVBO = 0;
UIShader::UIShader() {
    const char* uiVertexSrc = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    
    out vec2 TexCoord;
    
    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }
    )";

    const char* uiFragmentSrc = R"(
    #version 330 core
    out vec4 FragColor;
    
    in vec2 TexCoord;
    
    uniform sampler2D uiTexture;
    uniform vec4 color;
    
    void main() {
        FragColor = texture(uiTexture, TexCoord) * color;
    }
    )";
    const char* vertexSrc = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            
            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;
            
            void main() {
                gl_Position = projection * view * model * vec4(aPos, 1.0);
            }
        )";

    const char* fragmentSrc = R"(
            #version 330 core
            out vec4 FragColor;
            
            uniform vec3 lineColor;
            
            void main() {
                FragColor = vec4(lineColor, 1.0);
            }
        )";

    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexSrc, NULL);
    glCompileShader(vs);

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentSrc, NULL);
    glCompileShader(fs);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);

    glDeleteShader(vs);
    glDeleteShader(fs);

    // Obtener ubicaciones de uniforms
    modelLoc = glGetUniformLocation(shaderProgram, "model");
    viewLoc = glGetUniformLocation(shaderProgram, "view");
    projLoc = glGetUniformLocation(shaderProgram, "projection");
    colorLoc = glGetUniformLocation(shaderProgram, "lineColor");

    float vertices[] = {// Eje X (rojo)
                        0.0f, 0.0f, 0.0f, 100.0f, 0.0f, 0.0f,

                        // Eje Y (verde)
                        0.0f, 0.0f, 0.0f, 0.0f, 100.0f, 0.0f,

                        // Eje Z (azul)
                        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 100.0f};
    glGenVertexArrays(1, &axesVAO);
    glGenBuffers(1, &axesVBO);

    glBindVertexArray(axesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Outlines
    glGenVertexArrays(1, &outlinesVAO);
    glGenBuffers(1, &outlinesVBO);
    glBindVertexArray(outlinesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, outlinesVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Crosshair
    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);
    glBindVertexArray(crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    // Compilar shader para UI items
    unsigned int uiVS = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(uiVS, 1, &uiVertexSrc, NULL);
    glCompileShader(uiVS);

    unsigned int uiFS = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(uiFS, 1, &uiFragmentSrc, NULL);
    glCompileShader(uiFS);
    uiShaderProgram = glCreateProgram();
    glAttachShader(uiShaderProgram, uiVS);
    glAttachShader(uiShaderProgram, uiFS);
    glLinkProgram(uiShaderProgram);

    glDeleteShader(uiVS);
    glDeleteShader(uiFS);

    // Obtener uniforms del shader UI
    uiModelLoc = glGetUniformLocation(uiShaderProgram, "model");
    uiViewLoc = glGetUniformLocation(uiShaderProgram, "view");
    uiProjLoc = glGetUniformLocation(uiShaderProgram, "projection");
    uiTextureLoc = glGetUniformLocation(uiShaderProgram, "uiTexture");
    uiColorLoc = glGetUniformLocation(uiShaderProgram, "color");
    // Crear VAO/VBO para UI
    float uiVertices[] = {
        // posiciones    // texCoords
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
        1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // bottom-right
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, // top-right
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f  // top-left
    };
    unsigned int uiIndices[] = {0, 1, 2, 2, 3, 0};

    glGenVertexArrays(1, &uiVAO);
    glGenBuffers(1, &uiVBO);
    glGenBuffers(1, &uiEBO);

    glBindVertexArray(uiVAO);
    glBindBuffer(GL_ARRAY_BUFFER, uiVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uiVertices), uiVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uiEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uiIndices), uiIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    // Cargar texturas
    loadTexture("../textures/Hotbar.png", hotbarTextureID);
    loadTexture("../textures/Hotbar_selector.png", selectorTextureID);
    loadTexture("../textures/icons/dirt.png", iconTexturesID[3]);
    loadTexture("../textures/icons/stone.png", iconTexturesID[2]);
    loadTexture("../textures/icons/grass_block.png", iconTexturesID[4]);
    loadTexture("../textures/icons/bricks.png", iconTexturesID[8]);
    loadTexture("../textures/icons/cyan_wool.png", iconTexturesID[210]);
    loadTexture("../textures/icons/bookshelf.png", iconTexturesID[36]);
    loadTexture("../textures/icons/oak_sign.png", iconTexturesID[257]);
    loadTexture("../textures/icons/redstone.png", iconTexturesID[258]);
    loadTexture("../textures/icons/compass.png", iconTexturesID[259]);
    loadTexture("../textures/icons/diamond_pickaxe.png", iconTexturesID[260]);
    loadTexture("../textures/icons/netherite_sword.png", iconTexturesID[261]);
    loadTexture("../textures/icons/golden_apple.png", iconTexturesID[262]);
    loadTexture("../textures/icons/iron_ingot.png", iconTexturesID[263]);
    loadTexture("../textures/icons/creeper_spawn_egg.png", iconTexturesID[264]);
    loadTexture("../textures/icons/chest.png", iconTexturesID[265]);

    loadTexture("../textures/creativeInventory/tab_items.png", tabItemsTextureID);
    loadTexture("../textures/creativeInventory/tab_inventory.png", tabCreativeInventoryTextureID);
    loadTexture("../textures/creativeInventory/tab_item_search.png", tabItemSearchTextureID);
    loadTexture("../textures/creativeInventory/tab_top_unselected.png", tabTopUnselectedTextureID);
    loadTexture("../textures/creativeInventory/tab_top_selected_left.png", tabTopSelectedLeftTextureID);
    loadTexture("../textures/creativeInventory/tab_top_selected_right.png", tabTopSelectedRightTextureID);
    loadTexture("../textures/creativeInventory/tab_bottom_selected_right.png", tabBotSelectedRightTextureID);
    loadTexture("../textures/creativeInventory/tab_bottom_unselected.png", tabBotUnselectedTextureID);
    loadTexture("../textures/creativeInventory/tab_bottom_selected_middle.png", tabBotSelectedMidTextureID);
    loadTexture("../textures/creativeInventory/tab_bottom_selected_left.png", tabBotSelectedLeftTextureID);
    loadTexture("../textures/creativeInventory/tab_top_selected_middle.png", tabTopSelectedMidTextureID);
    loadTexture("../textures/creativeInventory/scroller.png", scrollerTextureID);
    loadTexture("../textures/creativeInventory/scroller_disabled.png", scrollerDisabledTextureID);
}
void UIShader::drawDebugAxes(const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(shaderProgram);
    setViewMatrix(glm::value_ptr(view));
    setProjectionMatrix(glm::value_ptr(projection));

    glm::mat4 model(1.0f);
    setModelMatrix(glm::value_ptr(model));

    glBindVertexArray(axesVAO);
    glDisable(GL_DEPTH_TEST); // Opcional: para ver ejes siempre

    // Eje X (rojo)
    setColor(1.0f, 0.0f, 0.0f);
    glDrawArrays(GL_LINES, 0, 2);

    // Eje Y (verde)
    setColor(0.0f, 1.0f, 0.0f);
    glDrawArrays(GL_LINES, 2, 2);

    // Eje Z (azul)
    setColor(0.0f, 0.0f, 1.0f);
    glDrawArrays(GL_LINES, 4, 2);

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}
void UIShader::drawOutline(int x, int y, int z, const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(shaderProgram);
    setViewMatrix(glm::value_ptr(view));
    setProjectionMatrix(glm::value_ptr(projection));

    glm::mat4 model(1.0f);
    setModelMatrix(glm::value_ptr(model));
    // 12 aristas del cubo (24 vértices)
    float vertices[] = {// Cara inferior
                        x - 0.5f, y - 1.0f, z - 0.5f, x + 0.5f, y - 1.0f, z - 0.5f, x + 0.5f, y - 1.0f, z - 0.5f, x + 0.5f, y - 1.0f, z + 0.5f, x + 0.5f, y - 1.0f,
                        z + 0.5f, x - 0.5f, y - 1.0f, z + 0.5f, x - 0.5f, y - 1.0f, z + 0.5f, x - 0.5f, y - 1.0f, z - 0.5f,

                        // Cara superior
                        x - 0.5f, y + 0.0f, z - 0.5f, x + 0.5f, y + 0.0f, z - 0.5f, x + 0.5f, y + 0.0f, z - 0.5f, x + 0.5f, y + 0.0f, z + 0.5f, x + 0.5f, y + 0.0f,
                        z + 0.5f, x - 0.5f, y + 0.0f, z + 0.5f, x - 0.5f, y + 0.0f, z + 0.5f, x - 0.5f, y + 0.0f, z - 0.5f,

                        // Aristas verticales
                        x - 0.5f, y - 1.0f, z - 0.5f, x - 0.5f, y + 0.0f, z - 0.5f, x + 0.5f, y - 1.0f, z - 0.5f, x + 0.5f, y + 0.0f, z - 0.5f, x - 0.5f, y - 1.0f,
                        z + 0.5f, x - 0.5f, y + 0.0f, z + 0.5f, x + 0.5f, y - 1.0f, z + 0.5f, x + 0.5f, y + 0.0f, z + 0.5f};

    glBindVertexArray(outlinesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, outlinesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glLineWidth(1.0f);
    setColor(0.0f, 0.0f, 0.0f);
    glDrawArrays(GL_LINES, 0, 24);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void UIShader::loadTexture(const char* path, unsigned int& textureID) {
    // Cargar textura de la hotbar completa
    int width, height, channels;
    unsigned char* data = stbi_load(path, &width, &height, &channels, 4);
    if (data) {
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        stbi_image_free(data);
    }
}
void UIShader::drawCreativeInventory(int screenWidth, int screenHeight, vector<int> itemsInInventory, int tabSelected, vector<int> blockInHotbar) {
    glUseProgram(uiShaderProgram);
    glm::mat4 projection = glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight);
    glUniformMatrix4fv(uiProjLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glm::mat4 view(1.0f);
    glUniformMatrix4fv(uiViewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    float iconSize = 32;
    float itemTabWidth = 194.0f * 2.0f;
    float itemTabHeight = 136.0f * 2.0f;
    float posX = (screenWidth - itemTabWidth) / 2.0f;
    float posY = (screenHeight - itemTabHeight) / 2.0f;
    // Dibujar tabUnselected
    float tabTopHeight = 60;
    float tabTopWidth = 52.0f;
    float posYTop = posY + itemTabHeight - 10;
    glBindTexture(GL_TEXTURE_2D, tabTopUnselectedTextureID);
    glUniform1i(uiTextureLoc, 0);
    glUniform4f(uiColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
    glBindVertexArray(uiVAO);
    vector<float> pos = {0.0f,
                         2.0f + tabTopWidth,
                         2 * (2.0f + tabTopWidth),
                         3 * (2.0f + tabTopWidth),
                         4 * (2.0f + tabTopWidth),
                         5 * (2.0f + tabTopWidth) + 12.0f,
                         itemTabWidth - tabTopWidth};
    glm::mat4 model;
    for (int i = 0; i < 7; i++) {
        if (i == tabSelected - 1)
            continue;
        int px = pos[i];
        model = glm::translate(glm::mat4(1.0f), glm::vec3(posX + px, posYTop, 0.0f));
        model = glm::scale(model, glm::vec3(tabTopWidth, tabTopHeight, 1.0f));
        glUniformMatrix4fv(uiModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    glBindTexture(GL_TEXTURE_2D, tabBotUnselectedTextureID);
    float posYBot = posY - tabTopWidth + 2;
    for (int i = 7; i < 14; i++) {
        if (i == tabSelected - 1 || i == 12)
            continue;
        int px = pos[i - 7];
        model = glm::translate(glm::mat4(1.0f), glm::vec3(posX + px, posYBot, 0.0f));
        model = glm::scale(model, glm::vec3(tabTopWidth, tabTopHeight, 1.0f));
        glUniformMatrix4fv(uiModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    // Dibujo tabItem
    if (tabSelected == 7) {
        glBindTexture(GL_TEXTURE_2D, tabItemSearchTextureID);
    } else { // falta cuando se selecciona el inventario
        glBindTexture(GL_TEXTURE_2D, tabItemsTextureID);
    }
    model = glm::translate(glm::mat4(1.0f), glm::vec3(posX, posY, 0.0f));
    model = glm::scale(model, glm::vec3(itemTabWidth, itemTabHeight, 1.0f));
    glUniformMatrix4fv(uiModelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // Dibujo tab selected
    tabTopHeight = 64.0f;
    tabTopWidth = 52.0f;
    float posYTab;
    posYTop = posY + itemTabHeight - 8;
    posYBot = posY - tabTopHeight + 8;
    if (tabSelected == 1) {
        glBindTexture(GL_TEXTURE_2D, tabTopSelectedLeftTextureID);
        posYTab = posYTop;
    } else if (tabSelected == 7) {
        glBindTexture(GL_TEXTURE_2D, tabTopSelectedRightTextureID);
        posYTab = posYTop;
    } else if (tabSelected == 8) {
        glBindTexture(GL_TEXTURE_2D, tabBotSelectedLeftTextureID);
        posYTab = posYBot;
    } else if (tabSelected == 14) {
        glBindTexture(GL_TEXTURE_2D, tabBotSelectedRightTextureID);
        posYTab = posYBot;
    } else if (tabSelected > 8) {
        glBindTexture(GL_TEXTURE_2D, tabBotSelectedMidTextureID);
        posYTab = posYBot;
    } else {
        glBindTexture(GL_TEXTURE_2D, tabTopSelectedMidTextureID);
        posYTab = posYTop;
    }
    int px;
    if (tabSelected > 7) {
        px = pos[tabSelected - 8];
    } else {
        px = pos[tabSelected - 1];
    }
    model = glm::translate(glm::mat4(1.0f), glm::vec3(posX + px, posYTab, 0.0f));
    model = glm::scale(model, glm::vec3(tabTopWidth, tabTopHeight, 1.0f));
    glUniformMatrix4fv(uiModelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // Dibujar iconos en las tabTop
    vector<int> posTextures = {8, 210, 4, 257, 258, 36, 259, 260, 261, 262, 263, 264, 265};
    posYTop += 13;
    for (int i = 0; i < 7; i++) {
        glBindTexture(GL_TEXTURE_2D, iconTexturesID[posTextures[i]]);
        int px = pos[i] + 10.0f;
        model = glm::translate(glm::mat4(1.0f), glm::vec3(posX + px, posYTop, 0.0f));
        model = glm::scale(model, glm::vec3(iconSize, iconSize, 1.0f));
        glUniformMatrix4fv(uiModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    posYBot += 16;
    for (int i = 7; i < 13; i++) {
        glBindTexture(GL_TEXTURE_2D, iconTexturesID[posTextures[i]]);
        int px;
        if (i == 12) {
            px = pos[i - 6] + 10.0f;
        } else if (i == 7) {
            px = pos[i - 7] + 9.0f;
        } else {
            px = pos[i - 7] + 10.0f;
        }
        model = glm::translate(glm::mat4(1.0f), glm::vec3(posX + px, posYBot, 0.0f));
        model = glm::scale(model, glm::vec3(iconSize, iconSize, 1.0f));
        glUniformMatrix4fv(uiModelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    // Draw items de la hotbar en el inv
    float slotSize = 36.0f;
    posY += 15;
    posX += 15;
    for (int i = 0; i < blockInHotbar.size(); i++) {
        int blockType = blockInHotbar[i];
        if (iconTexturesID[blockType] == 0)
            continue;
        glBindTexture(GL_TEXTURE_2D, iconTexturesID[blockType]);
        float slotX = posX + 2 + i * slotSize;
        glm::mat4 iconModel = glm::translate(glm::mat4(1.0f), glm::vec3(slotX, posY, 0.0f));
        iconModel = glm::scale(iconModel, glm::vec3(iconSize, iconSize, 1.0f));
        glUniformMatrix4fv(uiModelLoc, 1, GL_FALSE, glm::value_ptr(iconModel));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    //   Restaurar estado
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}
void UIShader::drawHotbar(int screenWidth, int screenHeight, int selected, vector<int> blockTypes) {
    glUseProgram(uiShaderProgram);

    glm::mat4 projection = glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight);
    glUniformMatrix4fv(uiProjLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glm::mat4 view(1.0f);
    glUniformMatrix4fv(uiViewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // Configurar blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    // Dimensiones
    float hotbarWidth = 364.0f;
    float hotbarHeight = 44.0f;
    float selectorSize = 48.0f;
    int slotWidth = 40;
    float slotHeight = hotbarHeight;
    float posX = (screenWidth - hotbarWidth) / 2.0f;
    float posY = 10.0f;

    // Dibujar hotbar (Fondo)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hotbarTextureID);
    glUniform1i(uiTextureLoc, 0);
    glUniform4f(uiColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(posX, posY, 0.0f));
    model = glm::scale(model, glm::vec3(hotbarWidth, hotbarHeight, 1.0f));
    glUniformMatrix4fv(uiModelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(uiVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // Dibujar selector
    glBindTexture(GL_TEXTURE_2D, selectorTextureID);

    float selectorX = posX + (selected - 1) * slotWidth + 2 - (selectorSize - slotWidth) / 2.0f;
    float selectorY = posY - (selectorSize - hotbarHeight) / 2.0f;

    model = glm::translate(glm::mat4(1.0f), glm::vec3(selectorX, selectorY, 0.0f));
    model = glm::scale(model, glm::vec3(selectorSize, selectorSize, 1.0f));
    glUniformMatrix4fv(uiModelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // Dibujar icono
    float slotY = posY - 1.0f;
    float iconSize = 30.0f;
    for (int i = 0; i < blockTypes.size(); i++) {

        int blockType = blockTypes[i];
        if (iconTexturesID[blockType] == 0)
            continue;

        glBindTexture(GL_TEXTURE_2D, iconTexturesID[blockType]);
        float slotX = posX + 2 + i * slotWidth;

        float centerX = slotX + slotWidth / 2.0f;
        float centerY = slotY + hotbarHeight / 2.0f;

        float iconX = centerX - iconSize / 2.0f;
        float iconY = centerY - iconSize / 2.0f;

        glm::mat4 iconModel = glm::translate(glm::mat4(1.0f), glm::vec3(iconX, iconY, 0.0f));
        iconModel = glm::scale(iconModel, glm::vec3(iconSize, iconSize, 1.0f));

        glUniformMatrix4fv(uiModelLoc, 1, GL_FALSE, glm::value_ptr(iconModel));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    // Restaurar estado
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}
void UIShader::drawCrosshair(int screenWidth, int screenHeight, int size, float r, float g, float b) {
    // Configurar proyección ortográfica para 2D
    glm::mat4 ortho = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f);
    // Usar lineshader
    glUseProgram(shaderProgram);
    setProjectionMatrix(glm::value_ptr(ortho));
    setViewMatrix(glm::value_ptr(glm::mat4(1.0f)));
    setModelMatrix(glm::value_ptr(glm::mat4(1.0f)));
    int centerX = screenWidth / 2;
    int centerY = screenHeight / 2;
    float vertices[] = {// Línea horizontal
                        (float)centerX - size, (float)centerY, 0.0f, (float)centerX + size, (float)centerY, 0.0f,
                        // Línea vertical
                        (float)centerX, (float)centerY - size, 0.0f, (float)centerX, (float)centerY + size, 0.0f,
                        // Punto central
                        (float)centerX, (float)centerY, 0.0f};
    glBindVertexArray(crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    // Dibujar líneas
    setColor(r, g, b);
    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, 4);
}
UIShader::~UIShader() {
    glDeleteProgram(shaderProgram);
    glDeleteProgram(uiShaderProgram);

    glDeleteVertexArrays(1, &axesVAO);
    glDeleteBuffers(1, &axesVBO);
    glDeleteVertexArrays(1, &outlinesVAO);
    glDeleteBuffers(1, &outlinesVBO);
    glDeleteVertexArrays(1, &crosshairVAO);
    glDeleteBuffers(1, &crosshairVBO);
    glDeleteVertexArrays(1, &uiVAO);
    glDeleteBuffers(1, &uiVBO);
    glDeleteBuffers(1, &uiEBO);

    // Eliminar texturas individuales del mapa
    glDeleteTextures(1, &hotbarTextureID);
    glDeleteTextures(1, &selectorTextureID);
    glDeleteTextures(1, &tabItemsTextureID);
    glDeleteTextures(1, &tabTopUnselectedTextureID);
    glDeleteTextures(1, &tabTopSelectedLeftTextureID);
    glDeleteTextures(1, &tabTopSelectedRightTextureID);
    glDeleteTextures(1, &tabTopSelectedMidTextureID);
    glDeleteTextures(1, &scrollerTextureID);
    glDeleteTextures(1, &scrollerDisabledTextureID);

    // ELIMINAR TODAS LAS TEXTURAS DEL MAPA
    for (auto& pair : iconTexturesID) {
        if (pair.second != 0) {
            glDeleteTextures(1, &pair.second);
        }
    }

    // Limpiar el mapa (esto libera la memoria interna del unordered_map)
    iconTexturesID.clear();
}
