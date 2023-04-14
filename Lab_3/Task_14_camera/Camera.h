#include <GL/freeglut.h>
#include "math_3d.h"

const static float StepScale = 0.1f; // шаг при нажатии на клавишу

class Camera
{
private:
    Vector3f m_pos;
    Vector3f m_target;
    Vector3f m_up;

public:

    const Vector3f& GetPos() const
    {
        return m_pos;
    }

    const Vector3f& GetTarget() const
    {
        return m_target;
    }

    const Vector3f& GetUp() const
    {
        return m_up;
    }
    Camera();

    Camera(const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up);
   
    bool OnKeyboard(int Key);

};
