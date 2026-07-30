#include "Engine/Core/pch.h"
#include "ResourceManager.h"
#include "Engine/Core/Define.h"
#include "Engine/Renderer/GraphicManager.h"

#pragma comment(lib, "windowscodecs.lib")

bool ResourceManager::Initialize()
{
	// WIC Imaging Factory 생성
	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&m_pWICFactory)
	);

	if (FAILED(hr))
	{
		MessageBoxW(nullptr, L"WIC Factory 생성 실패", L"Error", MB_ICONERROR);
		return false;
	}
	return true;
}

ID2D1Bitmap* ResourceManager::LoadTexture(const std::wstring& key, const std::wstring& filePath)
{
	// 중복 로드 방지 : 이미 Pool에 존재하는 Key인지 확인
	auto iter = m_texturePool.find(key);
	if (iter != m_texturePool.end())	// 이미 존재하는 Key라면
	{
		return iter->second; // 이미 로드된 텍스처 반환
	}

	if (!m_pWICFactory) return nullptr;

	// RenderTarget GraphicManager에서 가져오기
	ID2D1RenderTarget* pRenderTarget = GraphicManager::GetInstance()->GetRenderTarget();
	if (!pRenderTarget) return nullptr;

	// WIC 디코딩 파이프라인 구성
	IWICBitmapDecoder* pDecoder = nullptr;
	IWICBitmapFrameDecode* pSource = nullptr;
	IWICFormatConverter* pConverter = nullptr;
	ID2D1Bitmap* pBitmap = nullptr;

	// 하드디스크에서 파일 열람
	HRESULT hr = m_pWICFactory->CreateDecoderFromFilename(
		filePath.c_str(),
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
	);
	if (FAILED(hr))return nullptr;

	// 이미지의 첫 번째 프레임 데이터 추출
	hr = pDecoder->GetFrame(0, &pSource);
	if (FAILED(hr)) { pDecoder->Release(); return nullptr; }

	// RGBA32 포맷으로 변환
	hr = m_pWICFactory->CreateFormatConverter(&pConverter);
	if (FAILED(hr)) { pSource->Release(); pDecoder->Release(); return nullptr; }

	hr = pConverter->Initialize(
		pSource,
		GUID_WICPixelFormat32bppPBGRA,	// 알파 채널(투명도) 포함 RGBA 포맷
		WICBitmapDitherTypeNone,
		nullptr,
		0.0,
		WICBitmapPaletteTypeCustom
	);

	if (SUCCEEDED(hr))
	{
		// Direct2D Bitmap으로 변환하여 GPU에 업로드
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			nullptr,
			&pBitmap
		);
	}

	if (pConverter) pConverter->Release();
	if (pSource) pSource->Release();
	if (pDecoder) pDecoder->Release();

	if (SUCCEEDED(hr) && pBitmap)
	{
		// 텍스처 풀에 등록
		m_texturePool[key] = pBitmap;
		return pBitmap;
	}

	return nullptr;
}

ID2D1Bitmap* ResourceManager::GetTexture(const std::wstring& key) const
{
	auto iter = m_texturePool.find(key);
	if (iter != m_texturePool.end())
	{
		return iter->second;
	}
	return nullptr;
}

bool ResourceManager::LoadResourcesFromJson(const std::string& filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open()) return false;

	json rootJson;
	file >> rootJson;
	file.close();

	if (rootJson.contains(EngineKey::Document::Textures.data()))
	{
		for (const auto& texData : rootJson[EngineKey::Document::Textures.data()])
		{
			std::string strKey = texData["Key"].get<std::string>();
			std::wstring wKey(strKey.begin(), strKey.end());

			// 아틀라스 JSON 경로가 있는 경우
			if (texData.contains("AtlasPath"))
			{
				std::string atlasPath = texData["AtlasPath"].get<std::string>();

				this->LoadSpriteAtlas(atlasPath, wKey);
			}
			// 일반 텍스처인 경우 단일 이미지 로드
			else if (texData.contains("Path"))
			{
				std::string strPath = texData["Path"].get<std::string>();
				std::wstring wPath(strPath.begin(), strPath.end());

				this->LoadTexture(wKey, wPath);
			}
		}
	}
	return true;
}

bool ResourceManager::LoadSpriteAtlas(const std::string& jsonPath, const std::wstring& textureKey)
{
	std::ifstream file(jsonPath);
	if (!file.is_open())return false;

	json atlasJson;
	file >> atlasJson;
	file.close();

	std::string textureName = atlasJson["meta"]["image"].get<std::string>();
	std::wstring wTextureKey(textureName.begin(), textureName.end());

	std::string fullPath = "Resources/Texture/" + textureName;
	std::wstring wFilePath(fullPath.begin(), fullPath.end());

	if (!LoadTexture(wTextureKey, wFilePath))return false;

	std::unordered_map<std::string, Sprite>tempSprites;
	for (const auto& f : atlasJson["frames"])
	{
		std::string filename = f["filename"].get<std::string>();

		Sprite sp;
		sp.textureKey = wTextureKey;

		float x = f["frame"]["x"].get<float>();
		float y = f["frame"]["y"].get<float>();
		float w = f["frame"]["w"].get<float>();
		float h = f["frame"]["h"].get<float>();
		sp.srcRect = D2D1::RectF(x, y, x + w, y + h);

		sp.offset.x = f["spriteSourceSize"]["x"].get<float>();
		sp.offset.y = f["spriteSourceSize"]["y"].get<float>();

		sp.originalWidth = f["sourceSize"]["w"].get<float>();
		sp.originalHeight = f["sourceSize"]["h"].get<float>();

		sp.pivot = D2D1::Point2F(0.5f, 0.5f);

		tempSprites[filename] = sp;
	}

	if (atlasJson["meta"].contains("custom_clips"))
	{
		for (const auto& clipData : atlasJson["meta"]["custom_clips"])
		{
			AnimationClip clip;
			std::string clipName = clipData["name"].get<std::string>();
			clip.name = std::wstring(clipName.begin(), clipName.end());
			clip.frameRate = clipData["frameRate"].get<float>();
			clip.bIsLoop = clipData["isLoop"].get<bool>();

			for (const auto& fNameJson : clipData["frames"])
			{
				std::string fName = fNameJson.get<std::string>();

				if (tempSprites.find(fName) != tempSprites.end())
				{
					clip.frames.push_back(tempSprites[fName]);
				}
			}
			m_MapClips[clip.name] = clip;
		}
	}

	return true;
}

const AnimationClip* ResourceManager::GetAnimationClip(const std::wstring& clipKey) const
{
	auto it = m_MapClips.find(clipKey);
	if (it != m_MapClips.end())return &(it->second);
	return nullptr;
}

void ResourceManager::Release()
{
	// 텍스처 풀에 등록된 모든 비트맵 리소스 해제
	for (auto& pair : m_texturePool)
	{
		if (pair.second)
		{
			pair.second->Release();
			pair.second = nullptr;
		}
	}

	m_texturePool.clear();

	// WIC Factory 해제
	if (m_pWICFactory)
	{
		m_pWICFactory->Release();
		m_pWICFactory = nullptr;
	}
}