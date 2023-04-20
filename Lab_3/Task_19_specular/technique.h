#pragma once
#include <GL/glew.h>
#include <list>

class Technique // инкапсуляция добавления и компиляции шейдеров
{
public:
    Technique();
    ~Technique();
    virtual bool Init();
    void Enable();

protected:
    bool AddShader(GLenum ShaderType, const char* pShaderText);
    bool Finalize();
    GLint GetUniformLocation(const char* pUniformName);

private:
    GLuint m_shaderProg; // шейдерная программа
    typedef std::list<GLuint> ShaderObjList; // лист шейдерных объектов
    ShaderObjList m_shaderObjList;
};