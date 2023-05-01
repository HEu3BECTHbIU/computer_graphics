#include <assert.h>
#include <iostream>
#include "mesh.h"

Mesh::MeshEntry::MeshEntry() // обнуление переменных при инициализации
{
    VB = INVALID_OGL_VALUE;
    IB = INVALID_OGL_VALUE;
    NumIndices = 0;
    MaterialIndex = INVALID_MATERIAL;
}

Mesh::MeshEntry::~MeshEntry() // деструктор (удаляет массивы)
{
    if (VB != INVALID_OGL_VALUE) 
    {
        glDeleteBuffers(1, &VB);
    }

    if (IB != INVALID_OGL_VALUE) 
    {
        glDeleteBuffers(1, &IB);
    }
}

bool Mesh::MeshEntry::Init(const std::vector<Vertex>& Vertices,
    const std::vector<unsigned int>& Indices)
{
    NumIndices = Indices.size();
    // создание вершинного массива OpenGL на основе модели
    glGenBuffers(1, &VB);
    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(),
        &Vertices[0], GL_STATIC_DRAW);
    // создание индексного массива OpenGL на основе модели
    glGenBuffers(1, &IB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * NumIndices,
        &Indices[0], GL_STATIC_DRAW);

    return true;
}

void Mesh::Clear() // функция удаления указателей на объекты текстуры
{
    for (unsigned int i = 0; i < m_Textures.size(); i++)
    {
        SAFE_DELETE(m_Textures[i]);
    }
}
// функция загрузки меша
bool Mesh::LoadMesh(const std::string& Filename) 
{
    // Удаляем данные предыдущей модели (если она была загружена)
    Clear();

    bool Ret = false;
    Assimp::Importer Importer;
    // загрузка модели из файла (флаги - деление полигонов на треугольники, подсчет нормалей (если еще нет), поворот вокруг оси y)
    const aiScene* pScene = Importer.ReadFile(Filename.c_str(),
        aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
    // если удачно, идем дальше
    if (pScene) 
    {
        Ret = InitFromScene(pScene, Filename);
    }
    else {
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), Importer.GetErrorString());
    }

    return Ret;
}
// функция инициализации объекта меша
bool Mesh::InitFromScene(const aiScene* pScene, const std::string& Filename)
{
    // изменям размер массивов в зависимости от количества мешей и текстур
    m_Entries.resize(pScene->mNumMeshes);
    m_Textures.resize(pScene->mNumMaterials);

    // Инициализируем меши один за другим
    for (unsigned int i = 0; i < m_Entries.size(); i++)
    {
        const aiMesh* paiMesh = pScene->mMeshes[i];
        InitMesh(i, paiMesh);
    }
    return InitMaterials(pScene, Filename);
}
// иницилазиация одного меша
void Mesh::InitMesh(unsigned int Index, const aiMesh* paiMesh)
{
    m_Entries[Index].MaterialIndex = paiMesh->mMaterialIndex;

    std::vector<Vertex> Vertices; //вектор вершин
    std::vector<unsigned int> Indices; // вектор индексов
    std::cout << paiMesh->mNumVertices << "\n";
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
    //заполняем массивы используя соответствующие поля объекта aiMesh
    for (unsigned int i = 0; i < paiMesh->mNumVertices; i++)
    {
        const aiVector3D* pPos = &(paiMesh->mVertices[i]); // позиция
        const aiVector3D* pNormal = &(paiMesh->mNormals[i]); // нормаль
        // кордината текстуры
        const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ?
            &(paiMesh->mTextureCoords[0][i]) : &Zero3D;
        // собираем все вместе в объект структуры
        Vertex v(Vector3f(pPos->x, pPos->y, pPos->z),
            Vector2f(pTexCoord->x, pTexCoord->y),
            Vector3f(pNormal->x, pNormal->y, pNormal->z));
        Vertices.push_back(v); // добавляем в массив вершин
    }
    // заполняем индексный массив при помощи объекта aiFace, хранящего индексы вершин модели
    for (unsigned int i = 0; i < paiMesh->mNumFaces; i++)
    {
        const aiFace& Face = paiMesh->mFaces[i];
        assert(Face.mNumIndices == 3);
        Indices.push_back(Face.mIndices[0]);
        Indices.push_back(Face.mIndices[1]);
        Indices.push_back(Face.mIndices[2]);
    }
    m_Entries[Index].Init(Vertices, Indices); // привязка (см Init)
}
// функция загружает все текстуры, которые используются в модели
bool Mesh::InitMaterials(const aiScene* pScene, const std::string& Filename)
{
 //    Извлекаем директорию из полного имени файла
    std::string::size_type SlashIndex = Filename.find_last_of("/");
    std::string Dir;

    if (SlashIndex == std::string::npos) 
    {
        Dir = ".";
    }
    else if (SlashIndex == 0) 
    {
        Dir = "/";
    }
    else
    {
        Dir = Filename.substr(0, SlashIndex);
    }
    bool Ret = true;

    // Инициализируем материал
    for (unsigned int i = 0; i < pScene->mNumMaterials; i++) 
    {
        const aiMaterial* pMaterial = pScene->mMaterials[i];

        m_Textures[i] = NULL;
        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) 
        {
            aiString Path;
            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path,
                NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) 
            {
                std::string FullPath = Dir + "/" +Path.data;
               std::cout << FullPath << " \n";
              // FullPath = "cube3.jpg";
                m_Textures[i] = new Texture(GL_TEXTURE_2D, FullPath.c_str());

                if (!m_Textures[i]->Load())
                {
                    printf("Error loading texture '%s'\n", FullPath.c_str());
                    delete m_Textures[i];
                    m_Textures[i] = NULL;
                    Ret = false;
                }
                else
                {
                    printf("Loaded texture '%s'\n", FullPath.c_str());
                }
            }
        }
        // Загружаем белую текстуру если модель не имеет собственной
       // if (!m_Textures[i])
       // {
        //    m_Textures[i] = new Texture(GL_TEXTURE_2D, "white.png");
        //    Ret = m_Textures[i]->Load();
       // }
    }
    return Ret;
}
// функция отрисовки
void Mesh::Render()
{
    // включаем атрибуты массива вершин
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    for (unsigned int i = 0; i < m_Entries.size(); i++) 
    {
        // указываем параметры каждого атрибута
        glBindBuffer(GL_ARRAY_BUFFER, m_Entries[i].VB);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Entries[i].IB);

        const unsigned int MaterialIndex = m_Entries[i].MaterialIndex;
        // привязываем // привязываем текстуры к конкретному модулю GL_TEXTURE
        if (MaterialIndex < m_Textures.size() && m_Textures[MaterialIndex])
        {
            m_Textures[MaterialIndex]->Bind(GL_TEXTURE0);
        }

        glDrawElements(GL_TRIANGLES, m_Entries[i].NumIndices, GL_UNSIGNED_INT, 0); // рисуем треугольники
    }
    // выключаем атрибуты массива вершин
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}