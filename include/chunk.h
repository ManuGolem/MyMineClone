#pragma once
#include "configShader.h"
#include <atomic>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <glm/ext/vector_int2.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <mutex>
using namespace std;
using namespace glm;
class World;
struct Rectangulo {
    int x1, x2, y1, y2;
    int tipoBloque = 0;
};

class Chunk {
  private:
    World *world;
    int16_t blocks[16][512][16];
    vector<float> vertexData;
    vector<unsigned int> indexData;
    int nroChunkX;
    int nroChunkZ;
    int vertexCount;
    mutex mutexVertex;
    atomic<bool> needsBufferUpdate{true};
    float calcularAO(int x, int y, int z, int dx1, int dy1, int dz1, int dx2, int dy2, int dz2);
    void cargarVertices(const Rectangulo &r, int eje, int direccion, int fijo, int16_t tipoBloque, vector<float> &vData,
                        vector<unsigned int> &iData, unsigned int &vCount);
    void cargarVerticesCross(const Rectangulo &r, int fijo, int16_t tipoBloque, vector<float> &vData,
                             vector<unsigned int> &iData, unsigned int &vCount);

  public:
    unique_ptr<ChunkBuffer> chunkBuffer;
    atomic<bool> isUpdating{false};
    mutex mutexBlocks;
    atomic<bool> needsUpdate = true;
    static Shader *sharedShader;
    Chunk();
    void setBlock(int x, int y, int z, const int16_t &block);
    int16_t getBlock(int x, int y, int z) const;
    void render();
    bool isEmpty() const;
    void setNroChunk(int, int);
    void generateMesh();
    int getNroChunkX() const { return nroChunkX; }
    int getNroChunkZ() const { return nroChunkZ; }
    void cleanup();
    void setWorld(World *w) { world = w; }
};
