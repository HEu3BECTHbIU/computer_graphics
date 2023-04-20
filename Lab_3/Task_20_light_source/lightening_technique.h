#pragma once

#include "technique.h"
#include "math_3d.h"

#define MAX_POINT_LIGHTS 3
#define INVALID_UNIFORM_LOCATION 0xFFFFFFFF
struct BaseLight
{
    Vector3f Color; // цвет света
    float AmbientIntensity; // интенсивность фонового
    float DiffuseIntensity; // инетнсивность рассеянного

    BaseLight()
    {
        Color = Vector3f(0.0f, 0.0f, 0.0f);
        AmbientIntensity = 0.0f;
        DiffuseIntensity = 0.0f;
    }
};

struct PointLight : public BaseLight // точечный свет
{
    Vector3f Position;

    struct
    {
        float Constant; // коэффициенты затухания
        float Linear;
        float Exp;
    } Attenuation;

    PointLight()
    {
        Position = Vector3f(0.0f, 0.0f, 0.0f);
        Attenuation.Constant = 1.0f;
        Attenuation.Linear = 0.0f;
        Attenuation.Exp = 0.0f;
    }
};
struct DirectionalLight : public BaseLight // направленный свет
{
    Vector3f Direction; // направление

    DirectionalLight()
    {
        Direction = Vector3f(0.0f, 0.0f, 0.0f);
    }
};

class LightingTechnique : public Technique // класс техники света
{
public:
    LightingTechnique();
    virtual bool Init();

    void SetWVP(const Matrix4f* WVP);
    void SetTextureUnit(unsigned int TextureUnit);
    void SetDirectionalLight(const DirectionalLight& Light);
    void SetWorldMatrix(const Matrix4f& WVP);

    void SetEyeWorldPos(const Vector3f& EyeWorldPos);
    void SetMatSpecularIntensity(float Intensity);
    void SetMatSpecularPower(float Power);
    void SetPointLights(unsigned int NumLights, const PointLight* pLights);

private:
    GLuint m_WVPLocation;
    GLuint m_WorldMatrixLocation;
    GLuint m_samplerLocation;

    GLuint m_eyeWorldPosition;
    GLuint m_matSpecularIntensityLocation;
    GLuint m_matSpecularPowerLocation;

    GLuint m_numPointLightsLocation;
    struct 
    {
        GLuint Color;
        GLuint AmbientIntensity;
        GLuint Direction;
        GLuint DiffuseIntensity;
    } m_dirLightLocation;

    struct
    {
        GLuint Color;
        GLuint AmbientIntensity;
        GLuint DiffuseIntensity;
        GLuint Position;
        struct
        {
            GLuint Constant;
            GLuint Linear;
            GLuint Exp;
        } Atten;
    } m_pointLightsLocation[MAX_POINT_LIGHTS];
};
