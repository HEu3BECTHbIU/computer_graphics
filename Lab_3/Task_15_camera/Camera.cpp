#include <GL/freeglut.h>

#include "camera.h"

const static float StepScale = 0.1f;
const static int MARGIN = 10;

Camera::Camera(int WindowWidth, int WindowHeight) // инициализация камеры по умолчанию
{
    m_windowWidth = WindowWidth;
    m_windowHeight = WindowHeight;
    m_pos = Vector3f(0.0f, 0.0f, 0.0f);
    m_target = Vector3f(0.0f, 0.0f, 1.0f);
    m_target.Normalize();
    m_up = Vector3f(0.0f, 1.0f, 0.0f);

    Init();
}

// инициализация камеры с параметрами
Camera::Camera(int WindowWidth, int WindowHeight, const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up)
{
    m_windowWidth = WindowWidth;
    m_windowHeight = WindowHeight;
    m_pos = Pos;

    m_target = Target;
    m_target.Normalize();

    m_up = Up;
    m_up.Normalize();

    Init();
}
// инициализация внутренних параметров камеры (вычисление горизонтального угла + вертикального)
void Camera::Init()
{
    Vector3f HTarget(m_target.x, 0.0, m_target.z);
    HTarget.Normalize();
    if (HTarget.z >= 0.0f)
    {
        if (HTarget.x >= 0.0f)
        {
            m_AngleH = 360.0f - ToDegree(asin(HTarget.z));
        }
        else
        {
            m_AngleH = 180.0f + ToDegree(asin(HTarget.z));
        }
    }
    else
    {
        if (HTarget.x >= 0.0f)
        {
            m_AngleH = ToDegree(asin(-HTarget.z));
        }
        else
        {
            m_AngleH = 90.0f + ToDegree(asin(-HTarget.z));
        }
    }

    m_AngleV = -ToDegree(asin(m_target.y));

    m_OnUpperEdge = false; // обнуление флагов нахождения на границе
    m_OnLowerEdge = false;
    m_OnLeftEdge = false;
    m_OnRightEdge = false;
    m_mousePos.x = m_windowWidth / 2;
    m_mousePos.y = m_windowHeight / 2;
    //printf("%i, %i", m_mousePos.x, m_mousePos.y);
    glutWarpPointer(m_mousePos.x, m_mousePos.y); // перестановка курсора в центр экрана
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

void Camera::OnMouse(int x, int y) // при перемещении мыши
{
    const int DeltaX = x - m_mousePos.x; // разница между новыми и старыми координатами
    const int DeltaY = y - m_mousePos.y;

    m_mousePos.x = x;
    m_mousePos.y = y;

    m_AngleH += (float)DeltaX / 20.0f; // изменение горизонтального и вертикального углов
    m_AngleV += (float)DeltaY / 20.0f;
    // если на границах
    if (DeltaX == 0) {
        if (x <= MARGIN) {
               // m_AngleH -= 1.0f;
            m_OnLeftEdge = true;
        }
        else if (x >= (m_windowWidth - MARGIN)) {
               // m_AngleH += 1.0f;
            m_OnRightEdge = true;
        }
    }
    else {
        m_OnLeftEdge = false;
        m_OnRightEdge = false;
    }

    if (DeltaY == 0) {
        if (y <= MARGIN) {
            m_OnUpperEdge = true;
        }
        else if (y >= (m_windowHeight - MARGIN)) {
            m_OnLowerEdge = true;
        }
    }
    else {
        m_OnUpperEdge = false;
        m_OnLowerEdge = false;
    }

    Update(); // вносим обновления
}


void Camera::OnRender() // функция, вращающая камеру автоматически (изменяет соответствующий угол)
{
    bool ShouldUpdate = false;

    if (m_OnLeftEdge)
    {
        m_AngleH -= 0.01f;
        ShouldUpdate = true;
    }
    else if (m_OnRightEdge) 
    {
        m_AngleH += 0.01f;
        ShouldUpdate = true;
    }

    if (m_OnUpperEdge)
    {
        if (m_AngleV > -90.0f)
        {
            m_AngleV -= 0.01f;
            ShouldUpdate = true;
        }
    }
    else if (m_OnLowerEdge)
    {
        if (m_AngleV < 90.0f)
        {
            m_AngleV += 0.01f;
            ShouldUpdate = true;
        }
    }
    if (ShouldUpdate)
    {
        Update();
    }
}

void Camera::Update() // обновление векторов камеры
{
    const Vector3f Vaxis(0.0f, 1.0f, 0.0f);

    // вращает вектор view на горизонтальный угол вокруг вертикальной оси
    Vector3f View(1.0f, 0.0f, 0.0f);
    View.Rotate(m_AngleH, Vaxis);
    View.Normalize();

    // вращает вектор view на вертикальный угол вокруг горизонтальной оси
    Vector3f Haxis = Vaxis.Cross(View);
    Haxis.Normalize();
    View.Rotate(m_AngleV, Haxis);

    m_target = View; // обновление векторов камеры 
    m_target.Normalize();

    m_up = m_target.Cross(Haxis);
    m_up.Normalize();
}