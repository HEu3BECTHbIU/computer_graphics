#include <stdio.h>
#include <string.h>

#include "technique.h"

Technique::Technique() 
{
    m_shaderProg = 0;
}

Technique::~Technique() // десткуктор
{
    // удаляем шейдеры для оптимизации
    for (ShaderObjList::iterator it = m_shaderObjList.begin(); it != m_shaderObjList.end(); it++)
    {
        glDeleteShader(*it);
    }
    // удаляем шейдерную программу
    if (m_shaderProg != 0)
    {
        glDeleteProgram(m_shaderProg);
        m_shaderProg = 0;
    }
}

bool Technique::Init() // функция создания шейдерной программы
{
    m_shaderProg = glCreateProgram();

    if (m_shaderProg == 0) 
    {
        fprintf(stderr, "Error creating shader program\n");
        return false;
    }

    return true;
}

//Используем этот метод для добавления шейдеров в программу. Когда заканчиваем - вызываем finalize()
bool Technique::AddShader(GLenum ShaderType, const char* pShaderText)
{
    GLuint ShaderObj = glCreateShader(ShaderType); // создание шейдерного объекта

    if (ShaderObj == 0) 
    {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        return false;
    }
    // Сохраним объект шейдера - он будет удален в декструкторе
    m_shaderObjList.push_back(ShaderObj);

    const GLchar* p[1];
    p[0] = pShaderText; // код шейдера (программа)
    GLint Lengths[1];
    Lengths[0] = strlen(pShaderText); // длина программы шейдера
    glShaderSource(ShaderObj, 1, p, Lengths); // привязка данных шейдера к шейдерному объекту

    glCompileShader(ShaderObj); // компиляция шейдера

    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);// проверка компиляции
    if (!success)  // в случае неудачной компиляции (ошибки в коде шейдера)
    {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        return false;
    }
    glAttachShader(m_shaderProg, ShaderObj); //привязка шейдерного объекта к программному

    return true;
}
// После добавления всех шейдеров в программу вызываем эту функцию
// для линковки и проверки программу на ошибки
bool Technique::Finalize() 
{
    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    glLinkProgram(m_shaderProg); // линковка программы (создание исполняемого файла) 

    glGetProgramiv(m_shaderProg, GL_LINK_STATUS, &Success); // проверка успешности линковки
    if (Success == 0) 
    {
        glGetProgramInfoLog(m_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        return false;
    }

    glValidateProgram(m_shaderProg); // проверка возможности запуска программы
    glGetProgramiv(m_shaderProg, GL_VALIDATE_STATUS, &Success);
    if (Success == 0) 
    {
        glGetProgramInfoLog(m_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        return false;
    }
    // Удаляем промежуточные объекты шейдеров, которые были добавлены в программу
    for (ShaderObjList::iterator it = m_shaderObjList.begin(); it != m_shaderObjList.end(); it++)
    {
        glDeleteShader(*it);
    }
    m_shaderObjList.clear();

    return true;
}

void Technique::Enable() 
{
    glUseProgram(m_shaderProg); // назначение шейдерной программы для использования в конвейере
}

GLint Technique::GetUniformLocation(const char* pUniformName)
{
    GLint Location = glGetUniformLocation(m_shaderProg, pUniformName);

    if (Location == 0xFFFFFFFF) 
    {
        fprintf(stderr, "Warning! Unable to get the location of uniform '%s'\n", pUniformName);
    }

    return Location;
}



















