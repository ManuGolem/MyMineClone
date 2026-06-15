#include "../include/world.h"
#include "../include/blocksRegistry.h"
#include <cstdint>
#include <cstdlib>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <memory>
#include <mutex>
#include <thread>
World::World() {
    // Configurar ruidos para altura
    terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    terrainNoise.SetFractalType(FastNoiseLite::FractalType_FBm);

    erosionNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

    detailNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

    // Configurar ruidos para biomas
    temperatureNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    humidityNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
}
void World::generateTree(shared_ptr<Chunk> chunk, int posX, int groundY, int posZ, int worldX, int worldZ, uint32_t hash, int treeType) {
    int trunkHeight;
    int leafRadius;
    int baseBlock;
    int leafStart;
    int leaf;
    if (treeType == 21) {
        leafRadius = 2;
        trunkHeight = 4 + (hash % 3);
        leafStart = groundY + trunkHeight - 2;
        baseBlock = 4;
        leaf = 53;
    } else if (treeType == 71) {
        trunkHeight = 2 + (hash % 1);
        baseBlock = 19;
    } else if (treeType == 117) {
        leafRadius = 3;
        trunkHeight = 6 + (hash % 3);
        baseBlock = 3;
        leafStart = groundY + trunkHeight - 6;
        leaf = 133;
    }
    if (chunk->getBlock(posX, groundY, posZ) != baseBlock) {
        return;
    }
    for (int y = 1; y <= trunkHeight + 1; y++) {
        if (chunk->getBlock(posX, groundY + y, posZ) != 0) {
            return;
        }
    }
    for (int y = 0; y < trunkHeight; y++) {
        chunk->setBlock(posX, groundY + y + 1, posZ, treeType);
    }

    if (treeType == 71) {
        return; // Cactus no necesita hojas
    }
    // Hojas
    for (int y = leafStart; y <= groundY + trunkHeight + 1; y++) {
        int dy = y - (groundY + trunkHeight);
        int currentRadius = leafRadius - abs(dy);
        for (int dx = -currentRadius; dx <= currentRadius; dx++) {
            for (int dz = -currentRadius; dz <= currentRadius; dz++) {
                float dist = sqrt(dx * dx + dz * dz + dy * dy);
                if (dist > leafRadius + 0.5f)
                    continue;
                // esquinas con probabilidad para hacerlo más orgánico
                if (abs(dx) == currentRadius && abs(dz) == currentRadius && rand() % 100 < 40)
                    continue;
                if (dx == dz && dx == 0 && dy != 1) {
                    continue;
                }
                int localX = posX + dx;
                int localZ = posZ + dz;
                if (localX < 0 || localX >= 16 || localZ < 0 || localZ >= 16) {
                    ivec2 pos = getChunkPos({worldX + dx, y, worldZ + dz});
                    int chunkX = pos.x;
                    int chunkZ = pos.y;
                    int futureLocalX = worldX + dx - chunkX * 16;
                    int futureLocalZ = worldZ + dz - chunkZ * 16;
                    shared_ptr<Chunk> chunkFuture;
                    {
                        lock_guard<mutex> lock(mapChunks);
                        chunkFuture = getChunk(pos.x, pos.y);
                    }
                    if (chunkFuture) {
                        chunkFuture->setBlock(futureLocalX, y, futureLocalZ, leaf);
                        if (!chunkFuture->isUpdating) {
                            chunkFuture->isUpdating = true;
                            {
                                lock_guard<mutex> lock(mutexChunkUpdate);
                                chunkRequestUpdate.push({pos.x, pos.y});
                            }
                            meshCV.notify_one();
                        }
                    } else {
                        pendingBlock futureBlock;
                        futureBlock.block = leaf;
                        futureBlock.x = futureLocalX;
                        futureBlock.y = y;
                        futureBlock.z = futureLocalZ;
                        lock_guard<mutex> lock(mutexPendingBlocks);
                        pendingBlocks[chunkX][chunkZ].push_back(futureBlock);
                    }
                } else {
                    if (chunk->getBlock(localX, y, localZ) == 0)
                        chunk->setBlock(localX, y, localZ, leaf);
                }
            }
        }
    }
}
BiomeType World::getBiome(int worldX, int worldZ, int height) {
    float rawTemp = temperatureNoise.GetNoise((float)worldX, (float)worldZ);
    float rawHumidity = humidityNoise.GetNoise((float)worldX, (float)worldZ);
    float temp = (rawTemp + 1.0f) * 0.5f;
    float hum = (rawHumidity + 1.0f) * 0.5f;

    float heightFactor = clamp((height - 64.0f) / 128.0f, 0.0f, 1.0f);
    temp -= heightFactor * 0.2f;
    hum -= heightFactor * 0.1f;
    // Valores totalmente de prueba
    if (height < 62) {
        return ocean;
    }
    if (temp < 0.25f && height > 85) {
        return mountains;
    }
    if (temp > 0.55f && hum < 0.4f && height < 90) {
        return desert;
    }
    if (hum > 0.55f && temp > 0.3f && temp < 0.7f && height < 100) {
        return forest;
    }
    return plains;
}
int World::getTerrainHeight(int worldX, int worldZ) {
    float continent = (terrainNoise.GetNoise((float)worldX, (float)worldZ) + 1.0f) * 0.5f;
    float erosion = (erosionNoise.GetNoise((float)worldX, (float)worldZ) + 1.0f) * 0.5f;
    float detail = (detailNoise.GetNoise((float)worldX, (float)worldZ) + 1.0f) * 0.5f;

    float baseHeight;

    if (continent < 0.3f) {
        // Océano (38-60)
        baseHeight = 38 + (continent / 0.3f) * 22;
    } else if (continent < 0.6f) {
        // Tierra baja (60-90)
        float t = (continent - 0.3f) / 0.3f;
        baseHeight = 60 + t * 30;
    } else if (continent < 0.7f) {
        // Colinas (90-150)
        float t = (continent - 0.6f) / 0.1f;
        baseHeight = 90 + t * 60;
    } else {
        // Montañas (150-450)
        float t = (continent - 0.7f) / 0.3f;
        t = t * t;
        baseHeight = 150 + t * 300;
    }
    // Valores totalmente de prueba
    float erosionEffect = (erosion - 0.5f) * 20;
    float detailEffect = (detail - 0.5f) * 8;
    float height = baseHeight + erosionEffect + detailEffect;
    return (int)height;
}
void World::generateWorldWithPerlin(int newSeed) {
    worldSeed = newSeed;
    terrainNoise.SetSeed(newSeed);
    terrainNoise.SetFrequency(0.003);
    terrainNoise.SetFractalOctaves(5);
    terrainNoise.SetFractalGain(0.5f);

    erosionNoise.SetSeed(newSeed + 1);
    erosionNoise.SetFrequency(0.008f);
    erosionNoise.SetFractalOctaves(3);

    detailNoise.SetSeed(newSeed + 2);
    detailNoise.SetFrequency(0.02f);
    detailNoise.SetFractalOctaves(2);

    temperatureNoise.SetSeed(newSeed + 3);
    temperatureNoise.SetFrequency(0.0005);
    temperatureNoise.SetFractalOctaves(3);
    temperatureNoise.SetFractalGain(0.5f);

    humidityNoise.SetSeed(newSeed + 4);
    humidityNoise.SetFrequency(0.0005);
    humidityNoise.SetFractalOctaves(3);
    humidityNoise.SetFractalGain(0.5f);

    createChunk(0, 0);
}

void World::generateFlatWorld(int width, int depth) {
    int chunksInX = (width + 15) / 16;
    int chunksInZ = (depth + 15) / 16;
    for (int cx = 0; cx < chunksInX; cx++) {
        for (int cz = 0; cz < chunksInZ; cz++) {
            chunks[cx][cz] = make_shared<Chunk>();
            Chunk& chunk = *chunks[cx][cz];
            chunk.setNroChunk(cx, cz);
            chunk.setWorld(this);
            for (int x = 0; x < 16; x++) {
                for (int z = 0; z < 16; z++) {
                    for (int y = 0; y < 512; y++) {
                        int wordlX = cx * 16 + x;
                        int wordlZ = cz * 16 + z;
                        if (wordlX < width && wordlZ < depth) {
                            int block;
                            if (y < 64) {
                                block = 2;
                            } else if (y == 64) {
                                block = 2;
                            } else {
                                block = 0;
                            }
                            chunk.setBlock(x, y, z, block);
                        }
                    }
                }
            }
        }
    }
}
void World::deleteWorld() {
    for (auto& xPair : chunks) {
        for (auto& zPair : xPair.second) {
            if (zPair.second)
                zPair.second->cleanup();
        }
    }
    chunks.clear();
}
shared_ptr<Chunk> World::getChunk(int x, int z) {
    auto itX = chunks.find(x);
    if (itX == chunks.end())
        return nullptr;
    auto itZ = itX->second.find(z);
    if (itZ == itX->second.end())
        return nullptr;
    return itZ->second;
}
ivec2 World::getChunkPos(vec3 worldPos) {
    int chunkX = (int)floor(worldPos.x / 16.0f);
    int chunkZ = (int)floor(worldPos.z / 16.0f);
    return ivec2(chunkX, chunkZ);
}
vector<pendingBlock> World::getPendingBlocksForChunk(int x, int z) {
    vector<pendingBlock> blocks;
    auto itX = pendingBlocks.find(x);
    if (itX != pendingBlocks.end()) {
        auto itZ = itX->second.find(z);
        if (itZ != itX->second.end()) {
            blocks = std::move(itZ->second);
            itX->second.erase(itZ);
        }
    }
    return blocks;
}
array<array<int16_t, 16>, 512> World::getFaceInXSafe(int nroChunkX, int nroChunkZ, int facex) {
    array<array<int16_t, 16>, 512> facesInX;
    lock_guard<mutex> lock(mapChunks);
    auto chunk = getChunk(nroChunkX, nroChunkZ);
    if (!chunk)
        return facesInX;

    for (int y = 0; y < 512; y++) {
        for (int z = 0; z < 16; z++) {
            facesInX[y][z] = chunk->getBlock(facex, y, z);
        }
    }

    return facesInX;
}

std::array<std::array<int16_t, 512>, 16> World::getFaceInZSafe(int nroChunkX, int nroChunkZ, int facez) {
    std::array<std::array<int16_t, 512>, 16> faceInZ;
    lock_guard<mutex> lock(mapChunks);
    auto chunk = getChunk(nroChunkX, nroChunkZ);
    if (!chunk)
        return faceInZ;

    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 512; y++) {
            faceInZ[x][y] = chunk->getBlock(x, y, facez);
        }
    }

    return faceInZ;
}
int16_t World::getBlockSafe(int x, int y, int z) {
    ivec2 posChunk = getChunkPos(vec3(x, y, z));
    lock_guard<mutex> lock(mapChunks);
    auto chunk = getChunk(posChunk.x, posChunk.y);
    if (!chunk || y > 512 || y < 0) {
        return 0;
    }
    int offsetX = chunk->getNroChunkX() * 16;
    int offsetZ = chunk->getNroChunkZ() * 16;
    return chunk->getBlock(x - offsetX, y, z - offsetZ);
}
void World::setBlockSafe(int x, int y, int z, int block) {
    ivec2 posChunk = getChunkPos(vec3(x, y, z));
    shared_ptr<Chunk> chunk;
    lock_guard<mutex> lock(mapChunks);
    chunk = getChunk(posChunk.x, posChunk.y);
    if (chunk) {
        chunk->setBlock(x - posChunk.x * 16, y, z - posChunk.y * 16, block);
    } else {
        return;
    }
    if (x - posChunk.x * 16 == 15) {
        auto chunkXPlus = getChunk(posChunk.x + 1, posChunk.y);
        if (chunkXPlus && !chunkXPlus->isUpdating) {
            chunkXPlus->needsUpdate = true;
        }
    } else if (x - posChunk.x * 16 == 0) {

        auto chunkXminus = getChunk(posChunk.x - 1, posChunk.y);
        if (chunkXminus && !chunkXminus->isUpdating) {
            chunkXminus->needsUpdate = true;
        }
    }
    if (z - posChunk.y * 16 == 15) {

        auto chunkZPlus = getChunk(posChunk.x, posChunk.y + 1);
        if (chunkZPlus && !chunkZPlus->isUpdating) {
            chunkZPlus->needsUpdate = true;
        }
    } else if (z - posChunk.y * 16 == 0) {
        auto chunkZMinus = getChunk(posChunk.x, posChunk.y - 1);
        if (chunkZMinus && !chunkZMinus->isUpdating) {
            chunkZMinus->needsUpdate = true;
        }
    }
}
void World::createChunk(int cx, int cz) {
    auto chunk = make_shared<Chunk>();
    chunk->setNroChunk(cx, cz);
    chunk->setWorld(this);
    // Generar terreno
    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            int worldX = cx * 16 + x;
            int worldZ = cz * 16 + z;

            int continentalHeight = getTerrainHeight(worldX, worldZ);
            BiomeType biome = getBiome(worldX, worldZ, continentalHeight);
            // Generar columna de bloques
            for (int y = 0; y <= continentalHeight; y++) {
                int block;

                // Asignar tipos de bloque según altura
                if (y == continentalHeight) {
                    if (biome == plains) {
                        block = BlockRegistry::getType("grass_block");
                    } else if (biome == desert) {
                        block = BlockRegistry::getType("sand");
                    } else if (biome == ocean) {
                        block = BlockRegistry::getType("water");
                    } else if (biome == mountains) {
                        block = BlockRegistry::getType("snowy_grass_block");
                    } else {
                        block = BlockRegistry::getType("dirt");
                    }
                } else if (y >= continentalHeight - 4) { // tierra debajo
                    if (biome == plains) {
                        block = BlockRegistry::getType("dirt");
                    } else if (biome == desert) {
                        block = BlockRegistry::getType("sand");
                    } else if (biome == ocean) {
                        block = BlockRegistry::getType("water");
                    } else if (biome == mountains) {
                        block = BlockRegistry::getType("dirt");
                    } else {
                        block = BlockRegistry::getType("dirt");
                    }
                } else if (y == 0) { // Bedrock
                    block = 18;
                } else { // Stone
                    block = 2;
                }

                chunk->setBlock(x, y, z, block);
            }

            // Si la altura es muy baja, generar agua
            if (continentalHeight < 60 && biome != desert) {
                for (int y = continentalHeight; y <= 60; y++) {
                    chunk->setBlock(x, y, z, 15); // Agua
                }
            }
        }
    }
    // Cargar hojas pendientes de chunks vecinos.
    vector<pendingBlock> leafs;
    {
        lock_guard<mutex> lock(mutexPendingBlocks);
        leafs = getPendingBlocksForChunk(cx, cz);
    }
    while (!leafs.empty()) {
        pendingBlock leaf = std::move(leafs.back());
        leafs.pop_back();
        if (chunk->getBlock(leaf.x, leaf.y, leaf.z) == 0)
            chunk->setBlock(leaf.x, leaf.y, leaf.z, leaf.block);
    }
    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            int worldX = cx * 16 + x;
            int worldZ = cz * 16 + z;

            // Obtener altura del terreno en esta posición
            int groundY = getTerrainHeight(worldX, worldZ);
            BiomeType biome = getBiome(worldX, worldZ, groundY);

            // Para hacerlo "aleatorio" multiplico por un primo de 9 digitos y despues mezclo bajos con altos (XOR).
            // Tambien agrego la seed del world para mas aleatoriedad(?)
            uint32_t hash = worldSeed;
            hash = hash * 475363961u + worldX * 374761393u + worldZ * 298347829u;
            hash = (hash ^ (hash >> 16)) * 1274126177u;
            float random = (hash & 0xFFFFFF) / float(0xFFFFFF);
            int treeType;
            float probTree;
            if (biome == plains) {
                treeType = 21;
                probTree = 0.002f;

            } else if (biome == desert) {
                treeType = 71;
                probTree = 0.0008f;
            } else if (biome == forest) {
                treeType = 117;
                probTree = 0.05f;
            } else {
                treeType = 21;
                probTree = 0.002f;
            }
            // Generar arboles .
            if (random < probTree) {
                generateTree(chunk, x, groundY, z, worldX, worldZ, hash, treeType);
            }
            if (random < 0.1f && biome == plains) {
                if (chunk->getBlock(x, groundY, z) == BlockRegistry::getType("grass_block")) {
                    if (chunk->getBlock(x, groundY + 1, z) == 0) {
                        chunk->setBlock(x, groundY + 1, z, BlockRegistry::getType("short_grass"));
                    }
                }
            }
        }
    }
    {
        lock_guard<mutex> lock(setChunkRequestMutex);
        requestedChunks.erase({cx, cz});
    }
    {
        lock_guard<mutex> lock(mutexChunkResult);
        chunkResultQueue.push(std::move(chunk));
    }
}

void extractFrustumPlanes(Plane planes[6], const mat4& m) {
    // Left
    planes[0].normal.x = m[0][3] + m[0][0];
    planes[0].normal.y = m[1][3] + m[1][0];
    planes[0].normal.z = m[2][3] + m[2][0];
    planes[0].d = m[3][3] + m[3][0];

    // Right
    planes[1].normal.x = m[0][3] - m[0][0];
    planes[1].normal.y = m[1][3] - m[1][0];
    planes[1].normal.z = m[2][3] - m[2][0];
    planes[1].d = m[3][3] - m[3][0];

    // Bottom
    planes[2].normal.x = m[0][3] + m[0][1];
    planes[2].normal.y = m[1][3] + m[1][1];
    planes[2].normal.z = m[2][3] + m[2][1];
    planes[2].d = m[3][3] + m[3][1];

    // Top
    planes[3].normal.x = m[0][3] - m[0][1];
    planes[3].normal.y = m[1][3] - m[1][1];
    planes[3].normal.z = m[2][3] - m[2][1];
    planes[3].d = m[3][3] - m[3][1];

    // Near
    planes[4].normal.x = m[0][3] + m[0][2];
    planes[4].normal.y = m[1][3] + m[1][2];
    planes[4].normal.z = m[2][3] + m[2][2];
    planes[4].d = m[3][3] + m[3][2];

    // Far
    planes[5].normal.x = m[0][3] - m[0][2];
    planes[5].normal.y = m[1][3] - m[1][2];
    planes[5].normal.z = m[2][3] - m[2][2];
    planes[5].d = m[3][3] - m[3][2];

    // Normalizar planos
    for (int i = 0; i < 6; i++) {
        float length = glm::length(planes[i].normal);
        planes[i].normal /= length;
        planes[i].d /= length;
    }
}
bool isBoxVisible(const Plane planes[6], const vec3& min, const vec3& max) {

    for (int i = 0; i < 6; i++) {

        vec3 p;

        p.x = (planes[i].normal.x >= 0) ? max.x : min.x;
        p.y = (planes[i].normal.y >= 0) ? max.y : min.y;
        p.z = (planes[i].normal.z >= 0) ? max.z : min.z;

        if (glm::dot(planes[i].normal, p) + planes[i].d < 0) {
            return false; // completamente fuera
        }
    }

    return true;
}
void World::insertChunks() {
    unique_lock<mutex> lockResult(mutexChunkResult);
    lock_guard<mutex> lockMap(mapChunks);

    while (!chunkResultQueue.empty()) {
        auto chunk = std::move(chunkResultQueue.front());
        chunkResultQueue.pop();
        lockResult.unlock();
        int cx = chunk->getNroChunkX();
        int cz = chunk->getNroChunkZ();
        chunks[cx][cz] = std::move(chunk);
        // Vecinos a actualizar (incluido el propio chunk)
        vector<pair<int, int>> vecinos = {{cx, cz}, {cx + 1, cz}, {cx - 1, cz}, {cx, cz + 1}, {cx, cz - 1}};
        {
            for (auto [nx, nz] : vecinos) {
                auto chunk = getChunk(nx, nz);
                if (chunk) {
                    if (chunk->chunkBuffer == nullptr) {
                        chunk->chunkBuffer = make_unique<ChunkBuffer>();
                    }
                    if (!chunk->isUpdating) {
                        chunk->isUpdating = true;
                        {
                            lock_guard<mutex> lock(mutexChunkUpdate);
                            chunkRequestUpdate.push({nx, nz});
                        }
                        meshCV.notify_one();
                    }
                }
            }
            lockResult.lock();
        }
    }
}
void World::render(vec3 cameraPos, mat4 view, mat4 projection, mat4 renderView, mat4 renderProjection) {
    ivec2 centerChunk = getChunkPos(cameraPos);
    int renderDist = 12;
    float maxDist = renderDist * 16.0f;
    float maxDist2 = maxDist * maxDist;
    vec2 camXZ = vec2(cameraPos.x, cameraPos.z);
    int generateDist = renderDist + 2;
    int cantChunks = 0;
    int maxChunksPerFrame = 1;
    Chunk::sharedShader->use();
    Chunk::sharedShader->setProjectionMatrix(value_ptr(renderProjection));
    Chunk::sharedShader->setViewMatrix(glm::value_ptr(renderView));
    mat4 vp = projection * view;
    Plane planes[6];
    extractFrustumPlanes(planes, vp);
    insertChunks();
    for (int nivel = 0; nivel <= generateDist; nivel++) {
        if (nivel == 0) {
            int chunkX = centerChunk.x;
            int chunkZ = centerChunk.y;
            shared_ptr<Chunk> chunk;
            {
                lock_guard<mutex> lock(mapChunks);
                chunk = getChunk(chunkX, chunkZ);
            }
            if (chunk) {
                chunk->render();

            } else {
                lock_guard<mutex> lock(setChunkRequestMutex);
                if (requestedChunks.find({chunkX, chunkZ}) == requestedChunks.end()) {
                    {
                        lock_guard<mutex> lock(mutexChunkRequest);
                        chunkRequestQueue.push({chunkX, chunkZ});
                    }
                    requestedChunks.insert({chunkX, chunkZ});
                    RequestCV.notify_one();
                }
            }
        } else {
            int x = nivel;
            int z = nivel;
            bool end = false;
            bool primera = true;
            bool segunda = true;
            bool tercera = true;
            bool cuarta = true;
            while (!end) {
                int chunkX = centerChunk.x + x;
                int chunkZ = centerChunk.y + z;
                vec2 chunkCenter = vec2(chunkX * 16 + 8, chunkZ * 16 + 8);
                vec2 diff = camXZ - chunkCenter;
                float dist2 = glm::dot(diff, diff);
                // Limito al circulo de vision desde el centro del chunk actual hasta el render.
                // Es decir en vez de renderizar en un rectangulo (cuadrado) lo hago en un circulo
                // Luego dentro del circulo recien pregunto por el frustrum.
                if (dist2 <= maxDist2) {
                    vec3 min(chunkX << 4, 0, chunkZ << 4);
                    vec3 max(min.x + 15, 512, min.z + 15);
                    if (isBoxVisible(planes, min, max)) {
                        shared_ptr<Chunk> chunk;
                        {
                            lock_guard<mutex> lock(mapChunks);
                            chunk = getChunk(chunkX, chunkZ);
                        }
                        if (nivel <= renderDist) {
                            if (chunk) {
                                chunk->render();
                            } else if (cantChunks < maxChunksPerFrame) {
                                lock_guard<mutex> lock(setChunkRequestMutex);
                                if (requestedChunks.find({chunkX, chunkZ}) == requestedChunks.end()) {
                                    {
                                        lock_guard<mutex> lock(mutexChunkRequest);
                                        chunkRequestQueue.push({chunkX, chunkZ});
                                    }
                                    requestedChunks.insert({chunkX, chunkZ});
                                    RequestCV.notify_one();
                                    cantChunks++;
                                }
                            }
                        }
                    }
                }
                if (z >= -nivel && primera) {
                    z--;
                    if (z == -nivel)
                        primera = false;
                } else if (x >= -nivel && segunda) {
                    x--;
                    if (x == -nivel)
                        segunda = false;
                } else if (z <= nivel && tercera) {
                    z++;
                    if (z == nivel)
                        tercera = false;
                } else if (x <= nivel && cuarta) {
                    x++;
                    if (x == nivel - 1)
                        cuarta = false;
                } else {
                    end = true;
                }
            }
        }
    }
}

void World::startCreationThread() {
    creationThread = thread(&World::loopCreation, this);
    meshThread = thread(&World::loopMesh, this);
    meshThread2 = thread(&World::loopMesh, this);
    meshThread3 = thread(&World::loopMesh, this);
    meshThread4 = thread(&World::loopMesh, this);
}
void World::loopCreation() {
    while (threadRunning) {
        unique_lock<mutex> lock(mutexChunkRequest);
        RequestCV.wait(lock, [this] { return !chunkRequestQueue.empty() || !threadRunning; });
        if (!threadRunning)
            break;
        auto [x, z] = chunkRequestQueue.front();
        chunkRequestQueue.pop();
        lock.unlock();
        createChunk(x, z);
    }
}
void World::loopMesh() {
    while (threadRunning) {
        unique_lock<mutex> lock(mutexChunkUpdate);
        meshCV.wait(lock, [this] { return !chunkRequestUpdate.empty() || !threadRunning; });
        if (!threadRunning)
            break;

        auto [x, z] = chunkRequestUpdate.front();
        chunkRequestUpdate.pop();
        lock.unlock();
        shared_ptr<Chunk> chunk;
        {
            lock_guard<mutex> lock(mapChunks);
            chunk = getChunk(x, z);
        }
        if (chunk) {
            chunk->generateMesh();
            chunk->isUpdating = false;
        }
    }
}
void World::update() {
}
World::~World() {
    threadRunning = false;
    RequestCV.notify_one();
    meshCV.notify_one();
    if (creationThread.joinable()) {
        creationThread.join();
    }
    if (meshThread.joinable()) {
        meshThread.join();
    }
    if (meshThread2.joinable()) {
        meshThread2.join();
    }
    if (meshThread3.joinable()) {
        meshThread3.join();
    }
    if (meshThread4.joinable()) {
        meshThread4.join();
    }
    if (meshThread5.joinable()) {
        meshThread5.join();
    }
    if (meshThread6.joinable()) {
        meshThread6.join();
    }
    if (meshThread7.joinable()) {
        meshThread7.join();
    }
    if (meshThread8.joinable()) {
        meshThread8.join();
    }
}
