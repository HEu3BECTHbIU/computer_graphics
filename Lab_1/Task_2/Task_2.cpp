#include <stdio.h>
#include <GL/glew.h> // подключение GLEW
#include <GL/freeglut.h> // подключение glut
#include "math_3d.h"

GLuint VBO; // переменная для хранения указателя на буфер вершин

static void RenderSceneCB() // функция рендера
{
    glClear(GL_COLOR_BUFFER_BIT); // очистка буфера кадра

    glEnableVertexAttribArray(0); // разрешение использования атрибутов вершины
    // индекс 0 - координаты вершины
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // повторная привязка буфера перед отрисовкой
    // следующий вызов говорит конвейеру как воспринимать данные внутри буфера
    // 0 - индекс атрибута, 3 - кол-во элементов в атрибуте (3 вершины), тип элементов
    // нужна ли нормализация атрибутов, кол-во байт между 2 экземплярами в атрибуте, смещение в структуре
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_POINTS, 0, 1); // отрисовка точки (GL_POINTS)

    glDisableVertexAttribArray(0); // запрет использования атрибутов вершины 

    glutSwapBuffers(); // смена фонового буфера и буфера кадра
}

static void InitializeGlutCallbacks()
{
    glutDisplayFunc(RenderSceneCB);
}

static void CreateVertexBuffer()
{
    Vector3f Vertices[1]; // создаем массив объектов типа Vector3f (точки)
    Vertices[0] = Vector3f(0.0f, 0.0f, 0.0f); // координаты точки (x,y,z)

    glGenBuffers(1, &VBO); // создание буфера (пока что без конкретизации цели)
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // указание, что буфер будет хранить массив вершин (GL_ARRAY_BUFFER) 
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    // привязка данных к буферу (массив Vertices) GL_STATIC_DRAW - значения буфера не будут меняться
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv); // инициализация glut
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Tutorial 02");

    InitializeGlutCallbacks(); // инициализация функций обратного вызова

    GLenum res = glewInit(); // инициализация glew 
    if (res != GLEW_OK) // проверка на ошибку инициализации
    {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    glClearColor(1.0f, 0.0f, 0.0f, 0.0f); // устанавливаем цвет
    CreateVertexBuffer(); // вызов функции создания буфера вершин
    glutMainLoop(); // передача управления glut

    return 0;
}



