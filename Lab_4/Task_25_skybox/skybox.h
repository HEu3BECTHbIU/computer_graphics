#pragma once
#include "camera.h"
#include "skybox_technique.h"
#include "cubemap_texture.h"
#include "mesh.h"
// Скайбокс - это метод, который визуально увеличивает сцену,
// делает ее более выразительной создав текстуру вокруг зрителя, которая окружает камеру на 360 градусов
class SkyBox
{
public:
    SkyBox(const Camera* pCamera, const PersProjInfo& p);

    ~SkyBox();

    bool Init(const string& Directory,
        const string& PosXFilename,
        const string& NegXFilename,
        const string& PosYFilename,
        const string& NegYFilename,
        const string& PosZFilename,
        const string& NegZFilename);

    void Render();

private:
    SkyboxTechnique* m_pSkyboxTechnique;
    const Camera* m_pCamera;
    CubemapTexture* m_pCubemapTex;
    Mesh* m_pMesh;
    PersProjInfo m_persProjInfo;
};
