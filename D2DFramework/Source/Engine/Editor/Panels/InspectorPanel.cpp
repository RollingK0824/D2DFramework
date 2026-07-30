#include "Engine/Core/pch.h"
#include "InspectorPanel.h"
#include "Engine/Editor/EditorSystem.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Base/Component.h"

void InspectorPanel::Initialize()
{
    GUISystem::GetInstance()->RegisterPanel(this);
}

void InspectorPanel::Release()
{
    GUISystem::GetInstance()->UnRegisterPanel(this);
}

void InspectorPanel::OnDrawGUI()
{
    ImGui::Begin("Inspector");

    GameObject* pSelectedObj = EditorSystem::GetInstance()->GetSelectedObject();
    if (pSelectedObj)
    {
        if (pSelectedObj->IsDead())
        {
            EditorSystem::GetInstance()->SetSelectedObject(nullptr);
            ImGui::End();
            return;
        }
        char nameBuffer[256];
        strcpy_s(nameBuffer, pSelectedObj->GetName().c_str());
        if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
        {
            pSelectedObj->SetName(nameBuffer);
        }

        bool isActive = pSelectedObj->IsActive();
        if (ImGui::Checkbox("Is Active", &isActive))
        {
            pSelectedObj->SetActive(isActive);
        }

        ImGui::Separator();

        for (auto* comp : pSelectedObj->GetComponents())
        {
            if (comp && ImGui::CollapsingHeader(comp->GetComponentType().data(), ImGuiTreeNodeFlags_DefaultOpen))
            {
                comp->OnDrawImGui();
            }
        }
    }

    ImGui::End();
}