#pragma once

#include "stdafx.h"
#include "DxException.h"
#include "GameTimer.h"

//#include <wrl.h> //26�б���
//#include <DirectXMath.h> //25�б���
//#include <DirectXPackedVector.h>
//#include <DirectXColors.h>
//#include <DirectXCollision.h>
//#include <dxgi1_4.h>

//linker
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace std;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

class D3DApp {
protected:

	D3DApp();
	// ά������
	D3DApp(const D3DApp& rhs) = delete;
	D3DApp& operator=(const D3DApp& rhs) = delete;
	virtual ~D3DApp();

public:

	static D3DApp* GetApp();

	int Run(HWND hwnd);// ��Ϣѭ��

	virtual bool Init(HINSTANCE hInstance, int nShowCmd, HWND hwnd);// ��ʼ�����ں�D3D
	virtual LRESULT CALLBACK MsgProc1(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);// ����1����
	virtual LRESULT CALLBACK MsgProc2(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);// ����2����

	//�봰�����
	void SetWindowInformation(int clientWidth, int clientHeight); //��ȡ������ʾλ�������Ϣ
	HWND graphicsWnd1 = 0;// ͼ�δ���1���
	HWND graphicsWnd2 = 0;// ͼ�δ���2���
	int graphicsWindowWidth;
	int graphicsWindowHeight;
	int graphicsPosX;
	int graphicsPosY;
	int createWhichGrapgic = 0; //���ĳ����ť������Ӧͼ��
	bool hasCreateGrapgic = false;
	int allGraphics[50];  //��Ŵ�����ͼ������
	int num = 0;		  //Ŀǰ��ͼ�θ���
	int cursorPosX;
	int cursorPosY;
	int hasPicked = 0; //�Ƿ�ѡ��ͼ��
	
	virtual void BuildGeometry(int type);
	virtual void BuildRenderItems(int num, int type, int whichWindow);
	virtual int Pick(int x, int y, int whichWindow);

	// ��Ϸ/����״̬
	bool      mAppPaused = false;  // is the application paused?
	bool      mMinimized = false;  // is the application minimized?
	bool      mMaximized = false;  // is the application maximized?
	bool      mResizing = false;   // are the resize bars being dragged?
	bool      mFullscreenState = false;// fullscreen enabled

protected:

	bool InitWindow1(HINSTANCE hInstance, int nShowCmd, HWND hwnd);// ��ʼ������1
	bool InitWindow2(HINSTANCE hInstance, int nShowCmd, HWND hwnd);// ��ʼ������2
	bool InitDirect3D();// ��ʼ��D3D

	virtual void Draw() = 0;
	virtual void Update() = 0;

	// ����¼�
	virtual void OnMouseDown(WPARAM btnState, int x, int y) = 0;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) {}
	virtual void OnMouseMove(WPARAM btnState, int x, int y, int whichWindow) {} //whichWindow��1��1���ڣ�2
	// ���ڳߴ�ı��¼�
	virtual void OnResize() = 0;

	void CreateDevice();// �����豸
	void CreateFence();// ����Χ��
	void GetDescriptorSize();// �õ���������С
	void SetMSAA();// ���MSAA����֧��
	void CreateCommandObject();// �����������
	void CreateSwapChain();// ����������
	void CreateDescriptorHeap();// ������������
	void CreateRTV();// ������ȾĿ����ͼ
	void CreateDSV();// �������/ģ����ͼ
	void CreateViewPortAndScissorRect();// ������ͼ�Ͳü��ռ����

	void FlushCmdQueue();// ʵ��Χ��
	void CalculateFrameState();// ����fps��mspf

protected:

	static D3DApp* mApp;

	//ָ��ӿںͱ�������
	ComPtr<ID3D12Device> d3dDevice;
	ComPtr<IDXGIFactory4> dxgiFactory;
	ComPtr<ID3D12Fence> fence;
	ComPtr<ID3D12CommandAllocator> cmdAllocator;
	ComPtr<ID3D12CommandQueue> cmdQueue;
	ComPtr<ID3D12GraphicsCommandList> cmdList;
	ComPtr<ID3D12Resource> depthStencilBuffer1;
	ComPtr<ID3D12Resource> depthStencilBuffer2;
	ComPtr<ID3D12Resource> swapChainBuffer1[2];
	ComPtr<IDXGISwapChain> swapChain1;
	ComPtr<ID3D12Resource> swapChainBuffer2[2];
	ComPtr<IDXGISwapChain> swapChain2;
	ComPtr<ID3D12DescriptorHeap> rtvHeap1;
	ComPtr<ID3D12DescriptorHeap> dsvHeap1;
	ComPtr<ID3D12DescriptorHeap> rtvHeap2;
	ComPtr<ID3D12DescriptorHeap> dsvHeap2;

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};// ��������

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels;// msaad�ȼ�����

	bool m4xMsaaState = false;// �Ƿ���4X MSAA
	UINT m4xMsaaQuality = 0;// 4xMSAA����

	UINT64 mCurrentFence = 0;// ��ǰΧ��ֵ

	D3D12_VIEWPORT viewPort;
	D3D12_RECT scissorRect;

	UINT rtvDescriptorSize = 0;// ��ȾĿ����ͼ��С
	UINT dsvDescriptorSize = 0;// ���/ģ��Ŀ����ͼ��С
	UINT cbv_srv_uavDescriptorSize = 0;// ����������ͼ��С

	_GameTimer::GameTimer gt;// ��Ϸʱ��ʵ��

	UINT mCurrentBackBuffer = 0;

	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	
	

	
};