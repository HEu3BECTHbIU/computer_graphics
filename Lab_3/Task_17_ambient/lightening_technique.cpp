#include "lightening_technique.h"

// вершинный шейдер
static const char* pVS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
layout (location = 0) in vec3 Position;                                             \n\
layout (location = 1) in vec2 TexCoord;                                             \n\
                                                                                    \n\
uniform mat4 gWVP;                                                                  \n\
                                                                                    \n\
out vec2 TexCoord0;                                                                 \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    gl_Position = gWVP * vec4(Position, 1.0);                                       \n\
    TexCoord0 = TexCoord;                                                           \n\
}";

// фрагментный шейдер
static const char* pFS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
in vec2 TexCoord0;                                                                  \n\
                                                                                    \n\
out vec4 FragColor;                                                                 \n\
                                                                                    \n\
struct DirectionalLight                                                             \n\
{                                                                                   \n\
    vec3 Color;                                                                     \n\
    float AmbientIntensity;                                                         \n\
};                                                                                  \n\
                                                                                    \n\
uniform DirectionalLight gDirectionalLight;                                         \n\
uniform sampler2D gSampler;                                                         \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    FragColor = texture2D(gSampler, TexCoord0.xy) *                                 \n\
                vec4(gDirectionalLight.Color, 1.0f) *                               \n\
                gDirectionalLight.AmbientIntensity;                                 \n\
}";
// цвет пикселя = семплер текстуры * цвет света *интенсивность освещения 

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

    m_WVPLocation = GetUniformLocation("gWVP"); // получение локации юниформ переменной 
    // семлер нужен, чтобы передать в фрагментный шейдер модуль текстуры для доступа в самом шейдере
    m_samplerLocation = GetUniformLocation("gSampler"); // получение локации семплера uniform переменной
    // получение локации uniform переменной пDirectionalLight.Color
    m_dirLightColorLocation = GetUniformLocation("gDirectionalLight.Color"); 
    m_dirLightAmbientIntensityLocation = GetUniformLocation("gDirectionalLight.AmbientIntensity");

    // если хотя бы одна не получена
    if (m_dirLightAmbientIntensityLocation == 0xFFFFFFFF ||
        m_WVPLocation == 0xFFFFFFFF ||
        m_samplerLocation == 0xFFFFFFFF ||
        m_dirLightColorLocation == 0xFFFFFFFF)
    {
        return false;
    }

    return true;
}

void LightingTechnique::SetWVP(const Matrix4f* WVP)
{
    // передаем в юниформ переменную матрицу трансформации
    glUniformMatrix4fv(m_WVPLocation, 1, GL_TRUE, (const GLfloat*)WVP->m);
}

void LightingTechnique::SetTextureUnit(unsigned int TextureUnit)
{
    // устанавливаем индексы модулей текстуры,
    // который мы собираемся использовать внутри сэмплера uniform-переменной в шейдере
    glUniform1i(m_samplerLocation, TextureUnit);
}

void LightingTechnique::SetDirectionalLight(const DirectionLight& Light)
{
    glUniform3f(m_dirLightColorLocation, Light.Color.x, Light.Color.y, Light.Color.z);
    glUniform1f(m_dirLightAmbientIntensityLocation, Light.AmbientIntensity);
}