// DirectX3DPractice.cpp : ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "DirectX3DPractice.h"
#include <d3d9.h>
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d9.lib")

#define MAX_LOADSTRING 100

// ���� ����:
HINSTANCE hInst;                                // ���� �ν��Ͻ��Դϴ�.
WCHAR szTitle[MAX_LOADSTRING];                  // ���� ǥ���� �ؽ�Ʈ�Դϴ�.
WCHAR szWindowClass[MAX_LOADSTRING];            // �⺻ â Ŭ���� �̸��Դϴ�.

// �� �ڵ� ��⿡ ��� �ִ� �Լ��� ������ �����Դϴ�.

HRESULT InitD3D(HWND hWnd);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

LPDIRECT3D9				g_pD3D = NULL;
LPDIRECT3DDEVICE9		g_pd3dDevice = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL;

/// ����� ������ ������ ����ü
struct CUSTOMVERTEX
{
	FLOAT x, y, z, rhw;	/// ������ ��ȯ�� ��ǥ(rhw ���� ������ ��ȯ�� �Ϸ�� �����̴�.
	DWORD color;		/// ������ ����
};

/// ����� ���� ����ü�� ���� ������ ��Ÿ���� FVF ��
/// ����ü�� X, Y, Z, RHW ���� Diffuse �������� �̷���� ������ �� �� �ִ�.
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

HRESULT InitD3D(HWND hWnd)
{
	///����̽��� �����ϱ� ���� D3D ��ü ����
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	D3DPRESENT_PARAMETERS d3dpp;				///����̽� ������ ���� ����ü
	ZeroMemory(&d3dpp, sizeof(d3dpp));			///�ݵ�� ZeroMemory() �Լ��� �̸� ����ü�� ������ ������ �Ѵ�.
	d3dpp.Windowed = TRUE;						///â���� ����
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;	/// ���� ȿ������ SWAP ȿ��
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;	/// ���� ����ȭ�� ��忡 ���缭 �ĸ� ���� ����

	/// ����̽��� ������ ���� �������� �����Ѵ�.
	/// 1. ����Ʈ ����ī�带 ����Ѵ� (��κ��� ����ī�尡 1����).
	/// 2. HAL ����̽��� �����Ѵ�(HW ������ġ�� ����ϰڴٴ� �ǹ�).
	/// 3. ���� ó���� ��� ī�忡�� �����ϴ� SW ó���� �����Ѵ�(HW�� ������ ��� ���� ���� ������ ����).
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, 
		D3DDEVTYPE_HAL, hWnd, 
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &g_pd3dDevice)))
	{
		return E_FAIL;
	}

	/// ����̽� ���� ������ ó���� ��� ���⿡�� �Ѵ�.
	return S_OK;
}
/**-------------------------------------------------------------------
  *�ʱ�ȭ�� ��ü���� �Ұ��Ѵ�.
  *-------------------------------------------------------------------
  */
VOID Cleanup()
{
	if (g_pd3dDevice != NULL)
		g_pd3dDevice->Release();

	if (g_pD3D != NULL)
		g_pD3D->Release();
}
/**--------------------------------------------------------------------
  * ���� ���۸� �����ϰ� ���� ���� ä�� �ִ´�.
  * ���� ���۶� �⺻������ ���� ������ ���� �ִ� �޸� ����̴�.
  * ���� ���۸� ������ �������� �ݵ�� Lock()�� Unlock()���� �����͸� ����
  * ���� ������ ���� ���ۿ� ��־�� �Ѵ�.
  * ���� D3D�� �ε��� ���۵� ��� �����ϴٴ� ���� �������.
  * ���� ���۳� �ε��� ���۴� �⺻ �ý��� �޸� �ܿ� ����̽� �޸�(����ī�� �޸�)
  * �� ������ �� �ִµ�, ��κ��� ����ī�忡���� �̷��� �� ��� ��û�� �ӵ��� �����
  * ���� �� �ִ�.
  *///-------------------------------------------------------------------
HRESULT InitVB()
{
	///�ﰢ���� �������ϱ� ���� �� ���� ������ ����
	CUSTOMVERTEX vertices[] =
	{
		{150.0f, 50.0f, 0.5f, 1.0f, 0xffff0000, }, // x, y, z, rhw, color
		{250.0f, 250.0f, 0.5f, 1.0f, 0xff00ff00,},
		{50.0f, 250.0f, 0.5f, 1.0f, 0xff00ffff,},
	};

	/// ���� ���� ����
	/// 3���� ����� ������ ������ �޸𸮸� �Ҵ��Ѵ�.
	/// FVF�� �����Ͽ� ������ �������� ������ �����Ѵ�.
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(3 * sizeof(CUSTOMVERTEX),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &g_pVB, NULL)))
	{
		return E_FAIL;
	}

	/// ���� ���۸� ������ ä���.
	/// ���� ������ Lock() �Լ��� ȣ���Ͽ� �����͸� ���´�.
	VOID* pVertices;
	if (FAILED(g_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0)))
		return E_FAIL;
	memcpy(pVertices, vertices, sizeof(vertices));
	g_pVB->Unlock();

	return S_OK;
}
/**--------------------------------------------------------------------
* ȭ�� �׸���
*///-------------------------------------------------------------------
VOID Render()
{
	/// �ĸ� ���۸� �Ķ���(0, 0, 255)���� �����.
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET,
		D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

	/// ������ ����
	if (SUCCEEDED(g_pd3dDevice->BeginScene()))
	{
		/// ���� ������ �ﰢ���� �׸���.
		/// 1. ���� ������ ����ִ� ���� ���۸� ��� ��Ʈ������ �Ҵ��Ѵ�.
		g_pd3dDevice->SetStreamSource(0, g_pVB, 0,
			sizeof(CUSTOMVERTEX));
		/// 2. D3D�� ���� ���̴� ������ �����Ѵ�. ��κ��� ��쿡�� FVF�� �����Ѵ�.
		g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
		/// 3. ���� ������ ����ϱ� ���� DrawPrimitive () �Լ� ȣ��
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);

		/// ������ ����
		g_pd3dDevice->EndScene();
	}

	/// �ĸ� ���۸� ���̴� ȭ������!
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}
/**-------------------------------------------------------------------
*������ ���ν���
*-------------------------------------------------------------------
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
/**-------------------------------------------------------------------
*���α׷� ������
*-------------------------------------------------------------------
*/
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	/// ������ Ŭ���� ���
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		"D3D Tutorial",NULL };
	RegisterClassEx(&wc);

	/// ������ ����
	HWND hWnd = CreateWindow("D3D Tutorial", "D3D Tutorial 01: CreateDevice",
		WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
		GetDesktopWindow(), NULL, wc.hInstance, NULL);

	/// Direct3D �ʱ�ȭ
	if (SUCCEEDED(InitD3D(hWnd)))
	{
		/// ���� ���� �ʱ�ȭ
		if (SUCCEEDED(InitVB()))
		{
			/// ������ ���
			ShowWindow(hWnd, SW_SHOWDEFAULT);
			UpdateWindow(hWnd);

			/// �޽��� ����
			MSG msg;
			ZeroMemory(&msg, SW_SHOWDEFAULT);
			while (msg.message != WM_QUIT)
			{
				///�޽��� ť�� �޽����� ������ �޽��� ó��
				if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					/// ó���� �޽����� ������ Render() �Լ� ȣ��
					Render();
				}
			}
		}
	}

	///��ϵ� Ŭ���� �Ұ�
	UnregisterClass("D3D Tutorial", wc.hInstance);
	return 0;
}

