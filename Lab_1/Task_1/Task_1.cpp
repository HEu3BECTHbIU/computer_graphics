#include <gl/freeglut.h> // подключение библиотеки glut 

void RenderSceneCB() // функция рендера
{
    glClear(GL_COLOR_BUFFER_BIT); // очистка буфера кадра
    glutSwapBuffers(); // поменять местами фоновый буфер и буфер кадра
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv); // инициализация glut
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // настройка опций glut
    // двойная буферизация и буфер цвета для отображения на экране
    glutInitWindowSize(1024, 768); // настройка размер отображаемого окна
    glutInitWindowPosition(100, 100); // настройка положения окна
    glutCreateWindow("Task 1"); // имя окна 

    glutDisplayFunc(RenderSceneCB); // вызываем RenderScene
    glClearColor(0.0f, 1.0f, 0.0f, 0.0f); // устанавливаем цвет, который
    // будет использоваться во время очистки буфера кадра
    glutMainLoop(); // передача управления glut
}