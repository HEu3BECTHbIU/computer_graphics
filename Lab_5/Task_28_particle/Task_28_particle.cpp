#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <math.h>

#ifdef __GNUC__
#  if __GNUC_PREREQ(4,7)
#include <unistd.h>
#  endif
#endif

#include "pipeline.h"
#include "camera.h"
#include "texture.h"
#include "lightening_technique.h"
#include "glut_backend.h"
#include "util.h"
#include "mesh.h"
#include "engine_common.h"
#include "particle_system.h"

#include <chrono>
#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080

/*
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h> // portable: uint64_t   MSVC: __int64 

// MSVC defines this in winsock2.h!?
struct timeval
{
    long tv_sec;
    long tv_usec;
};

int gettimeofday(struct timeval* tp, struct timezone* tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970 
    static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    time = ((uint64_t)file_time.dwLowDateTime);
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec = (long)((time - EPOCH) / 10000000L);
    tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
    return 0;
}

static long long GetCurrentTimeMillis()
{
    timeval t;
    gettimeofday(&t, NULL);

    long long ret = t.tv_sec * 1000 + t.tv_usec / 1000;
    return ret;
}
*/

static long long GetCurrentTimeMillis() {
    auto time = std::chrono::system_clock::now();

    auto since_epoch = time.time_since_epoch();

    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch);

    long long now = millis.count();
    return now;
}

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

            m_currentTimeMillis = GetCurrentTimeMillis();
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
        Vector3f Pos(0.0f, 0.4f, -0.5f);
        Vector3f Target(0.0f, 0.2f, 1.0f);
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
        // привязываем цвет текстуры к модулю текстуры 0
        m_pLightingTechnique->SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
        // привязываем карту нормалей к модулю текстуры 2
        m_pLightingTechnique->SetNormalMapTextureUnit(NORMAL_TEXTURE_UNIT_INDEX);

        m_pGround = new Mesh();

        if (!m_pGround->LoadMesh("../Content/quad.obj"))
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
        Vector3f ParticleSystemPos = Vector3f(0.0f, 0.0f, 1.0f);

        return m_particleSystem.InitParticleSystem(ParticleSystemPos);
    }

    void Run() // запуск инициализации
    {
        GLUTBackendRun(this);
    }

    virtual void RenderSceneCB() // функция отрисовки
    {
        long long TimeNowMillis = GetCurrentTimeMillis(); // получение времени в миллисекундах
        assert(TimeNowMillis >= m_currentTimeMillis);
        unsigned int DeltaTimeMillis = (unsigned int)(TimeNowMillis - m_currentTimeMillis);
        m_currentTimeMillis = TimeNowMillis;

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
        // рендер 
        m_particleSystem.Render(DeltaTimeMillis, p.GetVPTrans(), m_pGameCamera->GetPos());

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
    long long m_currentTimeMillis;
    LightingTechnique* m_pLightingTechnique;
    Camera* m_pGameCamera;
    DirectionalLight m_dirLight;
    Mesh* m_pGround;
    Texture* m_pTexture;
    Texture* m_pNormalMap;
    PersProjInfo m_persProjInfo;
    ParticleSystem m_particleSystem;
};

int main(int argc, char** argv)
{
    srand(time(nullptr));
    GLUTBackendInit(argc, argv);

    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 60, false, "Task 26"))
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