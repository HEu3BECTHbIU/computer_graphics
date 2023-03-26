#include <stdio.h>
#include <GL/glew.h> // ����������� GLEW
#include <GL/freeglut.h> // ����������� glut
#include "math_3d.h"

GLuint VBO; // ���������� ��� �������� ��������� �� ����� ������

static void RenderSceneCB() // ������� �������
{
    glClear(GL_COLOR_BUFFER_BIT); // ������� ������ �����

    glEnableVertexAttribArray(0); // ���������� ������������� ��������� �������
    // ������ 0 - ���������� �������
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // ��������� �������� ������ ����� ����������
    // ��������� ����� ������� ��������� ��� ������������ ������ ������ ������
    // 0 - ������ ��������, 3 - ���-�� ��������� � �������� (3 �������), ��� ���������
    // ����� �� ������������ ���������, ���-�� ���� ����� 2 ������������ � ��������, �������� � ���������
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_POINTS, 0, 1); // ��������� ����� (GL_POINTS)

    glDisableVertexAttribArray(0); // ������ ������������� ��������� ������� 

    glutSwapBuffers(); // ����� �������� ������ � ������ �����
}

static void InitializeGlutCallbacks()
{
    glutDisplayFunc(RenderSceneCB);
}

static void CreateVertexBuffer()
{
    Vector3f Vertices[1]; // ������� ������ �������� ���� Vector3f (�����)
    Vertices[0] = Vector3f(0.0f, 0.0f, 0.0f); // ���������� ����� (x,y,z)

    glGenBuffers(1, &VBO); // �������� ������ (���� ��� ��� ������������� ����)
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // ��������, ��� ����� ����� ������� ������ ������ (GL_ARRAY_BUFFER) 
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    // �������� ������ � ������ (������ Vertices) GL_STATIC_DRAW - �������� ������ �� ����� ��������
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv); // ������������� glut
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Tutorial 02");

    InitializeGlutCallbacks(); // ������������� ������� ��������� ������

    GLenum res = glewInit(); // ������������� glew 
    if (res != GLEW_OK) // �������� �� ������ �������������
    {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    glClearColor(1.0f, 0.0f, 0.0f, 0.0f); // ������������� ����
    CreateVertexBuffer(); // ����� ������� �������� ������ ������
    glutMainLoop(); // �������� ���������� glut

    return 0;
}



