#pragma once
#include <string>
#include <GL/glew.h>

using namespace std;
// Этот класс включает в себя реализацию кубической текстуры
// и предоставляет простой интерфейс для ее загрузки и использования.
class CubemapTexture
{
public:
    // директория и 6 имен файлов для сторон
    CubemapTexture(const string& Directory,
        const string& PosXFilename,
        const string& NegXFilename,
        const string& PosYFilename,
        const string& NegYFilename,
        const string& PosZFilename,
        const string& NegZFilename);

    ~CubemapTexture();

    bool Load();

    void Bind(GLenum TextureUnit);

private:

    string m_fileNames[6]; // имена файлов
    GLuint m_textureObj; // указатель на объект текстуры для доступа ко всем граням
};