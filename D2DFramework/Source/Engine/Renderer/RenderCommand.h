#pragma once
#include "Engine/Core/Define.h"

enum class RenderType
{
	BITMAP,
	DEBUG_RECT,
	DEBUG_CIRCLE,
	Debug_LINE,
	Debug_TEXT
};

struct RenderCommand
{
	RenderType type = RenderType::BITMAP;

	std::wstring textureKey = L"";					// 그릴 이미지 키값
	ID2D1Bitmap* pTexture = nullptr;				// 그릴 이미지 포인터

	D2D1_RECT_F srcRect = { 0.0f,0.0f,0.0f,0.0f };	// 스프라이트 렌더링 기준 영역
	Vector2 position;								// 월드 좌표계상의 위치
	float rotation = 0.0f;							// 회전 값
	float scaleX = 1.0f;							// X축 크기 배율
	float scaleY = 1.0f;							// Y축 크기 배율

	D2D1_POINT_2F offset = { 0.0f, 0.0f };
	float originalWidth = 0.0f;
	float originalHeight = 0.0f;
	D2D1_POINT_2F pivot = { 0.5f, 0.5f };

	int zOrder = 0;			// Sorting Order (낮을수록 먼저 높을수록 나중)
	float opacity = 1.0f;	// 투명도 등 추가 렌더링 옵션

	// 상하 좌우 반전 플래그
	bool flipX = false;					 
	bool flipY = false;

	D2D1::ColorF color = D2D1::ColorF::White;
	bool isFilled = true;

	std::wstring text = L"";

	bool isUI = false;
};