#pragma once
#include <GL/glew.h>

#include "ps_update_technique.h"
#include "random_texture.h"
#include "bilboard_technique.h"
#include "texture.h"

// инкапсулирует всю механику взаимодействия буферов transform feedback
class ParticleSystem
{
public:
    ParticleSystem();

    ~ParticleSystem();

    bool InitParticleSystem(const Vector3f& Pos);

    void Render(int DeltaTimeMillis, const Matrix4f& VP, const Vector3f& CameraPos);

private:

    void UpdateParticles(int DeltaTimeMillis);
    void RenderParticles(const Matrix4f& VP, const Vector3f& CameraPos);

    bool m_isFirst; // индикатор - вызывалась ли уже функция Render(),
    unsigned int m_currVB; // индексы на буферы (один буфер на вход, другой - на выход)
    unsigned int m_currTFB;
    GLuint m_particleBuffer[2]; // указатели на вершинные буферы
    GLuint m_transformFeedback[2]; // указатели на объекты transform feedback
    PSUpdateTechnique m_updateTechnique; // техника обновления
    BillboardTechnique m_billboardTechnique; // техника билборда
    RandomTexture m_randomTexture; 
    Texture* m_pTexture; // текстура частицы
    int m_time; // глобальное время
};
