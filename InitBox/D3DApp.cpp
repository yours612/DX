#include "D3DApp.h"
#include <windowsx.h>


D3DApp* D3DApp::mApp = nullptr;

D3DApp::D3DApp() {
	assert(mApp == nullptr);
	mApp = this;
}

D3DApp* D3DApp::GetApp() {
	return mApp;
}

D3DApp::~D3DApp() {
	if (d3dDevice != nullptr)
		FlushCmdQueue();

	//delete this;
}

void D3DApp::BuildGeometry(int type) {}
void D3DApp::BuildRenderItems(int num, int type, int whichWindow) {}
int D3DApp::Pick(int x, int y, int whichWindow) { return 0 ; }

// 初始化窗口和D3D
bool D3DApp::Init(HINSTANCE hInstance, int nShowCmd, HWND hwnd) {
	if (!InitWindow1(hInstance, nShowCmd, hwnd) || !InitWindow2(hInstance, nShowCmd, hwnd))
		return false;
	else if (!InitDirect3D())
		return false;

	// 创建RTV DSV ViewPort ScissorRect
	OnResize();

	return true;
}

LRESULT CALLBACK GraphicsWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	return D3DApp::GetApp()->MsgProc1(hwnd, msg, wParam, lParam);
}
LRESULT CALLBACK GraphicsWndProc1(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	return D3DApp::GetApp()->MsgProc2(hwnd, msg, wParam, lParam);
}
// 窗口1过程
LRESULT CALLBACK D3DApp::MsgProc1(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	// 消息处理
	switch (msg) {
		//禁止拖动窗口
	case WM_NCLBUTTONDOWN:
	{
		switch (wParam)
		{
		case HTCAPTION: // HTCAPTION 在标题条中。
			return 0;

		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}
			
	case WM_LBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); //通过 lParam 参数获得相关消息触发的坐标 (x , y)
		cursorPosX = GET_X_LPARAM(lParam);
		cursorPosY = GET_Y_LPARAM(lParam);
		if (num > 0)
		{
			hasPicked = Pick(cursorPosX, cursorPosY, 1);
		}
		
		if (createWhichGrapgic != 0)
		{
			switch (createWhichGrapgic)
			{
			case 1: //正方体
			{
				BuildGeometry(1); //在这里创建包围盒
				BuildRenderItems(num++, 1, 1);//在这里绑定包围盒
				allGraphics[num] = 1;
				break;
			}
				
			case 2: //球体
			{
				BuildGeometry(2);
				BuildRenderItems(num++, 2, 1);
				allGraphics[num] = 2;
				break;
			}
				
			case 3: //平面
			{
				BuildGeometry(3);
				BuildRenderItems(num++, 3, 1);
				allGraphics[num] = 3;
				break;
			}
				
			case 4: //柱体
			{
				BuildGeometry(4);
				BuildRenderItems(num++, 4, 1);
				allGraphics[num] = 4;
				break;
			}
				
			}
			createWhichGrapgic = 0;
			hasCreateGrapgic = true;
			SetClassLongPtr(graphicsWnd1, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
			SetClassLongPtr(graphicsWnd2, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
		}
		return 0;

	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
			//wParam为输入的虚拟键代码，lParam为系统反馈的光标信息
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); //通过 lParam 参数获得相关消息触发的坐标 (x , y)
		return 0;
		//鼠标按键抬起时的触发（左中右）
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
		//鼠标移动的触发
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 1);
		return 0;
		//当窗口尺寸发生变换时
	case WM_SIZE:
		graphicsWindowWidth = LOWORD(lParam);
		graphicsWindowHeight = HIWORD(lParam);
		if (d3dDevice) {
			//如果最小化,则暂停游戏，调整最小化和最大化状态
			if (wParam == SIZE_MINIMIZED) {
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
			}
			else if (wParam = SIZE_MAXIMIZED) {
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED) {
				if (mMinimized) {
					mAppPaused = false;
					mMinimized = false;
					OnResize();
				}
				else if (mMaximized) {
					mAppPaused = false;
					mMaximized = false;
					OnResize();
				}
				else if (mResizing) {
				}
				else {
					OnResize();
				}
			}
		}
		return 0;

		// 当窗口被销毁时，终止消息循环
	case WM_DESTROY:
		//delete this;
		PostQuitMessage(0);	// 终止消息循环，并发出WM_QUIT消息
		break;
		
	default:
		break;
	}
	// 将上面没有处理的消息转发给默认的窗口过程
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
// 窗口2过程
LRESULT CALLBACK D3DApp::MsgProc2(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	// 消息处理
	switch (msg) {
		//禁止拖动窗口
	case WM_NCLBUTTONDOWN:
	{
		switch (wParam)
		{
		case HTCAPTION: // HTCAPTION 在标题条中。
			return 0;

		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}

	case WM_LBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); //通过 lParam 参数获得相关消息触发的坐标 (x , y)
		cursorPosX = GET_X_LPARAM(lParam);
		cursorPosY = GET_Y_LPARAM(lParam);
		if (num > 0)
		{
			hasPicked = Pick(cursorPosX, cursorPosY, 2);
		}

		if (createWhichGrapgic != 0)
		{
			switch (createWhichGrapgic)
			{
			case 1: //正方体
			{
				BuildGeometry(1); //在这里创建包围盒
				BuildRenderItems(num++, 1, 2);//在这里绑定包围盒
				allGraphics[num] = 1;
				break;
			}

			case 2: //球体
			{
				BuildGeometry(2);
				BuildRenderItems(num++, 2, 2);
				allGraphics[num] = 2;
				break;
			}

			case 3: //平面
			{
				BuildGeometry(3);
				BuildRenderItems(num++, 3, 2);
				allGraphics[num] = 3;
				break;
			}

			case 4: //柱体
			{
				BuildGeometry(4);
				BuildRenderItems(num++, 4, 2);
				allGraphics[num] = 4;
				break;
			}

			}
			createWhichGrapgic = 0;
			hasCreateGrapgic = true;
			SetClassLongPtr(graphicsWnd1, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
			SetClassLongPtr(graphicsWnd2, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
		}
		return 0;

	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//wParam为输入的虚拟键代码，lParam为系统反馈的光标信息
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); //通过 lParam 参数获得相关消息触发的坐标 (x , y)
		return 0;
		//鼠标按键抬起时的触发（左中右）
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
		//鼠标移动的触发
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 2);
		return 0;
		//当窗口尺寸发生变换时
	case WM_SIZE:
		graphicsWindowWidth = LOWORD(lParam);
		graphicsWindowHeight = HIWORD(lParam);
		if (d3dDevice) {
			//如果最小化,则暂停游戏，调整最小化和最大化状态
			if (wParam == SIZE_MINIMIZED) {
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
			}
			else if (wParam = SIZE_MAXIMIZED) {
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED) {
				if (mMinimized) {
					mAppPaused = false;
					mMinimized = false;
					OnResize();
				}
				else if (mMaximized) {
					mAppPaused = false;
					mMaximized = false;
					OnResize();
				}
				else if (mResizing) {
				}
				else {
					OnResize();
				}
			}
		}
		return 0;

		// 当窗口被销毁时，终止消息循环
	case WM_DESTROY:
		//delete this;
		PostQuitMessage(0);	// 终止消息循环，并发出WM_QUIT消息
		break;

	default:
		break;
	}
	// 将上面没有处理的消息转发给默认的窗口过程
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

// 创建窗口1
bool D3DApp::InitWindow1(HINSTANCE hInstance, int nShowCmd, HWND hwnd) {
	// 窗口初始化描述结构体(WNDCLASS)
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;	// 当工作区宽高改变，则重新绘制窗口
	wc.lpfnWndProc = GraphicsWndProc;	// 指定窗口过程
	wc.cbClsExtra = 0;	// 借助这两个字段来为当前应用分配额外的内存空间（这里不分配，所以置0）
	wc.cbWndExtra = 0;	// 借助这两个字段来为当前应用分配额外的内存空间（这里不分配，所以置0）
	wc.hInstance = hInstance;	// 应用程序实例句柄（由WinMain传入）
	wc.hIcon = LoadIcon(0, IDC_ARROW);	//使用默认的应用程序图标
	wc.hCursor = LoadCursor(0, IDC_ARROW);	//使用标准的鼠标指针样式
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);	//指定了白色背景画刷句柄
	wc.lpszMenuName = 0;	//没有菜单栏
	wc.lpszClassName = L"GraphicsWnd1";	//窗口名
	// 窗口类注册失败
	if (!RegisterClass(&wc)) {
		MessageBox(0, L"RegisterClass Failed", 0, 0);
		return 0;
	}

	// 窗口类注册成功
	RECT R = { 0, 0, graphicsWindowWidth, graphicsWindowHeight };	// 裁剪矩形
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);	//根据窗口的客户区大小计算窗口的大小
	int width = R.right - R.left;
	int hight = R.bottom - R.top;

	// 创建窗口,返回布尔值
	graphicsWnd1 = CreateWindow(L"GraphicsWnd1", L"D3DApp1", WS_CHILD | WS_VISIBLE  | WS_CAPTION, 
		graphicsPosX, graphicsPosY, graphicsWindowWidth, graphicsWindowHeight, hwnd, 0, hInstance, 0);
	// 窗口创建失败
	if (!graphicsWnd1) {
		MessageBox(0, L"CreatWindow Failed", 0, 0);
		return 0;
	}
	// 窗口创建成功,则显示并更新窗口
	/*ShowWindow(graphicsWnd, nShowCmd);
	UpdateWindow(graphicsWnd);*/

	return true;
}
// 创建窗口2
bool D3DApp::InitWindow2(HINSTANCE hInstance, int nShowCmd, HWND hwnd) {
	// 窗口初始化描述结构体(WNDCLASS)
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;	// 当工作区宽高改变，则重新绘制窗口
	wc.lpfnWndProc = GraphicsWndProc1;	// 指定窗口过程
	wc.cbClsExtra = 0;	// 借助这两个字段来为当前应用分配额外的内存空间（这里不分配，所以置0）
	wc.cbWndExtra = 0;	// 借助这两个字段来为当前应用分配额外的内存空间（这里不分配，所以置0）
	wc.hInstance = hInstance;	// 应用程序实例句柄（由WinMain传入）
	wc.hIcon = LoadIcon(0, IDC_ARROW);	//使用默认的应用程序图标
	wc.hCursor = LoadCursor(0, IDC_ARROW);	//使用标准的鼠标指针样式
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);	//指定了白色背景画刷句柄
	wc.lpszMenuName = 0;	//没有菜单栏
	wc.lpszClassName = L"GraphicsWnd2";	//窗口名
	// 窗口类注册失败
	if (!RegisterClass(&wc)) {
		MessageBox(0, L"RegisterClass Failed", 0, 0);
		return 0;
	}

	// 窗口类注册成功
	RECT R = { 0, 0, graphicsWindowWidth, graphicsWindowHeight };	// 裁剪矩形
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);	//根据窗口的客户区大小计算窗口的大小
	int width = R.right - R.left;
	int hight = R.bottom - R.top;

	// 创建窗口,返回布尔值
	graphicsWnd2 = CreateWindow(L"GraphicsWnd2", L"D3DApp2", WS_CHILD | WS_VISIBLE | WS_CAPTION,
		graphicsPosX + graphicsWindowWidth, graphicsPosY, graphicsWindowWidth, graphicsWindowHeight, hwnd, 0, hInstance, 0);
	// 窗口创建失败
	if (!graphicsWnd2) {
		MessageBox(0, L"CreatWindow Failed", 0, 0);
		return 0;
	}
	// 窗口创建成功,则显示并更新窗口
	/*ShowWindow(graphicsWnd, nShowCmd);
	UpdateWindow(graphicsWnd);*/

	return true;
}


// 初始化D3D
bool D3DApp::InitDirect3D() {
	/*开启D3D12调试层*/
#if defined(DEBUG) || defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	CreateDevice();
	CreateFence();
	GetDescriptorSize();
	SetMSAA();
	CreateCommandObject();
	CreateSwapChain();
	CreateDescriptorHeap();
	
	//放到OnResize里
	/*CreateRTV();
	CreateDSV();
	CreateViewPortAndScissorRect();*/

	return true;
}


// 消息循环，采用"一种灵活的消息循环"
// Run()每运行一次 代表一帧
int D3DApp::Run(HWND hwnd) {
	// 定义消息结构体
	MSG msg = { 0 };
	// 每次消息循环重置一次计数器
	gt.Reset();
	// 如果PeekMessage函数不等于0，说明没有接受到WM_QUIT
	while (msg.message != WM_QUIT) {
		// 如果有窗口消息就进行处理
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) // PeekMessage函数会自动填充msg结构体元素
		{
			TranslateMessage(&msg);	// 键盘按键转换，将虚拟键消息转换为字符消息
			DispatchMessage(&msg);	// 把消息分派给相应的窗口过程
		}
		// 否则就执行动画和游戏逻辑
		else {
			gt.Tick();// 计算每两帧间隔时间 并获取现在的时间
			if (!gt.IsStopped()) {
				CalculateFrameState();
				Update();
				Draw();
				
			}
			else {
				Sleep(100);
			}
		}

		if (hasPicked == 1)
		{
			//UpdateWindow(graphicsWnd);
			PostMessage(hwnd, WM_USER + 1, 0, 0);
			hasPicked = 0;
		}
	}
	return (int)msg.wParam;
}

// 创建设备
void D3DApp::CreateDevice() {
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));
	ThrowIfFailed(D3D12CreateDevice(nullptr, // 此参数如果设置为nullptr，则使用主适配器
		D3D_FEATURE_LEVEL_12_0,		// 应用程序需要硬件所支持的最低功能级别
		IID_PPV_ARGS(&d3dDevice)));	// 返回所建设备
}

// 创建围栏
void D3DApp::CreateFence() {
	ThrowIfFailed(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
}


// 获取描述符大小 用来找到堆的偏移（用偏移量找到。。。）
void D3DApp::GetDescriptorSize() {
	// UINT 
	rtvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);// 渲染目标缓冲区描述符大小
	// UINT 
	dsvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);// 深度模板缓冲区描述符大小
	// UINT 
	cbv_srv_uavDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);// 常量缓冲区描述符大小
}

// 检测MSAA质量支持
// 注意：此处不使用MSAA，采样数量设置为1（即不采样）。
void D3DApp::SetMSAA() {
	msaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// UNORM是归一化处理的无符号整数
	msaaQualityLevels.SampleCount = 1;
	msaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;// 没有任何选项支持
	msaaQualityLevels.NumQualityLevels = 0;
	// 检测驱动是否支持这个MSAA描述 （注意：第二个参数即是输入又是输出）
	ThrowIfFailed(d3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQualityLevels, sizeof(msaaQualityLevels)));
	// 如果不支持支持这个MSAA描述，则Check函数返回的NumQualityLevels = 0
	assert(msaaQualityLevels.NumQualityLevels > 0);
}

// 创建命令队列
void D3DApp::CreateCommandObject() {
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};// 描述队列

	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;// 指定GPU可以执行的命令缓冲区，直接命令列表未继承任何GPU状态
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;// 默认命令队列
	ThrowIfFailed(d3dDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&cmdQueue)));// 创建命令队列
	ThrowIfFailed(d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocator)));// 创建命令分配器 &cmdAllocator等价于cmdAllocator.GetAddressOf
	// 创建命令列表
	ThrowIfFailed(d3dDevice->CreateCommandList(0, // 掩码值为0，单GPU
		D3D12_COMMAND_LIST_TYPE_DIRECT, // 命令列表类型
		cmdAllocator.Get(),	// 命令分配器接口指针
		nullptr,	// 流水线状态对象PSO，这里不绘制，所以空指针
		IID_PPV_ARGS(&cmdList)));	// 返回创建的命令列表
	cmdList->Close();	// 重置命令列表前必须将其关闭
}

// 创建交换链
void D3DApp::CreateSwapChain() {
	swapChain1.Reset();
	// 描述交换链
	DXGI_SWAP_CHAIN_DESC swapChainDesc;// 描述交换链结构体
	swapChainDesc.BufferDesc.Width = graphicsWindowWidth;	// 缓冲区分辨率的宽度
	swapChainDesc.BufferDesc.Height = graphicsWindowHeight;	// 缓冲区分辨率的高度
	swapChainDesc.BufferDesc.Format = mBackBufferFormat;	// 缓冲区的显示格式
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;	// 刷新率的分子
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;	// 刷新率的分母
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;	// 逐行扫描VS隔行扫描(未指定的)
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	// 图像相对屏幕的拉伸（未指定的）
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// 将数据渲染至后台缓冲区（即作为渲染目标）
	swapChainDesc.OutputWindow = graphicsWnd1;	// 渲染窗口句柄
	swapChainDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;;	// 多重采样数量
	swapChainDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;;	// 多重采样质量
	swapChainDesc.Windowed = true;	// 是否窗口化
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	// 固定写法
	swapChainDesc.BufferCount = 2;	// 后台缓冲区数量（双缓冲）
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	// 自适应窗口模式（自动选择最适于当前窗口尺寸的显示模式）
	// 利用DXGI接口下的工厂类创建交换链
	ThrowIfFailed(dxgiFactory->CreateSwapChain(cmdQueue.Get(), &swapChainDesc, swapChain1.GetAddressOf()));

	//————————————————————————————————————————————

	swapChain2.Reset();
	// 描述交换链
	DXGI_SWAP_CHAIN_DESC swapChainDesc2;// 描述交换链结构体
	swapChainDesc2.BufferDesc.Width = graphicsWindowWidth;	// 缓冲区分辨率的宽度
	swapChainDesc2.BufferDesc.Height = graphicsWindowHeight;	// 缓冲区分辨率的高度
	swapChainDesc2.BufferDesc.Format = mBackBufferFormat;	// 缓冲区的显示格式
	swapChainDesc2.BufferDesc.RefreshRate.Denominator = 1;	// 刷新率的分子
	swapChainDesc2.BufferDesc.RefreshRate.Numerator = 60;	// 刷新率的分母
	swapChainDesc2.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;	// 逐行扫描VS隔行扫描(未指定的)
	swapChainDesc2.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	// 图像相对屏幕的拉伸（未指定的）
	swapChainDesc2.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// 将数据渲染至后台缓冲区（即作为渲染目标）
	swapChainDesc2.OutputWindow = graphicsWnd2;	// 渲染窗口句柄
	swapChainDesc2.SampleDesc.Count = m4xMsaaState ? 4 : 1;;	// 多重采样数量
	swapChainDesc2.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;;	// 多重采样质量
	swapChainDesc2.Windowed = true;	// 是否窗口化
	swapChainDesc2.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	// 固定写法
	swapChainDesc2.BufferCount = 2;	// 后台缓冲区数量（双缓冲）
	swapChainDesc2.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	// 自适应窗口模式（自动选择最适于当前窗口尺寸的显示模式）
	// 利用DXGI接口下的工厂类创建交换链
	ThrowIfFailed(dxgiFactory->CreateSwapChain(cmdQueue.Get(), &swapChainDesc2, swapChain2.GetAddressOf()));
}

// 创建描述堆符
void D3DApp::CreateDescriptorHeap() {
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc;
	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc;
	// 首先创建RTV堆 RTV描述符表示的是渲染目标视图资源(render target view)。
	rtvDescriptorHeapDesc.NumDescriptors = 2;
	rtvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescriptorHeapDesc.NodeMask = 0;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvHeap1)));
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvHeap2)));
	// 然后创建DSV堆 DSV描述符表示的是深度/模板视图资源(depth/stencil view)。
	dsvDescriptorHeapDesc.NumDescriptors = 1;
	dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDescriptorHeapDesc.NodeMask = 0;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&dsvHeap1)));
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&dsvHeap2)));
}

// 创建渲染目标视图描述符
void D3DApp::CreateRTV() {
	//获取当前后台缓冲区的RTV
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle1(rtvHeap1->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle2(rtvHeap2->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < 2; i++) {
		// 获得存于交换链中的后台缓冲区资源
		swapChain1->GetBuffer(i, IID_PPV_ARGS(swapChainBuffer1[i].GetAddressOf()));
		swapChain2->GetBuffer(i, IID_PPV_ARGS(swapChainBuffer2[i].GetAddressOf()));
		// 创建RTV
		d3dDevice->CreateRenderTargetView(swapChainBuffer1[i].Get(),
			nullptr,	// 在交换链创建中已经定义了该资源的数据格式，所以这里指定为空指针
			rtvHeapHandle1);	// 描述符句柄结构体（这里是变体，继承自CD3DX12_CPU_DESCRIPTOR_HANDLE）

		// 偏移到描述符堆中的下一个缓冲区
		rtvHeapHandle1.Offset(1, rtvDescriptorSize);
		//————————————————————//
		d3dDevice->CreateRenderTargetView(swapChainBuffer2[i].Get(),
			nullptr,
			rtvHeapHandle2);
		rtvHeapHandle2.Offset(1, rtvDescriptorSize);
	}
}

// 创建深度/模板缓冲区及其视图
void D3DApp::CreateDSV() {
	D3D12_RESOURCE_DESC dsvResourceDesc;// 描述模板类型
	// 在CPU中创建好深度模板数据资源
	// 描述纹理资源
	dsvResourceDesc.Alignment = 0;	// 指定对齐
	dsvResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	// 指定资源维度（类型）为TEXTURE2D
	dsvResourceDesc.DepthOrArraySize = 1;	// 纹理深度为1
	dsvResourceDesc.Width = graphicsWindowWidth;	// 资源宽
	dsvResourceDesc.Height = graphicsWindowHeight;	// 资源高
	dsvResourceDesc.MipLevels = 1;	// MIPMAP层级数量
	dsvResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;	// 指定纹理布局（这里不指定）
	dsvResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	// 深度模板资源的Flag
	dsvResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// 24位深度，8位模板,还有个无类型的格式DXGI_FORMAT_R24G8_TYPELESS也可以使用
	dsvResourceDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;	// 多重采样数量
	dsvResourceDesc.SampleDesc.Quality = msaaQualityLevels.NumQualityLevels - 1;	// 多重采样质量
	// 描述清除资源的优化值？？？
	CD3DX12_CLEAR_VALUE optClear;	// 清除资源的优化值，提高清除操作的执行速度（CreateCommittedResource函数中传入）
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;// 24位深度，8位模板,还有个无类型的格式DXGI_FORMAT_R24G8_TYPELESS也可以使用
	optClear.DepthStencil.Depth = 1;	// 初始深度值为1
	optClear.DepthStencil.Stencil = 0;	// 初始模板值为0
	// 创建一个资源和一个堆，并将资源提交至堆中（将深度模板数据提交至GPU显存中）
	CD3DX12_HEAP_PROPERTIES hpp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);// 堆所具有的属性，设置为默认堆。
	ThrowIfFailed(d3dDevice->CreateCommittedResource(&hpp,	// 堆类型为默认堆（不能写入）
		D3D12_HEAP_FLAG_NONE,	// Flag
		&dsvResourceDesc,	// 上面定义的DSV资源指针
		D3D12_RESOURCE_STATE_COMMON,	// 资源的状态为初始状态
		&optClear,	// 上面定义的优化值指针
		IID_PPV_ARGS(&depthStencilBuffer1)));	// 返回深度模板资源
	ThrowIfFailed(d3dDevice->CreateCommittedResource(&hpp,	D3D12_HEAP_FLAG_NONE,	
		&dsvResourceDesc,	D3D12_RESOURCE_STATE_COMMON,	&optClear,
		IID_PPV_ARGS(&depthStencilBuffer2)));

	d3dDevice->CreateDepthStencilView(depthStencilBuffer1.Get(),
		nullptr,	// D3D12_DEPTH_STENCIL_VIEW_DESC类型指针，可填&dsvDesc（见上注释代码），
							// 由于在创建深度模板资源时已经定义深度模板数据属性，所以这里可以指定为空指针
		dsvHeap1->GetCPUDescriptorHandleForHeapStart());	// DSV句柄
	d3dDevice->CreateDepthStencilView(depthStencilBuffer2.Get(),
		nullptr,	// D3D12_DEPTH_STENCIL_VIEW_DESC类型指针，可填&dsvDesc（见上注释代码），
							// 由于在创建深度模板资源时已经定义深度模板数据属性，所以这里可以指定为空指针
		dsvHeap2->GetCPUDescriptorHandleForHeapStart());	// DSV句柄
	// 把资源从初始状态转换到深度缓冲状态
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(depthStencilBuffer1.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(depthStencilBuffer2.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));
}

// 标记深度/模板资源的状态
//void D3DApp::resourceBarrierBuild() {
//	cmdList->ResourceBarrier(1,	// Barrier屏障个数
//		&CD3DX12_RESOURCE_BARRIER::Transition(depthStencilBuffer.Get(),// 视图的COM接口(?)。指定之前的深度/模板缓冲区的接口。
//			D3D12_RESOURCE_STATE_COMMON,	// 转换前状态（创建时的状态，即CreateCommittedResource函数中定义的状态）
//			D3D12_RESOURCE_STATE_DEPTH_WRITE));	// 转换后状态为可写入的深度图，还有一个D3D12_RESOURCE_STATE_DEPTH_READ是只可读的深度图
//	// 等所有命令都进入cmdList后，将命令从命令列表传入命令队列，也就是从CPU传入GPU的过程。
//	// 注意：在传入命令队列前必须关闭命令列表。
//	ThrowIfFailed(cmdList->Close());	// 命令添加完后将其关闭
//	ID3D12CommandList* cmdLists[] = { cmdList.Get() };	// 声明并定义命令列表数组
//	cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);	// 将命令从命令列表传至命令队列
//}

// 实现围栏
void D3DApp::FlushCmdQueue() {
	mCurrentFence++;	// CPU传完命令并关闭后，将当前围栏值+1
	cmdQueue->Signal(fence.Get(), mCurrentFence);	// 当GPU处理完CPU传入的命令后，将fence接口中的围栏值+1，即fence->GetCompletedValue()+1
	if (fence->GetCompletedValue() < mCurrentFence)	// 如果小于，说明GPU没有处理完所有命令
	{
		HANDLE eventHandle = CreateEvent(nullptr, false, false, L"FenceSetDone");	// 创建事件
		fence->SetEventOnCompletion(mCurrentFence, eventHandle);// 当围栏达到mCurrentFence值（即执行到Signal（）指令修改了围栏值）时触发的eventHandle事件
		WaitForSingleObject(eventHandle, INFINITE);// 等待GPU命中围栏，激发事件（阻塞当前线程直到事件触发，注意此Enent需先设置再等待，
							   // 如果没有Set就Wait，就死锁了，Set永远不会调用，所以也就没线程可以唤醒这个线程）
		CloseHandle(eventHandle);
	}
}

// 设置视口和裁剪矩形
void D3DApp::CreateViewPortAndScissorRect() {
	// 视口设置
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = static_cast<float>(graphicsWindowWidth);
	viewPort.Height = static_cast<float>(graphicsWindowHeight);
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	// 裁剪矩形设置（矩形外的像素都将被剔除）
	// 前两个为左上点坐标，后两个为右下点坐标
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = graphicsWindowWidth;
	scissorRect.bottom = graphicsWindowHeight;
}

// 计算帧率和每帧多少毫秒
void D3DApp::CalculateFrameState() {
	using namespace _GameTimer;
	static int frameCnt = 0;// 总帧数
	static float timeElapsed = 0.0f;//总时间
	frameCnt++;
	if (gt.TotalTime() - timeElapsed >= 1.0f) {
		float fps = frameCnt;// 每秒多少帧
		float mspf = 1000.0f / fps;// 每帧多少毫秒

		std::wstring fpsStr = std::to_wstring(fps);
		std::wstring mspfStr = std::to_wstring(mspf);
		// 将帧数显示在窗口上
		std::wstring windowText = L"D3DApp   fps: " + fpsStr + L"    " + L"mspf: " + mspfStr;
		SetWindowText(graphicsWnd1, windowText.c_str());

		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

//窗口尺寸改变事件
void D3DApp::OnResize() {
	assert(d3dDevice);
	assert(swapChain1);
	assert(swapChain2);
	assert(cmdAllocator); //非空指针，可以通过assert

	// 改变资源前先同步
	FlushCmdQueue();
	ThrowIfFailed(cmdList->Reset(cmdAllocator.Get(), nullptr));

	//释放之前的资源，为我们重新创建做好准备
	for (int i = 0; i < 2; i++) // SwapChainBufferCount = 2
	{
		swapChainBuffer1[i].Reset();
		swapChainBuffer2[i].Reset();
	}
	depthStencilBuffer1.Reset();
	depthStencilBuffer2.Reset();

	// 重新调整后台缓冲区资源的大小
	ThrowIfFailed(swapChain1->ResizeBuffers(2, 
		graphicsWindowWidth,
		graphicsWindowHeight, 
		mBackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
	ThrowIfFailed(swapChain2->ResizeBuffers(2,
		graphicsWindowWidth,
		graphicsWindowHeight,
		mBackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	// 后台缓冲区索引置零
	mCurrentBackBuffer = 0;

	CreateRTV();
	CreateDSV();
	// 执行Resize命令
	ThrowIfFailed(cmdList->Close());
	ID3D12CommandList* cmdsLists[] = { cmdList.Get() };
	cmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// 等待Resize命令完成
	FlushCmdQueue();

	// 更新视口/投影矩阵适应窗口变换
	CreateViewPortAndScissorRect();
}

void D3DApp::SetWindowInformation(int clientWidth, int clientHeight)
{
	graphicsWindowWidth = clientWidth / 5 * 2;
	graphicsWindowHeight = clientHeight;
	graphicsPosX = clientWidth / 5 ;
	graphicsPosY = 0;
}
