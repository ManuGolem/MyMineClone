#pragma once
#include "configShader.h"
#include <atomic>
#include <cmath>
#include <glm/ext/vector_int2.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <mutex>
using namespace std;
using namespace glm;
class World;
struct Block {
    bool active;
    int type;
};
struct Rectangulo {
    int x1, x2, y1, y2;
    int tipoBloque;
};

class Chunk {
  private:
    World* world;
    Block blocks[16][256][16];
    vector<float> vertexData;
    vector<unsigned int> indexData;
    int nroChunkX;
    int nroChunkZ;
    int caras;
    int vertexCount;
    mutex mutexVertex;
    atomic<bool> needsBufferUpdate{true};
    unique_ptr<ChunkBuffer> chunkBuffer;
    void cargarVertices(const Rectangulo& r, int eje, int direccion, int fijo, int tipoBloque, vector<float>& vData, vector<unsigned int>& iData, unsigned int& vCount, int& carasLocal);

  public:
    mutex mutexUpdate;
    bool needsUpdate = true;
    static Shader* sharedShader;
    Chunk();
    void setBlock(int x, int y, int z, const Block& block);
    Block getBlock(int x, int y, int z) const;
    void render();
    bool isEmpty() const;
    void setNroChunk(int, int);
    void generateMesh();
    int getCaras() const {
        return caras;
    }
    int getNroChunkX() const {
        return nroChunkX;
    }
    int getNroChunkZ() const {
        return nroChunkZ;
    }
    void cleanup();
    void setWorld(World* w) {
        world = w;
    }
    void setUpdate() {
        lock_guard<mutex> lock(mutexUpdate);
        needsUpdate = true;
    };
    void setBlockSafe(int x, int y, int z, const Block& block);
};
