#include "../include/lineShader.h"
#include "../include/chunk.h"
#include "../include/stb_image.h"
unsigned int LineShader::axesVAO = 0;
unsigned int LineShader::axesVBO = 0;
unsigned int LineShader::outlinesVAO = 0;
unsigned int LineShader::outlinesVBO = 0;
unsigned int LineShader::crosshairVAO = 0;
unsigned int LineShader::crosshairVBO = 0;
LineShader::LineShader() {
    const char *uiVertexSrc = R"(
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

    const char *uiFragmentSrc = R"(
    #version 330 core
    out vec4 FragColor;
    
    in vec2 TexCoord;
    
    uniform sampler2D uiTexture;
    uniform vec4 color;
    
    void main() {
        FragColor = texture(uiTexture, TexCoord) * color;
    }
    )";
    const char *vertexSrc = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            
            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;
            
            void main() {
                gl_Position = projection * view * model * vec4(aPos, 1.0);
            }
        )";

    const char *fragmentSrc = R"(
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    // Outlines
    glGenVertexArrays(1, &outlinesVAO);
    glGenBuffers(1, &outlinesVBO);
    glBindVertexArray(outlinesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, outlinesVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    // Crosshair
    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);
    glBindVertexArray(crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(uiVertices), uiVertices,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uiEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uiIndices), uiIndices,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    // Cargar texturas
    loadHotbarTexture();
}
void LineShader::drawDebugAxes(const glm::mat4 &view,
                               const glm::mat4 &projection) {
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
void LineShader::drawOutline(int x, int y, int z, const glm::mat4 &view,
                             const glm::mat4 &projection) {
    glUseProgram(shaderProgram);
    setViewMatrix(glm::value_ptr(view));
    setProjectionMatrix(glm::value_ptr(projection));

    glm::mat4 model(1.0f);
    setModelMatrix(glm::value_ptr(model));
    // 12 aristas del cubo (24 vértices)
    float vertices[] = {
        // Cara inferior
        x - 0.5f, y - 1.0f, z - 0.5f, x + 0.5f, y - 1.0f, z - 0.5f, x + 0.5f,
        y - 1.0f, z - 0.5f, x + 0.5f, y - 1.0f, z + 0.5f, x + 0.5f, y - 1.0f,
        z + 0.5f, x - 0.5f, y - 1.0f, z + 0.5f, x - 0.5f, y - 1.0f, z + 0.5f,
        x - 0.5f, y - 1.0f, z - 0.5f,

        // Cara superior
        x - 0.5f, y + 0.0f, z - 0.5f, x + 0.5f, y + 0.0f, z - 0.5f, x + 0.5f,
        y + 0.0f, z - 0.5f, x + 0.5f, y + 0.0f, z + 0.5f, x + 0.5f, y + 0.0f,
        z + 0.5f, x - 0.5f, y + 0.0f, z + 0.5f, x - 0.5f, y + 0.0f, z + 0.5f,
        x - 0.5f, y + 0.0f, z - 0.5f,

        // Aristas verticales
        x - 0.5f, y - 1.0f, z - 0.5f, x - 0.5f, y + 0.0f, z - 0.5f, x + 0.5f,
        y - 1.0f, z - 0.5f, x + 0.5f, y + 0.0f, z - 0.5f, x - 0.5f, y - 1.0f,
        z + 0.5f, x - 0.5f, y + 0.0f, z + 0.5f, x + 0.5f, y - 1.0f, z + 0.5f,
        x + 0.5f, y + 0.0f, z + 0.5f};

    glBindVertexArray(outlinesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, outlinesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glLineWidth(1.0f);
    setColor(0.0f, 0.0f, 0.0f);
    glDrawArrays(GL_LINES, 0, 24);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void LineShader::loadHotbarTexture() {
    // Cargar textura de la hotbar completa
    int width, height, channels;
    unsigned char *data =
        stbi_load("../textures/Hotbar.png", &width, &height, &channels, 4);
    if (data) {
        glGenTextures(1, &hotbarTextureID);
        glBindTexture(GL_TEXTURE_2D, hotbarTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        stbi_image_free(data);
    }
    // Cargar textura del selector
    data = stbi_load("../textures/Hotbar_selector.png", &width, &height,
                     &channels, 4);
    if (data) {
        glGenTextures(1, &selectorTextureID);
        glBindTexture(GL_TEXTURE_2D, selectorTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        stbi_image_free(data);
    }

    data =
        stbi_load("../textures/icons/dirt.png", &width, &height, &channels, 4);
    if (data) {
        glGenTextures(1, &iconTexturesID[3]);
        glBindTexture(GL_TEXTURE_2D, iconTexturesID[3]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        stbi_image_free(data);
    }
    data =
        stbi_load("../textures/icons/stone.png", &width, &height, &channels, 4);
    if (data) {
        glGenTextures(1, &iconTexturesID[2]);
        glBindTexture(GL_TEXTURE_2D, iconTexturesID[2]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        stbi_image_free(data);
    }
    data = stbi_load("../textures/icons/grass_block.png", &width, &height,
                     &channels, 4);
    if (data) {
        glGenTextures(1, &iconTexturesID[4]);
        glBindTexture(GL_TEXTURE_2D, iconTexturesID[4]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        stbi_image_free(data);
    }
}
void LineShader::drawHotbar(int screenWidth, int screenHeight, int selected,
                            vector<int> blockTypes) {
    glUseProgram(uiShaderProgram);

    glm::mat4 projection =
        glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight);
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

    glm::mat4 model =
        glm::translate(glm::mat4(1.0f), glm::vec3(posX, posY, 0.0f));
    model = glm::scale(model, glm::vec3(hotbarWidth, hotbarHeight, 1.0f));
    glUniformMatrix4fv(uiModelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(uiVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // Dibujar selector
    glBindTexture(GL_TEXTURE_2D, selectorTextureID);

    float selectorX = posX + (selected - 1) * slotWidth + 2 -
                      (selectorSize - slotWidth) / 2.0f;
    float selectorY = posY - (selectorSize - hotbarHeight) / 2.0f;

    model =
        glm::translate(glm::mat4(1.0f), glm::vec3(selectorX, selectorY, 0.1f));
    model = glm::scale(model, glm::vec3(selectorSize, selectorSize, 1.0f));
    glUniformMatrix4fv(uiModelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // Dibujar icono
    float iconSize = 32.0f;

    glBindVertexArray(uiVAO); // usamos el mismo quad

    for (int i = 0; i < blockTypes.size(); i++) {

        int blockType = blockTypes[i];
        if (iconTexturesID[blockType] == 0)
            continue;

        glBindTexture(GL_TEXTURE_2D, iconTexturesID[blockType]);
        glUniform1i(uiTextureLoc, 0);
        glUniform4f(uiColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

        float slotX = posX + 2 + i * slotWidth;
        float slotY = posY;

        float centerX = slotX + slotWidth / 2.0f;
        float centerY = slotY + hotbarHeight / 2.0f;

        float iconX = centerX - iconSize / 2.0f;
        float iconY = centerY - iconSize / 2.0f;

        glm::mat4 iconModel =
            glm::translate(glm::mat4(1.0f), glm::vec3(iconX, iconY, 0.2f));
        iconModel = glm::scale(iconModel, glm::vec3(iconSize, iconSize, 1.0f));

        glUniformMatrix4fv(uiModelLoc, 1, GL_FALSE, glm::value_ptr(iconModel));

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    // Restaurar estado
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}
void LineShader::drawCrosshair(int screenWidth, int screenHeight, int size,
                               float r, float g, float b) {
    // Configurar proyección ortográfica para 2D
    glm::mat4 ortho =
        glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f);

    // Usar lineshader
    glUseProgram(shaderProgram);
    setProjectionMatrix(glm::value_ptr(ortho));
    setViewMatrix(glm::value_ptr(glm::mat4(1.0f)));
    setModelMatrix(glm::value_ptr(glm::mat4(1.0f)));

    int centerX = screenWidth / 2;
    int centerY = screenHeight / 2;

    float vertices[] = {// Línea horizontal
                        (float)centerX - size, (float)centerY, 0.0f,
                        (float)centerX + size, (float)centerY, 0.0f,

                        // Línea vertical
                        (float)centerX, (float)centerY - size, 0.0f,
                        (float)centerX, (float)centerY + size, 0.0f,

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
LineShader::~LineShader() {
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
    glDeleteTextures(1, &hotbarTextureID);
    glDeleteTextures(1, &selectorTextureID);
}
