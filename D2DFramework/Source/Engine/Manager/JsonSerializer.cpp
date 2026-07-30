#include "Engine/Core/pch.h"
#include "JsonSerializer.h"
#include "Engine/Framework/Scene.h"
#include "Engine/Framework/GameObject.h"
#include "Engine/Framework/Base/Component.h"
#include "Engine/Framework/Components/Core/TransformComponent.h"

bool JsonSerializer::SaveScene(Scene* pScene, const std::string& filePath)
{
	if (pScene == nullptr)return false;

	nlohmann::json sceneJson;
	sceneJson[EngineKey::Document::SceneName.data()] = pScene->GetSceneName();
	sceneJson[EngineKey::Document::GameObjects.data()] = std::vector<nlohmann::json>();

	const auto& gameObjects = pScene->GetGameObjects();
	for (auto* obj : gameObjects)
	{
		if (obj == nullptr || obj->IsDead())continue;

		sceneJson[EngineKey::Document::GameObjects.data()].push_back(SerializeGameObject(obj));
	}

	std::ofstream file(filePath);
	if (!file.is_open())
	{
		std::cout << "파일을 저장할 수 없습니다. ->" << filePath << std::endl;
		return false;
	}

	file << sceneJson.dump(4);
	file.close();
	return true;
}

bool JsonSerializer::LoadScene(Scene* pScene, nlohmann::json& sceneJson)
{
	if (pScene == nullptr) return false;

	if (sceneJson.contains(EngineKey::Document::GameObjects.data()))
	{
		for (const auto& objJson : sceneJson[EngineKey::Document::GameObjects.data()])
		{
			GameObject* newObj = pScene->CreateGameObject();

			ApplyJsonToGameObject(newObj, objJson);
		}
	}
	return true;
}


bool JsonSerializer::SavePrefab(GameObject* pObj, const std::string& saveDirectory)
{
	if (pObj == nullptr) return false;

	nlohmann::json prefabJson = SerializeGameObject(pObj);

	std::filesystem::path dirPath = saveDirectory;
	std::filesystem::path finalPath = dirPath / (pObj->GetName() + ".prefab");

	if (!std::filesystem::exists(dirPath))
	{
		std::filesystem::create_directories(dirPath);
	}

	std::ofstream file(finalPath);
	if (!file.is_open()) return false;

	file << prefabJson.dump(4);
	file.close();
	return true;
}

GameObject* JsonSerializer::InstantiateFromPrefabData(Scene* pScene, const nlohmann::json& prefabJson)
{
	if (pScene == nullptr || prefabJson.empty()) return nullptr;

	GameObject* cloneObj = pScene->CreateGameObject();

	ApplyJsonToGameObject(cloneObj, prefabJson);

	return cloneObj;
}

nlohmann::json JsonSerializer::SerializeGameObject(GameObject* pObj)
{
	nlohmann::json objJson;
	objJson[EngineKey::Property::Name.data()] = pObj->GetName();
	objJson[EngineKey::Property::IsActive.data()] = pObj->IsActive();
	objJson[EngineKey::Property::Components.data()] = std::vector<nlohmann::json>();

	nlohmann::json transformJson;
	std::string trName = pObj->transform.GetComponentType().data();
	if (trName.empty())trName = EngineKey::Component::Trnasform.data();

	transformJson[EngineKey::Property::Type.data()] = trName;

	nlohmann::json transformData;
	pObj->transform.Serialize(transformData);
	transformJson[EngineKey::Property::Data.data()] = transformData;

	objJson[EngineKey::Property::Components.data()].push_back(transformJson);

	const auto& components = pObj->GetComponents();
	for (auto* comp : components)
	{
		if (comp == nullptr)continue;

		nlohmann::json compJson;
		compJson[EngineKey::Property::Type.data()] = comp->GetComponentType();

		nlohmann::json compData;
		comp->Serialize(compData);
		compJson[EngineKey::Property::Data.data()] = compData;

		objJson[EngineKey::Property::Components.data()].push_back(compJson);
	}
	return objJson;
}

void JsonSerializer::ApplyJsonToGameObject(GameObject* pObj, const nlohmann::json& objJson)
{
	if (pObj == nullptr || objJson.empty()) return;

	if (objJson.contains(EngineKey::Property::IsActive.data()))
	{
		pObj->SetActive(objJson[EngineKey::Property::IsActive.data()].get<bool>());
	}
	if (objJson.contains(EngineKey::Property::Name.data()))
	{
		pObj->SetName(objJson[EngineKey::Property::Name.data()].get<std::string>());
	}

	if (objJson.contains(EngineKey::Property::Components.data()))
	{
		for (const auto& compJson : objJson[EngineKey::Property::Components.data()])
		{
			std::string type = compJson[EngineKey::Property::Type.data()].get<std::string>();
			nlohmann::json data = compJson[EngineKey::Property::Data.data()];

			if (type == EngineKey::Component::Trnasform.data())
			{
				pObj->transform.Deserialize(data);
				continue;
			}

			auto it = GetComponentFactory().find(type);
			if (it != GetComponentFactory().end())
			{
				Component* newComp = it->second(pObj);
				if (newComp)
				{
					newComp->Deserialize(data);
				}
			}
		}
	}
}

void JsonSerializer::RegisterComponentFactory(const std::string& typeName, std::function<Component* (GameObject*)> factory)
{
	GetComponentFactory()[typeName] = factory;
}

std::unordered_map<std::string, std::function<Component* (GameObject*)>>& JsonSerializer::GetComponentFactory()
{
	static std::unordered_map<std::string, std::function<Component* (GameObject*)>> factory;
	return factory;
}
