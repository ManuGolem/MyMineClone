#include "../include/chunk.h"
#include "../include/world.h"
#include <memory>
#include <mutex>
bool esTransparent(int type) {
    return (type == 53);
}
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
Chunk::Chunk() : world(nullptr), needsUpdate(true) {
    if (sharedShader == nullptr) {
        sharedShader = new Shader();
    }
}
void Chunk::cargarVertices(const Rectangulo& r, int eje, int direccion, int fijo, int tipo_bloque, vector<float>& vData, vector<unsigned int>& iData,
                           unsigned int& vCount) {
    float base = vCount;
    float offsetX = nroChunkX * 16.0f;
    float offsetZ = nroChunkZ * 16.0f;
    float rcolor = 1.0f;
    float gcolor = 1.0f;
    float bcolor = 1.0f;
    if (tipo_bloque == 54 || tipo_bloque == 53) {
        // Leaf
        rcolor = 0.3f;
        gcolor = 0.8f;
        bcolor = 0.3f;
    }

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
        vData.insert(vData.end(), std::begin(vertex), std::end(vertex));
        if (direccion == 1) {
            // Cara mirando hacia +X
            iData.push_back(base);     // 0: Inf izq
            iData.push_back(base + 3); // 3: Sup izq
            iData.push_back(base + 2); // 2: Sup der
            iData.push_back(base);     // 0: Inf izq
            iData.push_back(base + 2); // 2: Sup der
            iData.push_back(base + 1); // 1: Inf der
        } else {
            // Cara mirando hacia -X
            iData.push_back(base + 1); // 1: Inf der
            iData.push_back(base + 2); // 2: Sup der
            iData.push_back(base + 3); // 3: Sup izq
            iData.push_back(base + 1); // 1: Inf der
            iData.push_back(base + 3); // 3: Sup izq
            iData.push_back(base);     // 0: Inf izq
        }
    } else if (eje == 1) { // CARAS EN Y
        alto = r.x2 - r.x1 + 1;
        ancho = r.y2 - r.y1 + 1;
        if (tipo_bloque == 4) {
            if (direccion == 1) {
                rcolor = 0.3f;
                gcolor = 0.8f;
                bcolor = 0.3f;
                columna = 8;
                fila = 2;
            } else {
                columna = 2;
                fila = 0;
            }
        } else if (tipo_bloque == 21) {
            columna = 5;
            fila = 1;
        } else if (tipo_bloque == 36) {
            columna = 4;
            fila = 0;
        }
        offsetU = columna * tileSize;
        offsetV = 1.0f - (fila + 1) * tileSize;

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

        vData.insert(vData.end(), std::begin(vertex), std::end(vertex));

        if (direccion == 1) {
            // Cara mirando hacia +Y
            iData.push_back(base);     // 0: Inf izq
            iData.push_back(base + 3); // 3: Sup izq
            iData.push_back(base + 2); // 2: Sup der
            iData.push_back(base);     // 0: Inf izq
            iData.push_back(base + 2); // 2: Sup der
            iData.push_back(base + 1); // 1: Inf der
        } else {
            // Cara mirando hacia -Y
            iData.push_back(base + 1); // 1: Inf der
            iData.push_back(base + 2); // 2: Sup der
            iData.push_back(base + 3); // 3: Sup izq
            iData.push_back(base + 1); // 1: Inf der
            iData.push_back(base + 3); // 3: Sup izq
            iData.push_back(base);     // 0: Inf izq
        }
    } else if (eje == 2) { // CARAS EN Z
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

        vData.insert(vData.end(), std::begin(vertex), std::end(vertex));

        if (direccion == -1) {
            // Cara mirando hacia -Z
            iData.push_back(base);     // 0: Inf izq
            iData.push_back(base + 3); // 3: Sup izq
            iData.push_back(base + 2); // 2: Sup der
            iData.push_back(base);     // 0: Inf izq
            iData.push_back(base + 2); // 2: Sup der
            iData.push_back(base + 1); // 1: Inf der
        } else {
            // Cara mirando hacia +Z
            iData.push_back(base + 1); // 1: Inf der
            iData.push_back(base + 2); // 2: Sup der
            iData.push_back(base + 3); // 3: Sup izq
            iData.push_back(base + 1); // 1: Inf der
            iData.push_back(base + 3); // 3: Sup izq
            iData.push_back(base);     // 0: Inf izq
        }
    }
    vCount += 4;
}
void Chunk::generateMesh() {
    lock_guard<mutex> lock(mutexBlocks);
    std::vector<float> newVertexData;
    std::vector<unsigned int> newIndexData;
    unsigned int newVertexCount = 0;
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
                        if (world) {
                            Block block = world->getBlockSafe(globalX + 1, i, globalZ);
                            if (!block.active || esTransparent(block.type))
                                capasDerechas[i][j] = blocks[x][i][j].type;
                        }
                    } else if (!blocks[x + 1][i][j].active || esTransparent(blocks[x + 1][i][j].type)) {
                        capasDerechas[i][j] = blocks[x][i][j].type;
                    }
                    // Cara izquierda (x-)
                    if (x == 0) {
                        if (world) {
                            Block block = world->getBlockSafe(globalX - 1, i, globalZ);
                            if (!block.active || esTransparent(block.type))
                                capasIzquierdas[i][j] = blocks[x][i][j].type;
                        }
                    } else if (!blocks[x - 1][i][j].active || esTransparent(blocks[x - 1][i][j].type)) {
                        capasIzquierdas[i][j] = blocks[x][i][j].type;
                    }
                }
            }
        }
        vector<Rectangulo> rectsDer = formarRectangulos(capasDerechas);
        for (const Rectangulo& r : rectsDer) {
            cargarVertices(r, 0, 1, x, r.tipoBloque, newVertexData, newIndexData, newVertexCount);
        }
        vector<Rectangulo> rectsIzq = formarRectangulos(capasIzquierdas);

        for (const Rectangulo& r : rectsIzq) {
            cargarVertices(r, 0, -1, x, r.tipoBloque, newVertexData, newIndexData, newVertexCount);
        }
    }
    for (int y = 0; y < 256; y++) {
        int capasSuperiores[16][16] = {0};
        int capasInferiores[16][16] = {0};
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                if (blocks[i][y][j].active && blocks[i][y][j].type != 0) {
                    // Cara inferior (y-)
                    if ((y == 0) || !blocks[i][y - 1][j].active || esTransparent(blocks[i][y - 1][j].type)) {
                        capasInferiores[i][j] = blocks[i][y][j].type;
                    }
                    // Cara superior (y+)
                    if ((y == 255) || !blocks[i][y + 1][j].active || esTransparent(blocks[i][y + 1][j].type)) {
                        capasSuperiores[i][j] = blocks[i][y][j].type;
                    }
                }
            }
        }
        vector<Rectangulo> rectsInf = formarRectangulos(capasInferiores);
        for (const Rectangulo& r : rectsInf) {
            cargarVertices(r, 1, -1, y, r.tipoBloque, newVertexData, newIndexData, newVertexCount);
        }
        vector<Rectangulo> rectsSup = formarRectangulos(capasSuperiores);

        for (const Rectangulo& r : rectsSup) {
            cargarVertices(r, 1, 1, y, r.tipoBloque, newVertexData, newIndexData, newVertexCount);
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
                    // z+
                    if (z == 15) {
                        if (world) {
                            Block block = world->getBlockSafe(globalX, j, globalZ + 1);
                            if (!block.active || esTransparent(block.type))
                                capasFrontal[i][j] = blocks[i][j][z].type;
                        }
                    } else if (!blocks[i][j][z + 1].active || esTransparent(blocks[i][j][z + 1].type)) {
                        capasFrontal[i][j] = blocks[i][j][z].type;
                    }
                    // Cara izquierda (Z-)
                    if (z == 0) {
                        if (world) {
                            Block block = world->getBlockSafe(globalX, j, globalZ - 1);
                            if (!block.active || esTransparent(block.type))
                                capasTrasera[i][j] = blocks[i][j][z].type;
                        }
                    } else if (!blocks[i][j][z - 1].active || esTransparent(blocks[i][j][z - 1].type)) {
                        capasTrasera[i][j] = blocks[i][j][z].type;
                    }
                }
            }
        }

        vector<Rectangulo> rectsFrontal = formarRectangulos(capasFrontal);
        for (const Rectangulo& r : rectsFrontal) {
            cargarVertices(r, 2, 1, z, r.tipoBloque, newVertexData, newIndexData, newVertexCount);
        }

        vector<Rectangulo> rectsTrasera = formarRectangulos(capasTrasera);
        for (const Rectangulo& r : rectsTrasera) {
            cargarVertices(r, 2, -1, z, r.tipoBloque, newVertexData, newIndexData, newVertexCount);
        }
    }

    {
        std::lock_guard<std::mutex> lock(mutexVertex);
        vertexData.swap(newVertexData);
        indexData.swap(newIndexData);
        vertexCount = newVertexCount;
    }
    needsUpdate = false;
    needsBufferUpdate.store(true);
}
void Chunk::setBlock(int x, int y, int z, const Block& block) {
    lock_guard<mutex> lock(mutexBlocks);
    blocks[x][y][z] = block;
    needsUpdate = true;
}
Block Chunk::getBlock(int x, int y, int z) const {

    if (x < 0 || x >= 16 || z < 0 || z >= 16) {
        return Block{false, 0};
    }
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
    if (needsBufferUpdate.exchange(false)) {
        lock_guard<mutex> lock(mutexVertex);
        if (!vertexData.empty()) {
            chunkBuffer->uploadData(vertexData, indexData);
        }
    }
    if (needsUpdate && !isUpdating) {
        generateMesh();
        if (!vertexData.empty()) {
            chunkBuffer->uploadData(vertexData, indexData);
        }
    }
    chunkBuffer->render();
}
void Chunk::cleanup() {
    chunkBuffer.reset();

    // Limpiar datos
    vertexData.clear();
    indexData.clear();
    vertexCount = 0;
    needsUpdate = true;

    // Opcional: liberar la memoria de los vectores
    vector<float>().swap(vertexData);
    vector<unsigned int>().swap(indexData);
}
