#include <iostream>
//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "cubemap_texture.h"
#include "util.h"

// типы кубических текстур
static const GLenum types[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                                  GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                                  GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                                  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                                  GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                                  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };

// загрузка путей к текстурам
CubemapTexture::CubemapTexture(const string& Directory,
    const string& PosXFilename,
    const string& NegXFilename,
    const string& PosYFilename,
    const string& NegYFilename,
    const string& PosZFilename,
    const string& NegZFilename)
{
    string::const_iterator it = Directory.end();
    it--;
    string BaseDir = (*it == '/') ? Directory : Directory + "/";

    m_fileNames[0] = BaseDir + PosXFilename;
    m_fileNames[1] = BaseDir + NegXFilename;
    m_fileNames[2] = BaseDir + PosYFilename;
    m_fileNames[3] = BaseDir + NegYFilename;
    m_fileNames[4] = BaseDir + PosZFilename;
    m_fileNames[5] = BaseDir + NegZFilename;

    m_textureObj = 0;
}

CubemapTexture::~CubemapTexture()
{
    if (m_textureObj != 0) {
        glDeleteTextures(1, &m_textureObj);
    }
}

bool CubemapTexture::Load()
{
    glGenTextures(1, &m_textureObj); // генерация объекта текстуры
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureObj); // привязка объекта к метке GL_TEXTURE_CUBE_MAP

    stbi_set_flip_vertically_on_load(0);
    int width = 0, height = 0, bpp = 0;
    unsigned char* imagedata = NULL;
    // цикл, который содержит перечисление GL, которое представляет стороны кубической текстуры
    // (GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X и т.д)
    for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(types); i++)
    {
        imagedata = stbi_load(m_fileNames[i].c_str(), &width, &height, &bpp, 0); // вызов функции загрузки файла
        if (!imagedata) // если неудачно
        {
            printf("Cant load texture from '%s' - %s/n", m_fileNames[i].c_str(), stbi_failure_reason());
        }
        // данные текстуры (тип должен совпадать с текстурой из file_names
        glTexImage2D(types[i], 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imagedata);

        stbi_image_free(imagedata); // удаляем указатель
    }
    // устанавливаем некоторые флаги
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // линейная интерполяция
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return true;
}

// функция привязки текстуры для рисования
void CubemapTexture::Bind(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureObj);
}