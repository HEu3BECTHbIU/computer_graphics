#pragma once

#include "technique.h"
#include "math_3d.h"

struct DirectionLight // направленный свет
{
    Vector3f Color; // цвет
    float AmbientIntensity; // интенсивность
};

class LightingTechnique : public Technique // класс техники света
{
public:
    LightingTechnique();
    virtual bool Init();

    void SetWVP(const Matrix4f* WVP);
    void SetTextureUnit(unsigned int TextureUnit);
    void SetDirectionalLight(const DirectionLight& Light);

private:
    GLuint m_WVPLocation;
    GLuint m_samplerLocation;
    GLuint m_dirLightColorLocation;
    GLuint m_dirLightAmbientIntensityLocation;
};
