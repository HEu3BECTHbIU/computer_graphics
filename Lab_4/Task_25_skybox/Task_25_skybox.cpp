#include <stdio.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <math.h>
#include "pipeline.h"
#include "camera.h"
#include "texture.h"
#include "lightening_technique.h"
#include "glut_backend.h"
#include "util.h"
#include "mesh.h"
#include "skybox.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080


class Main : public ICallbacks
{
public:

    Main()
    {
        m_pLightingTechnique = NULL;
        m_pGameCamera = NULL;
        m_pMesh = NULL;
        m_scale = 0.0f;
        m_pSkyBox = NULL;
        // направленный свет
        m_dirLight.AmbientIntensity = 0.2f;
        m_dirLight.DiffuseIntensity = 0.8f;
        m_dirLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_dirLight.Direction = Vector3f(1.0f, -1.0f, 0.0f);
        // проекция перспективы
        m_persProjInfo.FOV = 60.0f;
        m_persProjInfo.Height = WINDOW_HEIGHT;
        m_persProjInfo.Width = WINDOW_WIDTH;
        m_persProjInfo.zNear = 1.0f;
        m_persProjInfo.zFar = 100.0f;
    }

    virtual ~Main()
    {
        SAFE_DELETE(m_pLightingTechnique);
        SAFE_DELETE(m_pGameCamera);        
        SAFE_DELETE(m_pMesh);        
        SAFE_DELETE(m_pSkyBox);
    }

    bool Init()
    {
        Vector3f Pos(0.0f, 1.0f, -20.0f);
        Vector3f Target(0.0f, 0.0f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);
        // инициализация камеры 
        m_pGameCamera = new Camera(WINDOW_WIDTH / 1.25, WINDOW_HEIGHT / 1.25, Pos, Target, Up);

        m_pLightingTechnique = new LightingTechnique();// инициализация техники света

        if (!m_pLightingTechnique->Init())
        {
            printf("Error initializing the lighting technique\n");
            return false;
        }
        m_pLightingTechnique->Enable(); // назначаем шейдер света
        m_pLightingTechnique->SetDirectionalLight(m_dirLight); // устанавливаем направленный свет
        m_pLightingTechnique->SetTextureUnit(0); // назначаем модуль текстуры 0 для объектов

        m_pMesh = new Mesh();
        if (!m_pMesh->LoadMesh("../Content/spider.obj"))
        {
            return false;
        }

        m_pSkyBox = new SkyBox(m_pGameCamera, m_persProjInfo);

        if (!m_pSkyBox->Init(".",
            "../Content/sp3right.jpg",
            "../Content/sp3left.jpg",
            "../Content/sp3top.jpg",
            "../Content/sp3bot.jpg",
            "../Content/sp3front.jpg",
            "../Content/sp3back.jpg")) {
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
        m_scale += 0.05f;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // очистка буферов и цвета и глубины

        m_pLightingTechnique->Enable();
        Pipeline p;
        p.Scale(0.1f, 0.1f, 0.1f);
        p.Rotate(0.0f, m_scale, 0.0f); // поворот объекта вокруг оси
        p.WorldPos(sin(m_scale / 10) * 10, -5.0f, 3.0f); // положение объекта
        // установка камеры
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        // проекция перспективы
        p.SetPerspectiveProj(m_persProjInfo);
        // передача матриц трансформаций в шейдер
        m_pLightingTechnique->SetWVP(p.GetWVPTrans());
        m_pLightingTechnique->SetWorldMatrix(p.GetWorldTrans());

        m_pMesh->Render();
        // рендер скайбокса
        m_pSkyBox->Render();

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
            // m_directionalLight.AmbientIntensity += 0.05f;
            break;

        case 's':// уменьшение интенсивности фонового света
            //  m_directionalLight.AmbientIntensity -= 0.05f;
            break;

        case 'z': // увеличение интенсивности рассеяного света
            // m_directionalLight.DiffuseIntensity += 0.05f;
            break;

        case 'x': // уменьшение интенсивности рассеяного света
            // m_directionalLight.DiffuseIntensity -= 0.05f;
            break;
        }
    }

    virtual void PassiveMouseCB(int x, int y)
    {
        m_pGameCamera->OnMouse(x, y);
    }

private:
    LightingTechnique* m_pLightingTechnique; // свет
    Camera* m_pGameCamera; // камера
    float m_scale;
    DirectionalLight m_dirLight; // направленный свет
    Mesh* m_pMesh; // меш модели
    SkyBox* m_pSkyBox; // скайбокс
    PersProjInfo m_persProjInfo; // проекция перспективы
};

int main(int argc, char** argv)
{
    GLUTBackendInit(argc, argv);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH , WINDOW_HEIGHT , 60, false, "Task 25"))
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