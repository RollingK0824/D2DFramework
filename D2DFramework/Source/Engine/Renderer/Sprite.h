#pragma once
#include <string>
#include <vector>
#include <d2d1.h>

struct Sprite
{
	std::wstring textureKey;
	D2D1_RECT_F srcRect;
	D2D1_POINT_2F pivot;

	D2D1_POINT_2F offset;
	float originalWidth = 0.0f;
	float originalHeight = 0.0f;
};

struct AnimationClip
{
	std::wstring name;
	std::vector<Sprite> frames;
	float frameRate = 0.1f;
	bool bIsLoop = true;
};
