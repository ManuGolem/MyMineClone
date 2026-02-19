#pragma once    
#include "configShader.h"
#include <cmath>
#include <glm/ext/vector_int2.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
using namespace std;
using namespace glm;
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
    Block blocks[16][256][16];
    vector<float> vertexData;
    vector<unsigned int> indexData;
    bool needsUpdate;
    int nroChunkX;
    int nroChunkZ;
    int caras;
    int vertexCount;
    void generateMesh();
    ChunkBuffer chunkBuffer;
    void cargarVertices(const Rectangulo& r, int eje, int direccion, int fijo, int tipoBloque);

  public:
    static Shader* sharedShader;
    Chunk();
    void setBlock(int x, int y, int z, const Block& block);
    Block getBlock(int x, int y, int z) const;
    void render();
    bool isEmpty() const;
    void setNroChunk(int, int);
    int getCaras() const {
        return caras;
    }
    int getNroChunkX() const {
        return nroChunkX;
    }
    int getNroChunkZ() const {
        return nroChunkZ;
    }
};
