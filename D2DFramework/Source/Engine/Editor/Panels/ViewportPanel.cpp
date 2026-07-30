#include "Engine/Core/pch.h"
#include "ViewportPanel.h"
#include "Engine/Renderer/GraphicManager.h"

void ViewportPanel::Initialize()
{
    GUISystem::GetInstance()->RegisterPanel(this);
}

void ViewportPanel::Release()
{
    GUISystem::GetInstance()->UnRegisterPanel(this);
}

void ViewportPanel::OnDrawGUI()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImVec2 currentPanelSize = ImGui::GetContentRegionAvail();

    // 뷰포트 패널 크기가 변했을 때 오프스크린 렌더타겟 텍스처 리사이즈
    if ((currentPanelSize.x != m_viewportSize.x || currentPanelSize.y != m_viewportSize.y) &&
        (currentPanelSize.x > 0 && currentPanelSize.y > 0))
    {
        m_viewportSize = currentPanelSize;
        GraphicManager::GetInstance()->ResizeViewportBuffers((UINT)m_viewportSize.x, (UINT)m_viewportSize.y);
    }

    // GraphicManager의 Viewport SRV를 받아 ImGui::Image로 출력
    ImTextureID viewportSRV = (ImTextureID)GraphicManager::GetInstance()->GetViewportSRV();
    if (viewportSRV)
    {
        ImGui::Image(viewportSRV, m_viewportSize);
    }

    m_bIsHovered = ImGui::IsWindowHovered();
    m_bIsFocused = ImGui::IsWindowFocused();

    ImGui::End();
    ImGui::PopStyleVar();
}
