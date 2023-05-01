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
#include "mesh.h"
#include "shadow_map_fbo.h"
#include "shadow_map_technique.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080


class Main : public ICallbacks
{
public:

    Main()
    {
        m_pGameCamera = NULL; 
        m_pShadowMapTech = NULL; 
        m_pMesh = NULL; 
        m_pQuad = NULL; 
        m_pEffect = NULL; 
        m_scale = 0.0f;
        // свет типа прожектор
        m_spotLight.AmbientIntensity = 0.0f;
        m_spotLight.DiffuseIntensity = 0.9f;
        m_spotLight.Color = Vector3f(1.0f, 1.0f, 1.0f);
        m_spotLight.Attenuation.Linear = 0.01f;
        m_spotLight.Position = Vector3f(-20.0, 20.0, 5.0f);
        m_spotLight.Direction = Vector3f(1.0f, -1.0f, 0.0f);
        m_spotLight.Cutoff = 20.0f;
    }

    ~Main()
    {
        SAFE_DELETE(m_pEffect);
        SAFE_DELETE(m_pShadowMapTech);
        SAFE_DELETE(m_pGameCamera);
        SAFE_DELETE(m_pMesh);
        SAFE_DELETE(m_pQuad);
    }

    bool Init()
    {
        if (!m_shadowMapFBO.Init(WINDOW_WIDTH, WINDOW_HEIGHT)) 
        {
            return false;
        }
        // инициализация камеры 
        m_pGameCamera = new Camera(WINDOW_WIDTH / 1.25, WINDOW_HEIGHT / 1.25);

        m_pEffect = new LightingTechnique();// инициализация света

        if (!m_pEffect->Init())
        {
            printf("Error initializing the lighting technique\n");
            return false;
        }
        m_pShadowMapTech = new ShadowMapTechnique(); // инициализация теней

        if (!m_pShadowMapTech->Init()) 
        {
            printf("Error initializing the shadow map technique\n");
            return false;
        }
        m_pShadowMapTech->Enable(); // назначение шейдерной программы для использования в конвейере
        // m_pEffect->Enable();  // назначение шейдерной программы для использования в конвейере

        //m_pEffect->SetTextureUnit(0); // передача в шейдер модуля текстуры (0)
        m_pQuad = new Mesh(); // инициализация mesh модели куба
        if (!m_pQuad->LoadMesh("quad.txt")) 
        {
            return false;
        }
        m_pMesh = new Mesh();
        return m_pMesh->LoadMesh("phoenix.md2"); // загрузка меш модели
    }

    void Run() // запуск инициализации
    {
        GLUTBackendRun(this);
    }

    virtual void RenderSceneCB() // функция отрисовки
    {
        m_pGameCamera->OnRender();
        m_scale += 0.05f;

        ShadowMapPass(); // вызываем функцию для рендера в карту теней
        RenderPass(); // отображение результата

        glutSwapBuffers();
    }

    virtual void ShadowMapPass()
    {
        m_shadowMapFBO.BindForWriting(); // привязка FBO
        //начиная с этого момента все значения высот будут переходить в нашу текстуру карты теней,
        //  а цвет будет выброшен

        glClear(GL_DEPTH_BUFFER_BIT); // очистка буфера глубины

        Pipeline p;
        p.Scale(0.2f, 0.2f, 0.2f);
        p.Rotate(0.0f, m_scale, 0.0f); // поворот объекта вокруг оси
        p.WorldPos(0.0f, 0.0f, 5.0f); // положение объекта
        // установка векторов камеры
        p.SetCamera(m_spotLight.Position, m_spotLight.Direction, Vector3f(0.0f, 1.0f, 0.0f));
        // проекция перспективы
        p.SetPerspectiveProj(60.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 1.0f, 50.0f);
        // передача матрицы трасформации в шейдер
        m_pShadowMapTech->SetWVP(p.GetWVPTrans());
        // рендер меша (модель из игры)
        m_pMesh->Render();

        glBindFramebuffer(GL_FRAMEBUFFER, 0); // переключаемся обратно в стандартный буфер кадра
    }

    virtual void RenderPass()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // очистка буферов и цвета и глубины

        m_pShadowMapTech->SetTextureUnit(0); // привязка модуля текстуры 0
        m_shadowMapFBO.BindForReading(GL_TEXTURE0); // привязываем карту теней для чтения в модуле 0
        // масштабирование квадрат, помещение перед камерой и рендер
        Pipeline p;
        p.Scale(5.0f, 5.0f, 5.0f);
        p.WorldPos(0.0f, 0.0f, 10.0f);
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(60.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 1.0f, 50.0f);
        m_pShadowMapTech->SetWVP(p.GetWVPTrans());
        // отрисовка меша (куб)
        m_pQuad->Render();
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
    LightingTechnique* m_pEffect; // свет
    ShadowMapTechnique* m_pShadowMapTech; // тени
    Camera* m_pGameCamera; // камера
    SpotLight m_spotLight; // прожектор
    Mesh* m_pMesh; // меш модели
    Mesh* m_pQuad; // меш квадрата
    ShadowMapFBO m_shadowMapFBO; // карта теней
    DirectionalLight m_directionalLight;
    float m_scale;
};

int main(int argc, char** argv)
{
    GLUTBackendInit(argc, argv);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 60, true, "Task 23"))
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