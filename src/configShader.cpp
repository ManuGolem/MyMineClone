#include "../include/configShader.h"
#include "../include/stb_image.h"
#include <iostream>
using namespace std;
unsigned int cargarTextura(const char* ruta) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    // Configurar parámetros de la textura
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(ruta, &width, &height, &nrChannels, 4);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        cout << "Textura cargada: " << ruta << " (" << width << "x" << height << ", " << nrChannels << " canales)" << endl;
    } else {
        cout << "ERROR: No se pudo cargar la textura: " << ruta << endl;
    }
    stbi_image_free(data); // Liberar memoria de la imagen
    return textureID;
}
Shader::Shader() {
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSrc, NULL);
    glCompileShader(vs);
    // El otro shader
    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSrc, NULL);
    glCompileShader(fs);

    // Crear programa
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

    glUseProgram(shaderProgram);
    // Configurar matriz model por defecto
    glm::mat4 model = glm::mat4(1.0f);
    setModelMatrix(glm::value_ptr(model));

    textureID = cargarTextura("../textures/textures.png");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -2.0f);
    glUniform1i(glGetUniformLocation(shaderProgram, "textureBlock"), 0);
    float textureSize = 1 / 16.0f;
    glUniform1f(glGetUniformLocation(shaderProgram, "textureSize"), textureSize);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    useTextureLoc = glGetUniformLocation(shaderProgram, "useTexture");
}
Shader::~Shader() {
    glDeleteProgram(shaderProgram);
}
void Shader::use() {
    glUseProgram(shaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, getTextureID());
    setUseTexture(true);
}
void Shader::setModelMatrix(const float* matrix) {
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, matrix);
}

void Shader::setViewMatrix(const float* matrix) {
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, matrix);
}

void Shader::setProjectionMatrix(const float* matrix) {
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, matrix);
}

ChunkBuffer::ChunkBuffer() : indexCount(0) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // Posición
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Textura
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Offset
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
}
ChunkBuffer::~ChunkBuffer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}
void ChunkBuffer::uploadData(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
    indexCount = indices.size();
    glBindVertexArray(VAO);
    // VBO - datos de vértices
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    // EBO - índices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

    // Verificación opcional
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error in uploadData: " << err << std::endl;
    }

    glBindVertexArray(0);
}
void ChunkBuffer::render() {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}
