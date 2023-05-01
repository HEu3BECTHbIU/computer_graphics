#include <stdio.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <math.h>
#include "pipeline.h"
#include "camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"
#include "lightening_technique.h"
#include "glut_backend.h"
#include "util.h"

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

class Main : public ICallbacks
{
public:

    Main()
    {
        m_pGameCamera = NULL;
        m_pTexture = NULL;
        m_pEffect = NULL;
        m_scale = 0.0f;
        m_directionalLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_directionalLight.AmbientIntensity = 0.0f;
        m_directionalLight.DiffuseIntensity = 0.0f;
        m_directionalLight.Direction = Vector3f(1.0f, 0.0f, 0.0f);
    }

    ~Main()
    {
        delete m_pEffect;
        delete m_pGameCamera;
        delete m_pTexture;
    }

    bool Init()
    {
        Vector3f Pos(-10.0f, 0.0f, -10.0f); // векторы камеры
        Vector3f Target(1.0f, 0.0f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);
        // инициализация камеры 
        m_pGameCamera = new Camera(WINDOW_WIDTH / 1.25, WINDOW_HEIGHT / 1.25, Pos, Target, Up);

        // индексный массив (4 вершины формируют одну грань куба)
        unsigned int Indices[] = {0, 3, 2, 1
                                    };
        CreateIndexBuffer(Indices, sizeof(Indices)); // создание индексного массива

        CreateVertexBuffer(Indices, ARRAY_SIZE_IN_ELEMENTS(Indices)); // создание массива вершин

        m_pEffect = new LightingTechnique();// инициализация света

        if (!m_pEffect->Init())
        {
            printf("Error initializing the lighting technique\n");
            return false;
        }

        m_pEffect->Enable(); // // назначение шейдерной программы для использования в конвейере

        m_pEffect->SetTextureUnit(0); // передача в шейдер модуля текстуры (0)
        m_pTexture = new Texture(GL_TEXTURE_2D, "cube.jpg"); // инициализация текстуры

        if (!m_pTexture->Load()) {
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
        // массив, содержаций 2 прожектора
        SpotLight sl[2];
        sl[0].DiffuseIntensity = 25.0f; // интенсивность
        sl[0].Color = Vector3f(1.0f, 0.0f, 0.0f); // цвет
        sl[0].Position = Vector3f(-0.0f, -1.9f, -0.0f); // начальное положение
        sl[0].Direction = Vector3f(sinf(m_scale / 10), 0.0f, cosf(m_scale / 10)); // направление
        sl[0].Attenuation.Linear = 0.1f; //  коэфф затухания
        sl[0].Cutoff = 25.0f; // радиус затухания

        // прожектор, исходящий из камеры
        sl[1].DiffuseIntensity = 10.0f;
        sl[1].Color = Vector3f(1.0f, 1.0f, 1.0f);
        sl[1].Position = m_pGameCamera->GetPos();
        sl[1].Direction = m_pGameCamera->GetTarget();
        sl[1].Attenuation.Linear = 0.1f;
        sl[1].Cutoff = 10.0f;

        m_pEffect->SetSpotLights(2, sl);

        Pipeline p;
        p.Rotate(0.0f, 0.0f, 0.0f); // поворот объекта вокруг оси
        p.WorldPos(0.0f, 0.0f, 1.0f);// положение объекта

        // установка векторов камеры
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        // проекция перспективы
        p.SetPerspectiveProj(60.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.1f, 100.0f);
        // передача матрицы трасформации в шейдер
        m_pEffect->SetWVP(p.GetTrans());
        
        const Matrix4f& WorldTransformation = p.GetWorldTrans(); // получение матрицы мировых преборазований
        m_pEffect->SetWorldMatrix(WorldTransformation); // передача в юниформ переменную
        m_pEffect->SetDirectionalLight(m_directionalLight);
        m_pEffect->SetEyeWorldPos(m_pGameCamera->GetPos());
        // коэфф. отражения материала
        m_pEffect->SetMatSpecularIntensity(1.0f);
        // интенсивность отражения
        m_pEffect->SetMatSpecularPower(32);
        // разрешение использования атрибутов вершины (0 - координата, 1 - текстура. 2 - нормаль)
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO); // привязка вершинного буфера 
        // параметры аттрибутов вершин
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO); // призязка индексного буфера
        m_pTexture->Bind(GL_TEXTURE0); // привязываем текстуру к конкретному модулю GL_TEXTURE
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, 0); // функция отрисовки элементов (прямоугольники)

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
        for (unsigned int i = 0; i < VertexCount; i++) {
            pVertices[i].m_normal.Normalize();
        }
    }
    // создание вершинного буфера
    void CreateVertexBuffer(const unsigned int* pIndices, unsigned int IndexCount)
    {
        Vertex Vertices[4] = { Vertex(Vector3f(-10.0f, -2.0f, -10.0f), Vector2f(0.0f, 0.0f)),
                               Vertex(Vector3f(10.0f, -2.0f, -10.0f), Vector2f(1.0f, 0.0f)),
                               Vertex(Vector3f(10.0f, -2.0f, 10.0f), Vector2f(1.0f, 1.0f)),
                               Vertex(Vector3f(-10.0f, -2.0f, 10.0f), Vector2f(0.0f, 1.0f)) };

        unsigned int VertexCount = ARRAY_SIZE_IN_ELEMENTS(Vertices);

        CalcNormals(pIndices, IndexCount, Vertices, VertexCount);

        glGenBuffers(1, &m_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    }
    // создание индексного буфера
    void CreateIndexBuffer(const unsigned int* pIndices, unsigned int SizeInBytes)
    {
        glGenBuffers(1, &m_IBO); // генерация буфера
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO); // привязка буфера в качестве индексного
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, SizeInBytes, pIndices, GL_DYNAMIC_DRAW); //привязка данных буфера
    }

    GLuint m_VBO;
    GLuint m_IBO;
    LightingTechnique* m_pEffect;
    Texture* m_pTexture;
    Camera* m_pGameCamera;
    float m_scale;
    DirectionalLight m_directionalLight;
};

int main(int argc, char** argv)
{
    GLUTBackendInit(argc, argv);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 60, true, "Task 21"))
    {
        return 1;
    }

    Main* pApp = new Main();

    if (!pApp->Init()) {
        return 1;
    }

    pApp->Run();

    delete pApp;

    return 0;
}