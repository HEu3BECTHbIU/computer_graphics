#pragma once
#include "technique.h"
#include "math_3d.h"

class BillboardTechnique : public Technique
{
public:

    BillboardTechnique();

    virtual bool Init();

    void SetVP(const Matrix4f& VP);
    void SetCameraPosition(const Vector3f& Pos);
    void SetColorTextureUnit(unsigned int TextureUnit);
    void SetBillboardSize(float BillboardSize);

private:
    // переменные для хранения местоположения юниформ переменных
    GLuint m_VPLocation;
    GLuint m_cameraPosLocation;
    GLuint m_colorMapLocation;
    GLuint m_billboardSizeLocation;
};