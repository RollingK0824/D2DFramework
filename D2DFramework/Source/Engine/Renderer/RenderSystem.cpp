#include "Engine/Core/pch.h"
#include "RenderSystem.h"
#include "Engine/Manager/CameraManager.h"
#include "Engine/Renderer/GraphicManager.h"
#include "Engine/Framework/Components/Core/CameraComponent.h"

bool RenderSystem::Initialize()
{
	m_commands.reserve(1000);
	return true;
}

void RenderSystem::SubmitCommand(const RenderCommand& command)
{
	m_commands.push_back(command);
}

void RenderSystem::Release()
{
	m_commands.clear();
	// 임시 vector를 만들어 swap 함수 종료 시 완전 삭제
	std::vector<RenderCommand>().swap(m_commands);
}

void RenderSystem::Render()
{
	if (m_commands.empty())return;

	// zOrder 기준으로 오름차순 정렬
	std::sort(m_commands.begin(), m_commands.end(),
		[](const RenderCommand& a, const RenderCommand& b) {
			return a.zOrder < b.zOrder;
		});

	ID2D1RenderTarget* pRT = GraphicManager::GetInstance()->GetRenderTarget();
	if (!pRT) return;

	// 기본 행렬 백업 연산 전 기본 상태 저장
	D2D1_MATRIX_3X2_F originMatrix;
	pRT->GetTransform(&originMatrix);

	for (const auto& cmd : m_commands)
	{
		if (cmd.type == RenderType::BITMAP)
		{
			DrawBitmap(pRT, cmd);
			continue;
		}

		ID2D1SolidColorBrush* pBrush = nullptr;
		pRT->CreateSolidColorBrush(cmd.color, &pBrush);
		if (!pBrush)continue;

		switch (cmd.type)
		{
		case RenderType::DEBUG_RECT: DrawDebugRect(pRT, cmd, pBrush); break;
		case RenderType::DEBUG_CIRCLE:DrawDebugCircle(pRT, cmd, pBrush); break;
		case RenderType::Debug_LINE:DrawDebugLine(pRT, cmd, pBrush); break;
		case RenderType::Debug_TEXT:DrawDebugText(pRT, cmd, pBrush); break;
		}

		pBrush->Release();
	}

	// RenderTarget 원상 복구 
	pRT->SetTransform(originMatrix);

	m_commands.clear();
}

void RenderSystem::DrawBitmap(ID2D1RenderTarget* pRT, const RenderCommand& cmd)
{
	if (!cmd.pTexture) return;

	float srcWidth = cmd.srcRect.right - cmd.srcRect.left;
	float srcHeight = cmd.srcRect.bottom - cmd.srcRect.top;

	float origW = (cmd.originalWidth > 0.0f) ? cmd.originalWidth : srcWidth;
	float origH = (cmd.originalHeight > 0.0f) ? cmd.originalHeight : srcHeight;

	pRT->SetTransform(CalculateSRTMatrix(cmd, origW, origH));

	D2D1_RECT_F destRect = D2D1::RectF(
		cmd.offset.x,
		cmd.offset.y,
		cmd.offset.x + srcWidth,
		cmd.offset.y + srcHeight);

	// 최종 렌더 타겟에 스프라이트 드로우 명령 하달
	pRT->DrawBitmap(
		cmd.pTexture,
		destRect,
		cmd.opacity, // 알파 브렌딩 투명도 수치 적용
		D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, // 픽셀 보간 모드 설정
		&cmd.srcRect
	);
}

void RenderSystem::DrawDebugRect(ID2D1RenderTarget* pRT, const RenderCommand& cmd, ID2D1SolidColorBrush* pBrush)
{
	float width = cmd.srcRect.right - cmd.srcRect.left;
	float height = cmd.srcRect.bottom - cmd.srcRect.top;

	pRT->SetTransform(CalculateSRTMatrix(cmd, width, height));

	D2D1_RECT_F drawRect = D2D1::RectF(0.0f, 0.0f, width, height);

	if (cmd.isFilled)
		pRT->FillRectangle(drawRect, pBrush);
	else
		pRT->DrawRectangle(drawRect, pBrush);
}

void RenderSystem::DrawDebugCircle(ID2D1RenderTarget* pRT, const RenderCommand& cmd, ID2D1SolidColorBrush* pBrush)
{
	float radius = cmd.srcRect.left;
	float diameter = radius * 2.0f;

	pRT->SetTransform(CalculateSRTMatrix(cmd, diameter, diameter));

	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(0.0f, 0.0f), radius, radius);

	if (cmd.isFilled)
		pRT->FillEllipse(ellipse, pBrush);
	else
		pRT->DrawEllipse(ellipse, pBrush, 2.0f);
}

void RenderSystem::DrawDebugLine(ID2D1RenderTarget* pRT, const RenderCommand& cmd, ID2D1SolidColorBrush* pBrush)
{
	D2D1_MATRIX_3X2_F viewMatrix = D2D1::Matrix3x2F::Identity();

	if (!cmd.isUI)
	{
		CameraComponent* pMainCamera = CameraManager::GetInstance()->GetMainCamera();
		if (pMainCamera) viewMatrix = pMainCamera->GetViewMatrix();
	}

	pRT->SetTransform(viewMatrix);

	D2D1_POINT_2F startPoint = D2D1::Point2F(cmd.position.x, cmd.position.y);
	D2D1_POINT_2F endPoint = D2D1::Point2F(cmd.srcRect.left, cmd.srcRect.top);

	pRT->DrawLine(startPoint, endPoint, pBrush, cmd.scaleX);
}

void RenderSystem::DrawDebugText(ID2D1RenderTarget* pRT, const RenderCommand& cmd, ID2D1SolidColorBrush* pBrush)
{
	IDWriteFactory* pWriteFactory = GraphicManager::GetInstance()->GetWriteFactory();
	if (!pWriteFactory) return;

	IDWriteTextFormat* pTextFormat = nullptr;
	HRESULT hr = pWriteFactory->CreateTextFormat(
		L"Consolas",                // 폰트 종류
		nullptr,                    // 폰트 컬렉션 (기본값)
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		cmd.scaleX,               // 폰트 크기
		L"ko-KR",                   // 지역 레이아웃
		&pTextFormat
	);

	if (SUCCEEDED(hr) && pTextFormat)
	{
		D2D1_MATRIX_3X2_F transformMatrix = D2D1::Matrix3x2F::Translation(cmd.position.x, cmd.position.y);

		if (!cmd.isUI)
		{
			CameraComponent* pMainCamera = CameraManager::GetInstance()->GetMainCamera();
			if (pMainCamera)
			{
				transformMatrix = transformMatrix * pMainCamera->GetViewMatrix();
			}
		}
		pRT->SetTransform(transformMatrix);
		D2D1_RECT_F layoutRect = D2D1::RectF(0.0f, 0.0f, 800.0f, 300.0f);
		pRT->DrawTextW(
			cmd.text.c_str(),
			static_cast<UINT32>(cmd.text.length()),
			pTextFormat,
			layoutRect,
			pBrush
		);
		pTextFormat->Release();
	}
}

D2D1_MATRIX_3X2_F RenderSystem::CalculateSRTMatrix(const RenderCommand& cmd, float width, float height)
{
	float scaleX = cmd.flipX ? -cmd.scaleX : cmd.scaleX;
	float scaleY = cmd.flipY ? -cmd.scaleY : cmd.scaleY;

	D2D1_POINT_2F localCenter = D2D1::Point2F(width * cmd.pivot.x, height * cmd.pivot.y);

	D2D1_MATRIX_3X2_F worldMatrix =
		D2D1::Matrix3x2F::Scale(scaleX, scaleY, localCenter) *
		D2D1::Matrix3x2F::Rotation(cmd.rotation, localCenter) *
		D2D1::Matrix3x2F::Translation(cmd.position.x - localCenter.x, cmd.position.y - localCenter.y);

	if (cmd.isUI)
	{
		return worldMatrix;
	}

	CameraComponent* pMainCamera = CameraManager::GetInstance()->GetMainCamera();
	if (pMainCamera)
	{
		D2D1_MATRIX_3X2_F viewMatrix = pMainCamera->GetViewMatrix();

		return worldMatrix * viewMatrix;
	}

	return worldMatrix;
}