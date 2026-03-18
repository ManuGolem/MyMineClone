#pragma once
#include "FastNoiseLite.h"
#include "chunk.h"
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <glm/fwd.hpp>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <thread>
#include <utility>
struct pendingBlock {
    int x, y, z;
    int block;
};
struct Plane {
    vec3 normal;
    float d;
};
class World {
  private:
    unordered_map<int, unordered_map<int, shared_ptr<Chunk>>> chunks;
    unordered_map<int, unordered_map<int, vector<pendingBlock>>> pendingBlocks;
    FastNoiseLite terrainNoise;
    FastNoiseLite erosionNoise;
    FastNoiseLite detailNoise;
    ivec2 chunkAnterior = ivec2(9999, 9999);
    thread creationThread;
    thread meshThread;
    thread meshThread2;
    thread meshThread3;
    thread meshThread4;
    atomic<bool> threadRunning = true;

  public:
    // Para el thread secundario (mejorar despues)
    mutex mapChunks;
    mutex mutexPendingBlocks;
    mutex mutexChunkResult;
    mutex mutexChunkRequest;
    mutex mutexChunkUpdate;
    queue<pair<int, int>> chunkRequestQueue;  // cola de chunks listos para insertar
    queue<pair<int, int>> chunkRequestUpdate; // cola de chunks para generar mesh
    set<pair<int, int>> requestedChunks;
    mutex setUpdateChunk;
    mutex setChunkRequestMutex;
    queue<shared_ptr<Chunk>> chunkResultQueue;
    condition_variable RequestCV;
    condition_variable meshCV;
    World();
    ivec2 getChunkPos(vec3 worldPos);
    shared_ptr<Chunk> getChunk(int chunkX, int chunkZ);
    void generateFlatWorld(int width, int depth);
    void generateWorldWithPerlin();
    void insertChunks();
    void render(vec3 cameraPos, mat4 view, mat4 projection, mat4 renderView, mat4 renderProjection);
    int getTerrainHeight(int worldX, int worldZ);
    void deleteWorld();
    void createChunk(int, int);
    void update();
    void generateTree(shared_ptr<Chunk> chunk, int x, int y, int z, int wx, int wz, uint32_t hash, int treeType);
    void startCreationThread();
    void loopCreation();
    void loopMesh();
    void setBlockSafe(int x, int y, int z, int block);
    int getBlockSafe(int x, int y, int z);
    void processChunk(int chunkX, int chunkZ, int nivel);
    vector<pendingBlock> getPendingBlocksForChunk(int x, int z);
    ~World();
};
