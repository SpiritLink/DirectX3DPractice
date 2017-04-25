// DirectX3DPractice.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "DirectX3DPractice.h"
#include <mmsystem.h>
#include <d3dx9.h>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d9.lib")

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.

LPDIRECT3D9				g_pD3D = NULL;			/// D3D 디바이스를 생성할 D3D 객체 변수
LPDIRECT3DDEVICE9		g_pd3dDevice = NULL;	/// 렌더링에 사용될 D3D 디바이스
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL;			/// 정점을 보관할 정점 버퍼

/// 사용자 정점을 정의할 구조체
struct CUSTOMVERTEX
{
	FLOAT x, y, z, rhw;	/// 정점의 변환된 좌표(rhw 값이 있으면 변환이 완료된 정점이다.
	DWORD color;		/// 정점의 색깔
};

/// 사용자 정점 구조체에 관한 정보를 나타내는 FVF 값
/// 구조체는 X, Y, Z, RHW 값과 Diffuse 색깔값으로 이루어져 있음을 알 수 있다.
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

/**--------------------------------------------------
  * Driect3D 초기화
  *--------------------------------------------------
  */
HRESULT InitD3D(HWND hWnd)
{
	/// 디바이스를 생성하기 위한 D3D 객체 생성
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	/// 디바이스를 생성할 구조체
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	/// 디바이스 생성
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &g_pd3dDevice)))
	{
		return E_FAIL;
	}
	/// 컬링 기능을 끈다. 삼각형의 앞면, 뒷면을 모두 렌더링한다.
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

/// 정점에 색깔값이 있으므로, 광원 기능을 끈다.
g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

return S_OK;
}
/**--------------------------------------------------
  * 기하 정보 초기화
  *--------------------------------------------------
  */
HRESULT InitGeometry()
{
	/// 삼각형을 렌더링하기 위해 세 개의 정점 선언
	CUSTOMVERTEX g_Vertices[] =
	{
		{-1.0f, -1.0f, 0.0f,	0xffff0000,},
		{1.0f, -1.0f, 0.0f,		0xff0000ff,},
		{0.0f,1.0f,0.0f,		0xffffffff,},
	};

	/// 정점 버퍼 생성
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(3 * sizeof(CUSTOMVERTEX),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &g_pVB, NULL)))
	{
		return E_FAIL;
	}

	/// 정점 버퍼를 값으로 채운다.
	VOID* pVertices;
	if (FAILED(g_pVB->Lock(0, sizeof(g_Vertices),
		(void**)&pVertices, 0)))
		return E_FAIL;
	memcpy(pVertices, g_Vertices, sizeof(g_Vertices));
	g_pVB->Unlock();

	return S_OK;
}
/**--------------------------------------------------
  * 초기화된 객체들 소거
  *--------------------------------------------------
  */
VOID Cleanup()
{
	if (g_pVB != NULL)
		g_pVB->Release();

	if (g_pd3dDevice != NULL)
		g_pd3dDevice->Release();

	if (g_pD3D != NULL)
		g_pD3D->Release();
}
/**--------------------------------------------------
  * 행렬 설정
  * 행렬을 세 개가 있고, 각각 월드, 뷰, 프로젝션 행렬이다.
  *--------------------------------------------------
  */
VOID SetupMatrices()
{
	///월드 행렬
	D3DXMATRIXA16 matWorld;

	UINT iTime = timeGetTime() % 1000;	///float 연산의 정밀도를 위해서 1000으로 나머지 연산한다.
	FLOAT fAngle = iTime * (2.0f * D3DX_PI) / 1000.0f;	/// 1000밀리초마다 한 바퀴씩 (2 * pi) 회전 애니메이션 행렬을 만든다.
	D3DXMatrixRotationY(&matWorld, fAngle);	///Y축을 회전축으로 회전행렬 생성
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);	///생성한 회전 행렬을 월드 행렬로 디바이스에 설정

	/// 뷰 행렬을 정의하기 위해서는 세 가지 값이 필요하다.
	D3DXVECTOR3 vEyePt(0.0f, 3.0f, -5.0f);		/// 1. 눈의 위치(0, 3.0, -5)
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);	/// 2. 눈이 바라보는 위치 (0,0,0);
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);		/// 3. 천정 방향을 나타내는 상방벡터 (0,1,0)
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	/// 1, 2, 3,의 값으로 뷰 행렬 생성
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);	/// 생성한 뷰 행렬을 디바이스에 설정

	/// 프로젝션 행렬을 정의하기 위해서느 시야각(FOV = Field Of View)과 종횡비
	/// (aspect ratio), 클리핑 평면의 값이 필요하다.
	D3DXMATRIXA16 matProj;
	/// matProj : 값이 설정된 행렬
	/// D3DX_PI / 4 : FOV(D3DX_PI / 4 = 45도)
	/// 1.0f : 종횡비
	/// 1.0f : 근접 클리핑 평면(near clipping plane)
	/// 100.0f : 원거리 클리핑 평면(far clipping plane)
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);	///생성한 프로젝션 행렬을 디바이스에 설정
}
/**--------------------------------------------------
  * 화면 그리기
  *--------------------------------------------------
  */
VOID Render()
{
	/// 후면 버퍼를 파란색(0,0,255)으로 지운다.
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET,
		D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

	/// 렌더링 시작
	if (SUCCEEDED(g_pd3dDevice->BeginScene()))
	{
		/// 월드, 뷰, 프로젝션 행렬을 설정한다.
		SetupMatrices();

		/// 정점 버퍼의 내용을 그린다.
		g_pd3dDevice->SetStreamSource(0, g_pVB, 0,
			sizeof(CUSTOMVERTEX));
		g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 1);

		/// 렌더링 종료
		g_pd3dDevice->EndScene();
	}

	/// 후면 버퍼를 보이는 화면으로!
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}
/**--------------------------------------------------
  * 윈도우 프로시저
  *--------------------------------------------------
  */
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		Cleanup();
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
/**--------------------------------------------------
  * 프로그램 시작점
  *--------------------------------------------------
  */
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	/// 윈도우 클래스 등록
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
	GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
	"D3D Tutorial", NULL };
	RegisterClassEx(&wc);

	/// 윈도우 생성
	HWND hWnd = CreateWindow("D3D Tutorial", "D3D Tutorial 03: Matrices",
		WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
		GetDesktopWindow(), NULL, wc.hInstance, NULL);
	
	/// Direct3D 초기화
	if (SUCCEEDED(InitD3D(hWnd)))
	{
		/// 장면에 사용될 기하 정보 초기화
		if (SUCCEEDED(InitGeometry()))
		{
			/// 윈도우 출력
			ShowWindow(hWnd, SW_SHOWDEFAULT);
			UpdateWindow(hWnd);

			/// 메시지 루프
			MSG msg;

			ZeroMemory(&msg, sizeof(msg));
			while (msg.message != WM_QUIT)
			{
				if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					Render();
				}
			}
		}
	}

	UnregisterClass("D3D Tutorial", wc.hInstance);
	return 0;
}
