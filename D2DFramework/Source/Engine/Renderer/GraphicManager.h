#pragma once
#include "Engine/Core/Singleton.h"	
#include "Engine/Framework/Base/ISystem.h"
#include "Engine/Framework/Base/IRenderable.h"

class GraphicManager : public Singleton<GraphicManager>, public ISystem, public IRenderable
{
	friend class Singleton<GraphicManager>;

public:
	virtual bool Initialize() override;
	virtual void Release() override;

	virtual void PreRender() override;
	virtual void PostRender() override;

	ID2D1Factory* GetFactory() const { return m_pFactory; }
	IDWriteFactory* GetWriteFactory() const { return m_pWriteFactory; }

	ID2D1RenderTarget* GetRenderTarget() const { return m_pRenderTarget; }

	float GetScreenWidth() const { return m_pRenderTarget ? 
		m_pRenderTarget->GetSize().width : 0.0f;}

	float GetScreenHeight() const {
		return m_pRenderTarget ?
			m_pRenderTarget->GetSize().height : 0.0f;
	}

	Vector2 GetScreenSize() const 
	{
		return m_pRenderTarget ? 
			Vector2(m_pRenderTarget->GetSize().width, m_pRenderTarget->GetSize().height) 
			: Vector2(0,0);
	}

	// ImGui가 사용할 DX11 장치 Getter
	ID3D11Device* GetD3DDevice() const { return m_pD3DDevice; }
	ID3D11DeviceContext* GetD3DContext() const { return m_pD3DContext; }

	void BeginDraw();
	void EndD2DDraw(); // 명시적으로 D2D 그리기를 종료하는 함수
	void Clear(const D2D1_COLOR_F& color);
private:
	GraphicManager() = default;
	virtual ~GraphicManager() = default;

	// D2D & DWrite 객체
	ID2D1Factory* m_pFactory = nullptr;
	IDWriteFactory* m_pWriteFactory = nullptr;
	ID2D1RenderTarget* m_pRenderTarget = nullptr;

	// DX11 객체들
	ID3D11Device* m_pD3DDevice = nullptr;
	ID3D11DeviceContext* m_pD3DContext = nullptr;
	IDXGISwapChain* m_pSwapChain = nullptr;
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;

	bool m_bIsD2DDrawing = false;
};