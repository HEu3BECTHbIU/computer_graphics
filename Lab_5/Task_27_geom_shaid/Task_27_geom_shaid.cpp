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
#include "engine_common.h"
#include "bilboard_list.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080


class Main : public ICallbacks
{
public:

    Main()
    {
        {
            m_pLightingTechnique = NULL;
            m_pGameCamera = NULL;
            m_pGround = NULL;
            m_pTexture = NULL;
            m_pNormalMap = NULL;

            m_dirLight.AmbientIntensity = 0.2f;
            m_dirLight.DiffuseIntensity = 0.8f;
            m_dirLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
            m_dirLight.Direction = Vector3f(1.0f, 0.0f, 0.0f);

            m_persProjInfo.FOV = 60.0f;
            m_persProjInfo.Height = WINDOW_HEIGHT;
            m_persProjInfo.Width = WINDOW_WIDTH;
            m_persProjInfo.zNear = 1.0f;
            m_persProjInfo.zFar = 100.0f;

        }
    }

    virtual ~Main()
    {
        SAFE_DELETE(m_pLightingTechnique);
        SAFE_DELETE(m_pGameCamera);
        SAFE_DELETE(m_pGround);
        SAFE_DELETE(m_pTexture);
        SAFE_DELETE(m_pNormalMap);
    }

    bool Init()
    {
        Vector3f Pos(0.0f, 1.0f, -1.0f);
        Vector3f Target(0.0f, -0.5f, 1.0f);
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
        // m_pLightingTechnique->SetTextureUnit(0); 
        m_pLightingTechnique->SetColorTextureUnit(0); // // // привязываем цвет текстуры к модулю текстуры 0
        m_pLightingTechnique->SetNormalMapTextureUnit(2); // привязываем карту нормалей к модулю текстуры 2
        
        m_pGround = new Mesh();

        if (!m_pGround->LoadMesh("../Content/quad.obj"))
        {
            return false;
        }

        if (!m_billboardList.Init("../Content/main.png"))
        {
            return false;
        }
        m_pTexture = new Texture(GL_TEXTURE_2D, "../Content/bricks.jpg");

        if (!m_pTexture->Load())
        {
            return false;
        }

        m_pTexture->Bind(COLOR_TEXTURE_UNIT);

        m_pNormalMap = new Texture(GL_TEXTURE_2D, "../Content/normal_map.jpg");

        if (!m_pNormalMap->Load())
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

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // очистка буферов и цвета и глубины

        m_pLightingTechnique->Enable();

        m_pTexture->Bind(COLOR_TEXTURE_UNIT);
         m_pNormalMap->Bind(NORMAL_TEXTURE_UNIT);

        Pipeline p;
        p.Scale(20.0f, 20.0f, 1.0f); // масштабирование
        p.Rotate(90.0f, 0.0, 0.0f); // поворот объекта вокруг оси
        // установка камеры
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        // проекция перспективы
        p.SetPerspectiveProj(m_persProjInfo);

        // передача матриц трансформаций в шейдер

        m_pLightingTechnique->SetWVP(p.GetWVPTrans());
        m_pLightingTechnique->SetWorldMatrix(p.GetWorldTrans());

        m_pGround->Render();
        m_billboardList.Render(p.GetVPTrans(), m_pGameCamera->GetPos());

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
        }
    }

    virtual void PassiveMouseCB(int x, int y)
    {
        m_pGameCamera->OnMouse(x, y);
    }

private:

    LightingTechnique* m_pLightingTechnique;
    Camera* m_pGameCamera;
    DirectionalLight m_dirLight;
    Mesh* m_pGround;
    Texture* m_pTexture;
    Texture* m_pNormalMap;
    PersProjInfo m_persProjInfo;
    BillboardList m_billboardList;
};

int main(int argc, char** argv)
{
    GLUTBackendInit(argc, argv);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 60, true, "Task 27"))
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