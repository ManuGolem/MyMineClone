#include "../include/world.h"
#include <cstdlib>
#include <glm/geometric.hpp>
World::World() {
  // Configurar ruidos
  terrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
  terrainNoise.SetFrequency(0.02f);
  terrainNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
  terrainNoise.SetFractalOctaves(4);
  terrainNoise.SetFractalGain(0.5f);

  erosionNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
  erosionNoise.SetFrequency(0.002f);
  erosionNoise.SetFractalOctaves(2);

  detailNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
  detailNoise.SetFrequency(0.01f);
  detailNoise.SetFractalOctaves(1);
}
bool World::canPlaceTree(int worldX, int groundY, int worldZ, int treeHeight,
                         int canopyRadius) {
  // Verificar que el suelo es válido (hierba/tierra)
  Block groundBlock = getBlock(worldX, groundY, worldZ);
  if (!groundBlock.active || (groundBlock.type != 4 && groundBlock.type != 3)) {
    return false;
  }

  // Verificar espacio para el tronco (que no haya bloques)
  for (int y = 1; y <= treeHeight; y++) {
    Block block = getBlock(worldX, groundY + y, worldZ);
    if (block.active) {
      return false;
    }
  }
  Block ceilTrunk = getBlock(worldX, groundY + treeHeight + 1, worldZ);
  if (ceilTrunk.active) {
    return false;
  }
  return true;
}
void World::generateTree(int worldX, int groundY, int worldZ, int treeType) {
  int trunkHeight = 4 + (rand() % 3); // 4–6

  int leafRadius = 2;
  int leafStart = groundY + trunkHeight - 2;

  if (!canPlaceTree(worldX, groundY, worldZ, trunkHeight + 2, leafRadius + 1))
    return;

  for (int y = 0; y < trunkHeight; y++) {
    Block wood;
    wood.active = true;
    wood.type = 21;
    ivec2 chunkPos = getChunkPos(vec3(worldX, groundY + y + 1, worldZ));
    Chunk *chunk = getChunk(chunkPos.x, chunkPos.y);

    if (chunk) {
      int localX = worldX - chunkPos.x * 16;
      int localZ = worldZ - chunkPos.y * 16;
      chunk->setBlock(localX, groundY + y + 1, localZ, wood);
    }
  }

  // Hojas
  for (int y = leafStart; y <= groundY + trunkHeight + 1; y++) {
    int dy = y - (groundY + trunkHeight);
    int currentRadius = leafRadius - abs(dy);
    for (int dx = -currentRadius; dx <= currentRadius; dx++) {
      for (int dz = -currentRadius; dz <= currentRadius; dz++) {
        float dist = sqrt(dx * dx + dz * dz + dy * dy);
        if (dist > leafRadius + 0.5f)
          continue;
        // esquinas con probabilidad para hacerlo más orgánico
        if (abs(dx) == currentRadius && abs(dz) == currentRadius &&
            rand() % 100 < 40)
          continue;
        if (dx == dz && dx == 0 && dy != 1) {
          continue;
        }
        Block leaf;
        leaf.active = true;
        leaf.type = 53; // Hojas transparentes

        int lx = worldX + dx;
        int ly = y;
        int lz = worldZ + dz;

        ivec2 chunkPos = getChunkPos(vec3(lx, ly, lz));
        Chunk *chunk = getChunk(chunkPos.x, chunkPos.y);

        if (chunk) {
          int localX = lx - chunkPos.x * 16;
          int localZ = lz - chunkPos.y * 16;
          chunk->setBlock(localX, ly, localZ, leaf);
        }
      }
    }
  }
}
int World::getTerrainHeight(int worldX, int worldZ) {
  // Combinar los tres ruidos para un terreno más natural
  float continent = terrainNoise.GetNoise((float)worldX, (float)worldZ);
  float erosion = erosionNoise.GetNoise((float)worldX, (float)worldZ);
  float detail = detailNoise.GetNoise((float)worldX, (float)worldZ);

  // Normalizar cada uno a [0,1]
  float nContinent = (continent + 1.0f) * 0.5f;
  float nErosion = (erosion + 1.0f) * 0.5f;
  float nDetail = (detail + 1.0f) * 0.5f;

  // 1. CURVA DE DISTRIBUCIÓN para tener más control
  // Esto hace que los valores alrededor de 0.5 sean más comunes
  float shaped;
  if (nContinent < 0.3f) {
    // Océanos (30% del mapa)
    shaped = nContinent * 0.4f; // Comprime océanos
  } else if (nContinent < 0.7f) {
    // Tierras (40% del mapa) - EXPANDIMOS este rango
    float t = (nContinent - 0.3f) / 0.4f;
    shaped = 0.12f + t * 0.56f; // Mapea a [0.12, 0.68]
  } else {
    // Montañas (30% del mapa)
    float t = (nContinent - 0.7f) / 0.3f;
    shaped = 0.68f + t * 0.32f; // Mapea a [0.68, 1.0]
  }

  // 2. AÑADIR EROSIÓN (20% influencia)
  float withErosion = shaped * 0.8f + nErosion * 0.2f;

  // 3. AÑADIR DETALLE (10% influencia)
  float finalNoise = withErosion * 0.9f + nDetail * 0.1f;

  // 4. MAPEO A ALTURAS MÁS REALISTAS
  const int OCEAN_FLOOR = 38;   // Fondo marino
  const int SHALLOW_WATER = 54; // Aguas poco profundas
  const int BEACH = 58;         // Costa
  const int LAND_LOW = 62;      // Tierra baja
  const int LAND_MID = 75;      // Colinas suaves
  const int LAND_HIGH = 100;    // Montañas
  const int PEAK = 170;         // Picos

  if (finalNoise < 0.12f) {
    // Océano profundo
    return OCEAN_FLOOR +
           (int)((SHALLOW_WATER - OCEAN_FLOOR) * (finalNoise / 0.12f));
  } else if (finalNoise < 0.2f) {
    // Plataforma continental / aguas someras
    float t = (finalNoise - 0.12f) / 0.08f;
    return SHALLOW_WATER + (int)((BEACH - SHALLOW_WATER) * t);
  } else if (finalNoise < 0.35f) {
    // Costa / tierra baja
    float t = (finalNoise - 0.2f) / 0.15f;
    return BEACH + (int)((LAND_LOW - BEACH) * t);
  } else if (finalNoise < 0.55f) {
    // Tierras medias / colinas suaves
    float t = (finalNoise - 0.35f) / 0.2f;
    return LAND_LOW + (int)((LAND_MID - LAND_LOW) * t);
  } else if (finalNoise < 0.75f) {
    // Colinas altas / montañas bajas
    float t = (finalNoise - 0.55f) / 0.2f;
    return LAND_MID + (int)((LAND_HIGH - LAND_MID) * t);
  } else {
    // Montañas altas / picos
    float t = (finalNoise - 0.75f) / 0.25f;
    return LAND_HIGH + (int)((PEAK - LAND_HIGH) * t);
  }
}
void World::generateWorldWithPerlin(int width, int depth) {
  // srand(time(NULL));
  // int newSeed1 = rand();
  // int newSeed2 = rand();
  // int newSeed3 = rand();
  int newSeed1 = 123;
  int newSeed2 = 123;
  int newSeed3 = 123;

  terrainNoise.SetSeed(newSeed1);
  erosionNoise.SetSeed(newSeed2);
  detailNoise.SetSeed(newSeed3);
  float freq1 = 0.0005f + (rand() % 100) / 10000.0f;
  terrainNoise.SetFrequency(freq1);

  float freq2 = 0.001f + (rand() % 90) / 10000.0f;
  erosionNoise.SetFrequency(freq2);

  float freq3 = 0.005f + (rand() % 45) / 1000.0f;
  detailNoise.SetFrequency(freq3);

  int octavas1 = 2 + rand() % 3;
  int octavas2 = 1 + rand() % 3;
  int octavas3 = 1 + rand() % 2;
  terrainNoise.SetFractalOctaves(octavas1);
  erosionNoise.SetFractalOctaves(octavas2);
  detailNoise.SetFractalOctaves(octavas3);

  float gain = 0.3f + (rand() % 40) / 100.0f;
  terrainNoise.SetFractalGain(gain);

  int chunksInX = (width + 15) / 16;
  int chunksInZ = (depth + 15) / 16;

  for (int cx = -chunksInX / 2; cx < chunksInX / 2; cx++) {
    for (int cz = -chunksInZ / 2; cz < chunksInZ / 2; cz++) {
      createChunk(cx, cz);
    }
  }
}
void World::generateFlatWorld(int width, int depth) {
  int chunksInX = (width + 15) / 16;
  int chunksInZ = (depth + 15) / 16;
  for (int cx = 0; cx < chunksInX; cx++) {
    for (int cz = 0; cz < chunksInZ; cz++) {
      Chunk &chunk = chunks[cx][cz];
      chunk.setNroChunk(cx, cz);
      chunk.setWorld(this);
      for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
          for (int y = 0; y < 255; y++) {
            int wordlX = cx * 16 + x;
            int wordlZ = cz * 16 + z;
            if (wordlX < width && wordlZ < depth) {
              Block block;
              block.active = true;
              if (y < 64) {
                block.type = 2;
              } else if (y == 64) {
                block.type = 2;
              } else {
                block.type = 0;
                block.active = false;
              }
              chunk.setBlock(x, y, z, block);
            }
          }
        }
      }
    }
  }
}
void World::deleteWorld() {
  for (auto &xPair : chunks) {
    for (auto &zPair : xPair.second) {
      zPair.second.cleanup();
    }
  }
  chunks.clear();
}
Chunk *World::getChunk(int x, int z) {
  auto itX = chunks.find(x);
  if (itX == chunks.end())
    return nullptr;
  auto itZ = itX->second.find(z);
  if (itZ == itX->second.end())
    return nullptr;
  return &itZ->second;
}
ivec2 World::getChunkPos(vec3 worldPos) {
  int chunkX = (int)floor(worldPos.x / 16.0f);
  int chunkZ = (int)floor(worldPos.z / 16.0f);
  return ivec2(chunkX, chunkZ);
}
Block World::getBlock(int x, int y, int z) {
  ivec2 posChunk = getChunkPos(vec3(x, y, z));
  Chunk *chunk = getChunk(posChunk.x, posChunk.y);
  if (chunk == nullptr || y > 255) {
    Block empty;
    empty.active = false;
    empty.type = 0;
    return empty;
  }
  int offsetX = chunk->getNroChunkX() * 16;
  int offsetZ = chunk->getNroChunkZ() * 16;
  return chunk->getBlock(x - offsetX, y, z - offsetZ);
}
void World::createChunk(int cx, int cz) {
  Chunk &chunk = chunks[cx][cz];
  chunk.setNroChunk(cx, cz);
  chunk.setWorld(this);
  // Generar terreno
  for (int x = 0; x < 16; x++) {
    for (int z = 0; z < 16; z++) {
      int worldX = cx * 16 + x;
      int worldZ = cz * 16 + z;

      int continentalHeight = getTerrainHeight(worldX, worldZ);

      // Generar columna de bloques
      for (int y = 0; y <= continentalHeight; y++) {
        Block block;
        block.active = true;

        // Asignar tipos de bloque según altura
        if (y == continentalHeight) {
          block.type = 4; // Hierba en la superficie
        } else if (y >= continentalHeight - 4) {
          block.type = 3; // Tierra debajo de la hierba
        } else if (y == 0) {
          block.type = 18; // Bedrock en el fondo
        } else {
          block.type = 2; // Piedra en el resto
        }

        chunk.setBlock(x, y, z, block);
      }

      // Si la altura es muy baja, generar agua
      if (continentalHeight < 60) {
        for (int y = continentalHeight + 1; y <= 60; y++) {
          Block waterBlock;
          waterBlock.active = true;
          waterBlock.type = 15; // Agua
          chunk.setBlock(x, y, z, waterBlock);
        }
      }
    }
  }
  for (int x = 0; x < 16; x++) {
    for (int z = 0; z < 16; z++) {
      int worldX = cx * 16 + x;
      int worldZ = cz * 16 + z;

      // Obtener altura del terreno en esta posición
      int groundY = getTerrainHeight(worldX, worldZ);
      uint32_t hash = worldX * 374761393u + worldZ * 668265263u;
      hash = (hash ^ (hash >> 13)) * 1274126177u;

      float random = (hash & 0xFFFFFF) / float(0xFFFFFF);

      if (random < 0.002f) { // 0.2% probabilidad
        generateTree(worldX, groundY, worldZ, 0);
      }
    }
  }
}
struct Plane {
  vec3 normal;
  float d;
};
void extractFrustumPlanes(Plane planes[6], const mat4 &m) {
  // Left
  planes[0].normal.x = m[0][3] + m[0][0];
  planes[0].normal.y = m[1][3] + m[1][0];
  planes[0].normal.z = m[2][3] + m[2][0];
  planes[0].d = m[3][3] + m[3][0];

  // Right
  planes[1].normal.x = m[0][3] - m[0][0];
  planes[1].normal.y = m[1][3] - m[1][0];
  planes[1].normal.z = m[2][3] - m[2][0];
  planes[1].d = m[3][3] - m[3][0];

  // Bottom
  planes[2].normal.x = m[0][3] + m[0][1];
  planes[2].normal.y = m[1][3] + m[1][1];
  planes[2].normal.z = m[2][3] + m[2][1];
  planes[2].d = m[3][3] + m[3][1];

  // Top
  planes[3].normal.x = m[0][3] - m[0][1];
  planes[3].normal.y = m[1][3] - m[1][1];
  planes[3].normal.z = m[2][3] - m[2][1];
  planes[3].d = m[3][3] - m[3][1];

  // Near
  planes[4].normal.x = m[0][3] + m[0][2];
  planes[4].normal.y = m[1][3] + m[1][2];
  planes[4].normal.z = m[2][3] + m[2][2];
  planes[4].d = m[3][3] + m[3][2];

  // Far
  planes[5].normal.x = m[0][3] - m[0][2];
  planes[5].normal.y = m[1][3] - m[1][2];
  planes[5].normal.z = m[2][3] - m[2][2];
  planes[5].d = m[3][3] - m[3][2];

  // Normalizar planos
  for (int i = 0; i < 6; i++) {
    float length = glm::length(planes[i].normal);
    planes[i].normal /= length;
    planes[i].d /= length;
  }
}
bool isBoxVisible(const Plane planes[6], const vec3 &min, const vec3 &max) {

  for (int i = 0; i < 6; i++) {

    vec3 p;

    p.x = (planes[i].normal.x >= 0) ? max.x : min.x;
    p.y = (planes[i].normal.y >= 0) ? max.y : min.y;
    p.z = (planes[i].normal.z >= 0) ? max.z : min.z;

    if (glm::dot(planes[i].normal, p) + planes[i].d < 0) {
      return false; // completamente fuera
    }
  }

  return true;
  // Habria que hacer esto en cada chunk:
}
void World::render(vec3 cameraPos, mat4 view, mat4 projection, mat4 renderView,
                   mat4 renderProjection) {
  ivec2 centerChunk = getChunkPos(cameraPos);
  int cantRect = 0;
  int renderDist = 32;
  int generateDist = renderDist + 5;
  int maxChunksPerFrame = 5;
  int chunksCreados = 0;
  Chunk::sharedShader->use();
  Chunk::sharedShader->setUseTexture(true);
  Chunk::sharedShader->setProjectionMatrix(value_ptr(renderProjection));
  Chunk::sharedShader->setViewMatrix(glm::value_ptr(renderView));
  mat4 vp = projection * view;
  Plane planes[6];
  extractFrustumPlanes(planes, vp);
  if (centerChunk != chunkAnterior || 1 != 0) {
    chunkAnterior = centerChunk;
    for (int nivel = 0; nivel <= generateDist; nivel++) {
      if (nivel == 0) {
        int chunkX = centerChunk.x;
        int chunkZ = centerChunk.y;
        Chunk *chunk = getChunk(chunkX, chunkZ);
        if (chunk) {
          chunk->render();
        } else if (chunksCreados < maxChunksPerFrame) {
          createChunk(chunkX, chunkZ);
          chunksCreados++;
        }
      } else {
        int x = nivel;
        int z = nivel;
        while (z != -nivel) {
          int chunkX = centerChunk.x + x;
          int chunkZ = centerChunk.y + z;
          vec3 min(chunkX * 16, 0, chunkZ * 16);
          vec3 max(min.x + 15, 256, min.z + 15);
          if (isBoxVisible(planes, min, max)) {
            Chunk *chunk = getChunk(chunkX, chunkZ);
            if (chunk) {
              if (nivel <= renderDist)
                chunk->render();
            } else if (chunksCreados < maxChunksPerFrame) {
              createChunk(chunkX, chunkZ);
              chunksCreados++;
            }
          }
          z--;
        }
        while (x != -nivel) {

          int chunkX = centerChunk.x + x;
          int chunkZ = centerChunk.y + z;
          Chunk *chunk = getChunk(chunkX, chunkZ);
          vec3 min(chunkX * 16, 0, chunkZ * 16);
          vec3 max(min.x + 15, 256, min.z + 15);
          if (isBoxVisible(planes, min, max)) {

            if (chunk) {
              if (nivel <= renderDist)
                chunk->render();
            } else if (chunksCreados < maxChunksPerFrame) {
              createChunk(chunkX, chunkZ);
              chunksCreados++;
            }
          }

          x--;
        }
        while (z != nivel) {

          int chunkX = centerChunk.x + x;
          int chunkZ = centerChunk.y + z;
          Chunk *chunk = getChunk(chunkX, chunkZ);
          vec3 min(chunkX * 16, 0, chunkZ * 16);
          vec3 max(min.x + 15, 256, min.z + 15);
          if (isBoxVisible(planes, min, max)) {
            if (chunk) {
              if (nivel <= renderDist)
                chunk->render();
            } else if (chunksCreados < maxChunksPerFrame) {
              createChunk(chunkX, chunkZ);
              chunksCreados++;
            }
          }

          z++;
        }
        while (x != nivel) {

          int chunkX = centerChunk.x + x;
          int chunkZ = centerChunk.y + z;
          Chunk *chunk = getChunk(chunkX, chunkZ);
          vec3 min(chunkX * 16, 0, chunkZ * 16);
          vec3 max(min.x + 15, 256, min.z + 15);
          if (isBoxVisible(planes, min, max)) {
            if (chunk) {
              if (nivel <= renderDist)
                chunk->render();
            } else if (chunksCreados < maxChunksPerFrame) {
              createChunk(chunkX, chunkZ);
              chunksCreados++;
            }
          }

          x++;
        }
      }
    }
  } else {
    for (int dx = -renderDist; dx <= renderDist; dx++) {
      for (int dz = -renderDist; dz <= renderDist; dz++) {
        int chunkX = centerChunk.x + dx;
        int chunkZ = centerChunk.y + dz;
        Chunk *chunk = getChunk(chunkX, chunkZ);
        vec3 min(chunkX * 16, 0, chunkZ * 16);
        vec3 max(min.x + 15, 256, min.z + 15);
        if (isBoxVisible(planes, min, max)) {
          if (chunk) {
            chunk->render();
          } else if (chunksCreados < maxChunksPerFrame) {
            createChunk(chunkX, chunkZ);
            chunksCreados++;
          }
        }
      }
    }
  }
}
void World::update() {}
