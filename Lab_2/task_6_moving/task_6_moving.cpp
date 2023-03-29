#include <stdio.h>
#include <gl/glew.h> // подключение glew
#include <gl/freeglut.h> // подключение glut
#include "math_3d.h"
#include <cassert>
#include <math.h>

// вершинный шейдер. Получает матрицу трансформации и изменяет положение вершины на экране
static const char* VertexSh = "                                               \n\
#version 330                                                                  \n\
                                                                              \n\
layout (location = 0) in vec3 Position;                                       \n\
uniform mat4 gScale;                                                         \n\               \n\
void main()                                                                   \n\
{   vec3 new_Position = vec3 (Position.x * 0.5, Position.y * 0.5, Position.z);                                                                          \n\
    gl_Position = gScale * vec4 (new_Position, 1.0);  \n\
}";
// фрагментный шейдер. Отвечает за отрисовку отдельного пикселя
static const char* FragmentSh = "                                             \n\
#version 330                                                                  \n\
                                                                              \n\
out vec4 FragColor;                                                           \n\
                                                                              \n\
void main()                                                                   \n\
{                                                                             \n\
    FragColor = vec4(0.0, 0.0, 1.0, 1.0);                                     \n\
}";

GLuint Vbuf;
static float Scale = 0.0f;
GLuint gWorldLocation;
void CreateVertexBuffer() // создание массива вершин
{
    Vector3f vertices[3];
    vertices[0] = Vector3f(-1.0f, -1.0f, 0.0f);
    vertices[1] = Vector3f(1.0f, -1.0f, 0.0f);
    vertices[2] = Vector3f(0.0f, 1.0f, 0.0f);
    glGenBuffers(1, &Vbuf);
    glBindBuffer(GL_ARRAY_BUFFER, Vbuf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
}
void RenderScene() // функция рендера
{
    glClear(GL_COLOR_BUFFER_BIT);
    Matrix4f World; // инициализация матрицы трансформации
    Scale += 0.001f;
    World.m[0][0] = 1.0f; World.m[0][1] = 0.0f; World.m[0][2] = 0.0f; World.m[0][3] = 0.0;
    World.m[1][0] = 0.0f; World.m[1][1] = 1.0f; World.m[1][2] = 0.0f; World.m[1][3] = sin(Scale);
    World.m[2][0] = 0.0f; World.m[2][1] = 0.0f; World.m[2][2] = 1.0f; World.m[2][3] = 0.0f;
    World.m[3][0] = 0.0f; World.m[3][1] = 0.0f; World.m[3][2] = 0.0f; World.m[3][3] = 1.0f;
    glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, &World.m[0][0]); // передача матрицы через юниформ переменную в шейдер
    glEnableVertexAttribArray(0); // разрешение использования атрибутов вершины
    glBindBuffer(GL_ARRAY_BUFFER, Vbuf); // привязка вершинного буфера перед отрисовкой

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, 3); // функция отрисовки. В данном случае - треугольников
    glDisableVertexAttribArray(0);
    glutSwapBuffers(); // смена фонового буфера и буфера кадра
    glutPostRedisplay(); // указание на необходимость перерисовки текущего окна 
}

void InitializeGlutCallbacks()
{
    glutDisplayFunc(RenderScene); // установка RenderScene как функции обратного вызова
    glutIdleFunc(RenderScene); // теперь RenderScene будет вызываться постоянно, а не
    // только при необходимости перерисовки (изменение размера, масштаба окна)
}
// функция добавления шейдера
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
// функция компиляции всех шейдеров
static void CompileShaders()
{
    GLuint ShaderProgram = glCreateProgram(); // создание программного объекта

    if (ShaderProgram == 0) // в случае неуспешного создания
    {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }
    AddShader(ShaderProgram, VertexSh, GL_VERTEX_SHADER); // добавление вершинного шейдера
    AddShader(ShaderProgram, FragmentSh, GL_FRAGMENT_SHADER); // добавление фрагментного шейдера

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    glLinkProgram(ShaderProgram); // линковка программы (создание исполняемого файла) 
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success); // проверка успешности линковки
    if (Success == 0)
    {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }
    glValidateProgram(ShaderProgram); // проверка возможности запуска программы
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success) // в случае невозможности запуска
    {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }
    glUseProgram(ShaderProgram); // назначение шейдерной программы для использования в конвейере
    gWorldLocation = glGetUniformLocation(ShaderProgram, "gScale"); // получение позиции юниформ
    // переменной в шейдерной программе. Необходимо для передачи значений напрямую в шейдер
    assert(gWorldLocation != 0xFFFFFFFF);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(1024, 728); // инициализация окна
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Moving"); // создание окна

    InitializeGlutCallbacks();

    GLenum res = glewInit();
    if (res != GLEW_OK)
    {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }
    CreateVertexBuffer(); // создание массива вершин
    CompileShaders(); // компиляция шейдеров
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glutMainLoop(); // передача управления glut (запуск цикла)
    return 0;
}