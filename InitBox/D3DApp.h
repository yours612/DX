#pragma once

#include "stdafx.h"
#include "DxException.h"
#include "GameTimer.h"

//#include <wrl.h> //26行必须
//#include <DirectXMath.h> //25行必须
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
	// 维护单例
	D3DApp(const D3DApp& rhs) = delete;
	D3DApp& operator=(const D3DApp& rhs) = delete;
	virtual ~D3DApp();

public:

	static D3DApp* GetApp();

	int Run(HWND hwnd);// 消息循环

	virtual bool Init(HINSTANCE hInstance, int nShowCmd, HWND hwnd);// 初始化窗口和D3D
	virtual LRESULT CALLBACK MsgProc1(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);// 窗口1过程
	virtual LRESULT CALLBACK MsgProc2(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);// 窗口2过程

	//与窗口相关
	void SetWindowInformation(int clientWidth, int clientHeight); //获取窗口显示位置相关信息
	HWND graphicsWnd1 = 0;// 图形窗口1句柄
	HWND graphicsWnd2 = 0;// 图形窗口2句柄
	int graphicsWindowWidth;
	int graphicsWindowHeight;
	int graphicsPosX;
	int graphicsPosY;
	int createWhichGrapgic = 0; //点击某个按钮创建对应图形
	bool hasCreateGrapgic = false;
	int allGraphics[50];  //存放创建的图形类型
	int num = 0;		  //目前的图形个数
	int cursorPosX;
	int cursorPosY;
	int hasPicked = 0; //是否选中图形
	
	virtual void BuildGeometry(int type);
	virtual void BuildRenderItems(int num, int type, int whichWindow);
	virtual int Pick(int x, int y, int whichWindow);

	// 游戏/窗口状态
	bool      mAppPaused = false;  // is the application paused?
	bool      mMinimized = false;  // is the application minimized?
	bool      mMaximized = false;  // is the application maximized?
	bool      mResizing = false;   // are the resize bars being dragged?
	bool      mFullscreenState = false;// fullscreen enabled

protected:

	bool InitWindow1(HINSTANCE hInstance, int nShowCmd, HWND hwnd);// 初始化窗口1
	bool InitWindow2(HINSTANCE hInstance, int nShowCmd, HWND hwnd);// 初始化窗口2
	bool InitDirect3D();// 初始化D3D

	virtual void Draw() = 0;
	virtual void Update() = 0;

	// 鼠标事件
	virtual void OnMouseDown(WPARAM btnState, int x, int y) = 0;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) {}
	virtual void OnMouseMove(WPARAM btnState, int x, int y, int whichWindow) {} //whichWindow：1是1窗口，2
	// 窗口尺寸改变事件
	virtual void OnResize() = 0;

	void CreateDevice();// 创建设备
	void CreateFence();// 创建围栏
	void GetDescriptorSize();// 得到描述符大小
	void SetMSAA();// 检测MSAA质量支持
	void CreateCommandObject();// 创建命令队列
	void CreateSwapChain();// 创建交换链
	void CreateDescriptorHeap();// 创建描述符堆
	void CreateRTV();// 创建渲染目标视图
	void CreateDSV();// 创建深度/模板视图
	void CreateViewPortAndScissorRect();// 创建视图和裁剪空间矩形

	void FlushCmdQueue();// 实现围栏
	void CalculateFrameState();// 计算fps和mspf

protected:

	static D3DApp* mApp;

	//指针接口和变量声明
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

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};// 描述队列

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaQualityLevels;// msaad等级描述

	bool m4xMsaaState = false;// 是否开启4X MSAA
	UINT m4xMsaaQuality = 0;// 4xMSAA质量

	UINT64 mCurrentFence = 0;// 当前围栏值

	D3D12_VIEWPORT viewPort;
	D3D12_RECT scissorRect;

	UINT rtvDescriptorSize = 0;// 渲染目标视图大小
	UINT dsvDescriptorSize = 0;// 深度/模板目标视图大小
	UINT cbv_srv_uavDescriptorSize = 0;// 常量缓冲视图大小

	_GameTimer::GameTimer gt;// 游戏时间实例

	UINT mCurrentBackBuffer = 0;

	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	
	

	
};