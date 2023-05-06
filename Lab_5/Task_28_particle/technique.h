#pragma once
#include <GL/glew.h>
#include <list>

#define INVALID_UNIFORM_LOCATION 0xFFFFFFFF
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

    GLint GetProgramParam(GLint param);

    GLuint m_shaderProg; // шейдерная программа

private:
    typedef std::list<GLuint> ShaderObjList; // лист шейдерных объектов
    ShaderObjList m_shaderObjList;
};