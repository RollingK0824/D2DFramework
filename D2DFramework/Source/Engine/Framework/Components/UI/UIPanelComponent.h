#pragma once
#include "Engine/Framework/Base/Component.h"

class UIPanelComponent : public Component
{
public:
	CLONEABLE_COMPONENT(UIPanelComponent)
	
	UIPanelComponent(GameObject* owner, TransformComponent* transform);
	virtual ~UIPanelComponent() override = default;

	void AddChildUI(GameObject* pChildUI);

	virtual void OnEnable() override;
	virtual void OnDisable() override;

	virtual std::string_view GetComponentType() const override
	{
		return EngineKey::Component::UIPanelComponent;
	}

private:
	std::vector<GameObject*> m_vChildUIObjects;
};