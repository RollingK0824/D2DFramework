#pragma once
#include "Engine/Core/Define.h"
#include <string_view>

class GameObject;
class ColliderComponent;
class TransformComponent;

enum class PropType { Int, Float, Bool, String };

struct ExposedProperty
{
	std::string name;
	PropType type;
	void* data;
};

#define CLONEABLE_COMPONENT(Type) \
	virtual Component* Clone() override { return new Type(*this); }

class Component
{
public:
	Component(GameObject* owner, TransformComponent* transform)
		: m_pOwnerGameObject(owner)
		, m_pTransform(transform)
		, m_bIsEnabled(true)
	{
	}

	Component(GameObject* owner)
		:m_pOwnerGameObject(owner)
		,m_pTransform(reinterpret_cast<TransformComponent*>(this))
		,m_bIsEnabled(true)
	{
	}

	virtual ~Component() = default;

	virtual Component* Clone() = 0;

	virtual void OnEnable() {}
	virtual void OnDisable() {}

public:
	__declspec(property(get = GetGameObjectInternal)) GameObject& gameObject;
	__declspec(property(get = GetTransformInternal)) TransformComponent& transform;

public:
	void BindToNewObject(GameObject* newOwner, TransformComponent* newTransform)
	{
		m_pOwnerGameObject = newOwner;
		m_pTransform = newTransform;
	}

	void SetDestroyCallback(std::function<void(Component*)> callback)
	{
		m_OnDestroyCallback = callback;
	}

	virtual void OnDestroy()
	{
		if (m_OnDestroyCallback)
		{
			m_OnDestroyCallback(this);
			m_OnDestroyCallback = nullptr;
		}
	}

	void ExposeVariable(const std::string& name, int* var) { m_vProperties.push_back({ name,PropType::Int,var }); }
	void ExposeVariable(const std::string& name, float* var) { m_vProperties.push_back({ name,PropType::Float,var }); }
	void ExposeVariable(const std::string& name, bool* var) { m_vProperties.push_back({ name,PropType::Bool,var }); }
	void ExposeVariable(const std::string& name, std::string* var) { m_vProperties.push_back({ name,PropType::String,var }); }

	virtual void OnDrawImGui()
	{
		int idCounter = 0;

		for (auto& prop : m_vProperties)
		{
			ImGui::PushID(idCounter++);

			switch (prop.type)
			{
			case PropType::Int:
				ImGui::DragInt(prop.name.c_str(), static_cast<int*>(prop.data), 1);
				break;

			case PropType::Float:
				ImGui::DragFloat(prop.name.c_str(), static_cast<float*>(prop.data), 0.1f);
				break;

			case PropType::Bool:
				ImGui::Checkbox(prop.name.c_str(), static_cast<bool*>(prop.data));
				break;

			case PropType::String:
			{
				std::string* pStr = static_cast<std::string*>(prop.data);
				char buffer[256];
				strcpy_s(buffer, pStr->c_str());

				if (ImGui::InputText(prop.name.c_str(), buffer, sizeof(buffer)))
				{
					*pStr = buffer;
				}
				break;
			}
			}

			ImGui::PopID();
		}
	}

	virtual void Awake() {};
	virtual void Start() {};

	void SetEnabled(bool enabled) 
	{ 
		if (m_bIsEnabled == enabled) return;
		m_bIsEnabled = enabled; 

		if (m_bIsEnabled)OnEnable();
		else OnDisable();
	}

	bool IsEnabled() const { return m_bIsEnabled; }

	virtual void OnCollision(ColliderComponent* pOtherCollider) {};

	void SetSceneVectorIndex(size_t index) { m_sceneVectorIndex = index; }
	size_t GetSceneVectorIndex()const { return m_sceneVectorIndex; }

	virtual std::string_view GetComponentType() const = 0;

	virtual void Serialize(nlohmann::json& outJson) const
	{
		outJson[EngineKey::Property::IsEnabled.data()] = m_bIsEnabled;
	}
	virtual void Deserialize(const nlohmann::json& inJson)
	{
		if (inJson.contains(EngineKey::Property::IsEnabled.data()))
		{
			m_bIsEnabled = inJson[EngineKey::Property::IsEnabled.data()].get<bool>();
		}
	}

public:
	GameObject& GetGameObjectInternal() const
	{
		assert(m_pOwnerGameObject != nullptr && "NullReference: GameObject is missing!");
		return *m_pOwnerGameObject;
	}
	TransformComponent& GetTransformInternal() const
	{
		assert(m_pTransform != nullptr && "NullReference: Transform is missing!");
		return *m_pTransform;
	}

protected:
	bool m_bIsEnabled = true;

private:
	GameObject* m_pOwnerGameObject = nullptr;
	TransformComponent* m_pTransform = nullptr;

private:
	size_t m_sceneVectorIndex = 0;
	std::vector<ExposedProperty> m_vProperties;

	std::function<void(Component*)> m_OnDestroyCallback = nullptr;
};
