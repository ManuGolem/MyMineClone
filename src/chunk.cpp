#include "../include/chunk.h"
#include "../include/world.h"
#include <filesystem>
template <size_t FILAS, size_t COLUMNAS> vector<Rectangulo> formarRectangulos(int (&tipos)[FILAS][COLUMNAS]) {
    vector<Rectangulo> rectangulos;
    bool procesado[FILAS][COLUMNAS] = {false};

    for (int i = 0; i < FILAS; i++) {
        for (int j = 0; j < COLUMNAS; j++) {
            if (tipos[i][j] != 0 && !procesado[i][j]) {
                int tipo_actual = tipos[i][j];
                int ancho = 1;

                // Expandir ancho mientras sea el MISMO tipo
                while (i + ancho < FILAS && tipos[i + ancho][j] == tipo_actual && !procesado[i + ancho][j]) {
                    ancho++;
                }

                int alto = 1;
                bool expandible = true;

                // Expandir alto mientras TODA la fila sea del MISMO tipo
                while (j + alto < COLUMNAS && expandible) {
                    for (int dj = 0; dj < ancho; dj++) {
                        if (tipos[i + dj][j + alto] != tipo_actual || procesado[i + dj][j + alto]) {
                            expandible = false;
                            break;
                        }
                    }
                    if (expandible) {
                        alto++;
                    }
                }
                for (int k = i; k < i + ancho; k++) {
                    for (int l = j; l < j + alto; l++) {
                        procesado[k][l] = true;
                    }
                }
                Rectangulo r;
                r.y1 = i;
                r.x1 = j;
                r.y2 = i + ancho - 1;
                r.x2 = j + alto - 1;
                r.tipoBloque = tipo_actual;
                rectangulos.push_back(r);
            }
        }
    }
    return rectangulos;
}

Shader* Chunk::sharedShader = nullptr;
Chunk::Chunk() : world(nullptr) {
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 256; y++) {
            for (int z = 0; z < 16; z++) {
                blocks[x][y][z].active = false;
                blocks[x][y][z].type = 0;
            }
        }
    }
    if (sharedShader == nullptr) {
        sharedShader = new Shader();
    }
}
void Chunk::cargarVertices(const Rectangulo& r, int eje, int direccion, int fijo, int tipo_bloque) {
    float base = vertexCount;
    float offsetX = nroChunkX * 16.0f;
    float offsetZ = nroChunkZ * 16.0f;

    // Calculo uv para el atlas (0,0) es abajo a la izquierda
    float u_min, u_max, v_min, v_max;
    float ancho = r.x2 - r.x1 + 1;
    float alto = r.y2 - r.y1 + 1;
    int columna = (tipo_bloque - 1) % 16;
    int fila = (tipo_bloque - 1) / 16;
    float tileSize = 1.0f / 16.0f;
    float offsetU = columna * tileSize;
    float offsetV = 1.0f - (fila + 1) * tileSize;
    if (eje == 0) {
        float xPos = offsetX + fijo + (direccion == 1 ? 0.5f : -0.5f);
        float rcolor = 1.0f;
        float gcolor = 1.0f;
        float bcolor = 1.0f;

        float y1 = r.y1 - 1.0f;
        float y2 = r.y2;

        float z1 = offsetZ + r.x1 - 0.5f;
        float z2 = offsetZ + r.x2 + 0.5f;

        float vertex[40] = {// VERTEX 1
                            xPos, y1, z1, rcolor, gcolor, bcolor, 0.0f, 0.0f, offsetU, offsetV,
                            // VERTEX 2
                            xPos, y1, z2, rcolor, gcolor, bcolor, ancho, 0.0f, offsetU, offsetV,
                            // VERTEX 3
                            xPos, y2, z2, rcolor, gcolor, bcolor, ancho, alto, offsetU, offsetV,
                            // VERTEX 4
                            xPos, y2, z1, rcolor, gcolor, bcolor, 0.0f, alto, offsetU, offsetV};
        vertexData.insert(vertexData.end(), std::begin(vertex), std::end(vertex));
        if (direccion == 1) {
            // Cara mirando hacia +X
            indexData.push_back(base);     // 0: Inf izq
            indexData.push_back(base + 3); // 3: Sup izq
            indexData.push_back(base + 2); // 2: Sup der
            indexData.push_back(base);     // 0: Inf izq
            indexData.push_back(base + 2); // 2: Sup der
            indexData.push_back(base + 1); // 1: Inf der
        } else {
            // Cara mirando hacia -X
            indexData.push_back(base + 1); // 1: Inf der
            indexData.push_back(base + 2); // 2: Sup der
            indexData.push_back(base + 3); // 3: Sup izq
            indexData.push_back(base + 1); // 1: Inf der
            indexData.push_back(base + 3); // 3: Sup izq
            indexData.push_back(base);     // 0: Inf izq
        }
    } else if (eje == 1) { // CARAS EN Y
        alto = r.x2 - r.x1 + 1;
        ancho = r.y2 - r.y1 + 1;
        float rcolor = 1.0f;
        float gcolor = 1.0f;
        float bcolor = 1.0f;
        if (tipo_bloque == 4) {
            if (direccion == 1) {
                rcolor = 0.3f;
                gcolor = 0.8f;
                bcolor = 0.3f;
                columna = 8;
                fila = 2.0f;
            } else {
                columna = 2;
                fila = 0.0f;
            }
            offsetU = columna * tileSize;
            offsetV = 1.0f - (fila + 1) * tileSize;
        }
        float yPos = fijo + (direccion == 1 ? 0.0f : -1.0f);
        float x1 = offsetX + r.y1 - 0.5f;
        float x2 = offsetX + r.y2 + 0.5f;
        float z1 = offsetZ + r.x1 - 0.5f;
        float z2 = offsetZ + r.x2 + 0.5f;
        float vertex[40] = {// VERTEX 1
                            x1, yPos, z1, rcolor, gcolor, bcolor, 0.0f, 0.0f, offsetU, offsetV,
                            // VERTEX 2
                            x2, yPos, z1, rcolor, gcolor, bcolor, ancho, 0.0f, offsetU, offsetV,
                            // VERTEX 3
                            x2, yPos, z2, rcolor, gcolor, bcolor, ancho, alto, offsetU, offsetV,
                            // VERTEX 4
                            x1, yPos, z2, rcolor, gcolor, bcolor, 0.0f, alto, offsetU, offsetV};

        vertexData.insert(vertexData.end(), std::begin(vertex), std::end(vertex));

        if (direccion == 1) {
            // Cara mirando hacia +Y
            indexData.push_back(base);     // 0: Inf izq
            indexData.push_back(base + 3); // 3: Sup izq
            indexData.push_back(base + 2); // 2: Sup der
            indexData.push_back(base);     // 0: Inf izq
            indexData.push_back(base + 2); // 2: Sup der
            indexData.push_back(base + 1); // 1: Inf der
        } else {
            // Cara mirando hacia -Y
            indexData.push_back(base + 1); // 1: Inf der
            indexData.push_back(base + 2); // 2: Sup der
            indexData.push_back(base + 3); // 3: Sup izq
            indexData.push_back(base + 1); // 1: Inf der
            indexData.push_back(base + 3); // 3: Sup izq
            indexData.push_back(base);     // 0: Inf izq
        }
    } else if (eje == 2) { // CARAS EN Z
        float rcolor = 1.0f;
        float gcolor = 1.0f;
        float bcolor = 1.0f;
        float zPos = offsetZ + fijo + (direccion == 1 ? 0.5f : -0.5f);
        float y1 = r.x1 - 1.0f;
        float y2 = r.x2;
        float x1 = offsetX + r.y1 - 0.5f;
        float x2 = offsetX + r.y2 + 0.5f;
        float vertex[40] = {// VERTEX 1
                            x1, y1, zPos, rcolor, gcolor, bcolor, 0.0f, 0.0f, offsetU, offsetV,
                            // VERTEX 2
                            x2, y1, zPos, rcolor, gcolor, bcolor, alto, 0.0f, offsetU, offsetV,
                            // VERTEX 3
                            x2, y2, zPos, rcolor, gcolor, bcolor, alto, ancho, offsetU, offsetV,
                            // VERTEX 4
                            x1, y2, zPos, rcolor, gcolor, bcolor, 0.0f, ancho, offsetU, offsetV};

        vertexData.insert(vertexData.end(), std::begin(vertex), std::end(vertex));

        if (direccion == -1) {
            // Cara mirando hacia -Z
            indexData.push_back(base);     // 0: Inf izq
            indexData.push_back(base + 3); // 3: Sup izq
            indexData.push_back(base + 2); // 2: Sup der
            indexData.push_back(base);     // 0: Inf izq
            indexData.push_back(base + 2); // 2: Sup der
            indexData.push_back(base + 1); // 1: Inf der
        } else {
            // Cara mirando hacia +Z
            indexData.push_back(base + 1); // 1: Inf der
            indexData.push_back(base + 2); // 2: Sup der
            indexData.push_back(base + 3); // 3: Sup izq
            indexData.push_back(base + 1); // 1: Inf der
            indexData.push_back(base + 3); // 3: Sup izq
            indexData.push_back(base);     // 0: Inf izq
        }
    }
    caras++;
    vertexCount += 4;
}
void Chunk::generateMesh() {
    if (!needsUpdate)
        return;
    vertexData.clear();
    indexData.clear();
    vertexCount = 0;
    caras = 0;
    // CARAS EN X
    for (int x = 0; x < 16; x++) {
        int capasIzquierdas[256][16] = {0};
        int capasDerechas[256][16] = {0};
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 16; j++) {
                if (blocks[x][i][j].active && blocks[x][i][j].type != 0) {
                    int globalX = x + nroChunkX * 16;
                    int globalZ = j + nroChunkZ * 16;
                    // Cara derecha (x+)
                    if (x == 15) {
                        if (!world || !world->getBlock(globalX + 1, i, globalZ).active) {
                            capasDerechas[i][j] = blocks[x][i][j].type;
                        }
                    } else if (!blocks[x + 1][i][j].active) {
                        capasDerechas[i][j] = blocks[x][i][j].type;
                    }
                    // Cara izquierda (x-)
                    if (x == 0) {
                        if (!world || !world->getBlock(globalX - 1, i, globalZ).active) {
                            capasIzquierdas[i][j] = blocks[x][i][j].type;
                        }
                    } else if (!blocks[x - 1][i][j].active) {
                        capasIzquierdas[i][j] = blocks[x][i][j].type;
                    }
                }
            }
        }
        vector<Rectangulo> rectsDer = formarRectangulos(capasDerechas);
        for (const Rectangulo& r : rectsDer) {
            cargarVertices(r, 0, 1, x, r.tipoBloque);
        }
        vector<Rectangulo> rectsIzq = formarRectangulos(capasIzquierdas);

        for (const Rectangulo& r : rectsIzq) {
            cargarVertices(r, 0, -1, x, r.tipoBloque);
        }
    }
    for (int y = 0; y < 256; y++) {
        int capasSuperiores[16][16] = {0};
        int capasInferiores[16][16] = {0};
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                if (blocks[i][y][j].active && blocks[i][y][j].type != 0) {
                    // Cara inferior (y-)
                    if ((y == 0) || !blocks[i][y - 1][j].active) {
                        capasInferiores[i][j] = blocks[i][y][j].type;
                    }
                    // Cara superior (y+)
                    if ((y == 255) || !blocks[i][y + 1][j].active) {
                        capasSuperiores[i][j] = blocks[i][y][j].type;
                    }
                }
            }
        }
        vector<Rectangulo> rectsInf = formarRectangulos(capasInferiores);
        for (const Rectangulo& r : rectsInf) {
            cargarVertices(r, 1, -1, y, r.tipoBloque);
        }
        vector<Rectangulo> rectsSup = formarRectangulos(capasSuperiores);

        for (const Rectangulo& r : rectsSup) {
            cargarVertices(r, 1, 1, y, r.tipoBloque);
        }
    }
    for (int z = 0; z < 16; z++) {
        int capasFrontal[16][256] = {0};
        int capasTrasera[16][256] = {0};

        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 256; j++) {
                if (blocks[i][j][z].active) {
                    int globalX = i + nroChunkX * 16;
                    int globalZ = z + nroChunkZ * 16;
                    if (z == 15) {
                        if (!world || !world->getBlock(globalX, j, globalZ + 1).active) {
                            capasFrontal[i][j] = blocks[i][j][z].type;
                        }
                    } else if (!blocks[i][j][z + 1].active) {
                        capasFrontal[i][j] = blocks[i][j][z].type;
                    }
                    // Cara izquierda (x-)
                    if (z == 0) {
                        if (!world || !world->getBlock(globalX, j, globalZ - 1).active) {
                            capasTrasera[i][j] = blocks[i][j][z].type;
                        }
                    } else if (!blocks[i][j][z - 1].active) {
                        capasTrasera[i][j] = blocks[i][j][z].type;
                    }
                }
            }
        }

        vector<Rectangulo> rectsFrontal = formarRectangulos(capasFrontal);
        for (const Rectangulo& r : rectsFrontal) {
            cargarVertices(r, 2, 1, z, r.tipoBloque);
        }

        vector<Rectangulo> rectsTrasera = formarRectangulos(capasTrasera);
        for (const Rectangulo& r : rectsTrasera) {
            cargarVertices(r, 2, -1, z, r.tipoBloque);
        }
    }

    needsUpdate = false;
}
void Chunk::setBlock(int x, int y, int z, const Block& block) {
    blocks[x][y][z] = block;
    if (x == 15) {
        Chunk* chunk = world->getChunk(nroChunkX + 1, nroChunkZ);
        if (chunk) {
            chunk->setUpdate();
        }
    } else if (x == 0) {
        Chunk* chunk = world->getChunk(nroChunkX - 1, nroChunkZ);
        if (chunk) {
            chunk->setUpdate();
        }
    }
    if (z == 15) {
        Chunk* chunk = world->getChunk(nroChunkX, nroChunkZ + 1);
        if (chunk) {
            chunk->setUpdate();
        }
    } else if (z == 0) {
        Chunk* chunk = world->getChunk(nroChunkX, nroChunkZ - 1);
        if (chunk) {
            chunk->setUpdate();
        }
    }
    needsUpdate = true;
}
Block Chunk::getBlock(int x, int y, int z) const {
    return blocks[x][y][z];
}
bool Chunk::isEmpty() const {
    return vertexData.empty();
}
void Chunk::setNroChunk(int chunkx, int chunkz) {
    nroChunkX = chunkx;
    nroChunkZ = chunkz;
}
void Chunk::render() {
    if (needsUpdate) {
        generateMesh();
        if (!vertexData.empty()) {
            chunkBuffer.uploadData(vertexData, indexData);
        }
        needsUpdate = false;
    }
    if (vertexData.empty())
        return;

    chunkBuffer.render();
}
void Chunk::cleanup() {
    chunkBuffer.cleanup();

    // Limpiar datos
    vertexData.clear();
    indexData.clear();
    caras = 0;
    vertexCount = 0;
    needsUpdate = true;

    // Opcional: liberar la memoria de los vectores
    vector<float>().swap(vertexData);
    vector<unsigned int>().swap(indexData);
}
