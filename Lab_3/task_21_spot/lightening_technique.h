#ifndef LIGHTING_TECHNIQUE_H
#define	LIGHTING_TECHNIQUE_H
#include "technique.h"
#include "math_3d.h"

// #define MAX_POINT_LIGHTS 3
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

struct DirectionalLight : public BaseLight // направленный свет
{
    Vector3f Direction; // направление

    DirectionalLight()
    {
        Direction = Vector3f(0.0f, 0.0f, 0.0f);
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

struct SpotLight : public PointLight // свет типа прожектор
{
    Vector3f Direction; // направление
    float Cutoff; // радиус, после которого свет затухает

    SpotLight()
    {
        Direction = Vector3f(0.0f, 0.0f, 0.0f);
        Cutoff = 0.0f;
    }
};

class LightingTechnique : public Technique // класс техники света
{
public:

    static const unsigned int MAX_POINT_LIGHTS = 2; // максимальное количество точечных источников
    static const unsigned int MAX_SPOT_LIGHTS = 2; // макс. количество прожекторов

    LightingTechnique();

    virtual bool Init();

    void SetWVP(const Matrix4f& WVP);
    void SetWorldMatrix(const Matrix4f& WVP);
    void SetTextureUnit(unsigned int TextureUnit);
    void SetDirectionalLight(const DirectionalLight& Light);
    void SetPointLights(unsigned int NumLights, const PointLight* pLights);
    void SetSpotLights(unsigned int NumLights, const SpotLight* pLights);
    void SetEyeWorldPos(const Vector3f& EyeWorldPos);
    void SetMatSpecularIntensity(float Intensity);
    void SetMatSpecularPower(float Power);

private:
    // переменные, в которые будет помещено местоположение юниформ переменных в шейдере
    GLuint m_WVPLocation;
    GLuint m_WorldMatrixLocation;
    GLuint m_samplerLocation;
    GLuint m_eyeWorldPosLocation;
    GLuint m_matSpecularIntensityLocation;
    GLuint m_matSpecularPowerLocation;
    GLuint m_numPointLightsLocation;
    GLuint m_numSpotLightsLocation;

    // структура, в поля которой будут помещены местонахождения юниформ переменных
    // соответствующей структуры в шейдере (направленный свет)
    struct
    {
        GLuint Color;
        GLuint AmbientIntensity;
        GLuint DiffuseIntensity;
        GLuint Direction;
    } m_dirLightLocation;

    // аналогичный массив структур для точечных источников света 
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

    // аналогичный массив структур для прожекторов
    struct
    {
        GLuint Color;
        GLuint AmbientIntensity;
        GLuint DiffuseIntensity;
        GLuint Position;
        GLuint Direction;
        GLuint Cutoff;
        struct 
        {
            GLuint Constant;
            GLuint Linear;
            GLuint Exp;
        } Atten;
    } m_spotLightsLocation[MAX_SPOT_LIGHTS];
};


#endif	/* LIGHTING_TECHNIQUE_H */
