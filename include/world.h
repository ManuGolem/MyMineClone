#pragma once
#include "FastNoiseLite.h"
#include "camera.h"
#include "chunk.h"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <thread>
#include <utility>
class World {
  private:
    unordered_map<int, unordered_map<int, unique_ptr<Chunk>>> chunks;
    FastNoiseLite terrainNoise;
    FastNoiseLite erosionNoise;
    FastNoiseLite detailNoise;
    ivec2 chunkAnterior = ivec2(9999, 9999);
    thread creationThread;
    thread meshThread1;
    thread meshThread2;
    atomic<bool> threadRunning = true;

  public:
    // Para el thread secundario (mejorar despues)
    mutex mapChunks;
    mutex mutexChunkResult;
    mutex mutexChunkRequest;
    mutex mutexChunkUpdateRequest;
    queue<pair<int, int>> chunkRequestQueue;
    queue<pair<int, int>> chunkRequestUpdateQueue;
    set<pair<int, int>> requestedChunks;
    set<pair<int, int>> requestedUpdateChunks;
    mutex setUpdateChunk;
    mutex setChunkRequestMutex;
    queue<unique_ptr<Chunk>> chunkResultQueue;
    condition_variable RequestCV;
    condition_variable meshCV;
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
    void createChunkSingle(int, int);
    void update();
    void generateTree(int x, int y, int z, int treeType);
    bool canPlaceTree(int worldX, int groundY, int worldZ, int treeHeight, int canopyRadius);
    void startCreationThread();
    void loopCreation();
    void loopMesh();
    void setBlockSafe(int x, int y, int z, Block block);
    Block getBlockSafe(int x, int y, int z);
    void processChunk(int chunkX, int chunkZ, int nivel);
    ~World();
};
