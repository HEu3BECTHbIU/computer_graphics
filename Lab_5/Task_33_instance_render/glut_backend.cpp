//#include <unistd.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "glut_backend.h"

static ICallbacks* s_pCallbacks = NULL;

static void SpecialKeyboardCB(int Key, int x, int y)
{
    s_pCallbacks->SpecialKeyboardCB(Key, x, y);
}

static void KeyboardCB(unsigned char Key, int x, int y)
{
    s_pCallbacks->KeyboardCB(Key, x, y);
}

static void PassiveMouseCB(int x, int y)
{
    s_pCallbacks->PassiveMouseCB(x, y);
}

static void RenderSceneCB()
{
    s_pCallbacks->RenderSceneCB();
}

static void IdleCB()
{
    s_pCallbacks->IdleCB();
}

static void InitCallbacks() // инициализация функций обратного вызова 
{
    glutDisplayFunc(RenderSceneCB); // установка RenderScene как функции обратного вызова
    glutIdleFunc(IdleCB);// теперь RenderScene будет вызываться постоянно, а не
    // только при необходимости перерисовки (изменение размера, масштаба окна)
    glutSpecialFunc(SpecialKeyboardCB); // регистрация функции в качестве спец. функции 
    glutPassiveMotionFunc(PassiveMouseCB); // регистрация функции пассивного действия
    glutKeyboardFunc(KeyboardCB); // регистрация спец. функции нажатия клавиш
    ///if (isFullScreen)
    //{
//glutDestroyWindow(1); // разрушение окна (небоходимо для работы gamemode)
   // }
}

void GLUTBackendInit(int argc, char** argv) // инициализация glut
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // параметры отображения
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
}
// инициализация окна
bool GLUTBackendCreateWindow(unsigned int Width, unsigned int Height, unsigned int bpp, bool isFullScreen, const char* pTitle)
{
    if (isFullScreen) // если полный экран, то игровой режим
    {
        char ModeString[64] = { 0 };
        snprintf(ModeString, sizeof(ModeString), "%dx%d@%d", Width, Height, bpp);
        glutGameModeString(ModeString);
        glutEnterGameMode();
    }
    else  // иначе просто окно
    {
        glutInitWindowSize(Width, Height);
        glutCreateWindow(pTitle);
    }
    // инициализация glew
    GLenum res = glewInit();
    if (res != GLEW_OK)
    {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return false;
    }

    return true;
}

void GLUTBackendRun(ICallbacks* pCallbacks) // функция, запускающая инициализацию
{
    if (!pCallbacks) // если обратных вызовов не зарегистрировано
    {
        fprintf(stderr, "%s : callbacks not specified!\n", __FUNCTION__);
        return;
    }
    glEnable(GL_DEPTH_TEST); // тест глубины для корректной отрисовки (Z тест)

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_CULL_FACE); // включаем отсечение нелицевых граней
    glFrontFace(GL_CW); // обход лицевых граней - по часовой
    glCullFace(GL_BACK); //не отрисовывать нелицевые грани грани

    s_pCallbacks = pCallbacks;
    InitCallbacks();
    glutMainLoop(); // передача управления циклу glut
}