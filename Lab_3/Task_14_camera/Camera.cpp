#include <GL/freeglut.h>
#include "Camera.h"

Camera::Camera()
{
    m_pos = Vector3f(0.0f, 0.0f, 0.0f);
    m_target = Vector3f(0.0f, 0.0f, 1.0f);
    m_up = Vector3f(0.0f, 1.0f, 0.0f);
}


Camera::Camera(const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up)
{
    m_pos = Pos;
    m_target = Target;
    m_target.Normalize();
    m_up = Up;
    m_up.Normalize();
}

bool Camera::OnKeyboard(int Key) // функция, изменяющая положение камеры в зависимости от нажатой клавиши
{
    bool Ret = false;

    switch (Key)
    {
    case GLUT_KEY_UP: // если стрелка вверх
    {
        m_pos += (m_target * StepScale); // прибавляем к позиции вектор направления (приближаемся)
        Ret = true;
    }
    break;

    case GLUT_KEY_DOWN: // если стрелка вниз
    {
        m_pos -= (m_target * StepScale); // отнимаем от позиции вектор направления
        Ret = true;
    }
    break;

    case GLUT_KEY_LEFT: // если стрелка влево
    {
        Vector3f Left = m_target.Cross(m_up); // вычисляем вектор налево
        Left.Normalize(); // нормализуем
        Left *= StepScale;
        m_pos += Left; // прибавляем к позиции вектор налево (перемещаем камеру вдоль вектора)
        Ret = true;
    }
    break;

    case GLUT_KEY_RIGHT: // если стрелка вправо
    {
        Vector3f Right = m_up.Cross(m_target); // вычисляем вектор направо 
        Right.Normalize();
        Right *= StepScale;
        m_pos += Right; // прибавляем к позиции вектор направо (перемещаем камеру вдоль вектора)
        Ret = true;
    }
    break;
    }
    return Ret;
}