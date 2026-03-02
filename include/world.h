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
#include <stack>
#include <thread>
#include <utility>
struct pendingBlock {
    int x, y, z;
    Block block;
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
    thread creationThread2;
    thread meshThread;
    thread meshThread2;
    atomic<bool> threadRunning = true;

  public:
    // Para el thread secundario (mejorar despues)
    mutex mapChunks;
    mutex mutexPendingBlocks;
    mutex mutexChunkResult;
    mutex mutexChunkRequest;
    mutex mutexChunkUpdateRequest;
    queue<pair<int, int>> chunkRequestQueue;           // cola de chunks listos para insertar
    queue<pair<int, int>> chunkRequestUpdateLowQueue;  // cola de chunks para generar la mesh de forma secundaria
    queue<pair<int, int>> chunkRequestUpdateHighQueue; // cola de chunks para generar mesh que estan en el frustrum de la camara
    set<pair<int, int>> requestedChunks;
    set<pair<int, int>> requestedUpdateChunks; // conjunto de chunks en cola para generar la mesh.
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
    vector<pendingBlock> getPendingBlocksForChunk(int x, int z);
    ~World();
};
