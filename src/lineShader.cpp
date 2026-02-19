#include "../include/lineShader.h"
unsigned int LineShader::axesVAO = 0;
unsigned int LineShader::axesVBO = 0;
unsigned int LineShader::outlinesVAO = 0;
unsigned int LineShader::outlinesVBO = 0;
unsigned int LineShader::crosshairVAO = 0;
unsigned int LineShader::crosshairVBO = 0;
LineShader::LineShader() {
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

    // Compilar shaders (igual que en tu Shader actual)
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
}
void LineShader::drawDebugAxes(const glm::mat4& view, const glm::mat4& projection) {
    use();
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
void LineShader::drawOutline(int x, int y, int z, const glm::mat4& view, const glm::mat4& projection) {
    GLint previousProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);
    use();
    setViewMatrix(glm::value_ptr(view));
    setProjectionMatrix(glm::value_ptr(projection));

    glm::mat4 model(1.0f);
    setModelMatrix(glm::value_ptr(model));
    // 12 aristas del cubo (24 vértices)
    float vertices[] = {// Cara inferior
                        x - 0.5f, y - 1.0f, z - 0.5f, x + 0.5f, y - 1.0f, z - 0.5f, x + 0.5f, y - 1.0f, z - 0.5f, x + 0.5f, y - 1.0f, z + 0.5f, x + 0.5f,
                        y - 1.0f, z + 0.5f, x - 0.5f, y - 1.0f, z + 0.5f, x - 0.5f, y - 1.0f, z + 0.5f, x - 0.5f, y - 1.0f, z - 0.5f,

                        // Cara superior
                        x - 0.5f, y + 0.0f, z - 0.5f, x + 0.5f, y + 0.0f, z - 0.5f, x + 0.5f, y + 0.0f, z - 0.5f, x + 0.5f, y + 0.0f, z + 0.5f, x + 0.5f,
                        y + 0.0f, z + 0.5f, x - 0.5f, y + 0.0f, z + 0.5f, x - 0.5f, y + 0.0f, z + 0.5f, x - 0.5f, y + 0.0f, z - 0.5f,

                        // Aristas verticales
                        x - 0.5f, y - 1.0f, z - 0.5f, x - 0.5f, y + 0.0f, z - 0.5f, x + 0.5f, y - 1.0f, z - 0.5f, x + 0.5f, y + 0.0f, z - 0.5f, x - 0.5f,
                        y - 1.0f, z + 0.5f, x - 0.5f, y + 0.0f, z + 0.5f, x + 0.5f, y - 1.0f, z + 0.5f, x + 0.5f, y + 0.0f, z + 0.5f};

    glBindVertexArray(outlinesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, outlinesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    // Dibujar con depth test NORMAL (no modificamos nada)
    glLineWidth(1.0f);
    setColor(0.0f, 0.0f, 0.0f);    // Negro
    glDrawArrays(GL_LINES, 0, 24); // 12 líneas * 2 vértices

    // Restaurar shader anterior si es necesario
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (previousProgram != 0 && previousProgram != (GLint)shaderProgram) {
        glBindVertexArray(0);
        glUseProgram(previousProgram);
    }
}
void LineShader::drawCrosshair(int screenWidth, int screenHeight, int size, float r, float g, float b) {
    // Guardar shader activo
    GLint previousProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);
    // Configurar proyección ortográfica para 2D
    glm::mat4 ortho = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f);

    // Usar este shader
    use();
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

    // Restaurar shader anterior si es necesario
    if (previousProgram != 0 && previousProgram != (GLint)shaderProgram) {
        glUseProgram(previousProgram);
    }
}
LineShader::~LineShader() {
    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &axesVAO);
    glDeleteBuffers(1, &axesVBO);
    glDeleteVertexArrays(1, &outlinesVAO);
    glDeleteBuffers(1, &outlinesVBO);
    glDeleteVertexArrays(1, &crosshairVAO);
    glDeleteBuffers(1, &crosshairVBO);
}
