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
    bool active = false;
    int type = 0;
};
struct Rectangulo {
    int x1, x2, y1, y2;
    int tipoBloque = 0;
};

class Chunk {
  private:
    World* world;
    Block blocks[16][256][16];
    vector<float> vertexData;
    vector<unsigned int> indexData;
    int nroChunkX;
    int nroChunkZ;
    int vertexCount;
    mutex mutexVertex;
    atomic<bool> needsBufferUpdate{true};
    Rectangulo processFaceX(int x, int i, int j, bool (&procesado)[256][16], int globalX, int globalZ, int tipoActual, int dir);
    Rectangulo processFaceZ(int z, int i, int j, bool (&procesado)[16][256], int globalX, int globalZ, int tipoActual, int dir);
    Rectangulo processFaceY(int y, int i, int j, bool (&procesado)[16][16], int tipoActual, int dir);
    void cargarVertices(const Rectangulo& r, int eje, int direccion, int fijo, int tipoBloque, vector<float>& vData, vector<unsigned int>& iData, unsigned int& vCount);

  public:
    unique_ptr<ChunkBuffer> chunkBuffer;
    atomic<bool> isUpdating{false};
    mutex mutexBlocks;
    atomic<bool> needsUpdate = true;
    static Shader* sharedShader;
    Chunk();
    void setBlock(int x, int y, int z, const Block& block);
    Block getBlock(int x, int y, int z) const;
    void render();
    bool isEmpty() const;
    void setNroChunk(int, int);
    void generateMesh();
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
};
