#pragma once
#include "configShader.h"
#include <atomic>
#include <cstdint>
#include <glm/ext/vector_int2.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <mutex>
using namespace std;
using namespace glm;
class World;
struct faceAO {
    float ao0, ao1, ao2, ao3;
};
struct Rectangulo {
    int x1, x2, y1, y2;
    int tipoBloque = 0;
    faceAO face;
};
struct bloqueCapa {
    int16_t tipo;
    faceAO ao;
};
class Chunk {
  private:
    World* world;
    int16_t blocks[16][512][16];
    vector<float> vertexData;
    vector<unsigned int> indexData;
    int nroChunkX;
    int nroChunkZ;
    int vertexCount;
    mutex mutexVertex;
    atomic<bool> needsBufferUpdate{true};
    void cargarVertices(const Rectangulo& r, int eje, int direccion, int fijo, int16_t tipoBloque, vector<float>& vData, vector<unsigned int>& iData,
                        unsigned int& vCount);
    void cargarVerticesCross(const Rectangulo& r, int fijo, int16_t tipoBloque, vector<float>& vData, vector<unsigned int>& iData, unsigned int& vCount);
    faceAO calcularAO(int globalX, int globalY, int globalZ, int eje, int direccion);
    float calcularAOVertex(int globalX, int globalY, int globalZ, int eje, int direccion, int vertice);

  public:
    unique_ptr<ChunkBuffer> chunkBuffer;
    atomic<bool> isUpdating{false};
    mutex mutexBlocks;
    atomic<bool> needsUpdate = true;
    static Shader* sharedShader;
    Chunk();
    void setBlock(int x, int y, int z, const int16_t& block);
    int16_t getBlock(int x, int y, int z) const;
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
