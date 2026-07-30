#pragma once
#include "Engine/Framework/Components/Physics/ColliderComponent.h"

class BoxCollider : public ColliderComponent
{
public:
    CLONEABLE_COMPONENT(BoxCollider)

    BoxCollider(GameObject* owner, TransformComponent* transform);
	virtual ~BoxCollider() = default;

    virtual void Serialize(nlohmann::json& outJson) const override
    {
        ColliderComponent::Serialize(outJson);
        outJson["HalfWidth"] = m_HalfWidth;
        outJson["HalfHeight"] = m_HalfHeight;
    }

    virtual void Deserialize(const nlohmann::json& inJson) override
    {
        ColliderComponent::Deserialize(inJson);
        if (inJson.contains("HalfWidth")) m_HalfWidth = inJson["HalfWidth"].get<float>();
        if (inJson.contains("HalfHeight")) m_HalfHeight = inJson["HalfHeight"].get<float>();
        RebuildShape();
    }

    virtual void OnDrawImGui() override
    {
        ColliderComponent::OnDrawImGui(); // 부모(공통 속성) 먼저 그리기

        bool isChanged = false;
        if (ImGui::DragFloat("Half Width", &m_HalfWidth, 1.0f, 0.1f, 1000.0f)) isChanged = true;
        if (ImGui::DragFloat("Half Height", &m_HalfHeight, 1.0f, 0.1f, 1000.0f)) isChanged = true;

        if (isChanged) RebuildShape();
    }

	void SetSize(float width, float height)
	{
		m_HalfWidth = width * 0.5f;
		m_HalfHeight = height * 0.5f;

		RebuildShape();
	}

    virtual std::string_view GetComponentType() const
    {
        return EngineKey::Component::BoxCollider;
    }

    virtual void DrawDebug() override;

protected:
	virtual b2ShapeId CreateShape(b2BodyId bodyId, const b2ShapeDef* shapeDef)override
	{
		if (m_HalfWidth <= 0.0f || m_HalfHeight <= 0.0f)return b2_nullShapeId;

		b2Polygon box = b2MakeBox(PixelToMeter(m_HalfWidth),PixelToMeter(m_HalfHeight));
		return b2CreatePolygonShape(bodyId, shapeDef, & box);
	}

private:
	float m_HalfWidth = 0.0f;
	float m_HalfHeight = 0.0f;
};
