# D2DFramework
C++20 &amp; Direct2D 기반 2D 게임 엔진 및 에디터 프레임워크 (ImGui, Box2D 3.0, WinSock2 연동)
# 🎮 D2DFramework (2D Game Engine & Framework)

![C++20](https://img.shields.io/badge/C++-20-blue.svg)
![Direct2D](https://img.shields.io/badge/DirectX-Direct2D%20%7C%20DirectWrite-0078D4)
![DirectX11](https://img.shields.io/badge/DirectX-DirectX11-green)
![Platform](https://img.shields.io/badge/Platform-Windows%20(WinAPI)-0078D4)

> **C++20과 Direct2D를 기반으로 자체 제작한 2D 게임 엔진 및 에디터 프레임워크입니다.**  
> 컴포넌트 기반 아키텍처, Box2D 물리 연동, ImGui 에디터 통합, WinSock2 네트워크 통신 모듈을 지원합니다.

---

## 🛠️ Tech Stack & External Libraries

### Core & Engine Architecture
- **Language**: C++20
- **Platform API**: Windows API (Win32), WinSock 2.2 (`ws2_32.lib` 네트워크 통신)
- **Graphics & Text Engine**: Direct2D 1.1, DirectWrite, Windows Imaging Component (WIC)
- **ImGui Backend**: DirectX 11 (`d3d11.lib` 기반 디버그/에디터 파이프라인)

### External Libraries
- **[Box2D 3.0](https://box2d.org/)**: 2D 강체 물리 연산 및 충돌 감지 엔진
- **[Dear ImGui](https://github.com/ocornut/imgui)**: 엔진 인스펙터, 개체 계층구조 및 디버그 UI 시스템 (Win32 + DX11)
- **[nlohmann/json](https://github.com/nlohmann/json)**: 씬 데이터, 개체 프리팹 및 엔진 설정 JSON 직렬화/역직렬화

---

## ⭐️ Key Features

1. **EngineKernel & Lifecycle Management**: 씬(Scene) 수명주기, 개체 생성/소멸 관리 및 루프 최적화
2. **Direct2D Rendering Pipeline**: 고성능 2D 비트맵 렌더링, 레이어 렌더링, 알파 블렌딩 및 폰트 렌더링
3. **Physics Integration**: Box2D 3.0 기반의 물리 월드 관리, 리지드바디 및 콜라이더 컴포넌트화
4. **Editor & Debug Tools**: Dear ImGui 기반 실시간 오버레이, 개체 속성 수정(Inspector) 및 성능 프로파일링
5. **Network Ready**: WinSock 2.2 기반 소켓 멀티플레이어 통신 모듈

---

## 💻 Build & Environment

- **IDE**: Visual Studio 2022 이상
- **Platform**: Windows x64
- **Toolset**: v143 (C++20 Language Standard)
