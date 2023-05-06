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

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080


class Main : public ICallbacks
{
public:

    Main()
    {
        {
            m_pGameCamera = NULL;
            m_pEffect = NULL;
            m_scale = 0.0f;
            m_directionalLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
            m_directionalLight.AmbientIntensity = 0.25f;
            m_directionalLight.DiffuseIntensity = 0.9f;
            m_directionalLight.Direction = Vector3f(1.0f, 0.0, 0.0);

            m_persProjInfo.FOV = 60.0f;
            m_persProjInfo.Height = WINDOW_HEIGHT;
            m_persProjInfo.Width = WINDOW_WIDTH;
            m_persProjInfo.zNear = 1.0f;
            m_persProjInfo.zFar = 100.0f;

            m_pMesh1 = NULL;
            m_pMesh2 = NULL;
            m_pMesh3 = NULL;

        }
    }

    virtual ~Main()
    {
        SAFE_DELETE(m_pEffect);
        SAFE_DELETE(m_pGameCamera);
        SAFE_DELETE(m_pMesh1);
        SAFE_DELETE(m_pMesh2);
        SAFE_DELETE(m_pMesh3);
    }

    bool Init()
    {
        Vector3f Pos(3.0f, 7.0f, -10.0f);
        Vector3f Target(0.0f, -0.2f, 1.0f);
        Vector3f Up(0.0, 1.0f, 0.0f);
        // инициализация камеры 
        m_pGameCamera = new Camera(WINDOW_WIDTH / 1.25, WINDOW_HEIGHT / 1.25, Pos, Target, Up);

        m_pEffect = new LightingTechnique();// инициализация техники света

        if (!m_pEffect->Init())
        {
            printf("Error initializing the lighting technique\n");
            return false;
        }
        m_pEffect->Enable(); // назначаем шейдер света

        m_pEffect->SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX); // привязываем текстуру через модуль текстуры 0
        m_pEffect->SetDirectionalLight(m_directionalLight); // устанавливаем направленный свет
        m_pEffect->SetMatSpecularIntensity(0.5f);
        m_pEffect->SetMatSpecularPower(16);

        m_pMesh1 = new Mesh(); // паук

        if (!m_pMesh1->LoadMesh("../Content/spider.obj"))
        {
            return false;
        }
        m_pMesh2 = new Mesh(); // джип

        if (!m_pMesh2->LoadMesh("../Content/jeep.obj")) 
        {
            return false;
        }
        m_pMesh3 = new Mesh(); // вертолет

        if (!m_pMesh3->LoadMesh("../Content/hheli.obj")) 
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
        m_scale += 0.01f;
        m_pGameCamera->OnRender();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // очистка буферов и цвета и глубины

        m_pEffect->SetEyeWorldPos(m_pGameCamera->GetPos());

        Pipeline p; // преобразования для первого меша
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.Rotate(0.0f, m_scale, 0.0f);
        p.SetPerspectiveProj(m_persProjInfo);

        p.Scale(0.05f, 0.05f, 0.05f);
        p.WorldPos(-6.0f, -2.0f, 10.0f);
        m_pEffect->SetWVP(p.GetWVPTrans());
        m_pEffect->SetWorldMatrix(p.GetWorldTrans());
        m_pMesh1->Render();

        // преобразования для второго меша
        p.Scale(0.01f, 0.01f, 0.01f);
        p.WorldPos(6.0f, -2.0f, 10.0f);
        m_pEffect->SetWVP(p.GetWVPTrans());
        m_pEffect->SetWorldMatrix(p.GetWorldTrans());
        m_pMesh2->Render();

        // преобразования для третьего меша
        p.Scale(0.04f, 0.04f, 0.04f);
        p.WorldPos(0.0f, 6.0f, 10.0f);
        m_pEffect->SetWVP(p.GetWVPTrans());
        m_pEffect->SetWorldMatrix(p.GetWorldTrans());
        m_pMesh3->Render();

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

    LightingTechnique* m_pEffect; // техника света
    Camera* m_pGameCamera; // камера
    float m_scale;
    DirectionalLight m_directionalLight; // свет
    Mesh* m_pMesh1; // указатели на меши
    Mesh* m_pMesh2;
    Mesh* m_pMesh3;

    PersProjInfo m_persProjInfo;
};

int main(int argc, char** argv)
{
    GLUTBackendInit(argc, argv);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 60, false, "Task 32"))
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