#pragma once
#include "FastNoiseLite.h"
#include "camera.h"
#include "chunk.h"

class World {
  private:
    unordered_map<int, unordered_map<int, Chunk>> chunks;
    FastNoiseLite terrainNoise;
    FastNoiseLite erosionNoise;
    FastNoiseLite detailNoise;
    ivec2 chunkAnterior = vec2(9999, 9999);

  public:
    World();
    ivec2 getChunkPos(vec3 worldPos);
    Chunk* getChunk(int chunkX, int chunkZ);
    void generateFlatWorld(int width, int depth);
    void generateWorldWithPerlin(int width, int depth);
    Block getBlock(int x, int y, int z);
    void render(vec3 cameraPos, mat4 view, mat4 projection, mat4 renderView, mat4 renderProjection);
    int getTerrainHeight(int worldX, int worldZ);
    void deleteWorld();
    void createChunk(int, int);
    void update();
    void generateTree(int x, int y, int z, int treeType);
    bool canPlaceTree(int worldX, int groundY, int worldZ, int treeHeight, int canopyRadius);
};
