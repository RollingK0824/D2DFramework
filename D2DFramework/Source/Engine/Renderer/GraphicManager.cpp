#include "Engine/Core/pch.h"
#include "GraphicManager.h"
#include "Engine/Core/GameApp.h"

bool GraphicManager::Initialize()
{
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pFactory);
	if (FAILED(hr))
	{
		MessageBoxW(nullptr, L"Direct2D Factory 생성 실패", L"Error", MB_ICONERROR);
		return false;
	}

	HWND hWnd = GameApp::GetInstance()->GetWindowHandle();
	if (!hWnd) return false;

	RECT rc;
	GetClientRect(hWnd, &rc);
	UINT width = (rc.right - rc.left) > 0 ? (rc.right - rc.left) : 1;
	UINT height = (rc.bottom - rc.top) > 0 ? (rc.bottom - rc.top) : 1;

	// DX11 스왑체인(SwapChain) 설정
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // D2D 호환을 위한 포맷
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;

	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT; // D2D와 상호 운용하기 위한 플래그

	// DX11 디바이스 및 스왑체인 생성
	hr = D3D11CreateDeviceAndSwapChain(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags,
		nullptr, 0, D3D11_SDK_VERSION, &sd,
		&m_pSwapChain, &m_pD3DDevice, nullptr, &m_pD3DContext
	);
	if (FAILED(hr)) return false;

	// DX11 렌더 타겟 뷰 생성 (DX11 화면 지우기용)
	ID3D11Texture2D* pBackBuffer = nullptr;
	m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	if (FAILED(hr))return false;

	m_pD3DDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);
	if (FAILED(hr))
	{
		pBackBuffer->Release();
		return false;
	}

	// D2D Factory 생성
	// DXGI 백버퍼를 기반으로 D2D 렌더 타겟 생성
	IDXGISurface* pDxgiSurface = nullptr;
	pBackBuffer->QueryInterface(__uuidof(IDXGISurface), (void**)&pDxgiSurface);

	if (FAILED(hr) || pDxgiSurface == nullptr)
	{
		pBackBuffer->Release();
		return false;
	}

	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
	);

	hr = m_pFactory->CreateDxgiSurfaceRenderTarget(pDxgiSurface, &props, &m_pRenderTarget);

	// 사용 완료된 임시 포인터 해제
	pDxgiSurface->Release();
	pBackBuffer->Release();

	if (FAILED(hr)) return false;

	// DWrite 생성
	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&m_pWriteFactory)
	);

	return SUCCEEDED(hr);
}

void GraphicManager::BeginDraw()
{
	if (m_pRenderTarget)
	{
		m_pRenderTarget->BeginDraw();
		m_bIsD2DDrawing = true;
	}
}

void GraphicManager::EndD2DDraw()
{
	if (!m_bIsD2DDrawing) return;
	m_bIsD2DDrawing = false;

	if (m_pRenderTarget)
	{
		// D2D 그리기를 완전히 종료하여 도화지 사용을 마침
		HRESULT hr = m_pRenderTarget->EndDraw();

		// 장치 손실(창 크기 변경 등) 복구
		if (hr == D2DERR_RECREATE_TARGET)
		{
			Release();
			Initialize();
			return;
		}

		// D2D 그리기가 성공적으로 끝난 직후에, 
		// ImGui(DX11)가 도화지 위에 에디터를 그릴 수 있도록 바인딩을 넘김
		if (m_pD3DContext && m_pRenderTargetView)
		{
			HWND hWnd = GameApp::GetInstance()->GetWindowHandle();
			RECT rc;
			GetClientRect(hWnd, &rc);

			D3D11_VIEWPORT vp = {};
			vp.Width = static_cast<float>(rc.right - rc.left);
			vp.Height = static_cast<float>(rc.bottom - rc.top);
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;

			m_pD3DContext->RSSetViewports(1, &vp);
			m_pD3DContext->OMSetRenderTargets(1, &m_pRenderTargetView, nullptr);
		}
	}
}

void GraphicManager::Clear(const D2D1_COLOR_F& color)
{
	if (m_pRenderTarget)
	{
		m_pRenderTarget->Clear(color);
	}
}

void GraphicManager::Release()
{
	if (m_pRenderTarget) { m_pRenderTarget->Release(); m_pRenderTarget = nullptr; }
	if (m_pRenderTargetView) { m_pRenderTargetView->Release(); m_pRenderTargetView = nullptr; }
	if (m_pSwapChain) { m_pSwapChain->Release(); m_pSwapChain = nullptr; }
	if (m_pD3DContext) { m_pD3DContext->Release(); m_pD3DContext = nullptr; }
	if (m_pD3DDevice) { m_pD3DDevice->Release(); m_pD3DDevice = nullptr; }

	if (m_pWriteFactory) { m_pWriteFactory->Release(); m_pWriteFactory = nullptr; }
	if (m_pFactory) { m_pFactory->Release(); m_pFactory = nullptr; }
}

void GraphicManager::PreRender()
{
	if (m_pRenderTarget)
	{
		m_pRenderTarget->BeginDraw();
		m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
		m_bIsD2DDrawing = true;
	}
}

void GraphicManager::PostRender()
{
	EndD2DDraw();

	if (m_pSwapChain)
	{
		// 최종적으로 완성된 화면(D2D 게임 화면 + DX11 ImGui)을 모니터에 송출
		m_pSwapChain->Present(1, 0);

		// 다음 프레임의 PreRender에서 D2D가 다시 도화지를 쓸 수 있도록,
		// DX11이 쥐고 있던 렌더 타겟 바인딩을 강제로 해제(nullptr)
		if (m_pD3DContext)
		{
			ID3D11RenderTargetView* nullRTV = nullptr;
			m_pD3DContext->OMSetRenderTargets(1, &nullRTV, nullptr);
		}
	}
}
