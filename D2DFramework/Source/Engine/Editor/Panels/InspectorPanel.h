#pragma once
#include "Engine/Manager/GUISystem.h"

class InspectorPanel : public IGUIPanel
{
public:
    InspectorPanel() = default;
    virtual ~InspectorPanel() = default;

    void Initialize();
    void Release();

    virtual void OnDrawGUI() override;
};