#include "../include/world.h"

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
                            if (y < 64) {
                                block.type = 2;
                            } else if (y == 64) {
                                block.type = 4;
                            } else {
                                block.type = 0;
                                block.active = false;
                            }
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
Block World::getBlock(int x, int y, int z) {
    vec3 pos = {x, y, z};
    ivec2 posChunk = getChunkPos(pos);
    Chunk* chunk = getChunk(posChunk.x, posChunk.y);
    int offsetX = chunk->getNroChunkX() * 16;
    int offsetZ = chunk->getNroChunkZ() * 16;
    Block copia = chunk->getBlock(x - offsetX, y, z - offsetZ);
    return copia;
}
void World::render(vec3 cameraPos, mat4 view) {
    ivec2 centerChunk = getChunkPos(cameraPos);
    int cantRect = 0;
    int renderDist = 32;
    Chunk::sharedShader->use();
    Chunk::sharedShader->setUseTexture(true);
    Chunk::sharedShader->setViewMatrix(glm::value_ptr(view));
    for (int dx = -renderDist; dx <= renderDist; dx++) {
        for (int dz = -renderDist; dz <= renderDist; dz++) {
            Chunk* chunk = getChunk(centerChunk.x + dx, centerChunk.y + dz);
            if (chunk) {
                chunk->render();
                cantRect += chunk->getCaras();
            }
        }
    }
    // cout << "Cantidad de rectangulos generados: " << cantRect << endl;
}
void World::update() {
}
