#pragma once
#include <GL/glew.h>

// класс RandomTexture очень удобен для обеспечения случайных данных в шейдере
class RandomTexture
{
public:
    RandomTexture();

    ~RandomTexture();

    bool InitRandomTexture(unsigned int Size);

    void Bind(GLenum TextureUnit);

private:
    GLuint m_textureObj;
};
