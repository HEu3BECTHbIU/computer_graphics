#include <stdio.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <cassert>
#include <stb/stb_image.h>
#include <math.h>
#include "Pipeline.h"
#include "Camera.h"
#include "texture.h"
#include <iostream>
// вершинный шейдер
static const char* VertexSh = "                                                    \n\
#version 330                                                                        \n\
                                                                                    \n\
layout (location = 0) in vec3 Position;                                             \n\
layout (location = 1) in vec2 TexCoord;                                             \n\                                                                                \n\
uniform mat4 gScale;                                                                \n\
                                                                                    \n\
out vec4 Color; \n\
out vec2 TexCoord0; \n\
void main()                                                                         \n\
{                                                                                   \n\
    vec3 new_position = vec3 (Position.x, Position.y, Position.z); \n\
    gl_Position = gScale * vec4(new_position, 1.0);                                 \n\
        TexCoord0 = TexCoord;                                                           \n\
}";
// фрагментный шейдер
static const char* FragmentSh = "                                                    \n\
#version 330                                                                        \n\
out vec4 FragColor;                                                                 \n\
in vec2 TexCoord0;                                                                  \n\
uniform sampler2D gSampler;                                                         \n\
void main()                                                                         \n\
{                                                                                   \n\
FragColor = texture2D(gSampler, TexCoord0);                                  \n\
}";

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
GLuint Vbuf;
GLuint Indexbuf;
static float Scale = 0.0f;
static float Scale2 = 0.0f;
GLuint gWorldLocation;
GLuint gSampler; // юниформ переменная текстуры
Texture* pTexture = NULL;
Camera* cam = NULL;

void CreateVertexBuffer()
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
     
    glGenBuffers(1, &Vbuf);
    glBindBuffer(GL_ARRAY_BUFFER, Vbuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    unsigned int indexes[] = { 0, 3, 5, 4,
                              4, 5, 6, 7,
                             6, 5, 3, 2,
                              4, 7, 1, 0,
                              1, 7, 6, 2,
                             1, 2, 3, 0 };
 
    // индексный массив (4 вершины формируют одну грань куба)
    glGenBuffers(1, &Indexbuf); // генерация буфера
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Indexbuf); // привязка буфера в качестве индексного
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_DYNAMIC_DRAW); // данные буфера
}
void RenderScene()
{
    cam->OnRender();
    glClear(GL_COLOR_BUFFER_BIT);
    Scale += 0.01f;
    Scale2 += 0.0001f;
    Pipeline p;
    p.Rotate(0.0f, (Scale), 0.0f);
    p.WorldPos(0.0f, 1.0f, 5.0f);
    p.SetCamera(cam->GetPos(), cam->GetTarget(), cam->GetUp());
    p.SetPerspectiveProj(60.0f, 1920, 1080, 1.0f, 100.0f);
    // передача матрицы преобразования p.GetTrans() в uniform переменную в шейдере
    glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, (const GLfloat*)p.GetTrans());

    pTexture->Bind(GL_TEXTURE1); // привязываем текстуру к конкретному модулю GL_TEXTURE

    glEnableVertexAttribArray(0);// разрешение использования атрибутов вершины (0 - позиция, 1 - текстура)
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, Vbuf);// призязка вершинного буфера

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0); // параметры аттрибутов вершин
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Indexbuf); // призязка индексного буфера
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glutSwapBuffers();
    glutPostRedisplay();
}

static void SpecialKeyboardCB(int Key, int x, int y) // инициализация функции обратного вызова для
// получения событий от лкавиатуры и мыши
{
    cam->OnKeyboard(Key); // передаем нажатую клавишу в функцию OnKeyboard
}

static void KeyboardCB(unsigned char Key, int x, int y) // функция закрытия окна по нажатию q
{
    switch (Key)
    {
    case 'q':
        exit(0);
    }
}

static void PassiveMouseCB(int x, int y)
{
    cam->OnMouse(x, y);
}
// инициализация функций обратного вызова
void InitializeGlutCallbacks()
{
    glutDisplayFunc(RenderScene); // установка RenderScene как функции обратного вызова
    glutIdleFunc(RenderScene); // теперь RenderScene будет вызываться постоянно, а не
    // только при необходимости перерисовки (изменение размера, масштаба окна)
    glutSpecialFunc(SpecialKeyboardCB); // регистрация функции в качестве спец. функции
    glutPassiveMotionFunc(PassiveMouseCB); // регистрация функции пассивного действия
    glutKeyboardFunc(KeyboardCB); // регистрация спец. функции нажатия клавиш
    glutDestroyWindow(1); // разрушение окна (небоходимо для работы gamemode)
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
    GLuint ShaderObj = glCreateShader(ShaderType); // создание шейдерного объекта

    if (ShaderObj == 0) // обработка неудачного создания
    {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }
    const GLchar* p[1];
    p[0] = pShaderText; // код шейдера (программа)
    GLint Lengths[1];
    Lengths[0] = strlen(pShaderText); // длина программы шейдера
    glShaderSource(ShaderObj, 1, p, Lengths); // привязка данных шейдера к шейдерному объекту
    glCompileShader(ShaderObj); // компиляция шейдера
    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success); // проверка компиляции
    if (!success) // в случае неудачной компиляции (ошибки в коде шейдера)
    {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }
    glAttachShader(ShaderProgram, ShaderObj); //привязка шейдерного объекта к программному
}

static void CompileShaders()
{
    GLuint ShaderProgram = glCreateProgram(); // создание программного объекта

    if (ShaderProgram == 0) // в случае неуспешного создания
    {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }
    AddShader(ShaderProgram, VertexSh, GL_VERTEX_SHADER);
    AddShader(ShaderProgram, FragmentSh, GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    glLinkProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if (Success == 0)
    {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }
    gWorldLocation = glGetUniformLocation(ShaderProgram, "gScale");
    assert(gWorldLocation != 0xFFFFFFFF);
    // семлер нужен, чтобы передать в фрагментный шейдер модуль текстуры для доступа в самом шейдере
    gSampler = glGetUniformLocation(ShaderProgram, "gSampler"); // получение семплера uniform переменной
    assert(gSampler != 0xFFFFFFFF);
    glValidateProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success) 
    {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }
    glUseProgram(ShaderProgram);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    // glutInitWindowSize(1024, 728);
    //glutInitWindowPosition(100, 100);
    glutCreateWindow("Task 16");
    glutGameModeString("1920x1080:32"); // разрешение и глубина цвета для игрового режима
    glutEnterGameMode(); // вход в игровой режим
    InitializeGlutCallbacks();

    GLenum res = glewInit();
    if (res != GLEW_OK)
    {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }
    cam = new Camera(1536, 864); // инициализация камеры
    glEnable(GL_CULL_FACE); // включаем отсечение нелицевых граней
    glFrontFace(GL_CCW); // обход лицевых граней - против часовой
    glCullFace(GL_BACK); //не отрисовывать нелицевые грани грани

    int texture_units = 0;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
    printf("Number of texture units %d\n", texture_units);

    CreateVertexBuffer();
    CompileShaders();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glUniform1i(gSampler, 1); // устанавливаем индексы модулей текстуры,
    // который мы собираемся использовать внутри сэмплера uniform-переменной в шейдере
    std::string path = "cube3.jpg"; // путь до текстуры

    pTexture = new Texture(GL_TEXTURE_2D, path); // инициализируем текстуру

    if (!pTexture->Load())
    {
        return 1;
    }
    glutMainLoop();
    return 0;
}