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

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080
struct Vertex
{
    Vector3f m_pos; // позиция
    Vector2f m_tex; // координата текстуры в вершине 

    Vertex() {}

    Vertex(Vector3f pos, Vector2f tex)
    {
        m_pos = pos;
        m_tex = tex;
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

        CreateVertexBuffer(); // создание массива вершин
        CreateIndexBuffer(); // создание индексного массива

        m_pEffect = new LightingTechnique(); // инициализация светового эффекта

        if (!m_pEffect->Init())
        {
            return false;
        }
        m_pEffect->Enable(); // включение светового эффекта

        m_pEffect->SetTextureUnit(1); // передача в шейдер модуля текстуры (1)

        m_pTexture = new Texture(GL_TEXTURE_2D, "cube.jpg"); // инициализация текстуры

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
        p.WorldPos(0.0f, 0.0f, 3.0f); // положение объекта
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp()); // установка векторов камеры
        p.SetPerspectiveProj(60.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 1.0f, 100.0f); // проекция перспективы
        m_pEffect->SetWVP(p.GetTrans()); // передача матрицы трасформации в шейдер
        m_pEffect->SetDirectionalLight(m_directionalLight);

        glEnableVertexAttribArray(0);// разрешение использования атрибутов вершины (0 - координата, 1 - текстура)
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO); // привязка вершинного буфера 
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0); // параметры аттрибутов вершин
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Indexbuf); // призязка индексного буфера
        m_pTexture->Bind(GL_TEXTURE1); // привязываем текстуру к конкретному модулю GL_TEXTURE
        glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, 0); // функция отрисовки элементов (прямоугольники)

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

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
        }
    }

    virtual void PassiveMouseCB(int x, int y)
    {
        m_pGameCamera->OnMouse(x, y);
    }

private:

    void CreateVertexBuffer() // вершинный буфер
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

        glGenBuffers(1, &m_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }

    void CreateIndexBuffer() // индексный буфер
    {
        unsigned int indexes[] = { 0, 3, 5, 4,
                              4, 5, 6, 7,
                             6, 5, 3, 2,
                              4, 7, 1, 0,
                              1, 7, 6, 2,
                             1, 2, 3, 0 };

        // индексный массив (4 вершины формируют одну грань куба)
        glGenBuffers(1, &Indexbuf); // генерация буфера
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Indexbuf); // привязка буфера в качестве индексного
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_DYNAMIC_DRAW); //привязка данных буфера
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

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 60, true, "Task 17"))
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