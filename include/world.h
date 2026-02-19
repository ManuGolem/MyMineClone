#pragma once
#include "camera.h"
#include "chunk.h"

class World {
  private:
    unordered_map<int, unordered_map<int, Chunk>> chunks;

  public:
    World();
    ivec2 getChunkPos(vec3 worldPos);
    Chunk* getChunk(int chunkX, int chunkZ);
    void generateFlatWorld(int width, int depth);
    Block getBlock(int x, int y, int z);
    void render(vec3 cameraPos, mat4 view, mat4 projection);
    void update();
};
