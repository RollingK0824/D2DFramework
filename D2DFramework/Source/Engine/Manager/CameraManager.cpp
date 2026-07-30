#include "Engine/Core/pch.h"
#include "CameraManager.h"
#include "Engine/Core/EngineKernel.h"
#include "Engine/Framework/Components/Core/CameraComponent.h"

D2D1_MATRIX_3X2_F CameraManager::GetActiveViewMatrix() const
{
    // Play 모드이고 씬에 게임 카메라가 존재하면 게임 카메라의 View Matrix 사용
    if (EngineKernel::GetInstance()->GetPlayState() == EnginePlayState::Play && m_pMainCamera)
    {
        return m_pMainCamera->GetViewMatrix();
    }

    // Edit 모드일 때는 에디터 독립 카메라 View Matrix 연산
    D2D1_POINT_2F center = { GWinSizeX * 0.5f, GWinSizeY * 0.5f };

    return D2D1::Matrix3x2F::Translation(-m_editorCamPos.x, -m_editorCamPos.y) *
        D2D1::Matrix3x2F::Scale(m_editorCamZoom, m_editorCamZoom, center) *
        D2D1::Matrix3x2F::Translation(center.x, center.y);
}

D2D1_POINT_2F CameraManager::ScreenToWorld(D2D1_POINT_2F screenPoint) const
{
    if (EngineKernel::GetInstance()->GetPlayState() == EnginePlayState::Play && m_pMainCamera)
    {
        return m_pMainCamera->ScreenToWorldPoint(screenPoint);
    }

    // Edit 모드 화면 -> 월드 좌표 변환
    D2D1_MATRIX_3X2_F viewMat = GetActiveViewMatrix();
    D2D1_MATRIX_3X2_F invViewMat = viewMat;
    if (D2D1InvertMatrix(&invViewMat))
    {
        return D2D1::Matrix3x2F::ReinterpretBaseType(&invViewMat)->TransformPoint(screenPoint);
    }
    return screenPoint;
}

void CameraManager::PanEditorCamera(Vector2 delta)
{
    // 줌 배율을 고려한 이동 델타 처리
    m_editorCamPos.x -= (delta.x / m_editorCamZoom);
    m_editorCamPos.y -= (delta.y / m_editorCamZoom);
}

void CameraManager::ZoomEditorCamera(float zoomDelta)
{
    m_editorCamZoom += zoomDelta;
    if (m_editorCamZoom < 0.1f) m_editorCamZoom = 0.1f;
    if (m_editorCamZoom > 5.0f) m_editorCamZoom = 5.0f;
}

void CameraManager::ResetEditorCamera()
{
    m_editorCamPos = { 0.0f, 0.0f };
    m_editorCamZoom = 1.0f;
}