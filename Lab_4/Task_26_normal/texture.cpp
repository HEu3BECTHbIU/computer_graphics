//#include <iostream>
//#include<stb/stb_image.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "texture.h"

Texture::Texture(GLenum TextureTarget, const std::string& FileName) // инициализация объекта текстуры
{
    m_textureTarget = TextureTarget;
    m_fileName = FileName;
}

bool Texture::Load() // загрузка текстуры
{
    stbi_set_flip_vertically_on_load(1);
    int width = 0, height = 0, bpp = 0;

    unsigned char* imagedata = stbi_load(m_fileName.c_str(), &width, &height, &bpp, 0); // вызов функции загрузки
    if (!imagedata) // если неудачно
    {
        printf("Cant load texture from '%s' - %s/n", m_fileName.c_str(), stbi_failure_reason());
    }

    glGenTextures(1, &m_textureObj); // генерируем массив текстур
    glBindTexture(m_textureTarget, m_textureObj); // привязываем массив к текстурному объекту Target
    // привязываем данные к объекту (размер, цвет и саму текстуру)
    glTexImage2D(m_textureTarget, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imagedata);
    // параметры интерполяции текстуры (линейная)
    glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   // glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   // glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   // glTexParameterf(m_textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP);
  //  glTexParameterf(m_textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glBindTexture(m_textureTarget, 0); // отвязываем текстуру 

    stbi_image_free(imagedata); // удаляем указатель
    return true;
}

void Texture::Bind(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit); // разрешаем использование модуля текстур
    glBindTexture(m_textureTarget, m_textureObj); // привязываем объект текструры к модулю
}