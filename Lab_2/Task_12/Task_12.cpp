﻿#include <stdio.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <cassert>
#include <math.h>

#include "Pipeline.h"
// вершинный шейдер
static const char* VertexSh = "                                                    \n\
#version 330                                                                        \n\
                                                                                    \n\
layout (location = 0) in vec3 Position;                                             \n\
                                                                                    \n\
uniform mat4 gScale;                                                                \n\
                                                                                    \n\
out vec4 Color; \n\
void main()                                                                         \n\
{                                                                                   \n\
    //vec3 new_position = vec3 (Position.x * 0.5, Position.y * 0.5, Position.z * 0.5); \n\
    gl_Position = gScale * vec4(Position, 1.0);                                 \n\
    Color = vec4 (clamp(Position, 0.0, 1.0), 1.0); \n\
}";
// фрагментный шейдер
static const char* FragmentSh = "                                                    \n\
#version 330                                                                        \n\
out vec4 FragColor;                                                                 \n\
in vec4 Color; \n\
void main()                                                                         \n\
{                                                                                   \n\
    FragColor = Color;                                           \n\
}";

GLuint Vbuf;
GLuint Indexbuf;
static float Scale = 0.0f;
static float Scale2 = 0.0f;
GLuint gWorldLocation;
// создание буфера вершин и индексного буфера
void CreateVertexBuffer()
{
   // Vector3f vertices[4];
    Vector3f vertices[8]; // вершинный массив
    vertices[0] = Vector3f(-1.0f, 1.0f, -1.0f);
    vertices[1] = Vector3f(-1.0f, -1.0f, -1.0f);
    vertices[2] = Vector3f(1.0f, -1.0f, -1.0f);
    vertices[3] = Vector3f(1.0f, 1.0f, -1.0f);
    vertices[4] = Vector3f(-1.0f, 1.0f, 1.0f);
    vertices[5] = Vector3f(1.0f, 1.0f, 1.0f);
    vertices[6] = Vector3f(1.0f, -1.0f, 1.0f);
    vertices[7] = Vector3f(-1.0f, -1.0f, 1.0f);
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
// исполняемая в цикле функция
void RenderScene()
{
    glClear(GL_COLOR_BUFFER_BIT);
    Scale += 0.01f;
    Scale2 += 0.0001f;
    Pipeline p; // переменная для управления трасформацией объекта
    p.Rotate(0.0f, Scale, 0.0f); // поворот объекта вокруг оси
    // p.WorldPos(sinf(Scale2), sinf(Scale2), 5.0f);
    p.WorldPos(0.0f, 0.0f, 5.0f); // положение объекта на экране
    p.Scale(1.0f, 1.0f, 1.0f); // масштабирование объекта
    p.SetPerspectiveProj(60.0f, 1024, 728, 1.0f, 100.0f); // проекция перспективы
    
    // передача матрицы преобразования p.GetTrans() в uniform переменную в шейдере
    glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, (const GLfloat*)p.GetTrans());

    glEnableVertexAttribArray(0); // разрешение использования атрибутов вершины
    glBindBuffer(GL_ARRAY_BUFFER, Vbuf); // привязка вершинного буфера перед отрисовкой
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Indexbuf); // призязка индексного буфера
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, 0);// функция отрисовки элементов (прямоугольники)
    glDisableVertexAttribArray(0);
    glutSwapBuffers(); // смена фонового буфера и буфера кадра
    glutPostRedisplay(); // указание на необходимость перерисовки текущего окна 
}
// инициализация функций обратного вызова
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
    glutCreateWindow("Task 12"); // создание окна

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