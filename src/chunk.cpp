#include "../include/chunk.h"
#include "../include/blocksRegistry.h"
#include "../include/world.h"
#include <array>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <iterator>
#include <memory>
#include <mutex>
static int OAK_LEAVES = BlockRegistry::getType("oak_leaves");
static int GRASS_BLOCK = BlockRegistry::getType("grass_block");
static int OAK_LOG = BlockRegistry::getType("oak_log");
static int SPRUCE_LOG = BlockRegistry::getType("spruce_log");
static int BOOKSHELF = BlockRegistry::getType("bookshelf");
static int CACTUS = BlockRegistry::getType("cactus");
static int SPRUCE_LEAVES = BlockRegistry::getType("spruce_leaves");
static int SNOWY_GRASS_BLOCK = BlockRegistry::getType("snowy_grass_block");
static int TNT_BLOCK = BlockRegistry::getType("tnt");
static int POPPY = BlockRegistry::getType("poppy");
static int DANDELION = BlockRegistry::getType("dandelion");
static int OAK_SAPLING = BlockRegistry::getType("oak_sapling");
static int JUNGLE_SAPLING = BlockRegistry::getType("jungle_sapling");
static int SPRUCE_SAPLING = BlockRegistry::getType("spruce_sapling");
static int RED_MUSHROOM = BlockRegistry::getType("red_mushroom");
static int SHORT_GRASS = BlockRegistry::getType("short_grass");
static int BROWN_MUSHROOM = BlockRegistry::getType("brown_mushroom");
bool isCrossBlock(int16_t type) {
    return (type == POPPY || type == DANDELION || type == OAK_SAPLING || type == JUNGLE_SAPLING ||
            type == SPRUCE_SAPLING || type == SHORT_GRASS || type == RED_MUSHROOM || type == BROWN_MUSHROOM);
}
bool esTransparent(int16_t type) {
    return (type == 0 || type == OAK_LEAVES || type == CACTUS || type == SPRUCE_LEAVES || isCrossBlock(type));
}
template <size_t FILAS, size_t COLUMNAS> vector<Rectangulo> formarRectangulos(int16_t (&tipos)[FILAS][COLUMNAS]) {
    vector<Rectangulo> rectangulos;
    bool procesado[FILAS][COLUMNAS] = {false};

    for (int i = 0; i < FILAS; i++) {
        for (int j = 0; j < COLUMNAS; j++) {
            if (tipos[i][j] != 0 && !procesado[i][j]) {
                int16_t tipo_actual = tipos[i][j];
                int ancho = 1;
                int alto = 1;
                // Expandir ancho mientras sea el MISMO tipo
                while (i + ancho < FILAS && tipos[i + ancho][j] == tipo_actual && !procesado[i + ancho][j]) {
                    ancho++;
                }

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

Shader *Chunk::sharedShader = nullptr;
Chunk::Chunk() : world(nullptr), needsUpdate(true), isUpdating(false) {
    memset(blocks, 0, sizeof(blocks));
    if (sharedShader == nullptr) {
        sharedShader = new Shader();
    }
}
float Chunk::calcularAO(int x, int y, int z, int dx1, int dy1, int dz1, int dx2, int dy2, int dz2) {
    //Crear funcion
}
void Chunk::cargarVerticesCross(const Rectangulo &r, int fijo, int16_t tipo_bloque, vector<float> &vData,
                                vector<unsigned int> &iData, unsigned int &vCount) {
    //El ao en esta funcion lo dejo fijo en 1, es decir que todo bloque cross no tiene AO
                                    int eje = 0;
    int direccion = 1;
    float base = vCount;
    float offsetX = nroChunkX * 16.0f;
    float offsetZ = nroChunkZ * 16.0f;
    float rcolor = 1.0f;
    float gcolor = 1.0f;
    float bcolor = 1.0f;
    if (tipo_bloque == SHORT_GRASS) {
        rcolor = 0.5f;
        gcolor = 0.9f;
        bcolor = 0.5f;
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
    float xPos = offsetX + fijo + 0.5;
    float y1 = r.y1 - 1.0f;
    float y2 = r.y2;
    float z1 = offsetZ + r.x1 - 0.5f;
    float z2 = offsetZ + r.x2 + 0.5f;
    float vertex[44] = {// VERTEX 1
                        xPos, y1, z1, rcolor, gcolor, bcolor, 0.0f, 0.0f, offsetU, offsetV, 1,
                        // VERTEX 2
                        xPos - 1.0f, y1, z2, rcolor, gcolor, bcolor, ancho, 0.0f, offsetU, offsetV, 1,
                        // VERTEX 3
                        xPos - 1.0f, y2, z2, rcolor, gcolor, bcolor, ancho, alto, offsetU, offsetV, 1,
                        // VERTEX 4
                        xPos, y2, z1, rcolor, gcolor, bcolor, 0.0f, alto, offsetU, offsetV, 1};
    vData.insert(vData.end(), std::begin(vertex), std::end(vertex));
    // Cara mirando hacia +X
    iData.push_back(base);     // Abajo derecha
    iData.push_back(base + 3); // Arriba derecha
    iData.push_back(base + 2); // Arriba izquierda
    iData.push_back(base);     // Abajo derecha
    iData.push_back(base + 2); // Arriba izquierda
    iData.push_back(base + 1); // Abajo derecha
    // Cara mirando -x
    iData.push_back(base + 1); // Abajo derecha
    iData.push_back(base + 2); // Arriba izquierda
    iData.push_back(base);     // Abajo derecha
    iData.push_back(base + 2); // Arriba izquierda
    iData.push_back(base + 3); // Arriba derecha
    iData.push_back(base);     // Abajo derecha
    // La otra cara diagonal
    float vertex2[44] = {// VERTEX 1
                         xPos - 1.0f, y1, z1, rcolor, gcolor, bcolor, 0.0f, 0.0f, offsetU, offsetV, 1,
                         // VERTEX 2
                         xPos, y1, z2, rcolor, gcolor, bcolor, ancho, 0.0f, offsetU, offsetV, 1,
                         // VERTEX 3
                         xPos, y2, z2, rcolor, gcolor, bcolor, ancho, alto, offsetU, offsetV, 1,
                         // VERTEX 4
                         xPos - 1.0f, y2, z1, rcolor, gcolor, bcolor, 0.0f, alto, offsetU, offsetV, 1};
    vData.insert(vData.end(), std::begin(vertex2), std::end(vertex2));
    base = base + 4;
    // Cara mirando hacia +z
    iData.push_back(base);     // Abajo derecha
    iData.push_back(base + 3); // Arriba derecha
    iData.push_back(base + 2); // Arriba izquierda
    iData.push_back(base);     // Abajo derecha
    iData.push_back(base + 2); // Arriba izquierda
    iData.push_back(base + 1); // Abajo derecha
    // Cara mirando -z
    iData.push_back(base + 1); // Abajo derecha
    iData.push_back(base + 2); // Arriba izquierda
    iData.push_back(base);     // Abajo derecha
    iData.push_back(base + 2); // Arriba izquierda
    iData.push_back(base + 3); // Arriba derecha
    iData.push_back(base);     // Abajo derecha
    vCount += 8;
}
void Chunk::cargarVertices(const Rectangulo &r, int eje, int direccion, int fijo, int16_t tipo_bloque,
                           vector<float> &vData, vector<unsigned int> &iData, unsigned int &vCount) {
    float base = vCount;
    float sizeOffset = 0.5f;
    float offsetX = nroChunkX * 16.0f;
    float offsetZ = nroChunkZ * 16.0f;
    float rcolor = 1.0f;
    float gcolor = 1.0f;
    float bcolor = 1.0f;
    if (tipo_bloque == OAK_LEAVES) {
        rcolor = 0.3f;
        gcolor = 0.8f;
        bcolor = 0.3f;
    } else if (tipo_bloque == SPRUCE_LEAVES) {
        rcolor = 0.3f;
        gcolor = 0.5f;
        bcolor = 0.3f;
    } else if (tipo_bloque == CACTUS) {
        sizeOffset = 0.4375f; // Si el bloque mide 1, cada pixel mide 1/16 => 0.5 - 1/16 (le
                              // quito un pixel de cada lado el cactus es 14*16*14)
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

    float ao0 = 1.0f;
    float ao1 = 1.0f;
    float ao2 = 1.0f;
    float ao3 = 1.0f;
    if (eje == 0) {
        float xPos = offsetX + fijo + (direccion == 1 ? sizeOffset : -sizeOffset);
        float y1 = r.y1 - 1.0f;
        float y2 = r.y2;
        float z1 = offsetZ + r.x1 - 0.5f;
        float z2 = offsetZ + r.x2 + 0.5f;
        float vertex[44] = {// VERTEX 1
                            xPos, y1, z1, rcolor, gcolor, bcolor, 0.0f, 0.0f, offsetU, offsetV, ao0,
                            // VERTEX 2
                            xPos, y1, z2, rcolor, gcolor, bcolor, ancho, 0.0f, offsetU, offsetV, ao1,
                            // VERTEX 3
                            xPos, y2, z2, rcolor, gcolor, bcolor, ancho, alto, offsetU, offsetV, ao2,
                            // VERTEX 4
                            xPos, y2, z1, rcolor, gcolor, bcolor, 0.0f, alto, offsetU, offsetV, ao3};
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
        if (tipo_bloque == GRASS_BLOCK) {
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
        } else if (tipo_bloque == OAK_LOG) {
            columna = 5;
            fila = 1;
        } else if (tipo_bloque == BOOKSHELF) {
            columna = 4;
            fila = 0;
        } else if (tipo_bloque == CACTUS) {
            columna = 5;
            fila = 4;
        } else if (tipo_bloque == SPRUCE_LOG) {
            columna = 5;
            fila = 1;
            rcolor = 0.55f;
            gcolor = 0.4f;
            bcolor = 0.3f;
        } else if (tipo_bloque == SNOWY_GRASS_BLOCK) {
            if (direccion == 1) {
                rcolor = 1.8f;
                gcolor = 1.8f;
                bcolor = 2.0f;
                columna = 8;
                fila = 4;
            } else {
                columna = 2;
                fila = 0;
            }
        } else if (tipo_bloque == TNT_BLOCK) {
            if (direccion == 1) {
                columna = 9;
                fila = 0;
            } else {
                columna = 10;
                fila = 0;
            }
        }
        offsetU = columna * tileSize;
        offsetV = 1.0f - (fila + 1) * tileSize;

        float yPos = fijo + (direccion == 1 ? 0.0f : -1.0f);
        float x1 = offsetX + r.y1 - 0.5f;
        float x2 = offsetX + r.y2 + 0.5f;
        float z1 = offsetZ + r.x1 - 0.5f;
        float z2 = offsetZ + r.x2 + 0.5f;
        float vertex[44] = {x1, yPos, z1, rcolor, gcolor, bcolor, 0.0f,  0.0f, offsetU, offsetV, ao0,
                            x2, yPos, z1, rcolor, gcolor, bcolor, ancho, 0.0f, offsetU, offsetV, ao1,
                            x2, yPos, z2, rcolor, gcolor, bcolor, ancho, alto, offsetU, offsetV, ao2,
                            x1, yPos, z2, rcolor, gcolor, bcolor, 0.0f,  alto, offsetU, offsetV, ao3};
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
        float zPos = offsetZ + fijo + (direccion == 1 ? sizeOffset : -sizeOffset);
        float y1 = r.x1 - 1.0f;
        float y2 = r.x2;
        float x1 = offsetX + r.y1 - 0.5f;
        float x2 = offsetX + r.y2 + 0.5f;

        float vertex[44] = {x1, y1, zPos, rcolor, gcolor, bcolor, 0.0f, 0.0f,  offsetU, offsetV, ao0,
                            x2, y1, zPos, rcolor, gcolor, bcolor, alto, 0.0f,  offsetU, offsetV, ao1,
                            x2, y2, zPos, rcolor, gcolor, bcolor, alto, ancho, offsetU, offsetV, ao2,
                            x1, y2, zPos, rcolor, gcolor, bcolor, 0.0f, ancho, offsetU, offsetV, ao3};
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
    vector<float> newVertexData;
    vector<unsigned int> newIndexData;
    unsigned int newVertexCount = 0;
    int baseX = nroChunkX * 16;
    int baseZ = nroChunkZ * 16;
    array<array<array<int16_t, 16>, 512>, 2> blocksInX;
    array<array<array<int16_t, 512>, 16>, 2> blocksInZ;
    blocksInX[0] = world->getFaceInXSafe(nroChunkX - 1, nroChunkZ, 15);
    blocksInX[1] = world->getFaceInXSafe(nroChunkX + 1, nroChunkZ, 0);
    blocksInZ[0] = world->getFaceInZSafe(nroChunkX, nroChunkZ - 1, 15);
    blocksInZ[1] = world->getFaceInZSafe(nroChunkX, nroChunkZ + 1, 0);
    lock_guard<mutex> lock(mutexBlocks);
    struct CrossBlock {
        int x, y, z;
        int16_t tipo;
    };
    vector<CrossBlock> crossBlock;
    for (int x = 0; x < 16; x++) {
        int16_t capasIzquierdas[512][16] = {0};
        int16_t capasDerechas[512][16] = {0};
        for (int i = 0; i < 512; i++) {
            for (int j = 0; j < 16; j++) {
                int16_t typeblock = blocks[x][i][j];
                if (isCrossBlock(typeblock)) {
                    crossBlock.push_back({x, i, j, typeblock});
                    continue;
                }
                if (typeblock != 0) {
                    int globalX = x + baseX;
                    int globalZ = j + baseZ;
                    // Cara derecha (x+)
                    if (x == 15) {
                        if (esTransparent(blocksInX[1][i][j]))
                            capasDerechas[i][j] = blocks[x][i][j];
                    } else if (esTransparent(blocks[x + 1][i][j])) {
                        capasDerechas[i][j] = blocks[x][i][j];
                    }
                    // Cara izquierda (x-)
                    if (x == 0) {
                        if (esTransparent(blocksInX[0][i][j]))
                            capasIzquierdas[i][j] = blocks[x][i][j];

                    } else if (esTransparent(blocks[x - 1][i][j])) {
                        capasIzquierdas[i][j] = blocks[x][i][j];
                    }
                }
            }
        }
        vector<Rectangulo> rectsDer = formarRectangulos(capasDerechas);
        for (const Rectangulo &r : rectsDer) {
            cargarVertices(r, 0, 1, x, r.tipoBloque, newVertexData, newIndexData, newVertexCount);
        }
        vector<Rectangulo> rectsIzq = formarRectangulos(capasIzquierdas);

        for (const Rectangulo &r : rectsIzq) {
            cargarVertices(r, 0, -1, x, r.tipoBloque, newVertexData, newIndexData, newVertexCount);
        }
    }
    for (int y = 0; y < 512; y++) {
        int16_t capasSuperiores[16][16] = {0};
        int16_t capasInferiores[16][16] = {0};
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                int16_t typeblock = blocks[i][y][j];
                if (isCrossBlock(typeblock)) {
                    continue;
                }
                if (typeblock != 0) {
                    // Cara inferior (y-)
                    if ((y == 0) || esTransparent(blocks[i][y - 1][j])) {
                        capasInferiores[i][j] = blocks[i][y][j];
                    }
                    // Cara superior (y+)
                    if ((y == 511) || esTransparent(blocks[i][y + 1][j])) {
                        capasSuperiores[i][j] = blocks[i][y][j];
                    }
                }
            }
        }
        vector<Rectangulo> rectsInf = formarRectangulos(capasInferiores);
        for (const Rectangulo &r : rectsInf) {
            cargarVertices(r, 1, -1, y, r.tipoBloque, newVertexData, newIndexData, newVertexCount);
        }
        vector<Rectangulo> rectsSup = formarRectangulos(capasSuperiores);

        for (const Rectangulo &r : rectsSup) {
            cargarVertices(r, 1, 1, y, r.tipoBloque, newVertexData, newIndexData, newVertexCount);
        }
    }
    for (int z = 0; z < 16; z++) {
        int16_t capasFrontal[16][512] = {0};
        int16_t capasTrasera[16][512] = {0};

        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 512; j++) {
                int16_t typeblock = blocks[i][j][z];
                if (isCrossBlock(typeblock)) {
                    continue;
                }
                if (typeblock != 0) {
                    int globalX = i + baseX;
                    int globalZ = z + baseZ;
                    // z+
                    if (z == 15) {
                        if (esTransparent(blocksInZ[1][i][j]))
                            capasFrontal[i][j] = blocks[i][j][z];
                    } else if (esTransparent(blocks[i][j][z + 1])) {
                        capasFrontal[i][j] = blocks[i][j][z];
                    }
                    // Cara izquierda (Z-)
                    if (z == 0) {
                        if (esTransparent(blocksInZ[0][i][j]))
                            capasTrasera[i][j] = blocks[i][j][z];
                    } else if (esTransparent(blocks[i][j][z - 1])) {
                        capasTrasera[i][j] = blocks[i][j][z];
                    }
                }
            }
        }

        vector<Rectangulo> rectsFrontal = formarRectangulos(capasFrontal);
        for (const Rectangulo &r : rectsFrontal) {
            cargarVertices(r, 2, 1, z, r.tipoBloque, newVertexData, newIndexData, newVertexCount);
        }

        vector<Rectangulo> rectsTrasera = formarRectangulos(capasTrasera);
        for (const Rectangulo &r : rectsTrasera) {
            cargarVertices(r, 2, -1, z, r.tipoBloque, newVertexData, newIndexData, newVertexCount);
        }
    }
    int crosSize = crossBlock.size();
    for (const auto &cb : crossBlock) {
        Rectangulo r;
        r.x1 = cb.z;
        r.x2 = cb.z;
        r.y1 = cb.y;
        r.y2 = cb.y;
        r.tipoBloque = cb.tipo;
        cargarVerticesCross(r, cb.x, cb.tipo, newVertexData, newIndexData, newVertexCount);
    }
    {
        std::lock_guard<std::mutex> lock(mutexVertex);
        vertexData.swap(newVertexData);
        indexData.swap(newIndexData);
        vertexCount = newVertexCount;
    }
    needsUpdate = false;
    needsBufferUpdate = true;
}
void Chunk::setBlock(int x, int y, int z, const int16_t &block) {
    lock_guard<mutex> lock(mutexBlocks);
    blocks[x][y][z] = block;
    needsUpdate = true;
}
int16_t Chunk::getBlock(int x, int y, int z) const {
    if (x < 0 || x >= 16 || z < 0 || z >= 16) {
        return 0;
    }
    return blocks[x][y][z];
}
bool Chunk::isEmpty() const { return vertexData.empty(); }
void Chunk::setNroChunk(int chunkx, int chunkz) {
    nroChunkX = chunkx;
    nroChunkZ = chunkz;
}
void Chunk::render() {
    if (needsUpdate && !isUpdating) {
        generateMesh();
    }
    if (needsBufferUpdate) {
        lock_guard<mutex> lock(mutexVertex);
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
