#include <stdio.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <cassert>
#include <math.h>
#include "pipeline.h"
#include "camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"
#include "lightening_technique.h"
#include "glut_backend.h"
#include <iostream>
#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080
struct Vertex
{
    Vector3f m_pos; // позиция
    Vector2f m_tex; // координата текстуры в вершине 
    Vector3f m_normal; // нормаль 
    Vertex() {}

    Vertex(Vector3f pos, Vector2f tex)
    {
        m_pos = pos;
        m_tex = tex;
        m_normal = Vector3f(0.0f, 0.0f, 0.0f);
    }
};
class Main : public ICallbacks // класс, реализующий интерфейс Icallbacks
{
public:

    Main() // констуктор
    {
        m_pGameCamera = NULL;
        m_pTexture = NULL;
        m_pEffect = NULL;
        m_scale = 0.0f;
        m_directionalLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_directionalLight.AmbientIntensity = 0.5f;
        m_directionalLight.DiffuseIntensity = 0.75f;
        m_directionalLight.Direction = Vector3f(1.0f, 0.0, 0.0);
    }

    ~Main()
    {
        delete m_pEffect;
        delete m_pGameCamera;
        delete m_pTexture;
    }

    bool Init()
    {
        m_pGameCamera = new Camera(WINDOW_WIDTH / 1.25, WINDOW_HEIGHT / 1.25); // инициализация камеры 
        
        unsigned int indexes[] = { 0, 3, 5, 4,
                              4, 5, 6, 7,
                             6, 5, 3, 2,
                              4, 7, 1, 0,
                              1, 7, 6, 2,
                             1, 2, 3, 0 };
        // индексный массив (4 вершины формируют одну грань куба)
        CreateIndexBuffer(indexes, sizeof(indexes)); // создание индексного массива

        CreateVertexBuffer(indexes, 24); // создание массива вершин

        m_pEffect = new LightingTechnique(); // инициализация светового эффекта

        if (!m_pEffect->Init())
        {
            return false;
        }
        m_pEffect->Enable(); // включение светового эффекта

        m_pEffect->SetTextureUnit(1); // передача в шейдер модуля текстуры (1)

        m_pTexture = new Texture(GL_TEXTURE_2D, "cube3.jpg"); // инициализация текстуры

        if (!m_pTexture->Load()) // если не удалось
        {
            return false;
        }
        return true;
    }
    void Run() // запуск инициализации
    {
        GLUTBackendRun(this);
    }

    virtual void RenderSceneCB() // функция отрисовки
    {
        m_pGameCamera->OnRender();

        glClear(GL_COLOR_BUFFER_BIT);

        m_scale += 0.01f;
        Pipeline p;
        p.Rotate(0.0f, m_scale, 0.0f); // поворот объекта вокруг оси
        p.WorldPos(0.0f, 1.0f, 5.0f); // положение объекта
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp()); // установка векторов камеры
        p.SetPerspectiveProj(60.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 1.0f, 100.0f); // проекция перспективы
        m_pEffect->SetWVP(p.GetTrans()); // передача матрицы трасформации в шейдер
        m_pEffect->SetDirectionalLight(m_directionalLight); // передача объекта структуры в шейдер

        const Matrix4f& WorldTransformation = p.GetWorldTrans(); // получение матрицы мировых преборазований
        m_pEffect->SetWorldMatrix(WorldTransformation); // передача в юниформ переменную
        glEnableVertexAttribArray(0);// разрешение использования атрибутов вершины (0 - координата, 1 - текстура. 2 - нормаль)
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO); // привязка вершинного буфера 

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0); // параметры аттрибутов вершин
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Indexbuf); // призязка индексного буфера
        m_pTexture->Bind(GL_TEXTURE1); // привязываем текстуру к конкретному модулю GL_TEXTURE
         glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, 0); // функция отрисовки элементов (прямоугольники)
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        glutSwapBuffers();
    }

    virtual void IdleCB()
    {
        RenderSceneCB();
    }

    virtual void SpecialKeyboardCB(int Key, int x, int y)
    {
        m_pGameCamera->OnKeyboard(Key);
    }

    virtual void KeyboardCB(unsigned char Key, int x, int y)
    {
        switch (Key)
        {
        case 'q': // закрытие программы
            glutLeaveMainLoop();
            break;

        case 'a': // увеличение интенсивности фонового света
            m_directionalLight.AmbientIntensity += 0.05f;
            break;

        case 's':// уменьшение интенсивности фонового света
            m_directionalLight.AmbientIntensity -= 0.05f;
            break;

        case 'z': // увеличение интенсивности рассеяного света
            m_directionalLight.DiffuseIntensity += 0.05f;
            break;

        case 'x': // уменьшение интенсивности рассеяного света
            m_directionalLight.DiffuseIntensity -= 0.05f;
            break;
        }
    }

    virtual void PassiveMouseCB(int x, int y)
    {
        m_pGameCamera->OnMouse(x, y);
    }

private:
    // вычисление нормалей в вершинах (через векторное произведение)
    void CalcNormals(const unsigned int* pIndices, unsigned int IndexCount,
        Vertex* pVertices, unsigned int VertexCount)
    {
        for (unsigned int i = 0; i < IndexCount; i += 4)
        {
            unsigned int Index0 = pIndices[i];
            unsigned int Index1 = pIndices[i + 1];
            unsigned int Index2 = pIndices[i + 2];
            unsigned int Index3 = pIndices[i + 3];
            Vector3f v1 = pVertices[Index1].m_pos - pVertices[Index0].m_pos;
            Vector3f v2 = pVertices[Index2].m_pos - pVertices[Index0].m_pos;
            Vector3f Normal = v1.Cross(v2);
            Normal.Normalize();

            pVertices[Index0].m_normal += Normal;
            pVertices[Index1].m_normal += Normal;
            pVertices[Index2].m_normal += Normal;
            pVertices[Index3].m_normal += Normal;
        }

        for (unsigned int i = 0; i < VertexCount; i++)
        {
            pVertices[i].m_normal.Normalize();
           // std::cout << pVertices[i].m_normal.x << " " << pVertices[i].m_normal.y << " " << pVertices[i].m_normal.z << std::endl;
        }
    }
    void CreateVertexBuffer(const unsigned int* pIndices, unsigned int IndexCount) // вершинный буфер
    {
       Vertex vertices[8];
        vertices[0] = Vertex(Vector3f(-1.0f, 1.0f, -1.0f), Vector2f(0.0f, 0.0f)); // 0 0
        vertices[1] = Vertex(Vector3f(-1.0f, -1.0f, -1.0f), Vector2f(0.0f, 1.0f)); // 0 0
        vertices[2] = Vertex(Vector3f(1.0f, -1.0f, -1.0f), Vector2f(1.0f, 0.0f)); // 1 0
        vertices[3] = Vertex(Vector3f(1.0f, 1.0f, -1.0f), Vector2f(1.0f, 1.0f)); // 1 0 
        vertices[4] = Vertex(Vector3f(-1.0f, 1.0f, 1.0f), Vector2f(1.0f, 0.0f)); // 1 0
        vertices[5] = Vertex(Vector3f(1.0f, 1.0f, 1.0f), Vector2f(0.0f, 1.0f)); // 0 1
        vertices[6] = Vertex(Vector3f(1.0f, -1.0f, 1.0f), Vector2f(0.0f, 0.0f)); // 0 0
        vertices[7] = Vertex(Vector3f(-1.0f, -1.0f, 1.0f), Vector2f(1.0f, 1.0f)); // 1 1

        unsigned int VertexCount = 8;
        /*Vertex vertices[4] = { Vertex(Vector3f(-1.0f, -1.0f, 0.5773f), Vector2f(0.0f, 0.0f)),
                               Vertex(Vector3f(0.0f, -1.0f, -1.15475), Vector2f(0.5f, 0.0f)),
                               Vertex(Vector3f(1.0f, -1.0f, 0.5773f),  Vector2f(1.0f, 0.0f)),
                               Vertex(Vector3f(0.0f, 1.0f, 0.0f),      Vector2f(0.5f, 1.0f)) };
        unsigned int VertexCount = 4;*/
        CalcNormals(pIndices, IndexCount, vertices, VertexCount);
        glGenBuffers(1, &m_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }

    void CreateIndexBuffer(const unsigned int* pIndices, unsigned int SizeInBytes) // индексный буфер
    {
       
        glGenBuffers(1, &Indexbuf); // генерация буфера
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Indexbuf); // привязка буфера в качестве индексного
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, SizeInBytes, pIndices, GL_DYNAMIC_DRAW); //привязка данных буфера
    }

    GLuint m_VBO;
    GLuint Indexbuf;
    LightingTechnique* m_pEffect;
    Texture* m_pTexture;
    Camera* m_pGameCamera;
    float m_scale;
    DirectionLight m_directionalLight;
};
int main(int argc, char** argv)
{
    GLUTBackendInit(argc, argv); // инициализация glut

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 60, true, "Task 18"))
    {
        return 1;
    }

    Main* pApp = new Main();

    if (!pApp->Init())
    {
        return 1;
    }
    pApp->Run();

    delete pApp;

    return 0;
}