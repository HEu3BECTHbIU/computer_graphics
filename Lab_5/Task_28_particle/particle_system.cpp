#include "engine_common.h"
#include "util.h"
#include "particle_system.h"
#include "math_3d.h"

#define MAX_PARTICLES 1000
#define PARTICLE_LIFETIME 10.0f

#define PARTICLE_TYPE_LAUNCHER 0.0f
#define PARTICLE_TYPE_SHELL 1.0f
#define PARTICLE_TYPE_SECONDARY_SHELL 2.0f

// структура частицы
struct Particle
{
    float Type; // тип
    Vector3f Pos; // позиция
    Vector3f Vel; // скорость (для изменения положения)
    float LifetimeMillis; // жизненный цикл
};


ParticleSystem::ParticleSystem() // инициализация
{
    m_currVB = 0;
    m_currTFB = 1;
    m_isFirst = true;
    m_time = 0;
    m_pTexture = NULL;

    ZERO_MEM(m_transformFeedback);
    ZERO_MEM(m_particleBuffer);
}


ParticleSystem::~ParticleSystem()
{
    SAFE_DELETE(m_pTexture);

    if (m_transformFeedback[0] != 0) 
    {
        glDeleteTransformFeedbacks(2, m_transformFeedback);
    }
    if (m_particleBuffer[0] != 0) 
    {
        glDeleteBuffers(2, m_particleBuffer);
    }
}


bool ParticleSystem::InitParticleSystem(const Vector3f& Pos)
{
    Particle Particles[MAX_PARTICLES];
    ZERO_MEM(Particles);
    // инициализируем первую частицу (пусковую)
    Particles[0].Type = PARTICLE_TYPE_LAUNCHER;
    Particles[0].Pos = Pos;
    Particles[0].Vel = Vector3f(0.0f, 0.0001f, 0.0f);
    Particles[0].LifetimeMillis = 0.0f;

    // создаем 2 объекта tranform feedback
    // отрисовка будет происходить в один, в то время как другой пойдет на вход и наоборот
    glGenTransformFeedbacks(2, m_transformFeedback);
    glGenBuffers(2, m_particleBuffer);

    for (unsigned int i = 0; i < 2; i++) 
    {
        // привязываем соответсвующий объект буфера к метке GL_TRANSFORM_FEEDBACK_BUFFER
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[i]);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_particleBuffer[i]);
        // привязываем тот же самый объект буфера к GL_ARRAY_BUFFER,
        // что сделает его обычным вершинным буфером и загружаем содержимое массива частиц внутрь
        glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Particles), Particles, GL_DYNAMIC_DRAW);
    }
    // инициализиируем технику обновления
    if (!m_updateTechnique.Init()) 
    {
        return false;
    }

    m_updateTechnique.Enable();
    // устанавлливаем время жизни
    m_updateTechnique.SetRandomTextureUnit(RANDOM_TEXTURE_UNIT_INDEX);
    m_updateTechnique.SetLauncherLifetime(100.0f);
    m_updateTechnique.SetShellLifetime(10000.0f);
    m_updateTechnique.SetSecondaryShellLifetime(2500.0f);

    if (!m_randomTexture.InitRandomTexture(1000)) 
    {
        return false;
    }
    m_randomTexture.Bind(RANDOM_TEXTURE_UNIT);

    // инициализируем технику билборда
    if (!m_billboardTechnique.Init()) 
    {
        return false;
    }
    m_billboardTechnique.Enable();
    // устанавливаем модуль текстуры
    m_billboardTechnique.SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    // устанавливем размер
    m_billboardTechnique.SetBillboardSize(0.01f);

    m_pTexture = new Texture(GL_TEXTURE_2D, "../Content/fireworks_red.jpg");

    if (!m_pTexture->Load())
    {
        return false;
    }
    return GLCheckError();
}
// функция рендера
void ParticleSystem::Render(int DeltaTimeMillis, const Matrix4f& VP, const Vector3f& CameraPos)
{
    // увеличиваем время
    m_time += DeltaTimeMillis;
    // обновляем частицы
    UpdateParticles(DeltaTimeMillis);
    // рендерим обновленные частицы
    RenderParticles(VP, CameraPos);
    // меняем текущий буфер и объект tranform feedback
    m_currVB = m_currTFB;
    m_currTFB = (m_currTFB + 1) & 0x1;
}

void ParticleSystem::UpdateParticles(int DeltaTimeMillis)
{
    // включаем соответствующий метод и устанавливаем некоторые динамические переменные
    m_updateTechnique.Enable();
    m_updateTechnique.SetTime(m_time);
    m_updateTechnique.SetDeltaTimeMillis(DeltaTimeMillis);
    // привязываем случайную текстуру (через модуль GL_TEXTURE_3)
    m_randomTexture.Bind(RANDOM_TEXTURE_UNIT);

    // обрезаем примитивы и не допускаем их до растеризации
    glEnable(GL_RASTERIZER_DISCARD);

    //меняем роли 2 буферов, которые мы создали
    glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currVB]);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[m_currTFB]);

    // устанавливаем вершинные атрибуты частиц в вершинный буфер
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), 0);                          // type
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)4);         // position
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)16);        // velocity
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)28);          // lifetime

    // активируем transform feedback
    // Все вызовы отрисовки после него, и до вызова glEndTransformFeedback(), перенаправляют их выход в буфер transform feedback
    glBeginTransformFeedback(GL_POINTS);
    // если частица первая, то рисуем по обычному
    if (m_isFirst) 
    {
        glDrawArrays(GL_POINTS, 0, 1);
        m_isFirst = false;
    }
    // иначе отрисовывыем объекты в tranform feddback ( в нем уже находится необходимый буфер вершин)
    else 
    {
        glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currVB]);
    }

    glEndTransformFeedback();

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
}


void ParticleSystem::RenderParticles(const Matrix4f& VP, const Vector3f& CameraPos)
{
    // разрешаем метод billboarding и назначаем в него параметры.
    // Каждая вершина будет превращена в квадрат и текстура, которую мы привязали, будет на них наложена
    m_billboardTechnique.Enable();
    m_billboardTechnique.SetCameraPosition(CameraPos);
    m_billboardTechnique.SetVP(VP);
    m_pTexture->Bind(COLOR_TEXTURE_UNIT);
    // включаем растеризацию (т.к до этого ее выключили)
    glDisable(GL_RASTERIZER_DISCARD);

    glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currTFB]);

    // Частица в буфере transform feedback имеет 4 атрибута.
    // Для того, что бы рендерить ее нам требуется только позиция, поэтому только она и включена
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)4);  // position
    
    // функция отрисовки
    glDrawTransformFeedback(GL_POINTS, m_transformFeedback[m_currTFB]);

    glDisableVertexAttribArray(0);
}