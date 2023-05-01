#include "pipeline.h"

const Matrix4f& Pipeline::GetWorldTrans() // матрица мировых преборазований (без камеры, только объект)
{
    Matrix4f ScaleTrans, RotateTrans, TranslationTrans;

    ScaleTrans.InitScaleTransform(m_scale.x, m_scale.y, m_scale.z); // матрица изменения размера
    RotateTrans.InitRotateTransform(m_rotateInfo.x, m_rotateInfo.y, m_rotateInfo.z); // матрица поворота
    TranslationTrans.InitTranslationTransform(m_worldPos.x, m_worldPos.y, m_worldPos.z); // матрица перемещения объекта

    m_WorldTransformation = TranslationTrans * RotateTrans * ScaleTrans;
    return m_WorldTransformation;
}

const Matrix4f& Pipeline::GetWVPTrans() // функция получения матрицы трансформации
{
    GetWorldTrans(); //мировое преобразование
    Matrix4f CameraTranslationTrans, CameraRotateTrans, PersProjTrans;

    // матрица перемещения камеры (перемещение объекта в другую сторону)
    CameraTranslationTrans.InitTranslationTransform(-m_camera.Pos.x, -m_camera.Pos.y, -m_camera.Pos.z);

    CameraRotateTrans.InitCameraTransform(m_camera.Target, m_camera.Up); // матрица поворота камеры
    // матрица проекции объекта
    PersProjTrans.InitPersProjTransform(m_persProj.FOV, m_persProj.Width, m_persProj.Height, m_persProj.zNear, m_persProj.zFar);
    m_transformation = PersProjTrans * CameraRotateTrans * CameraTranslationTrans * m_WorldTransformation;
    return m_transformation;
}
