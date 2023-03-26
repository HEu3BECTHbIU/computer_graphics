
#include <stdio.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <cassert>
#include <math.h>
#include "Pipeline.h"
#include <iostream>
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
    vec3 new_position = vec3 (Position.x * 0.5, Position.y * 0.5, Position.z * 0.5); \n\
    gl_Position = gScale * vec4(new_position, 1.0);                                 \n\
    Color = vec4 (clamp(Position, 0.0, 1.0), 1.0); \n\
}";

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
void CreateVertexBuffer()
{
    // Vector3f vertices[4];
    Vector3f vertices[8];
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
    unsigned int indexes[] = { 0, 3, 2, 1,
                              1, 7, 6, 2,
                              6, 2, 3, 5,
                              5, 6, 7, 4,
                              4, 7, 1, 0,
                              0, 4, 5, 3 };
    glGenBuffers(1, &Indexbuf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Indexbuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_DYNAMIC_DRAW);
}
void RenderScene()
{
    glClear(GL_COLOR_BUFFER_BIT);
    Scale += 0.01f;
    Scale2 += 0.0001f;
    Pipeline p;
    p.Rotate(0.0f, (Scale), 0.0f);
    //p.WorldPos(sinf(Scale), sinf(Scale2), 5.0f);
    Vector3f CameraPos(2.0f, 1.0f, -5.0f);
    Vector3f CameraTarget(-0.75f, -0.25f, 1.0f);
    Vector3f CameraUp(0.0f, 1.0f, 0.0f);
    p.SetCamera(CameraPos, CameraTarget, CameraUp);
    p.SetPerspectiveProj(40.0f, 1024, 728, 1.0f, 100.0f);
    glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, (const GLfloat*)p.GetTrans());
   // static int count = 0;
    //if (count == 10)
    //{
//CameraUp.Print();
//std::cout << "\n";
//count++;
   // }
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, Vbuf);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Indexbuf);
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(0);
    glutSwapBuffers();
    glutPostRedisplay();
}

void InitializeGlutCallbacks()
{
    glutDisplayFunc(RenderScene);
    glutIdleFunc(RenderScene);
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0)
    {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }
    const GLchar* p[1];
    p[0] = pShaderText;
    GLint Lengths[1];
    Lengths[0] = strlen(pShaderText);
    glShaderSource(ShaderObj, 1, p, Lengths);
    glCompileShader(ShaderObj);
    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }
    glAttachShader(ShaderProgram, ShaderObj);
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

    glValidateProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
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
    glutInitWindowSize(1024, 728);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Shaiders");

    InitializeGlutCallbacks();

    GLenum res = glewInit();
    if (res != GLEW_OK)
    {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }
    CreateVertexBuffer();
    CompileShaders();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glutMainLoop();
    return 0;
}