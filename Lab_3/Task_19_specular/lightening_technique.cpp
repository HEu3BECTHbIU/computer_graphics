#include "lightening_technique.h"

// вершинный шейдер
// входят в шейдер - позиция, координата текстуры, и нормаль
// выходят - интерполированная координата текстуры и вектор нормали
static const char* pVS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
layout (location = 0) in vec3 Position;                                             \n\
layout (location = 1) in vec2 TexCoord;                                             \n\
layout (location = 2) in vec3 Normal;                                               \n\
                                                                                    \n\
uniform mat4 gWVP;                                                                  \n\
uniform mat4 gWorld;                                                                \n\
                                                                                    \n\
out vec2 TexCoord0;                                                                 \n\
out vec3 Normal0;                                                                   \n\
out vec3 WorldPos0;                                                                 \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    gl_Position = gWVP * vec4(Position, 1.0);                                       \n\
    TexCoord0 = TexCoord;                                                           \n\
    Normal0   = (gWorld * vec4(Normal, 0.0)).xyz;                                   \n\
    WorldPos0 = (gWorld * vec4(Position, 1.0)).xyz;                                 \n\
}";

// фрагментный шейдер
// на входе - интерполированное значение текстуры, вектор нормали
// выход - конкретный цвет пикселя FragColor
static const char* pFS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
in vec2 TexCoord0;                                                                  \n\
in vec3 Normal0;                                                                    \n\
in vec3 WorldPos0;                                                                  \n\
                                                                                    \n\
out vec4 FragColor;                                                                 \n\
                                                                                    \n\
struct DirectionalLight                                                             \n\
{                                                                                   \n\
    vec3 Color;                                                                     \n\
    float AmbientIntensity;                                                         \n\
    vec3 Direction;                                                                 \n\
    float DiffuseIntensity;                                                         \n\
};                                                                                  \n\
                                                                                    \n\
uniform DirectionalLight gDirectionalLight;                                         \n\
uniform sampler2D gSampler;                                                         \n\
                                                                                    \n\
uniform vec3 gEyeWorldPos;                                                          \n\
uniform float gMatSpecularIntensity;                                                \n\
uniform float gSpecularPower;                                                       \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    vec4 AmbientColor = vec4(gDirectionalLight.Color, 1.0f) *                       \n\
                        gDirectionalLight.AmbientIntensity;                         \n\
                                                                                    \n\
    vec3 LightDirection = -gDirectionalLight.Direction;                             \n\
    vec3 Normal = normalize(Normal0);                                               \n\
                                                                                    \n\
    float DiffuseFactor = dot(Normal, LightDirection);                              \n\
                                                                                    \n\
    vec4 DiffuseColor  = vec4(0, 0, 0, 0);                                          \n\
    vec4 SpecularColor = vec4(0, 0, 0, 0);                                          \n\
                                                                                    \n\
    if (DiffuseFactor > 0){                                                         \n\
        DiffuseColor = vec4(gDirectionalLight.Color, 1.0f) *                        \n\
                       gDirectionalLight.DiffuseIntensity *                         \n\
                       DiffuseFactor;                                               \n\
                                                                                    \n\
        vec3 VertexToEye = normalize(gEyeWorldPos - WorldPos0);                     \n\
        vec3 LightReflect = normalize(reflect(gDirectionalLight.Direction, Normal));\n\
        float SpecularFactor = dot(VertexToEye, LightReflect);                      \n\
        SpecularFactor = pow(SpecularFactor, gSpecularPower);                       \n\
                                                                                    \n\
        if (SpecularFactor > 0){                                                    \n\
            SpecularColor = vec4(gDirectionalLight.Color, 1.0f) *                   \n\
                            gMatSpecularIntensity *                                 \n\
                            SpecularFactor;                                         \n\
        }                                                                           \n\
    }                                                                               \n\
                                                                                    \n\
    FragColor = texture2D(gSampler, TexCoord0.xy) *                                 \n\
                (AmbientColor + DiffuseColor + SpecularColor);                      \n\
}";
// dot - скалярное произведение векторов. Чем больше - тем больше угол -> тем ярче должен быть пиксель
// цвет пикселя = семплер текстуры * цвет света * интенсивность фонового + рассеянного + зеркального освещения 

LightingTechnique::LightingTechnique()
{
}

bool LightingTechnique::Init()
{
    if (!Technique::Init())
    {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, pVS)) // добавление вершинного шейдера
    {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, pFS)) // добавление фрагментного шейдера
    {
        return false;
    }

    if (!Finalize())
    {
        return false;
    }
    m_WorldMatrixLocation = GetUniformLocation("gWorld"); // uniform переменная матрицы нормали

    m_WVPLocation = GetUniformLocation("gWVP"); // получение локации юниформ переменной матрицы трансформации
    // семлер нужен, чтобы передать в фрагментный шейдер модуль текстуры для доступа в самом шейдере
    m_samplerLocation = GetUniformLocation("gSampler"); // получение локации семплера uniform переменной
    // uniform переменные фонового света (цвет и интенсивность)
    m_dirLightLocation.Color = GetUniformLocation("gDirectionalLight.Color");
    m_dirLightLocation.AmbientIntensity = GetUniformLocation("gDirectionalLight.AmbientIntensity");
    // uniform переменные диффузного света (цвет и интенсивность)
    m_dirLightLocation.Direction = GetUniformLocation("gDirectionalLight.Direction");
    m_dirLightLocation.DiffuseIntensity = GetUniformLocation("gDirectionalLight.DiffuseIntensity");

    // юниформ переменные зеркального света (бликов) 
    m_eyeWorldPosition = GetUniformLocation("gEyeWorldPos");
    m_matSpecularIntensityLocation = GetUniformLocation("gMatSpecularIntensity");
    m_matSpecularPowerLocation = GetUniformLocation("gSpecularPower");
    // если хотя бы одна не получена
    if (m_dirLightLocation.AmbientIntensity == 0xFFFFFFFF ||
        m_WVPLocation == 0xFFFFFFFF ||
        m_WorldMatrixLocation == 0xFFFFFFFF ||
        m_samplerLocation == 0xFFFFFFFF ||
        m_dirLightLocation.Color == 0xFFFFFFFF ||
        m_dirLightLocation.DiffuseIntensity == 0xFFFFFFFF ||
        m_eyeWorldPosition == 0xFFFFFFFF ||
        m_matSpecularIntensityLocation == 0xFFFFFFFF ||
        m_matSpecularPowerLocation == 0xFFFFFFFF ||
        m_dirLightLocation.Direction == 0xFFFFFFFF) {
        return false;
    }

    return true;
}

void LightingTechnique::SetWVP(const Matrix4f* WVP)
{
    // передаем в юниформ переменную матрицу трансформации
    glUniformMatrix4fv(m_WVPLocation, 1, GL_TRUE, (const GLfloat*)WVP->m);
}

void LightingTechnique::SetWorldMatrix(const Matrix4f& WorldInverse)
{
    glUniformMatrix4fv(m_WorldMatrixLocation, 1, GL_TRUE, (const GLfloat*)WorldInverse.m);
}
void LightingTechnique::SetTextureUnit(unsigned int TextureUnit)
{
    // устанавливаем индексы модулей текстуры,
    // который мы собираемся использовать внутри сэмплера uniform-переменной в шейдере
    glUniform1i(m_samplerLocation, TextureUnit);
}
// передача соответствующих векторов цветов и значений интенсивности света в юниформ переменные
void LightingTechnique::SetDirectionalLight(const DirectionLight& Light)
{
    glUniform3f(m_dirLightLocation.Color, Light.Color.x, Light.Color.y, Light.Color.z);
    glUniform1f(m_dirLightLocation.AmbientIntensity, Light.AmbientIntensity);
    Vector3f Direction = Light.Direction;
    Direction.Normalize();
    glUniform3f(m_dirLightLocation.Direction, Direction.x, Direction.y, Direction.z);
    glUniform1f(m_dirLightLocation.DiffuseIntensity, Light.DiffuseIntensity);
}
// передача интенсивности света в юниформ переменную
void LightingTechnique::SetMatSpecularIntensity(float Intensity)
{
    glUniform1f(m_matSpecularIntensityLocation, Intensity);
}
// передача силы зеркального отражения света материала в юниформ переменную
void LightingTechnique::SetMatSpecularPower(float Power)
{
    glUniform1f(m_matSpecularPowerLocation, Power);
}
// передача позиции камеры в юниформ перемнную в шейдер
void LightingTechnique::SetEyeWorldPos(const Vector3f& EyeWorldPos)
{
    glUniform3f(m_eyeWorldPosition, EyeWorldPos.x, EyeWorldPos.y, EyeWorldPos.z);
}