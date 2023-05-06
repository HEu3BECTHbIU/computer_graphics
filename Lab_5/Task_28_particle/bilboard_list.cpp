#include "util.h"
#include "engine_common.h"
#include "bilboard_list.h"

#define NUM_ROWS 2
#define NUM_COLUMNS 2


BillboardList::BillboardList()
{
    m_pTexture = NULL;
    m_VB = INVALID_OGL_VALUE;
}


BillboardList::~BillboardList()
{
    SAFE_DELETE(m_pTexture);

    if (m_VB != INVALID_OGL_VALUE)
    {
        glDeleteBuffers(1, &m_VB);
    }
}


bool BillboardList::Init(const std::string& TexFilename) // инициализация биллборда
{
    m_pTexture = new Texture(GL_TEXTURE_2D, TexFilename.c_str());

    if (!m_pTexture->Load()) // загрузка текстуры
    {
        return false;
    }

    CreatePositionBuffer();

    if (!m_technique.Init())  // инициаллизация шейдеров
    {
        return false;
    }

    return true;
}


void BillboardList::CreatePositionBuffer() // генерация позиций для объектов
{
    Vector3f Positions[NUM_ROWS * NUM_COLUMNS];

    for (unsigned int j = 0; j < NUM_ROWS; j++)
    {
        for (unsigned int i = 0; i < NUM_COLUMNS; i++) 
        {
            Vector3f Pos((float)i, 0.0f, (float)j);
            Positions[j * NUM_COLUMNS + i] = Pos;
        }
    }

    glGenBuffers(1, &m_VB); // генерация массива
    glBindBuffer(GL_ARRAY_BUFFER, m_VB); // привязка массива
    glBufferData(GL_ARRAY_BUFFER, sizeof(Positions), &Positions[0], GL_STATIC_DRAW); // данные массива
}

void BillboardList::Render(const Matrix4f& VP, const Vector3f& CameraPos)
{
    m_technique.Enable(); // переключение на шейдеры билборда
    m_technique.SetVP(VP);
    m_technique.SetCameraPosition(CameraPos);

    m_pTexture->Bind(COLOR_TEXTURE_UNIT);

    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, m_VB);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);   // position

    glDrawArrays(GL_POINTS, 0, NUM_ROWS * NUM_COLUMNS); // рисует точки, которые будут переделаны в прямоугольник в GS

    glDisableVertexAttribArray(0);
}