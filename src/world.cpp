#include "../include/world.h"
#include <glm/ext/vector_int2.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
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
void Chunk::generateMesh() {
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
                    vertexData.push_back(cordx + 0.5f);
                    vertexData.push_back(y + 0.5f);
                    vertexData.push_back(cordz + 0.5f);

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
                    vertexData.push_back(cordx - 0.5f);
                    vertexData.push_back(y + 0.5f);
                    vertexData.push_back(cordz - 0.5f);

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
                    int wordlX = cx * 16 + x;
                    int wordlZ = cz * 16 + z;
                    if (wordlX < width && wordlZ < depth) {
                        Block block;
                        block.active = true;
                        block.type = 1;
                        chunk.setBlock(x, 0, z, block);
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
    int renderDist = 32;
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
