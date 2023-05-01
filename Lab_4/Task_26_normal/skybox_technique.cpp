#include <limits.h>
#include <string.h>

#include "skybox_technique.h"
#include "util.h"
// вершинный шейдер скайбокса
static const char* pVS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
layout (location = 0) in vec3 Position;                                             \n\
                                                                                    \n\
uniform mat4 gWVP;                                                                  \n\
                                                                                    \n\
out vec3 TexCoord0;                                                                 \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    vec4 WVP_Pos = gWVP * vec4(Position, 1.0);                                      \n\
    gl_Position = WVP_Pos.xyww;                                                     \n\
    TexCoord0   = Position;                                                         \n\
}";
// Фрагментный шейдер
// используем 'samplerCube' вместо 'sampler2D' для получения доступа к кубической текстуре
static const char* pFS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
in vec3 TexCoord0;                                                                  \n\
                                                                                    \n\
out vec4 FragColor;                                                                 \n\
                                                                                    \n\
uniform samplerCube gCubemapTexture;                                                \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    FragColor = texture(gCubemapTexture, TexCoord0);                                \n\
}";


SkyboxTechnique::SkyboxTechnique()
{
}

SkyboxTechnique::~SkyboxTechnique()
{
}

bool SkyboxTechnique::Init()  // инициализация шейдеров как и в других техниках
{
    if (!Technique::Init()) 
    {
        return false;
    }
    if (!AddShader(GL_VERTEX_SHADER, pVS))
    {
        return false;
    }
    if (!AddShader(GL_FRAGMENT_SHADER, pFS))
    {
        return false;
    }
    if (!Finalize())
    {
        return false;
    }
    // позиции юниформ перемнных
    m_WVPLocation = GetUniformLocation("gWVP");
    m_textureLocation = GetUniformLocation("gCubemapTexture");

    if (m_WVPLocation == INVALID_UNIFORM_LOCATION ||
        m_textureLocation == INVALID_UNIFORM_LOCATION) {
        return false;
    }

    return true;
}
// функция передачи матрицы в юниформ переменную в шейдер
void SkyboxTechnique::SetWVP(const Matrix4f& WVP)
{
    glUniformMatrix4fv(m_WVPLocation, 1, GL_TRUE, (const GLfloat*)WVP.m);
}

void SkyboxTechnique::SetTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(m_textureLocation, TextureUnit);
}