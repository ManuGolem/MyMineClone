#include "../include/chunk.h"
#include "../include/blocksRegistry.h"
#include "../include/world.h"
#include <array>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <iterator>
#include <memory>
#include <mutex>
static int OAK_LEAVES = BlockRegistry::getType("oak_leaves");
static int GRASS_BLOCK = BlockRegistry::getType("grass_block");
static int OAK_LOG = BlockRegistry::getType("oak_log");
static int SPRUCE_LOG = BlockRegistry::getType("spruce_log");
static int BOOKSHELF = BlockRegistry::getType("bookshelf");
static int CACTUS = BlockRegistry::getType("cactus");
static int SPRUCE_LEAVES = BlockRegistry::getType("spruce_leaves");
static int SNOWY_GRASS_BLOCK = BlockRegistry::getType("snowy_grass_block");
static int TNT_BLOCK = BlockRegistry::getType("tnt");
static int POPPY = BlockRegistry::getType("poppy");
static int DANDELION = BlockRegistry::getType("dandelion");
static int OAK_SAPLING = BlockRegistry::getType("oak_sapling");
static int JUNGLE_SAPLING = BlockRegistry::getType("jungle_sapling");
static int SPRUCE_SAPLING = BlockRegistry::getType("spruce_sapling");
static int RED_MUSHROOM = BlockRegistry::getType("red_mushroom");
static int SHORT_GRASS = BlockRegistry::getType("short_grass");
static int BROWN_MUSHROOM = BlockRegistry::getType("brown_mushroom");
bool isCrossBlock(int16_t type) {
    return (type == POPPY || type == DANDELION || type == OAK_SAPLING || type == JUNGLE_SAPLING || type == SPRUCE_SAPLING || type == SHORT_GRASS ||
            type == RED_MUSHROOM || type == BROWN_MUSHROOM);
}
bool esTransparent(int16_t type) {
    return (type == 0 || type == OAK_LEAVES || type == CACTUS || type == SPRUCE_LEAVES || isCrossBlock(type));
}
/* -> aca seria para comprobar solo los vertices adyacentes de la cara pero no me convencio ya que al formar el rectangulo se difuminaba y no quedaba bien.
 * -> pero deberia pensar algo similar ya que es mas optimo asi.[Creo menos rectangulos por ende menos triangulos]
bool mismoAOBordeHorizontal(const faceAO& izq, const faceAO& der) {
    const float eps = 1e-5f;
    return fabsf(izq.ao1 - der.ao0) < eps && fabsf(izq.ao2 - der.ao3) < eps;
}
bool mismoAOBordeVertical(const faceAO& arriba, const faceAO& abajo) {
    const float eps = 1e-5f;
    return fabsf(arriba.ao3 - abajo.ao0) < eps && fabsf(arriba.ao2 - abajo.ao1) < eps;
}
*/

bool mismoAO(const faceAO& a, const faceAO& b) {
    return a.ao0 == b.ao0 && a.ao1 == b.ao1 && a.ao2 == b.ao2 && a.ao3 == b.ao3;
}
template <size_t FILAS, size_t COLUMNAS> vector<Rectangulo> formarRectangulos(bloqueCapa (&capa)[FILAS][COLUMNAS]) {
    vector<Rectangulo> rectangulos;
    bool procesado[FILAS][COLUMNAS] = {false};
    for (int i = 0; i < FILAS; i++) {
        for (int j = 0; j < COLUMNAS; j++) {
            if (capa[i][j].tipo != 0 && !procesado[i][j]) {
                int16_t tipo_actual = capa[i][j].tipo;
                faceAO ao_actual = capa[i][j].ao;
                int ancho = 1;
                int alto = 1;
                // Expandir ancho mientras sea el MISMO tipo
                // Tambien debo comprobar si la cara tiene el mismo ao (no se como hacer esto aun)
                while (i + ancho < FILAS && capa[i + ancho][j].tipo == tipo_actual && !procesado[i + ancho][j] && mismoAO(ao_actual, capa[i + ancho][j].ao)) {
                    ancho++;
                }

                bool expandible = true;

                // Expandir alto mientras TODA la fila sea del MISMO tipo
                // Comprobar AO
                while (j + alto < COLUMNAS && expandible) {
                    for (int dj = 0; dj < ancho; dj++) {
                        if (capa[i + dj][j + alto].tipo != tipo_actual || procesado[i + dj][j + alto] || !mismoAO(ao_actual, capa[i + dj][j + alto].ao)) {
                            expandible = false;
                            break;
                        }
                    }
                    if (expandible) {
                        alto++;
                    }
                }
                for (int k = i; k < i + ancho; k++) {
                    for (int l = j; l < j + alto; l++) {
                        procesado[k][l] = true;
                    }
                }
                Rectangulo r;
                r.y1 = i;
                r.x1 = j;
                r.y2 = i + ancho - 1;
                r.x2 = j + alto - 1;
                r.tipoBloque = tipo_actual;
                r.face = ao_actual;
                rectangulos.push_back(r);
            }
        }
    }
    return rectangulos;
}

Shader* Chunk::sharedShader = nullptr;
Chunk::Chunk() : world(nullptr), needsUpdate(true), isUpdating(false) {
    memset(blocks, 0, sizeof(blocks));
    if (sharedShader == nullptr) {
        sharedShader = new Shader();
    }
}
faceAO Chunk::calcularAO(int globalX, int globalY, int globalZ, int eje, int direccion) {
    faceAO ao;

    ao.ao0 = calcularAOVertex(globalX, globalY, globalZ, eje, direccion, 0);
    ao.ao1 = calcularAOVertex(globalX, globalY, globalZ, eje, direccion, 1);
    ao.ao2 = calcularAOVertex(globalX, globalY, globalZ, eje, direccion, 2);
    ao.ao3 = calcularAOVertex(globalX, globalY, globalZ, eje, direccion, 3);

    return ao;
}

float Chunk::calcularAOVertex(int globalX, int globalY, int globalZ, int eje, int direccion, int vertice) {
    // Ver que bloque vecino tiene dependiendo del vertice (el 0 es abajo a la izquierda, luego el 1 es abajo a la derecha , el 2 es arriba a la derecha y el 3 es arriba
    // a la izquierda).
    // => vértice 0: (-1, -1), 1: (+1, -1), 2: (+1, +1), 3: (-1, +1)
    int signU, signV;
    if (vertice == 0) {
        signU = -1;
        signV = -1;
    } else if (vertice == 1) {
        signU = 1;
        signV = -1;
    } else if (vertice == 2) {
        signU = 1;
        signV = 1;
    } else {
        signU = -1;
        signV = 1;
    }

    int offX_side1 = 0, offY_side1 = 0, offZ_side1 = 0;
    int offX_side2 = 0, offY_side2 = 0, offZ_side2 = 0;
    int offX_corner = 0, offY_corner = 0, offZ_corner = 0;

    // desplazamiento de la normal de la cara, o sea los bloques que le hacen sombra tiene que estar mas "adelante" que el bloque sombreado(?)
    int normX = 0, normY = 0, normZ = 0;
    if (eje == 0)
        normX = direccion;
    else if (eje == 1)
        normY = direccion;
    else if (eje == 2)
        normZ = direccion;

    // Creo que puedo eliminar este if ya que la altura de los bloques las hago depender de la direccion,
    // o sea llamar normY=0 si esta en eje==2 o eje ==0 , no se me ocurre bien aun,
    if (eje == 0) { // cara perpendicular a X, ejes tangentes: U = Z, V = Y
        offX_side1 = normX;
        offY_side1 = 0;
        offZ_side1 = signU;
        offX_side2 = normX;
        offY_side2 = signV;
        offZ_side2 = 0;
        offX_corner = normX;
        offY_corner = signV;
        offZ_corner = signU;
    } else if (eje == 1) { // cara perpendicular a Y, ejes tangentes: U = X, V = Z
        offX_side1 = signU;
        offY_side1 = normY;
        offZ_side1 = 0;
        offX_side2 = 0;
        offY_side2 = normY;
        offZ_side2 = signV;
        offX_corner = signU;
        offY_corner = normY;
        offZ_corner = signV;
    } else if (eje == 2) { // cara perpendicular a Z, ejes tangentes: U = X, V = Y
        offX_side1 = signU;
        offY_side1 = 0;
        offZ_side1 = normZ;
        offX_side2 = 0;
        offY_side2 = signV;
        offZ_side2 = normZ;
        offX_corner = signU;
        offY_corner = signV;
        offZ_corner = normZ;
    }
    // Adyacente 1
    int wx1 = globalX + offX_side1;
    int wy1 = globalY + offY_side1;
    int wz1 = globalZ + offZ_side1;
    // Adyacente 2
    int wx2 = globalX + offX_side2;
    int wy2 = globalY + offY_side2;
    int wz2 = globalZ + offZ_side2;
    // Diagonal
    int wxc = globalX + offX_corner;
    int wyc = globalY + offY_corner;
    int wzc = globalZ + offZ_corner;
    // Creo que voy a tener que hacer un buffer con los bloques a consultar, al igual que al generar mesh.
    int16_t type1 = world->getBlockSafe(wx1, wy1, wz1);
    int16_t type2 = world->getBlockSafe(wx2, wy2, wz2);
    int16_t typeC = world->getBlockSafe(wxc, wyc, wzc);

    bool side1 = !esTransparent(type1);
    bool side2 = !esTransparent(type2);
    bool corner = !esTransparent(typeC);

    if (side1 && side2)
        return 0.1f;
    float ao = (3.0f - (side1 + side2 + corner)) / 3.0f;
    if (ao < 0.1f)
        ao = 0.1f;
    return ao;
}
void Chunk::cargarVerticesCross(const Rectangulo& r, int fijo, int16_t tipo_bloque, vector<float>& vData, vector<unsigned int>& iData, unsigned int& vCount) {
    // El ao en esta funcion lo dejo fijo en 1, es decir que todo bloque cross no tiene AO
    int eje = 0;
    int direccion = 1;
    float base = vCount;
    float offsetX = nroChunkX * 16.0f;
    float offsetZ = nroChunkZ * 16.0f;
    float rcolor = 1.0f;
    float gcolor = 1.0f;
    float bcolor = 1.0f;
    if (tipo_bloque == SHORT_GRASS) {
        rcolor = 0.5f;
        gcolor = 0.9f;
        bcolor = 0.5f;
    }
    // Calculo uv para el atlas (0,0) es abajo a la izquierda
    float u_min, u_max, v_min, v_max;
    float ancho = r.x2 - r.x1 + 1;
    float alto = r.y2 - r.y1 + 1;
    int columna = (tipo_bloque - 1) % 16;
    int fila = (tipo_bloque - 1) / 16;
    float tileSize = 1.0f / 16.0f;
    float offsetU = columna * tileSize;
    float offsetV = 1.0f - (fila + 1) * tileSize;
    float xPos = offsetX + fijo + 0.5;
    float y1 = r.y1 - 1.0f;
    float y2 = r.y2;
    float z1 = offsetZ + r.x1 - 0.5f;
    float z2 = offsetZ + r.x2 + 0.5f;
    float vertex[44] = {// VERTEX 1
                        xPos, y1, z1, rcolor, gcolor, bcolor, 0.0f, 0.0f, offsetU, offsetV, 1,
                        // VERTEX 2
                        xPos - 1.0f, y1, z2, rcolor, gcolor, bcolor, ancho, 0.0f, offsetU, offsetV, 1,
                        // VERTEX 3
                        xPos - 1.0f, y2, z2, rcolor, gcolor, bcolor, ancho, alto, offsetU, offsetV, 1,
                        // VERTEX 4
                        xPos, y2, z1, rcolor, gcolor, bcolor, 0.0f, alto, offsetU, offsetV, 1};
    vData.insert(vData.end(), std::begin(vertex), std::end(vertex));
    // Cara mirando hacia +X
    iData.push_back(base);     // Abajo derecha
    iData.push_back(base + 3); // Arriba derecha
    iData.push_back(base + 2); // Arriba izquierda
    iData.push_back(base);     // Abajo derecha
    iData.push_back(base + 2); // Arriba izquierda
    iData.push_back(base + 1); // Abajo derecha
    // Cara mirando -x
    iData.push_back(base + 1); // Abajo derecha
    iData.push_back(base + 2); // Arriba izquierda
    iData.push_back(base);     // Abajo derecha
    iData.push_back(base + 2); // Arriba izquierda
    iData.push_back(base + 3); // Arriba derecha
    iData.push_back(base);     // Abajo derecha
    // La otra cara diagonal
    float vertex2[44] = {// VERTEX 1
                         xPos - 1.0f, y1, z1, rcolor, gcolor, bcolor, 0.0f, 0.0f, offsetU, offsetV, 1,
                         // VERTEX 2
                         xPos, y1, z2, rcolor, gcolor, bcolor, ancho, 0.0f, offsetU, offsetV, 1,
                         // VERTEX 3
                         xPos, y2, z2, rcolor, gcolor, bcolor, ancho, alto, offsetU, offsetV, 1,
                         // VERTEX 4
                         xPos - 1.0f, y2, z1, rcolor, gcolor, bcolor, 0.0f, alto, offsetU, offsetV, 1};
    vData.insert(vData.end(), std::begin(vertex2), std::end(vertex2));
    base = base + 4;
    // Cara mirando hacia +z
    iData.push_back(base);     // Abajo derecha
    iData.push_back(base + 3); // Arriba derecha
    iData.push_back(base + 2); // Arriba izquierda
    iData.push_back(base);     // Abajo derecha
    iData.push_back(base + 2); // Arriba izquierda
    iData.push_back(base + 1); // Abajo derecha
    // Cara mirando -z
    iData.push_back(base + 1); // Abajo derecha
    iData.push_back(base + 2); // Arriba izquierda
    iData.push_back(base);     // Abajo derecha
    iData.push_back(base + 2); // Arriba izquierda
    iData.push_back(base + 3); // Arriba derecha
    iData.push_back(base);     // Abajo derecha
    vCount += 8;
}
void Chunk::cargarVertices(const Rectangulo& r, int eje, int direccion, int fijo, int16_t tipo_bloque, vector<float>& vData, vector<unsigned int>& iData,
                           unsigned int& vCount) {
    float base = vCount;
    float sizeOffset = 0.5f;
    float offsetX = nroChunkX * 16.0f;
    float offsetZ = nroChunkZ * 16.0f;
    float rcolor = 1.0f;
    float gcolor = 1.0f;
    float bcolor = 1.0f;
    if (tipo_bloque == OAK_LEAVES) {
        rcolor = 0.3f;
        gcolor = 0.8f;
        bcolor = 0.3f;
    } else if (tipo_bloque == SPRUCE_LEAVES) {
        rcolor = 0.3f;
        gcolor = 0.5f;
        bcolor = 0.3f;
    } else if (tipo_bloque == CACTUS) {
        sizeOffset = 0.4375f; // Si el bloque mide 1, cada pixel mide 1/16 => 0.5 - 1/16 (le
                              // quito un pixel de cada lado el cactus es 14*16*14)
    }

    // Calculo uv para el atlas (0,0) es abajo a la izquierda
    float u_min, u_max, v_min, v_max;
    float ancho = r.x2 - r.x1 + 1;
    float alto = r.y2 - r.y1 + 1;
    int columna = (tipo_bloque - 1) % 16;
    int fila = (tipo_bloque - 1) / 16;
    float tileSize = 1.0f / 16.0f;
    float offsetU = columna * tileSize;
    float offsetV = 1.0f - (fila + 1) * tileSize;

    float ao0 = r.face.ao0;
    float ao1 = r.face.ao1;
    float ao2 = r.face.ao2;
    float ao3 = r.face.ao3;
    if (eje == 0) {
        float xPos = offsetX + fijo + (direccion == 1 ? sizeOffset : -sizeOffset);
        float y1 = r.y1 - 1.0f;
        float y2 = r.y2;
        float z1 = offsetZ + r.x1 - 0.5f;
        float z2 = offsetZ + r.x2 + 0.5f;

        float vertex[44] = {// VERTEX 1
                            xPos, y1, z1, rcolor, gcolor, bcolor, 0.0f, 0.0f, offsetU, offsetV, ao0,
                            // VERTEX 2
                            xPos, y1, z2, rcolor, gcolor, bcolor, ancho, 0.0f, offsetU, offsetV, ao1,
                            // VERTEX 3
                            xPos, y2, z2, rcolor, gcolor, bcolor, ancho, alto, offsetU, offsetV, ao2,
                            // VERTEX 4
                            xPos, y2, z1, rcolor, gcolor, bcolor, 0.0f, alto, offsetU, offsetV, ao3};
        vData.insert(vData.end(), std::begin(vertex), std::end(vertex));

    } else if (eje == 1) { // CARAS EN Y
        alto = r.x2 - r.x1 + 1;
        ancho = r.y2 - r.y1 + 1;
        if (tipo_bloque == GRASS_BLOCK) {
            if (direccion == 1) {
                rcolor = 0.3f;
                gcolor = 0.8f;
                bcolor = 0.3f;
                columna = 8;
                fila = 2;
            } else {
                columna = 2;
                fila = 0;
            }
        } else if (tipo_bloque == OAK_LOG) {
            columna = 5;
            fila = 1;
        } else if (tipo_bloque == BOOKSHELF) {
            columna = 4;
            fila = 0;
        } else if (tipo_bloque == CACTUS) {
            columna = 5;
            fila = 4;
        } else if (tipo_bloque == SPRUCE_LOG) {
            columna = 5;
            fila = 1;
            rcolor = 0.55f;
            gcolor = 0.4f;
            bcolor = 0.3f;
        } else if (tipo_bloque == SNOWY_GRASS_BLOCK) {
            if (direccion == 1) {
                rcolor = 1.8f;
                gcolor = 1.8f;
                bcolor = 2.0f;
                columna = 8;
                fila = 4;
            } else {
                columna = 2;
                fila = 0;
            }
        } else if (tipo_bloque == TNT_BLOCK) {
            if (direccion == 1) {
                columna = 9;
                fila = 0;
            } else {
                columna = 10;
                fila = 0;
            }
        }
        offsetU = columna * tileSize;
        offsetV = 1.0f - (fila + 1) * tileSize;

        float yPos = fijo + (direccion == 1 ? 0.0f : -1.0f);
        float x1 = offsetX + r.y1 - 0.5f;
        float x2 = offsetX + r.y2 + 0.5f;
        float z1 = offsetZ + r.x1 - 0.5f;
        float z2 = offsetZ + r.x2 + 0.5f;

        float vertex[44] = {x1,      yPos,    z1,    rcolor, gcolor,  bcolor,  0.0f,   0.0f,   offsetU, offsetV, ao0,    x2,      yPos,    z1,    rcolor,
                            gcolor,  bcolor,  ancho, 0.0f,   offsetU, offsetV, ao1,    x2,     yPos,    z2,      rcolor, gcolor,  bcolor,  ancho, alto,
                            offsetU, offsetV, ao2,   x1,     yPos,    z2,      rcolor, gcolor, bcolor,  0.0f,    alto,   offsetU, offsetV, ao3};
        vData.insert(vData.end(), std::begin(vertex), std::end(vertex));

    } else if (eje == 2) { // CARAS EN Z
        float zPos = offsetZ + fijo + (direccion == 1 ? sizeOffset : -sizeOffset);
        float y1 = r.x1 - 1.0f;
        float y2 = r.x2;
        float x1 = offsetX + r.y1 - 0.5f;
        float x2 = offsetX + r.y2 + 0.5f;

        float vertex[44] = {x1,      y1,      zPos, rcolor, gcolor,  bcolor,  0.0f,   0.0f,   offsetU, offsetV, ao0,    x2,      y1,      zPos, rcolor,
                            gcolor,  bcolor,  alto, 0.0f,   offsetU, offsetV, ao1,    x2,     y2,      zPos,    rcolor, gcolor,  bcolor,  alto, ancho,
                            offsetU, offsetV, ao2,  x1,     y2,      zPos,    rcolor, gcolor, bcolor,  0.0f,    ancho,  offsetU, offsetV, ao3};
        vData.insert(vData.end(), std::begin(vertex), std::end(vertex));
    }
    if (eje == 2)
        direccion *= -1;
    if (direccion == 1) {
        iData.push_back(base);     // 0
        iData.push_back(base + 3); // 3
        iData.push_back(base + 2); // 2
        iData.push_back(base);     // 0
        iData.push_back(base + 2); // 2
        iData.push_back(base + 1); // 1
    } else {
        iData.push_back(base + 1); // 1
        iData.push_back(base + 2); // 2
        iData.push_back(base + 3); // 3
        iData.push_back(base + 1); // 1
        iData.push_back(base + 3); // 3
        iData.push_back(base);     // 0
    }
    vCount += 4;
}
void Chunk::generateMesh() {
    vector<float> newVertexData;
    vector<unsigned int> newIndexData;
    unsigned int newVertexCount = 0;
    int baseX = nroChunkX * 16;
    int baseZ = nroChunkZ * 16;
    array<array<array<int16_t, 16>, 512>, 2> blocksInX;
    array<array<array<int16_t, 512>, 16>, 2> blocksInZ;
    blocksInX[0] = world->getFaceInXSafe(nroChunkX - 1, nroChunkZ, 15);
    blocksInX[1] = world->getFaceInXSafe(nroChunkX + 1, nroChunkZ, 0);
    blocksInZ[0] = world->getFaceInZSafe(nroChunkX, nroChunkZ - 1, 15);
    blocksInZ[1] = world->getFaceInZSafe(nroChunkX, nroChunkZ + 1, 0);
    lock_guard<mutex> lock(mutexBlocks);
    struct CrossBlock {
        int x, y, z;
        int16_t tipo;
    };
    vector<CrossBlock> crossBlock;
    for (int x = 0; x < 16; x++) {
        bloqueCapa capasIzquierdas[512][16] = {};
        bloqueCapa capasDerechas[512][16] = {};
        for (int i = 0; i < 512; i++) {
            for (int j = 0; j < 16; j++) {
                int16_t typeblock = blocks[x][i][j];
                if (isCrossBlock(typeblock)) {
                    crossBlock.push_back({x, i, j, typeblock});
                    continue;
                }
                if (typeblock != 0) {
                    int globalX = x + baseX;
                    int globalZ = j + baseZ;
                    // Cara derecha (x+)
                    if (x == 15) {
                        if (esTransparent(blocksInX[1][i][j])) {
                            capasDerechas[i][j].tipo = blocks[x][i][j];
                            capasDerechas[i][j].ao = calcularAO(globalX, i, globalZ, 0, 1);
                        }
                    } else if (esTransparent(blocks[x + 1][i][j])) {
                        capasDerechas[i][j].tipo = blocks[x][i][j];
                        capasDerechas[i][j].ao = calcularAO(globalX, i, globalZ, 0, 1);
                    }
                    // Cara izquierda (x-)
                    if (x == 0) {
                        if (esTransparent(blocksInX[0][i][j])) {
                            capasIzquierdas[i][j].tipo = blocks[x][i][j];
                            capasIzquierdas[i][j].ao = calcularAO(globalX, i, globalZ, 0, -1);
                        }

                    } else if (esTransparent(blocks[x - 1][i][j])) {
                        capasIzquierdas[i][j].tipo = blocks[x][i][j];
                        capasIzquierdas[i][j].ao = calcularAO(globalX, i, globalZ, 0, -1);
                    }
                }
            }
        }
        vector<Rectangulo> rectsDer = formarRectangulos(capasDerechas);
        for (const Rectangulo& r : rectsDer) {
            cargarVertices(r, 0, 1, x, r.tipoBloque, newVertexData, newIndexData, newVertexCount);
        }
        vector<Rectangulo> rectsIzq = formarRectangulos(capasIzquierdas);

        for (const Rectangulo& r : rectsIzq) {
            cargarVertices(r, 0, -1, x, r.tipoBloque, newVertexData, newIndexData, newVertexCount);
        }
    }
    for (int y = 0; y < 512; y++) {
        bloqueCapa capasSuperiores[16][16] = {};
        bloqueCapa capasInferiores[16][16] = {};
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 16; j++) {
                int16_t typeblock = blocks[i][y][j];
                if (isCrossBlock(typeblock)) {
                    continue;
                }
                if (typeblock != 0) {
                    // Cara inferior (y-)
                    if ((y == 0) || esTransparent(blocks[i][y - 1][j])) {
                        capasInferiores[i][j].tipo = blocks[i][y][j];
                        capasInferiores[i][j].ao = calcularAO(i + baseX, y, j + baseZ, 1, -1);
                    }
                    // Cara superior (y+)
                    if ((y == 511) || esTransparent(blocks[i][y + 1][j])) {
                        capasSuperiores[i][j].tipo = blocks[i][y][j];
                        capasSuperiores[i][j].ao = calcularAO(i + baseX, y, j + baseZ, 1, 1);
                    }
                }
            }
        }
        vector<Rectangulo> rectsInf = formarRectangulos(capasInferiores);
        for (const Rectangulo& r : rectsInf) {
            cargarVertices(r, 1, -1, y, r.tipoBloque, newVertexData, newIndexData, newVertexCount);
        }
        vector<Rectangulo> rectsSup = formarRectangulos(capasSuperiores);

        for (const Rectangulo& r : rectsSup) {
            cargarVertices(r, 1, 1, y, r.tipoBloque, newVertexData, newIndexData, newVertexCount);
        }
    }
    for (int z = 0; z < 16; z++) {
        bloqueCapa capasFrontal[16][512] = {};
        bloqueCapa capasTrasera[16][512] = {};

        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 512; j++) {
                int16_t typeblock = blocks[i][j][z];
                if (isCrossBlock(typeblock)) {
                    continue;
                }
                if (typeblock != 0) {
                    int globalX = i + baseX;
                    int globalZ = z + baseZ;
                    // z+
                    if (z == 15) {
                        if (esTransparent(blocksInZ[1][i][j])) {
                            capasFrontal[i][j].tipo = blocks[i][j][z];
                            capasFrontal[i][j].ao = calcularAO(globalX, j, globalZ, 2, 1);
                        }
                    } else if (esTransparent(blocks[i][j][z + 1])) {
                        capasFrontal[i][j].tipo = blocks[i][j][z];
                        capasFrontal[i][j].ao = calcularAO(globalX, j, globalZ, 2, 1);
                    }
                    // Cara izquierda (Z-)
                    if (z == 0) {
                        if (esTransparent(blocksInZ[0][i][j])) {
                            capasTrasera[i][j].tipo = blocks[i][j][z];
                            capasTrasera[i][j].ao = calcularAO(globalX, j, globalZ, 2, -1);
                        }
                    } else if (esTransparent(blocks[i][j][z - 1])) {
                        capasTrasera[i][j].tipo = blocks[i][j][z];
                        capasTrasera[i][j].ao = calcularAO(globalX, j, globalZ, 2, -1);
                    }
                }
            }
        }

        vector<Rectangulo> rectsFrontal = formarRectangulos(capasFrontal);
        for (const Rectangulo& r : rectsFrontal) {
            cargarVertices(r, 2, 1, z, r.tipoBloque, newVertexData, newIndexData, newVertexCount);
        }

        vector<Rectangulo> rectsTrasera = formarRectangulos(capasTrasera);
        for (const Rectangulo& r : rectsTrasera) {
            cargarVertices(r, 2, -1, z, r.tipoBloque, newVertexData, newIndexData, newVertexCount);
        }
    }
    int crosSize = crossBlock.size();
    for (const auto& cb : crossBlock) {
        Rectangulo r;
        r.x1 = cb.z;
        r.x2 = cb.z;
        r.y1 = cb.y;
        r.y2 = cb.y;
        r.tipoBloque = cb.tipo;
        cargarVerticesCross(r, cb.x, cb.tipo, newVertexData, newIndexData, newVertexCount);
    }
    {
        std::lock_guard<std::mutex> lock(mutexVertex);
        vertexData.swap(newVertexData);
        indexData.swap(newIndexData);
        vertexCount = newVertexCount;
    }
    needsUpdate = false;
    needsBufferUpdate = true;
}
void Chunk::setBlock(int x, int y, int z, const int16_t& block) {
    lock_guard<mutex> lock(mutexBlocks);
    blocks[x][y][z] = block;
    needsUpdate = true;
}
int16_t Chunk::getBlock(int x, int y, int z) const {
    if (x < 0 || x >= 16 || z < 0 || z >= 16) {
        return 0;
    }
    return blocks[x][y][z];
}
bool Chunk::isEmpty() const {
    return vertexData.empty();
}
void Chunk::setNroChunk(int chunkx, int chunkz) {
    nroChunkX = chunkx;
    nroChunkZ = chunkz;
}
void Chunk::render() {
    if (needsUpdate && !isUpdating) {
        generateMesh();
    }
    if (needsBufferUpdate) {
        lock_guard<mutex> lock(mutexVertex);
        if (!vertexData.empty()) {
            chunkBuffer->uploadData(vertexData, indexData);
        }
    }
    chunkBuffer->render();
}
void Chunk::cleanup() {
    chunkBuffer.reset();

    // Limpiar datos
    vertexData.clear();
    indexData.clear();
    vertexCount = 0;
    needsUpdate = true;

    // Opcional: liberar la memoria de los vectores
    vector<float>().swap(vertexData);
    vector<unsigned int>().swap(indexData);
}
