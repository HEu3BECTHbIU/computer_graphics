#include <stdio.h>

#include "shadow_map_fbo.h"

ShadowMapFBO::ShadowMapFBO()
{
    m_fbo = 0;
    m_shadowMap = 0;
}

ShadowMapFBO::~ShadowMapFBO()
{
    if (m_fbo != 0)
    {
        glDeleteFramebuffers(1, &m_fbo); // удаление буфера кадра
    }

    if (m_shadowMap != 0) 
    {
        glDeleteFramebuffers(1, &m_shadowMap);
    }
}

bool ShadowMapFBO::Init(unsigned int WindowWidth, unsigned int WindowHeight)
{
    // Создаем FBO
    glGenFramebuffers(1, &m_fbo);

    // Создаем буфер глубины
    glGenTextures(1, &m_shadowMap);
    // создаем текстуру, которая будет служить в качестве карты теней
    glBindTexture(GL_TEXTURE_2D, m_shadowMap);
    // DEPTH_ATTACHMENT - текстура будет получать результат теста глубины
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    // привязываем FBO (draw для рендера в буфер)
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
    // прикрепляем текстуру для карты теней к FBO
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
        m_shadowMap, 0);

    // Отключаем запись в буфер цвета Так как мы не собираемся рендерить в него
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER); // проверка состояния буфера

    if (Status != GL_FRAMEBUFFER_COMPLETE) 
    {
        printf("FB error, status: 0x%x\n", Status);
        return false;
    }

    return true;
}
// привязка FBO (для рендера  в карту теней)
void ShadowMapFBO::BindForWriting()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
}
// функция будет использована перед вторым проходом для привязывания карты теней для чтения
void ShadowMapFBO::BindForReading(GLenum TextureUnit)
{
    // привязываем объект текстуры
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_2D, m_shadowMap);
}