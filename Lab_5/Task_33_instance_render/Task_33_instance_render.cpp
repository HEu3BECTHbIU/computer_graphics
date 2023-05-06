#include <stdio.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <math.h>
#include <iostream>
#include "pipeline.h"
#include "camera.h"
#include "texture.h"
#include "lightening_technique.h"
#include "glut_backend.h"
#include "util.h"
#include "mesh.h"
#include "engine_common.h"

//#include <freetype-gl.h>
#ifdef FREETYPE
#include "freetypeGL.h"
#endif
#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080

#define NUM_ROWS 5
#define NUM_COLS 10
#define NUM_INSTANCES NUM_ROWS * NUM_COLS

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
            m_directionalLight.AmbientIntensity = 0.55f;
            m_directionalLight.DiffuseIntensity = 0.9f;
            m_directionalLight.Direction = Vector3f(1.0f, 0.0, 0.0);

            m_persProjInfo.FOV = 60.0f;
            m_persProjInfo.Height = WINDOW_HEIGHT;
            m_persProjInfo.Width = WINDOW_WIDTH;
            m_persProjInfo.zNear = 1.0f;
            m_persProjInfo.zFar = 100.0f;

            m_pMesh = NULL;
            m_frameCount = 0;
            m_fps = 0.0f;

        }
    }

    virtual ~Main()
    {
        SAFE_DELETE(m_pEffect);
        SAFE_DELETE(m_pGameCamera);
        SAFE_DELETE(m_pMesh);
    }

    bool Init()
    {
        Vector3f Pos(7.0f, 3.0f, 0.0f);
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
        m_pEffect->SetMatSpecularIntensity(0.0f);
        m_pEffect->SetMatSpecularPower(0);
        m_pEffect->SetColor(0, Vector4f(1.0f, 0.5f, 0.5f, 0.0f));
        m_pEffect->SetColor(1, Vector4f(0.5f, 1.0f, 1.0f, 0.0f));
        m_pEffect->SetColor(2, Vector4f(1.0f, 0.5f, 1.0f, 0.0f));
        m_pEffect->SetColor(3, Vector4f(1.0f, 1.0f, 1.0f, 0.0f));

        m_pMesh = new Mesh(); // паук

        if (!m_pMesh->LoadMesh("../Content/spider.obj"))
        {
            return false;
        }
#ifdef FREETYPE
        if (!m_fontRenderer.InitFontRenderer()) {
            return false;
        }
#endif
        m_time = glutGet(GLUT_ELAPSED_TIME);

        CalcPositions();

        return true;
    }

    void Run() // запуск инициализации
    {
        GLUTBackendRun(this);
    }

    virtual void RenderSceneCB() // функция отрисовки
    {
        //CalcFPS();
        m_scale += 0.0005f;
        m_pGameCamera->OnRender();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // очистка буферов и цвета и глубины

        m_pEffect->Enable();
        m_pEffect->SetEyeWorldPos(m_pGameCamera->GetPos());

        Pipeline p; // преобразования для первого меша
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
        p.SetPerspectiveProj(m_persProjInfo);
        p.Rotate(0.0f, 90.0f, 0.0f);
        p.Scale(0.005f, 0.005f, 0.005f);

        Matrix4f WVPMatrics[NUM_INSTANCES];
        Matrix4f WorldMatrices[NUM_INSTANCES];

        for (unsigned int i = 0; i < NUM_INSTANCES; i++) 
        {
            Vector3f Pos(m_positions[i]);
            Pos.y += sinf(m_scale) * m_velocity[i];
            p.WorldPos(Pos);
            WVPMatrics[i] = p.GetWVPTrans().Transpose();
            WorldMatrices[i] = p.GetWorldTrans().Transpose();
        }
        m_pMesh->Render(NUM_INSTANCES, WVPMatrics, WorldMatrices);
       // std::cout << "I am here";
       // RenderFPS();

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

    void CalcFPS()
    {
        m_frameCount++;

        int time = glutGet(GLUT_ELAPSED_TIME);

        if (time - m_time > 1000) 
        {
            m_fps = (float)m_frameCount * 1000.0f / (time - m_time);
            m_time = time;
            m_frameCount = 0;
        }
    }

    void RenderFPS()
    {
        char text[32];
        SNPRINTF(text, sizeof(text), "FPS: %.2f", m_fps);
#ifdef FREETYPE
        m_fontRenderer.RenderText(10, 10, text);
#endif
    }

    void CalcPositions()
    {
        for (unsigned int i = 0; i < NUM_ROWS; i++) {
            for (unsigned int j = 0; j < NUM_COLS; j++) {
                unsigned int Index = i * NUM_COLS + j;
                m_positions[Index].x = (float)j;
                m_positions[Index].y = RandomFloat() * 5.0f;
                m_positions[Index].z = (float)i;
                m_velocity[Index] = RandomFloat();
                if (i & 1) {
                    m_velocity[Index] *= (-1.0f);
                }
            }
        }
    }

    LightingTechnique* m_pEffect; // техника света
    Camera* m_pGameCamera; // камера
    float m_scale;
    DirectionalLight m_directionalLight; // свет
    Mesh* m_pMesh;
    PersProjInfo m_persProjInfo;

#ifdef FREETYPE
    FontRenderer m_fontRenderer;
#endif
    int m_time;
    int m_frameCount;
    float m_fps;
    Vector3f m_positions[NUM_INSTANCES];
    float m_velocity[NUM_INSTANCES];
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