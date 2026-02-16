#include "../include/world.h"
#include <cmath>
#include <glm/ext/vector_int2.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

template <size_t FILAS, size_t COLUMNAS> vector<Rectangulo> formarRectangulos(bool (&mascara)[FILAS][COLUMNAS]) {
    vector<Rectangulo> rectangulos;
    bool procesado[FILAS][COLUMNAS] = {false};
    for (int i = 0; i < FILAS; i++) {
        for (int j = 0; j < COLUMNAS; j++) {
            if (mascara[i][j] && !procesado[i][j]) {
                int ancho = 1;
                while (j + ancho < COLUMNAS && mascara[i][j + ancho] && !procesado[i][j + ancho]) {
                    procesado[i][j + ancho] = true;
                    ancho++;
                }
                int alto = 1;
                bool expandible = true;
                while (i + alto < FILAS && expandible) {
                    for (int dj = 0; dj < ancho; dj++) {
                        if (!mascara[i + alto][j + dj] || procesado[i + alto][j + dj]) {
                            expandible = false;
                            break;
                        } else {
                            procesado[i + alto][j + dj] = true;
                        }
                    }
                    if (expandible)
                        alto++;
                }
                Rectangulo r;
                r.y1 = i;
                r.x1 = j;
                r.y2 = i + alto - 1;
                r.x2 = j + ancho - 1;
                rectangulos.push_back(r);
            }
        }
    }
    return rectangulos;
}

Shader* Chunk::sharedShader = nullptr;
using namespace std;
Chunk::Chunk() {
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
void Chunk::cargarVertices(const Rectangulo& r, int eje, int direccion, int fijo) {
    float base = vertexCount;
    float offsetX = nroChunkX * 16;
    float offsetZ = nroChunkZ * 16;
    // Los vertices son
    //(y2,z2)------(y2,z1)
    //     |
    //     |
    //     |
    //     |
    //     |
    //(y1,z2)------(y1,z1)
    // Hay que generarlos antihorarios

    if (eje == 0) {
        float xPos = offsetX + fijo + (direccion == 1 ? 0.5f : -0.5f);

        float y1 = r.y1 - 1.0f; // Base
        float y2 = r.y2;        // Techo
        float z1 = offsetZ + r.x1 - 0.5f;
        float z2 = offsetZ + r.x2 + 0.5f;

        // vértices en orden: inferior-izquierdo, inferior-derecho,
        // superior-derecho, superior-izquierdo
        vertexData.push_back(xPos);
        vertexData.push_back(y1);
        vertexData.push_back(z1); // v0
        vertexData.push_back(xPos);
        vertexData.push_back(y1);
        vertexData.push_back(z2); // v1
        vertexData.push_back(xPos);
        vertexData.push_back(y2);
        vertexData.push_back(z2); // v2
        vertexData.push_back(xPos);
        vertexData.push_back(y2);
        vertexData.push_back(z1); // v3

        // índices: dos triángulos (v0-v1-v2 y v0-v2-v3)
        indexData.push_back(base);
        indexData.push_back(base + 1);
        indexData.push_back(base + 2);

        indexData.push_back(base);
        indexData.push_back(base + 2);
        indexData.push_back(base + 3);
    } else if (eje == 1) {
        float yPos = fijo + (direccion == 1 ? 0.0f : -1.0f);

        // r.x1, r.x2 son X
        // r.y1, r.y2 son Z
        float x1 = offsetX + r.y1 - 0.5f;
        float x2 = offsetX + r.y2 + 0.5f;
        float z1 = offsetZ + r.x1 - 0.5f;
        float z2 = offsetZ + r.x2 + 0.5f;

        // vértices en orden: (x1,z1), (x2,z1), (x2,z2), (x1,z2)
        vertexData.push_back(x1);
        vertexData.push_back(yPos);
        vertexData.push_back(z1); // v0
        vertexData.push_back(x2);
        vertexData.push_back(yPos);
        vertexData.push_back(z1); // v1
        vertexData.push_back(x2);
        vertexData.push_back(yPos);
        vertexData.push_back(z2); // v2
        vertexData.push_back(x1);
        vertexData.push_back(yPos);
        vertexData.push_back(z2); // v3

        // índices (dos triángulos: v0-v1-v2 y v0-v2-v3)
        indexData.push_back(base);
        indexData.push_back(base + 1);
        indexData.push_back(base + 2);

        indexData.push_back(base);
        indexData.push_back(base + 2);
        indexData.push_back(base + 3);
    } else {
        float zPos = offsetZ + fijo + (direccion == 1 ? 0.5f : -0.5f);

        float y1 = r.x1 - 1.0f; // Base
        float y2 = r.x2;        // Techo
        float x1 = offsetX + r.y1 - 0.5f;
        float x2 = offsetX + r.y2 + 0.5f;

        // vértices en orden: inferior-izquierdo, inferior-derecho,
        // superior-derecho, superior-izquierdo
        vertexData.push_back(x1);
        vertexData.push_back(y1);
        vertexData.push_back(zPos); // v0
        vertexData.push_back(x2);
        vertexData.push_back(y1);
        vertexData.push_back(zPos); // v1
        vertexData.push_back(x2);
        vertexData.push_back(y2);
        vertexData.push_back(zPos); // v2
        vertexData.push_back(x1);
        vertexData.push_back(y2);
        vertexData.push_back(zPos); // v3

        // índices: dos triángulos (v0-v1-v2 y v0-v2-v3)
        indexData.push_back(base);
        indexData.push_back(base + 1);
        indexData.push_back(base + 2);

        indexData.push_back(base);
        indexData.push_back(base + 2);
        indexData.push_back(base + 3);
    }

    vertexCount += 4;
}
void Chunk::generateMesh() {
    if (!needsUpdate)
        return;
    vertexData.clear();
    indexData.clear();
    vertexCount = 0;

    for (int x = 0; x < 16; x++) {
        bool capasIzquierdas[256][16];
        bool capasDerechas[256][16];
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 16; j++) {
                if (blocks[x][i][j].active && blocks[x][i][j].type != 0) {
                    capasDerechas[i][j] = (x == 15) || !blocks[x + 1][i][j].active;
                    capasIzquierdas[i][j] = (x == 0) || !blocks[x - 1][i][j].active;
                } else {
                    capasDerechas[i][j] = false;
                    capasIzquierdas[i][j] = false;
                }
            }
        }
        vector<Rectangulo> rectsDer = formarRectangulos(capasDerechas);
        for (const Rectangulo& r : rectsDer) {
            cargarVertices(r, 0, 1, x);
        }
        vector<Rectangulo> rectsIzq = formarRectangulos(capasIzquierdas);

        for (const Rectangulo& r : rectsIzq) {
            cargarVertices(r, 0, -1, x);
        }
    }
    for (int y = 0; y < 256; y++) {
        bool capasSuperiores[16][16];
        bool capasInferiores[16][16];
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                if (blocks[i][y][j].active && blocks[i][y][j].type != 0) {
                    capasInferiores[i][j] = (y == 0) || !blocks[i][y - 1][j].active;
                    capasSuperiores[i][j] = (y == 255) || !blocks[i][y + 1][j].active;
                } else {
                    capasInferiores[i][j] = false;
                    capasSuperiores[i][j] = false;
                }
            }
        }
        vector<Rectangulo> rectsInf = formarRectangulos(capasInferiores);
        for (const Rectangulo& r : rectsInf) {
            cargarVertices(r, 1, -1, y);
        }
        vector<Rectangulo> rectsSup = formarRectangulos(capasSuperiores);

        for (const Rectangulo& r : rectsSup) {
            cargarVertices(r, 1, 1, y);
        }
    }
    for (int z = 0; z < 16; z++) {
        bool capasFrontal[16][256] = {false};
        bool capasTrasera[16][256] = {false};

        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 256; j++) {
                if (blocks[i][j][z].active) {
                    capasFrontal[i][j] = (z == 15) || !blocks[i][j][z + 1].active;
                    capasTrasera[i][j] = (z == 0) || !blocks[i][j][z - 1].active;
                }
            }
        }

        vector<Rectangulo> rectsFrontal = formarRectangulos(capasFrontal);
        for (const Rectangulo& r : rectsFrontal) {
            cargarVertices(r, 2, 1, z);
        }

        vector<Rectangulo> rectsTrasera = formarRectangulos(capasTrasera);
        for (const Rectangulo& r : rectsTrasera) {
            cargarVertices(r, 2, -1, z);
        }
    }

    needsUpdate = false;
}
void Chunk::generateMeshBlock() {
    if (!needsUpdate)
        return;
    vertexData.clear();
    indexData.clear();
    int vertexCount = 0;
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 256; y++) {
            for (int z = 0; z < 16; z++) {
                if (!blocks[x][y][z].active)
                    continue;
                int cordx = x + nroChunkX * 16;
                int cordz = z + nroChunkZ * 16;
                // Verificar las 6 caras
                // Cara frontal
                if (z == 15 || blocks[x][y][z + 1].type == 0) {
                    vertexData.push_back(cordx - 0.5f);
                    vertexData.push_back(y - 0.5f);
                    vertexData.push_back(cordz + 0.5f);

                    vertexData.push_back(cordx + 0.5f);
                    vertexData.push_back(y - 0.5f);
                    vertexData.push_back(cordz + 0.5f);

                    vertexData.push_back(cordx + 0.5f);
                    vertexData.push_back(y + 0.5f);
                    vertexData.push_back(cordz + 0.5f);

                    vertexData.push_back(cordx - 0.5f);
                    vertexData.push_back(y + 0.5f);
                    vertexData.push_back(cordz + 0.5f);
                    int base = vertexCount;
                    indexData.push_back(base);
                    indexData.push_back(base + 1);
                    indexData.push_back(base + 2);
                    indexData.push_back(base);
                    indexData.push_back(base + 2);
                    indexData.push_back(base + 3);

                    vertexCount += 4;
                }
                // Cara trasera
                if (z == 0 || blocks[x][y][z - 1].type == 0) {
                    vertexData.push_back(cordx - 0.5f);
                    vertexData.push_back(y - 0.5f);
                    vertexData.push_back(cordz - 0.5f);

                    // v1: (x+0.5, y-0.5, z-0.5)
                    vertexData.push_back(cordx + 0.5f);
                    vertexData.push_back(y - 0.5f);
                    vertexData.push_back(cordz - 0.5f);

                    // v2: (x+0.5, y+0.5, z-0.5)
                    vertexData.push_back(cordx + 0.5f);
                    vertexData.push_back(y + 0.5f);
                    vertexData.push_back(cordz - 0.5f);

                    // v3: (x-0.5, y+0.5, z-0.5)
                    vertexData.push_back(cordx - 0.5f);
                    vertexData.push_back(y + 0.5f);
                    vertexData.push_back(cordz - 0.5f);

                    int base = vertexCount;
                    indexData.push_back(base);
                    indexData.push_back(base + 2);
                    indexData.push_back(base + 1);
                    indexData.push_back(base);
                    indexData.push_back(base + 3);
                    indexData.push_back(base + 2);

                    vertexCount += 4;
                }
                // Cara derecha
                if (x == 15 || blocks[x + 1][y][z].type == 0) {
                    // v0: (x+0.5, y-0.5, z-0.5)
                    vertexData.push_back(cordx + 0.5f);
                    vertexData.push_back(y - 0.5f);
                    vertexData.push_back(cordz - 0.5f);

                    // v1: (x+0.5, y-0.5, z+0.5)
                    vertexData.push_back(cordx + 0.5f);
                    vertexData.push_back(y - 0.5f);
                    vertexData.push_back(cordz + 0.5f);

                    // v2: (x+0.5, y+0.5, z+0.5)
                    vertexData.push_back(x + 0.5f);
                    vertexData.push_back(y + 0.5f);
                    vertexData.push_back(z + 0.5f);

                    // v3: (x+0.5, y+0.5, z-0.5)
                    vertexData.push_back(cordx + 0.5f);
                    vertexData.push_back(y + 0.5f);
                    vertexData.push_back(cordz - 0.5f);

                    int base = vertexCount;
                    indexData.push_back(base);
                    indexData.push_back(base + 1);
                    indexData.push_back(base + 2);
                    indexData.push_back(base);
                    indexData.push_back(base + 2);
                    indexData.push_back(base + 3);

                    vertexCount += 4;
                }

                // Cara izquierda
                if (x == 0 || blocks[x - 1][y][z].type == 0) {
                    // v0: (x-0.5, y-0.5, z+0.5)
                    vertexData.push_back(cordx - 0.5f);
                    vertexData.push_back(y - 0.5f);
                    vertexData.push_back(cordz + 0.5f);

                    // v1: (x-0.5, y-0.5, z-0.5)
                    vertexData.push_back(cordx - 0.5f);
                    vertexData.push_back(y - 0.5f);
                    vertexData.push_back(cordz - 0.5f);

                    // v2: (x-0.5, y+0.5, z-0.5)
                    vertexData.push_back(x - 0.5f);
                    vertexData.push_back(y + 0.5f);
                    vertexData.push_back(z - 0.5f);

                    // v3: (x-0.5, y+0.5, z+0.5)
                    vertexData.push_back(cordx - 0.5f);
                    vertexData.push_back(y + 0.5f);
                    vertexData.push_back(cordz + 0.5f);

                    int base = vertexCount;
                    indexData.push_back(base);
                    indexData.push_back(base + 1);
                    indexData.push_back(base + 2);
                    indexData.push_back(base);
                    indexData.push_back(base + 2);
                    indexData.push_back(base + 3);

                    vertexCount += 4;
                }

                // Cara superior
                if (y == 255 || blocks[x][y + 1][z].type == 0) {
                    // v0: (x-0.5, y+0.5, z+0.5)
                    vertexData.push_back(cordx - 0.5f);
                    vertexData.push_back(y + 0.5f);
                    vertexData.push_back(cordz + 0.5f);

                    // v1: (x+0.5, y+0.5, z+0.5)
                    vertexData.push_back(cordx + 0.5f);
                    vertexData.push_back(y + 0.5f);
                    vertexData.push_back(cordz + 0.5f);

                    // v2: (x+0.5, y+0.5, z-0.5)
                    vertexData.push_back(cordx + 0.5f);
                    vertexData.push_back(y + 0.5f);
                    vertexData.push_back(cordz - 0.5f);

                    // v3: (x-0.5, y+0.5, z-0.5)
                    vertexData.push_back(cordx - 0.5f);
                    vertexData.push_back(y + 0.5f);
                    vertexData.push_back(cordz - 0.5f);

                    int base = vertexCount;
                    indexData.push_back(base);
                    indexData.push_back(base + 1);
                    indexData.push_back(base + 2);
                    indexData.push_back(base);
                    indexData.push_back(base + 2);
                    indexData.push_back(base + 3);

                    vertexCount += 4;
                }

                // Cara inferior
                if (y == 0 || blocks[x][y - 1][z].type == 0) {
                    // v0: (x-0.5, y-0.5, z-0.5)
                    vertexData.push_back(cordx - 0.5f);
                    vertexData.push_back(y - 0.5f);
                    vertexData.push_back(cordz - 0.5f);

                    // v1: (x+0.5, y-0.5, z-0.5)
                    vertexData.push_back(cordx + 0.5f);
                    vertexData.push_back(y - 0.5f);
                    vertexData.push_back(cordz - 0.5f);

                    // v2: (x+0.5, y-0.5, z+0.5)
                    vertexData.push_back(cordx + 0.5f);
                    vertexData.push_back(y - 0.5f);
                    vertexData.push_back(cordz + 0.5f);

                    // v3: (x-0.5, y-0.5, z+0.5)
                    vertexData.push_back(cordx - 0.5f);
                    vertexData.push_back(y - 0.5f);
                    vertexData.push_back(cordz + 0.5f);

                    int base = vertexCount;
                    indexData.push_back(base);
                    indexData.push_back(base + 2);
                    indexData.push_back(base + 1);
                    indexData.push_back(base);
                    indexData.push_back(base + 3);
                    indexData.push_back(base + 2);

                    vertexCount += 4;
                }
            }
        }
    }
    needsUpdate = false;
}

void Chunk::setBlock(int x, int y, int z, const Block& block) {
    if (x < 0 || x >= 16 || y < 0 || y >= 256 || z < 0 || z >= 16) {
        return;
    }
    blocks[x][y][z] = block;
    needsUpdate = true;
}
bool Chunk::isEmpty() const {
    return vertexData.empty();
}
void Chunk::setNroChunk(int chunkx, int chunkz) {
    nroChunkX = chunkx;
    nroChunkZ = chunkz;
}
void Chunk::render(const mat4& view) {
    if (needsUpdate) {
        generateMesh();
        if (!vertexData.empty()) {
            chunkBuffer.uploadData(vertexData, indexData);
        }
        needsUpdate = false;
    }
    if (vertexData.empty())
        return;
    sharedShader->use();
    sharedShader->setViewMatrix(glm::value_ptr(view));
    chunkBuffer.render();
}
World::World() {
}
void World::generateFlatWorld(int width, int depth) {
    int chunksInX = (width + 15) / 16;
    int chunksInZ = (depth + 15) / 16;
    for (int cx = 0; cx < chunksInX; cx++) {
        for (int cz = 0; cz < chunksInZ; cz++) {
            Chunk& chunk = chunks[cx][cz];
            chunk.setNroChunk(cx, cz);
            for (int x = 0; x < 16; x++) {
                for (int z = 0; z < 16; z++) {
                    for (int y = 0; y < 255; y++) {
                        int wordlX = cx * 16 + x;
                        int wordlZ = cz * 16 + z;
                        if (wordlX < width && wordlZ < depth) {
                            Block block;
                            block.active = true;
                            block.type = 1;
                            chunk.setBlock(x, y, z, block);
                        }
                    }
                }
            }
        }
    }
}
Chunk* World::getChunk(int x, int z) {
    auto itX = chunks.find(x);
    if (itX == chunks.end())
        return nullptr;
    auto itZ = itX->second.find(z);
    if (itZ == itX->second.end())
        return nullptr;
    return &itZ->second;
}
ivec2 World::getChunkPos(vec3 worldPos) {
    int chunkX = (int)floor(worldPos.x / 16.0f);
    int chunkZ = (int)floor(worldPos.z / 16.0f);
    return ivec2(chunkX, chunkZ);
}
void World::render(vec3 cameraPos, mat4 view) {
    ivec2 centerChunk = getChunkPos(cameraPos);
    int renderDist = 8;
    for (int dx = -renderDist; dx <= renderDist; dx++) {
        for (int dz = -renderDist; dz <= renderDist; dz++) {
            Chunk* chunk = getChunk(centerChunk.x + dx, centerChunk.y + dz);
            if (chunk) {
                chunk->render(view);
            }
        }
    }
}
void World::update() {
}
