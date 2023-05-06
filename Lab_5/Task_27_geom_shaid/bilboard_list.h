#pragma once
#include <string>

#include "texture.h"
#include "bilboard_technique.h"
// инкапсулирует все, что вам потребуется для генерации billboards
class BillboardList 
{
public:
    BillboardList();
    ~BillboardList();

    bool Init(const std::string& TexFilename);

    void Render(const Matrix4f& VP, const Vector3f& CameraPos);

private:
    void CreatePositionBuffer();

    GLuint m_VB; // вершинный буфер
    Texture* m_pTexture; // текстура для билборда
    BillboardTechnique m_technique;
};