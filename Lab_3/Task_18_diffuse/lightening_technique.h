#pragma once

#include "technique.h"
#include "math_3d.h"

struct DirectionLight // направленный свет
{
    Vector3f Color; // цвет
    float AmbientIntensity; // интенсивность фонового света
    Vector3f Direction; // направление луча
    float DiffuseIntensity; // интенсивность диффузного света
};

class LightingTechnique : public Technique // класс техники света
{
public:
    LightingTechnique();
    virtual bool Init();

    void SetWVP(const Matrix4f* WVP);
    void SetTextureUnit(unsigned int TextureUnit);
    void SetDirectionalLight(const DirectionLight& Light);
    void SetWorldMatrix(const Matrix4f& WVP);

private:
    GLuint m_WVPLocation;
    GLuint m_WorldMatrixLocation;
    GLuint m_samplerLocation;

    struct {
        GLuint Color;
        GLuint AmbientIntensity;
        GLuint Direction;
        GLuint DiffuseIntensity;
    } m_dirLightLocation;
};
