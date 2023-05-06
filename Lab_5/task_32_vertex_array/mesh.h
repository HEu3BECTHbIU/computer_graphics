#ifndef MESH_H
#define MESH_H

#include <assimp/Importer.hpp> // подключение библиотеки assimp для загрузки моделей
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <map>
#include <vector>
#include <GL/glew.h>

#include "util.h"
#include "math_3d.h"
#include "texture.h"
#include "engine_common.h"

struct Vertex
{
    Vector3f m_pos;
    Vector2f m_tex;
    Vector3f m_normal;

    Vertex() {}

    Vertex(const Vector3f& pos, const Vector2f& tex, const Vector3f& normal)
    {
        m_pos = pos;
        m_tex = tex;
        m_normal = normal;
    }
};

class Mesh // класс меш (полигональная сетка)
{
public:
    Mesh();
    ~Mesh();
    
    bool LoadMesh(const std::string& Filename);
    void Render();

private:
    bool InitFromScene(const aiScene* pScene, const std::string& Filename);
    void InitMesh(const aiMesh* paiMesh,
        std::vector<Vector3f>& Positions,
        std::vector<Vector3f>& Normals,
        std::vector<Vector2f>& TexCoords,
        std::vector<unsigned int>& Indices);
    bool InitMaterials(const aiScene* pScene, const std::string& Filename);
    void Clear();

#define INVALID_MATERIAL 0xFFFFFFFF

#define INDEX_BUFFER 0    
#define POS_VB       1
#define NORMAL_VB    2
#define TEXCOORD_VB  3

    // объект массива вершин.
    GLuint m_VAO;
    // 4 буфера - буферы индексов, позиции, нормалей и координат текстур
    GLuint m_Buffers[4];

    struct MeshEntry // структура отдельного меша
    {
        MeshEntry()
        {
            NumIndices = 0;
            BaseVertex = 0;
            BaseIndex = 0;
            MaterialIndex = INVALID_MATERIAL;
        }
        unsigned int NumIndices; // количество индексов в субкомпоненте
        unsigned int BaseVertex; // позиция, с которой начинается субкомпонент в вершинном буфере
        unsigned int BaseIndex; // где субкомпонент начинается внутри буфера индексов
        unsigned int MaterialIndex; // индекс материала, указывает на одну из текстур из вектора
    };

    std::vector<MeshEntry> m_Entries;  // вектор структур для хранения более 1 меша
    std::vector<Texture*> m_Textures; // вектор текстур
};

#endif /* MESH_H */