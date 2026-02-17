#pragma once
#include "camera.h"
#include "configShader.h"
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
    void generateMeshBlock();
    ChunkBuffer chunkBuffer;
    void cargarVertices(const Rectangulo& r, int eje, int direccion, int fijo, int tipoBloque);

  public:
    static Shader* sharedShader;
    Chunk();
    void setBlock(int x, int y, int z, const Block& block);
    Block getBlock(int x, int y, int z) const;
    void render(const mat4& view);
    bool isEmpty() const;
    void setNroChunk(int, int);
    int getCaras() const {
        return caras;
    }
};
class World {
  private:
    unordered_map<int, unordered_map<int, Chunk>> chunks;
    ivec2 getChunkPos(vec3 worldPos);
    Chunk* getChunk(int chunkX, int chunkZ);

  public:
    World();
    void generateFlatWorld(int width, int depth);
    void setBlock(int x, int y, int z, const Block& block);
    Block getBlock(int x, int y, int z);
    void render(vec3 cameraPos, mat4 view);
    void update();
};
class DebugAxes {
  private:
    ChunkBuffer buffer;
    bool inicializado;

  public:
    DebugAxes() : inicializado(false) {
    }

    void inicializar() {
        if (inicializado)
            return;

        // 6 vértices (3 líneas) con formato: posición(3) + color(3) + texCoord(2) +
        // texOffset(2) Para debug, texCoord y texOffset los dejamos en 0
        std::vector<float> vertices = {// Eje X rojo
                                       0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 100.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

                                       // Eje Y verde
                                       0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 100.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

                                       // Eje Z azul
                                       0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 100.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                                       0.0f};

        std::vector<unsigned int> indices = {0, 1, 2, 3, 4, 5}; // Líneas independientes

        buffer.uploadData(vertices, indices);
        inicializado = true;
    }

    void renderizar() {
        if (inicializado) {
            buffer.renderLines(6);
        }
    }
};
