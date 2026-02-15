#include "../include/world.h"
#include <glm/ext/vector_int2.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
Shader *Chunk::sharedShader = nullptr;
using namespace std;
Chunk::Chunk()
{
    for (int x = 0; x < 16; x++)
    {
        for (int y = 0; y < 256; y++)
        {
            for (int z = 0; z < 16; z++)
            {
                blocks[x][y][z].active = false;
                blocks[x][y][z].type = 0;
            }
        }
    }
    if (sharedShader == nullptr)
    {
        sharedShader = new Shader();
    }
}
void Chunk::generateMesh() {
    if (!needsUpdate) return;

    vertexData.clear();
    indexData.clear();
    int vertexCount = 0;

    for (int axis = 0; axis < 3; axis++) {
        for (int dir = -1; dir <= 1; dir += 2) {

            int fijoMax, uMax, vMax;

            if (axis == 0) { // Caras en X (plano YZ)
                fijoMax = 16;  // X
                uMax = 256;    // Y
                vMax = 16;     // Z
            }
            else if (axis == 1) { // Caras en Y (plano XZ)
                fijoMax = 256;    // Y
                uMax = 16;        // X
                vMax = 16;        // Z
            }
            else { // axis == 2, Caras en Z (plano XY)
                fijoMax = 16;     // Z
                uMax = 16;        // X
                vMax = 256;       // Y
            }

            bool merged[256][16] = {false};

            for (int fijo = 0; fijo < fijoMax; fijo++) {
                for (int u = 0; u < uMax; u++) {
                    for (int v = 0; v < vMax; v++) {

                        if (merged[u][v]) continue;

                        // Reconstruir coordenadas
                        int x, y, z;
                        if (axis == 0) { x = fijo; y = u; z = v; }
                        else if (axis == 1) { x = u; y = fijo; z = v; }
                        else { x = u; y = v; z = fijo; }

                        if (!blocks[x][y][z].active) continue;

                        // Verificar visibilidad
                        bool visible = false;
                        if (dir == 1) {
                            if (axis == 0) visible = (x == 15 || !blocks[x+1][y][z].active);
                            if (axis == 1) visible = (y == 255 || !blocks[x][y+1][z].active);
                            if (axis == 2) visible = (z == 15 || !blocks[x][y][z+1].active);
                        } else {
                            if (axis == 0) visible = (x == 0 || !blocks[x-1][y][z].active);
                            if (axis == 1) visible = (y == 0 || !blocks[x][y-1][z].active);
                            if (axis == 2) visible = (z == 0 || !blocks[x][y][z-1].active);
                        }

                        if (!visible) continue;

                        int type = blocks[x][y][z].type;

                        // Buscar ancho (en dirección u)
                        int width = 1;
                        while (u + width < uMax && !merged[u+width][v]) {
                            int nx, ny, nz;
                            if (axis == 0) { nx = fijo; ny = u+width; nz = v; }
                            else if (axis == 1) { nx = u+width; ny = fijo; nz = v; }
                            else { nx = u+width; ny = v; nz = fijo; }

                            if (!blocks[nx][ny][nz].active || blocks[nx][ny][nz].type != type) break;

                            bool nextVisible = false;
                            if (dir == 1) {
                                if (axis == 0) nextVisible = (nx == 15 || !blocks[nx+1][ny][nz].active);
                                if (axis == 1) nextVisible = (ny == 255 || !blocks[nx][ny+1][nz].active);
                                if (axis == 2) nextVisible = (nz == 15 || !blocks[nx][ny][nz+1].active);
                            } else {
                                if (axis == 0) nextVisible = (nx == 0 || !blocks[nx-1][ny][nz].active);
                                if (axis == 1) nextVisible = (ny == 0 || !blocks[nx][ny-1][nz].active);
                                if (axis == 2) nextVisible = (nz == 0 || !blocks[nx][ny][nz-1].active);
                            }

                            if (!nextVisible) break;
                            width++;
                        }

                        // Buscar alto (en dirección v)
                        int height = 1;
                        bool canExtend = true;
                        while (v + height < vMax && canExtend) {
                            for (int w = 0; w < width; w++) {
                                if (merged[u+w][v+height]) { canExtend = false; break; }

                                int nx, ny, nz;
                                if (axis == 0) { nx = fijo; ny = u+w; nz = v+height; }
                                else if (axis == 1) { nx = u+w; ny = fijo; nz = v+height; }
                                else { nx = u+w; ny = v+height; nz = fijo; }

                                if (!blocks[nx][ny][nz].active || blocks[nx][ny][nz].type != type) {
                                    canExtend = false; break;
                                }

                                bool nextVisible = false;
                                if (dir == 1) {
                                    if (axis == 0) nextVisible = (nx == 15 || !blocks[nx+1][ny][nz].active);
                                    if (axis == 1) nextVisible = (ny == 255 || !blocks[nx][ny+1][nz].active);
                                    if (axis == 2) nextVisible = (nz == 15 || !blocks[nx][ny][nz+1].active);
                                } else {
                                    if (axis == 0) nextVisible = (nx == 0 || !blocks[nx-1][ny][nz].active);
                                    if (axis == 1) nextVisible = (ny == 0 || !blocks[nx][ny-1][nz].active);
                                    if (axis == 2) nextVisible = (nz == 0 || !blocks[nx][ny][nz-1].active);
                                }

                                if (!nextVisible) { canExtend = false; break; }
                            }
                            if (canExtend) height++;
                        }

                        // Marcar fusionadas
                        for (int w = 0; w < width; w++) {
                            for (int h = 0; h < height; h++) {
                                merged[u+w][v+h] = true;
                            }
                        }


                        // Coordenadas globales
                        int globalX = x + nroChunkX * 16;
                        int globalY = y;
                        int globalZ = z + nroChunkZ * 16;

                        float fx1, fx2, fy1, fy2, fz1, fz2;

                        if (axis == 0) { // Caras en X
                            fx1 = fx2 = (dir == 1) ? globalX + 0.5f : globalX - 0.5f;
                            fy1 = globalY - 0.5f;
                            fy2 = globalY + width - 0.5f;    // width = Y
                            fz1 = globalZ - 0.5f;
                            fz2 = globalZ + height - 0.5f;   // height = Z

                            if (dir == 1) { // Cara derecha
                                vertexData.push_back(fx1); vertexData.push_back(fy1); vertexData.push_back(fz1);
                                vertexData.push_back(fx2); vertexData.push_back(fy1); vertexData.push_back(fz2);
                                vertexData.push_back(fx2); vertexData.push_back(fy2); vertexData.push_back(fz2);
                                vertexData.push_back(fx1); vertexData.push_back(fy2); vertexData.push_back(fz1);
                            } else { // Cara izquierda
                                vertexData.push_back(fx1); vertexData.push_back(fy1); vertexData.push_back(fz1);
                                vertexData.push_back(fx1); vertexData.push_back(fy2); vertexData.push_back(fz1);
                                vertexData.push_back(fx2); vertexData.push_back(fy2); vertexData.push_back(fz2);
                                vertexData.push_back(fx2); vertexData.push_back(fy1); vertexData.push_back(fz2);
                            }
                        }
                        else if (axis == 1) { // Caras en Y
                            fy1 = fy2 = (dir == 1) ? globalY + 0.5f : globalY - 0.5f;
                            fx1 = globalX - 0.5f;
                            fx2 = globalX + width - 0.5f;    // width = X
                            fz1 = globalZ - 0.5f;
                            fz2 = globalZ + height - 0.5f;   // height = Z

                            if (dir == 1) { // Cara superior
                                vertexData.push_back(fx1); vertexData.push_back(fy1); vertexData.push_back(fz1);
                                vertexData.push_back(fx2); vertexData.push_back(fy1); vertexData.push_back(fz1);
                                vertexData.push_back(fx2); vertexData.push_back(fy1); vertexData.push_back(fz2);
                                vertexData.push_back(fx1); vertexData.push_back(fy1); vertexData.push_back(fz2);
                            } else { // Cara inferior
                                vertexData.push_back(fx1); vertexData.push_back(fy1); vertexData.push_back(fz1);
                                vertexData.push_back(fx1); vertexData.push_back(fy1); vertexData.push_back(fz2);
                                vertexData.push_back(fx2); vertexData.push_back(fy1); vertexData.push_back(fz2);
                                vertexData.push_back(fx2); vertexData.push_back(fy1); vertexData.push_back(fz1);
                            }
                        }
                        else { // Caras en Z
                            fz1 = fz2 = (dir == 1) ? globalZ + 0.5f : globalZ - 0.5f;
                            fx1 = globalX - 0.5f;
                            fx2 = globalX + width - 0.5f;    // width = X
                            fy1 = globalY - 0.5f;
                            fy2 = globalY + height - 0.5f;   // height = Y

                            if (dir == 1) { // Cara frontal
                                vertexData.push_back(fx1); vertexData.push_back(fy1); vertexData.push_back(fz1);
                                vertexData.push_back(fx2); vertexData.push_back(fy1); vertexData.push_back(fz2);
                                vertexData.push_back(fx2); vertexData.push_back(fy2); vertexData.push_back(fz2);
                                vertexData.push_back(fx1); vertexData.push_back(fy2); vertexData.push_back(fz1);
                            } else { // Cara trasera
                                vertexData.push_back(fx1); vertexData.push_back(fy1); vertexData.push_back(fz1);
                                vertexData.push_back(fx1); vertexData.push_back(fy2); vertexData.push_back(fz1);
                                vertexData.push_back(fx2); vertexData.push_back(fy2); vertexData.push_back(fz2);
                                vertexData.push_back(fx2); vertexData.push_back(fy1); vertexData.push_back(fz2);
                            }
                        }

                        int base = vertexCount;
                        indexData.push_back(base);
                        indexData.push_back(base + 1);
                        indexData.push_back(base + 2);
                        indexData.push_back(base);
                        indexData.push_back(base + 2);
                        indexData.push_back(base + 3);

                        vertexCount += 4;
                    }
                }
            }
        }
    }

    needsUpdate = false;
}

void Chunk::setBlock(int x, int y, int z, const Block &block)
{
    if (x < 0 || x >= 16 || y < 0 || y >= 256 || z < 0 || z >= 16)
    {
        return;
    }
    blocks[x][y][z] = block;
    needsUpdate = true;
}
bool Chunk::isEmpty() const
{
    return vertexData.empty();
}
void Chunk::setNroChunk(int chunkx, int chunkz)
{
    nroChunkX = chunkx;
    nroChunkZ = chunkz;
}
void Chunk::render(const mat4 &view)
{
    if (needsUpdate)
    {
        generateMesh();
        if (!vertexData.empty())
        {
            chunkBuffer.uploadData(vertexData, indexData);
        }
        needsUpdate = false;
    }
    if (vertexData.empty())
        return;
    sharedShader->use();
    sharedShader->setViewMatrix(glm::value_ptr(view));
    chunkBuffer.render();
}
World::World()
{
}
void World::generateFlatWorld(int width, int depth)
{
    int chunksInX = (width + 15) / 16;
    int chunksInZ = (depth + 15) / 16;
    for (int cx = 0; cx < chunksInX; cx++)
    {
        for (int cz = 0; cz < chunksInZ; cz++)
        {
            Chunk &chunk = chunks[cx][cz];
            chunk.setNroChunk(cx, cz);
            for (int x = 0; x < 16; x++)
            {
                for (int z = 0; z < 16; z++)
                {
                    int wordlX = cx * 16 + x;
                    int wordlZ = cz * 16 + z;
                    if (wordlX < width && wordlZ < depth)
                    {
                        Block block;
                        block.active = true;
                        block.type = 1;
                        chunk.setBlock(x, 0, z, block);
                    }
                }
            }
        }
    }
}
Chunk *World::getChunk(int x, int z)
{
    auto itX = chunks.find(x);
    if (itX == chunks.end())
        return nullptr;
    auto itZ = itX->second.find(z);
    if (itZ == itX->second.end())
        return nullptr;
    return &itZ->second;
}
ivec2 World::getChunkPos(vec3 worldPos)
{
    int chunkX = (int)floor(worldPos.x / 16.0f);
    int chunkZ = (int)floor(worldPos.z / 16.0f);
    return ivec2(chunkX, chunkZ);
}
void World::render(vec3 cameraPos, mat4 view)
{
    ivec2 centerChunk = getChunkPos(cameraPos);
    int renderDist = 64;
    for (int dx = -renderDist; dx <= renderDist; dx++)
    {
        for (int dz = -renderDist; dz <= renderDist; dz++)
        {
            Chunk *chunk = getChunk(centerChunk.x + dx, centerChunk.y + dz);
            if (chunk)
            {
                chunk->render(view);
            }
        }
    }
}
void World::update()
{
}
