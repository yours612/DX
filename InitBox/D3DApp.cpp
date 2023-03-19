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

// ��ʼ�����ں�D3D
bool D3DApp::Init(HINSTANCE hInstance, int nShowCmd, HWND hwnd) {
	if (!InitWindow1(hInstance, nShowCmd, hwnd) || !InitWindow2(hInstance, nShowCmd, hwnd))
		return false;
	else if (!InitDirect3D())
		return false;

	// ����RTV DSV ViewPort ScissorRect
	OnResize();

	return true;
}

LRESULT CALLBACK GraphicsWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	return D3DApp::GetApp()->MsgProc1(hwnd, msg, wParam, lParam);
}
LRESULT CALLBACK GraphicsWndProc1(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	return D3DApp::GetApp()->MsgProc2(hwnd, msg, wParam, lParam);
}
// ����1����
LRESULT CALLBACK D3DApp::MsgProc1(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	// ��Ϣ����
	switch (msg) {
		//��ֹ�϶�����
	case WM_NCLBUTTONDOWN:
	{
		switch (wParam)
		{
		case HTCAPTION: // HTCAPTION �ڱ������С�
			return 0;

		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}
			
	case WM_LBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); //ͨ�� lParam ������������Ϣ���������� (x , y)
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
			case 1: //������
			{
				BuildGeometry(1); //�����ﴴ����Χ��
				BuildRenderItems(num++, 1, 1);//������󶨰�Χ��
				allGraphics[num] = 1;
				break;
			}
				
			case 2: //����
			{
				BuildGeometry(2);
				BuildRenderItems(num++, 2, 1);
				allGraphics[num] = 2;
				break;
			}
				
			case 3: //ƽ��
			{
				BuildGeometry(3);
				BuildRenderItems(num++, 3, 1);
				allGraphics[num] = 3;
				break;
			}
				
			case 4: //����
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
			//wParamΪ�������������룬lParamΪϵͳ�����Ĺ����Ϣ
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); //ͨ�� lParam ������������Ϣ���������� (x , y)
		return 0;
		//��갴��̧��ʱ�Ĵ����������ң�
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
		//����ƶ��Ĵ���
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 1);
		return 0;
		//�����ڳߴ緢���任ʱ
	case WM_SIZE:
		graphicsWindowWidth = LOWORD(lParam);
		graphicsWindowHeight = HIWORD(lParam);
		if (d3dDevice) {
			//�����С��,����ͣ��Ϸ��������С�������״̬
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

		// �����ڱ�����ʱ����ֹ��Ϣѭ��
	case WM_DESTROY:
		//delete this;
		PostQuitMessage(0);	// ��ֹ��Ϣѭ����������WM_QUIT��Ϣ
		break;
		
	default:
		break;
	}
	// ������û�д������Ϣת����Ĭ�ϵĴ��ڹ���
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
// ����2����
LRESULT CALLBACK D3DApp::MsgProc2(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	// ��Ϣ����
	switch (msg) {
		//��ֹ�϶�����
	case WM_NCLBUTTONDOWN:
	{
		switch (wParam)
		{
		case HTCAPTION: // HTCAPTION �ڱ������С�
			return 0;

		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}

	case WM_LBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); //ͨ�� lParam ������������Ϣ���������� (x , y)
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
			case 1: //������
			{
				BuildGeometry(1); //�����ﴴ����Χ��
				BuildRenderItems(num++, 1, 2);//������󶨰�Χ��
				allGraphics[num] = 1;
				break;
			}

			case 2: //����
			{
				BuildGeometry(2);
				BuildRenderItems(num++, 2, 2);
				allGraphics[num] = 2;
				break;
			}

			case 3: //ƽ��
			{
				BuildGeometry(3);
				BuildRenderItems(num++, 3, 2);
				allGraphics[num] = 3;
				break;
			}

			case 4: //����
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
		//wParamΪ�������������룬lParamΪϵͳ�����Ĺ����Ϣ
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); //ͨ�� lParam ������������Ϣ���������� (x , y)
		return 0;
		//��갴��̧��ʱ�Ĵ����������ң�
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
		//����ƶ��Ĵ���
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 2);
		return 0;
		//�����ڳߴ緢���任ʱ
	case WM_SIZE:
		graphicsWindowWidth = LOWORD(lParam);
		graphicsWindowHeight = HIWORD(lParam);
		if (d3dDevice) {
			//�����С��,����ͣ��Ϸ��������С�������״̬
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

		// �����ڱ�����ʱ����ֹ��Ϣѭ��
	case WM_DESTROY:
		//delete this;
		PostQuitMessage(0);	// ��ֹ��Ϣѭ����������WM_QUIT��Ϣ
		break;

	default:
		break;
	}
	// ������û�д������Ϣת����Ĭ�ϵĴ��ڹ���
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ��������1
bool D3DApp::InitWindow1(HINSTANCE hInstance, int nShowCmd, HWND hwnd) {
	// ���ڳ�ʼ�������ṹ��(WNDCLASS)
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;	// ����������߸ı䣬�����»��ƴ���
	wc.lpfnWndProc = GraphicsWndProc;	// ָ�����ڹ���
	wc.cbClsExtra = 0;	// �����������ֶ���Ϊ��ǰӦ�÷��������ڴ�ռ䣨���ﲻ���䣬������0��
	wc.cbWndExtra = 0;	// �����������ֶ���Ϊ��ǰӦ�÷��������ڴ�ռ䣨���ﲻ���䣬������0��
	wc.hInstance = hInstance;	// Ӧ�ó���ʵ���������WinMain���룩
	wc.hIcon = LoadIcon(0, IDC_ARROW);	//ʹ��Ĭ�ϵ�Ӧ�ó���ͼ��
	wc.hCursor = LoadCursor(0, IDC_ARROW);	//ʹ�ñ�׼�����ָ����ʽ
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);	//ָ���˰�ɫ������ˢ���
	wc.lpszMenuName = 0;	//û�в˵���
	wc.lpszClassName = L"GraphicsWnd1";	//������
	// ������ע��ʧ��
	if (!RegisterClass(&wc)) {
		MessageBox(0, L"RegisterClass Failed", 0, 0);
		return 0;
	}

	// ������ע��ɹ�
	RECT R = { 0, 0, graphicsWindowWidth, graphicsWindowHeight };	// �ü�����
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);	//���ݴ��ڵĿͻ�����С���㴰�ڵĴ�С
	int width = R.right - R.left;
	int hight = R.bottom - R.top;

	// ��������,���ز���ֵ
	graphicsWnd1 = CreateWindow(L"GraphicsWnd1", L"D3DApp1", WS_CHILD | WS_VISIBLE  | WS_CAPTION, 
		graphicsPosX, graphicsPosY, graphicsWindowWidth, graphicsWindowHeight, hwnd, 0, hInstance, 0);
	// ���ڴ���ʧ��
	if (!graphicsWnd1) {
		MessageBox(0, L"CreatWindow Failed", 0, 0);
		return 0;
	}
	// ���ڴ����ɹ�,����ʾ�����´���
	/*ShowWindow(graphicsWnd, nShowCmd);
	UpdateWindow(graphicsWnd);*/

	return true;
}
// ��������2
bool D3DApp::InitWindow2(HINSTANCE hInstance, int nShowCmd, HWND hwnd) {
	// ���ڳ�ʼ�������ṹ��(WNDCLASS)
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;	// ����������߸ı䣬�����»��ƴ���
	wc.lpfnWndProc = GraphicsWndProc1;	// ָ�����ڹ���
	wc.cbClsExtra = 0;	// �����������ֶ���Ϊ��ǰӦ�÷��������ڴ�ռ䣨���ﲻ���䣬������0��
	wc.cbWndExtra = 0;	// �����������ֶ���Ϊ��ǰӦ�÷��������ڴ�ռ䣨���ﲻ���䣬������0��
	wc.hInstance = hInstance;	// Ӧ�ó���ʵ���������WinMain���룩
	wc.hIcon = LoadIcon(0, IDC_ARROW);	//ʹ��Ĭ�ϵ�Ӧ�ó���ͼ��
	wc.hCursor = LoadCursor(0, IDC_ARROW);	//ʹ�ñ�׼�����ָ����ʽ
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);	//ָ���˰�ɫ������ˢ���
	wc.lpszMenuName = 0;	//û�в˵���
	wc.lpszClassName = L"GraphicsWnd2";	//������
	// ������ע��ʧ��
	if (!RegisterClass(&wc)) {
		MessageBox(0, L"RegisterClass Failed", 0, 0);
		return 0;
	}

	// ������ע��ɹ�
	RECT R = { 0, 0, graphicsWindowWidth, graphicsWindowHeight };	// �ü�����
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);	//���ݴ��ڵĿͻ�����С���㴰�ڵĴ�С
	int width = R.right - R.left;
	int hight = R.bottom - R.top;

	// ��������,���ز���ֵ
	graphicsWnd2 = CreateWindow(L"GraphicsWnd2", L"D3DApp2", WS_CHILD | WS_VISIBLE | WS_CAPTION,
		graphicsPosX + graphicsWindowWidth, graphicsPosY, graphicsWindowWidth, graphicsWindowHeight, hwnd, 0, hInstance, 0);
	// ���ڴ���ʧ��
	if (!graphicsWnd2) {
		MessageBox(0, L"CreatWindow Failed", 0, 0);
		return 0;
	}
	// ���ڴ����ɹ�,����ʾ�����´���
	/*ShowWindow(graphicsWnd, nShowCmd);
	UpdateWindow(graphicsWnd);*/

	return true;
}


// ��ʼ��D3D
bool D3DApp::InitDirect3D() {
	/*����D3D12���Բ�*/
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
	
	//�ŵ�OnResize��
	/*CreateRTV();
	CreateDSV();
	CreateViewPortAndScissorRect();*/

	return true;
}


// ��Ϣѭ��������"һ��������Ϣѭ��"
// Run()ÿ����һ�� ����һ֡
int D3DApp::Run(HWND hwnd) {
	// ������Ϣ�ṹ��
	MSG msg = { 0 };
	// ÿ����Ϣѭ������һ�μ�����
	gt.Reset();
	// ���PeekMessage����������0��˵��û�н��ܵ�WM_QUIT
	while (msg.message != WM_QUIT) {
		// ����д�����Ϣ�ͽ��д���
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) // PeekMessage�������Զ����msg�ṹ��Ԫ��
		{
			TranslateMessage(&msg);	// ���̰���ת�������������Ϣת��Ϊ�ַ���Ϣ
			DispatchMessage(&msg);	// ����Ϣ���ɸ���Ӧ�Ĵ��ڹ���
		}
		// �����ִ�ж�������Ϸ�߼�
		else {
			gt.Tick();// ����ÿ��֡���ʱ�� ����ȡ���ڵ�ʱ��
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

// �����豸
void D3DApp::CreateDevice() {
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));
	ThrowIfFailed(D3D12CreateDevice(nullptr, // �˲����������Ϊnullptr����ʹ����������
		D3D_FEATURE_LEVEL_12_0,		// Ӧ�ó�����ҪӲ����֧�ֵ���͹��ܼ���
		IID_PPV_ARGS(&d3dDevice)));	// ���������豸
}

// ����Χ��
void D3DApp::CreateFence() {
	ThrowIfFailed(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
}


// ��ȡ��������С �����ҵ��ѵ�ƫ�ƣ���ƫ�����ҵ���������
void D3DApp::GetDescriptorSize() {
	// UINT 
	rtvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);// ��ȾĿ�껺������������С
	// UINT 
	dsvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);// ���ģ�建������������С
	// UINT 
	cbv_srv_uavDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);// ������������������С
}

// ���MSAA����֧��
// ע�⣺�˴���ʹ��MSAA��������������Ϊ1��������������
void D3DApp::SetMSAA() {
	msaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// UNORM�ǹ�һ��������޷�������
	msaaQualityLevels.SampleCount = 1;
	msaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;// û���κ�ѡ��֧��
	msaaQualityLevels.NumQualityLevels = 0;
	// ��������Ƿ�֧�����MSAA���� ��ע�⣺�ڶ������������������������
	ThrowIfFailed(d3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaQualityLevels, sizeof(msaaQualityLevels)));
	// �����֧��֧�����MSAA��������Check�������ص�NumQualityLevels = 0
	assert(msaaQualityLevels.NumQualityLevels > 0);
}

// �����������
void D3DApp::CreateCommandObject() {
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};// ��������

	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;// ָ��GPU����ִ�е����������ֱ�������б�δ�̳��κ�GPU״̬
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;// Ĭ���������
	ThrowIfFailed(d3dDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&cmdQueue)));// �����������
	ThrowIfFailed(d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocator)));// ������������� &cmdAllocator�ȼ���cmdAllocator.GetAddressOf
	// ���������б�
	ThrowIfFailed(d3dDevice->CreateCommandList(0, // ����ֵΪ0����GPU
		D3D12_COMMAND_LIST_TYPE_DIRECT, // �����б�����
		cmdAllocator.Get(),	// ����������ӿ�ָ��
		nullptr,	// ��ˮ��״̬����PSO�����ﲻ���ƣ����Կ�ָ��
		IID_PPV_ARGS(&cmdList)));	// ���ش����������б�
	cmdList->Close();	// ���������б�ǰ���뽫��ر�
}

// ����������
void D3DApp::CreateSwapChain() {
	swapChain1.Reset();
	// ����������
	DXGI_SWAP_CHAIN_DESC swapChainDesc;// �����������ṹ��
	swapChainDesc.BufferDesc.Width = graphicsWindowWidth;	// �������ֱ��ʵĿ��
	swapChainDesc.BufferDesc.Height = graphicsWindowHeight;	// �������ֱ��ʵĸ߶�
	swapChainDesc.BufferDesc.Format = mBackBufferFormat;	// ����������ʾ��ʽ
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;	// ˢ���ʵķ���
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;	// ˢ���ʵķ�ĸ
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;	// ����ɨ��VS����ɨ��(δָ����)
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	// ͼ�������Ļ�����죨δָ���ģ�
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// ��������Ⱦ����̨������������Ϊ��ȾĿ�꣩
	swapChainDesc.OutputWindow = graphicsWnd1;	// ��Ⱦ���ھ��
	swapChainDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;;	// ���ز�������
	swapChainDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;;	// ���ز�������
	swapChainDesc.Windowed = true;	// �Ƿ񴰿ڻ�
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	// �̶�д��
	swapChainDesc.BufferCount = 2;	// ��̨������������˫���壩
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	// ����Ӧ����ģʽ���Զ�ѡ�������ڵ�ǰ���ڳߴ����ʾģʽ��
	// ����DXGI�ӿ��µĹ����ഴ��������
	ThrowIfFailed(dxgiFactory->CreateSwapChain(cmdQueue.Get(), &swapChainDesc, swapChain1.GetAddressOf()));

	//����������������������������������������������������������������������������������������

	swapChain2.Reset();
	// ����������
	DXGI_SWAP_CHAIN_DESC swapChainDesc2;// �����������ṹ��
	swapChainDesc2.BufferDesc.Width = graphicsWindowWidth;	// �������ֱ��ʵĿ��
	swapChainDesc2.BufferDesc.Height = graphicsWindowHeight;	// �������ֱ��ʵĸ߶�
	swapChainDesc2.BufferDesc.Format = mBackBufferFormat;	// ����������ʾ��ʽ
	swapChainDesc2.BufferDesc.RefreshRate.Denominator = 1;	// ˢ���ʵķ���
	swapChainDesc2.BufferDesc.RefreshRate.Numerator = 60;	// ˢ���ʵķ�ĸ
	swapChainDesc2.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;	// ����ɨ��VS����ɨ��(δָ����)
	swapChainDesc2.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	// ͼ�������Ļ�����죨δָ���ģ�
	swapChainDesc2.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// ��������Ⱦ����̨������������Ϊ��ȾĿ�꣩
	swapChainDesc2.OutputWindow = graphicsWnd2;	// ��Ⱦ���ھ��
	swapChainDesc2.SampleDesc.Count = m4xMsaaState ? 4 : 1;;	// ���ز�������
	swapChainDesc2.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;;	// ���ز�������
	swapChainDesc2.Windowed = true;	// �Ƿ񴰿ڻ�
	swapChainDesc2.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	// �̶�д��
	swapChainDesc2.BufferCount = 2;	// ��̨������������˫���壩
	swapChainDesc2.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	// ����Ӧ����ģʽ���Զ�ѡ�������ڵ�ǰ���ڳߴ����ʾģʽ��
	// ����DXGI�ӿ��µĹ����ഴ��������
	ThrowIfFailed(dxgiFactory->CreateSwapChain(cmdQueue.Get(), &swapChainDesc2, swapChain2.GetAddressOf()));
}

// ���������ѷ�
void D3DApp::CreateDescriptorHeap() {
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc;
	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc;
	// ���ȴ���RTV�� RTV��������ʾ������ȾĿ����ͼ��Դ(render target view)��
	rtvDescriptorHeapDesc.NumDescriptors = 2;
	rtvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescriptorHeapDesc.NodeMask = 0;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvHeap1)));
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvHeap2)));
	// Ȼ�󴴽�DSV�� DSV��������ʾ�������/ģ����ͼ��Դ(depth/stencil view)��
	dsvDescriptorHeapDesc.NumDescriptors = 1;
	dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDescriptorHeapDesc.NodeMask = 0;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&dsvHeap1)));
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&dsvHeap2)));
}

// ������ȾĿ����ͼ������
void D3DApp::CreateRTV() {
	//��ȡ��ǰ��̨��������RTV
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle1(rtvHeap1->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle2(rtvHeap2->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < 2; i++) {
		// ��ô��ڽ������еĺ�̨��������Դ
		swapChain1->GetBuffer(i, IID_PPV_ARGS(swapChainBuffer1[i].GetAddressOf()));
		swapChain2->GetBuffer(i, IID_PPV_ARGS(swapChainBuffer2[i].GetAddressOf()));
		// ����RTV
		d3dDevice->CreateRenderTargetView(swapChainBuffer1[i].Get(),
			nullptr,	// �ڽ������������Ѿ������˸���Դ�����ݸ�ʽ����������ָ��Ϊ��ָ��
			rtvHeapHandle1);	// ����������ṹ�壨�����Ǳ��壬�̳���CD3DX12_CPU_DESCRIPTOR_HANDLE��

		// ƫ�Ƶ����������е���һ��������
		rtvHeapHandle1.Offset(1, rtvDescriptorSize);
		//����������������������������������������//
		d3dDevice->CreateRenderTargetView(swapChainBuffer2[i].Get(),
			nullptr,
			rtvHeapHandle2);
		rtvHeapHandle2.Offset(1, rtvDescriptorSize);
	}
}

// �������/ģ�建����������ͼ
void D3DApp::CreateDSV() {
	D3D12_RESOURCE_DESC dsvResourceDesc;// ����ģ������
	// ��CPU�д��������ģ��������Դ
	// ����������Դ
	dsvResourceDesc.Alignment = 0;	// ָ������
	dsvResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	// ָ����Դά�ȣ����ͣ�ΪTEXTURE2D
	dsvResourceDesc.DepthOrArraySize = 1;	// �������Ϊ1
	dsvResourceDesc.Width = graphicsWindowWidth;	// ��Դ��
	dsvResourceDesc.Height = graphicsWindowHeight;	// ��Դ��
	dsvResourceDesc.MipLevels = 1;	// MIPMAP�㼶����
	dsvResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;	// ָ�������֣����ﲻָ����
	dsvResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	// ���ģ����Դ��Flag
	dsvResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// 24λ��ȣ�8λģ��,���и������͵ĸ�ʽDXGI_FORMAT_R24G8_TYPELESSҲ����ʹ��
	dsvResourceDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;	// ���ز�������
	dsvResourceDesc.SampleDesc.Quality = msaaQualityLevels.NumQualityLevels - 1;	// ���ز�������
	// ���������Դ���Ż�ֵ������
	CD3DX12_CLEAR_VALUE optClear;	// �����Դ���Ż�ֵ��������������ִ���ٶȣ�CreateCommittedResource�����д��룩
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;// 24λ��ȣ�8λģ��,���и������͵ĸ�ʽDXGI_FORMAT_R24G8_TYPELESSҲ����ʹ��
	optClear.DepthStencil.Depth = 1;	// ��ʼ���ֵΪ1
	optClear.DepthStencil.Stencil = 0;	// ��ʼģ��ֵΪ0
	// ����һ����Դ��һ���ѣ�������Դ�ύ�����У������ģ�������ύ��GPU�Դ��У�
	CD3DX12_HEAP_PROPERTIES hpp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);// �������е����ԣ�����ΪĬ�϶ѡ�
	ThrowIfFailed(d3dDevice->CreateCommittedResource(&hpp,	// ������ΪĬ�϶ѣ�����д�룩
		D3D12_HEAP_FLAG_NONE,	// Flag
		&dsvResourceDesc,	// ���涨���DSV��Դָ��
		D3D12_RESOURCE_STATE_COMMON,	// ��Դ��״̬Ϊ��ʼ״̬
		&optClear,	// ���涨����Ż�ֵָ��
		IID_PPV_ARGS(&depthStencilBuffer1)));	// �������ģ����Դ
	ThrowIfFailed(d3dDevice->CreateCommittedResource(&hpp,	D3D12_HEAP_FLAG_NONE,	
		&dsvResourceDesc,	D3D12_RESOURCE_STATE_COMMON,	&optClear,
		IID_PPV_ARGS(&depthStencilBuffer2)));

	d3dDevice->CreateDepthStencilView(depthStencilBuffer1.Get(),
		nullptr,	// D3D12_DEPTH_STENCIL_VIEW_DESC����ָ�룬����&dsvDesc������ע�ʹ��룩��
							// �����ڴ������ģ����Դʱ�Ѿ��������ģ���������ԣ������������ָ��Ϊ��ָ��
		dsvHeap1->GetCPUDescriptorHandleForHeapStart());	// DSV���
	d3dDevice->CreateDepthStencilView(depthStencilBuffer2.Get(),
		nullptr,	// D3D12_DEPTH_STENCIL_VIEW_DESC����ָ�룬����&dsvDesc������ע�ʹ��룩��
							// �����ڴ������ģ����Դʱ�Ѿ��������ģ���������ԣ������������ָ��Ϊ��ָ��
		dsvHeap2->GetCPUDescriptorHandleForHeapStart());	// DSV���
	// ����Դ�ӳ�ʼ״̬ת������Ȼ���״̬
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(depthStencilBuffer1.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(depthStencilBuffer2.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));
}

// ������/ģ����Դ��״̬
//void D3DApp::resourceBarrierBuild() {
//	cmdList->ResourceBarrier(1,	// Barrier���ϸ���
//		&CD3DX12_RESOURCE_BARRIER::Transition(depthStencilBuffer.Get(),// ��ͼ��COM�ӿ�(?)��ָ��֮ǰ�����/ģ�建�����Ľӿڡ�
//			D3D12_RESOURCE_STATE_COMMON,	// ת��ǰ״̬������ʱ��״̬����CreateCommittedResource�����ж����״̬��
//			D3D12_RESOURCE_STATE_DEPTH_WRITE));	// ת����״̬Ϊ��д������ͼ������һ��D3D12_RESOURCE_STATE_DEPTH_READ��ֻ�ɶ������ͼ
//	// �������������cmdList�󣬽�����������б���������У�Ҳ���Ǵ�CPU����GPU�Ĺ��̡�
//	// ע�⣺�ڴ����������ǰ����ر������б�
//	ThrowIfFailed(cmdList->Close());	// ������������ر�
//	ID3D12CommandList* cmdLists[] = { cmdList.Get() };	// ���������������б�����
//	cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);	// ������������б����������
//}

// ʵ��Χ��
void D3DApp::FlushCmdQueue() {
	mCurrentFence++;	// CPU��������رպ󣬽���ǰΧ��ֵ+1
	cmdQueue->Signal(fence.Get(), mCurrentFence);	// ��GPU������CPU���������󣬽�fence�ӿ��е�Χ��ֵ+1����fence->GetCompletedValue()+1
	if (fence->GetCompletedValue() < mCurrentFence)	// ���С�ڣ�˵��GPUû�д�������������
	{
		HANDLE eventHandle = CreateEvent(nullptr, false, false, L"FenceSetDone");	// �����¼�
		fence->SetEventOnCompletion(mCurrentFence, eventHandle);// ��Χ���ﵽmCurrentFenceֵ����ִ�е�Signal����ָ���޸���Χ��ֵ��ʱ������eventHandle�¼�
		WaitForSingleObject(eventHandle, INFINITE);// �ȴ�GPU����Χ���������¼���������ǰ�߳�ֱ���¼�������ע���Enent���������ٵȴ���
							   // ���û��Set��Wait���������ˣ�Set��Զ������ã�����Ҳ��û�߳̿��Ի�������̣߳�
		CloseHandle(eventHandle);
	}
}

// �����ӿںͲü�����
void D3DApp::CreateViewPortAndScissorRect() {
	// �ӿ�����
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = static_cast<float>(graphicsWindowWidth);
	viewPort.Height = static_cast<float>(graphicsWindowHeight);
	viewPort.MaxDepth = 1.0f;
	viewPort.MinDepth = 0.0f;
	// �ü��������ã�����������ض������޳���
	// ǰ����Ϊ���ϵ����꣬������Ϊ���µ�����
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = graphicsWindowWidth;
	scissorRect.bottom = graphicsWindowHeight;
}

// ����֡�ʺ�ÿ֡���ٺ���
void D3DApp::CalculateFrameState() {
	using namespace _GameTimer;
	static int frameCnt = 0;// ��֡��
	static float timeElapsed = 0.0f;//��ʱ��
	frameCnt++;
	if (gt.TotalTime() - timeElapsed >= 1.0f) {
		float fps = frameCnt;// ÿ�����֡
		float mspf = 1000.0f / fps;// ÿ֡���ٺ���

		std::wstring fpsStr = std::to_wstring(fps);
		std::wstring mspfStr = std::to_wstring(mspf);
		// ��֡����ʾ�ڴ�����
		std::wstring windowText = L"D3DApp   fps: " + fpsStr + L"    " + L"mspf: " + mspfStr;
		SetWindowText(graphicsWnd1, windowText.c_str());

		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

//���ڳߴ�ı��¼�
void D3DApp::OnResize() {
	assert(d3dDevice);
	assert(swapChain1);
	assert(swapChain2);
	assert(cmdAllocator); //�ǿ�ָ�룬����ͨ��assert

	// �ı���Դǰ��ͬ��
	FlushCmdQueue();
	ThrowIfFailed(cmdList->Reset(cmdAllocator.Get(), nullptr));

	//�ͷ�֮ǰ����Դ��Ϊ�������´�������׼��
	for (int i = 0; i < 2; i++) // SwapChainBufferCount = 2
	{
		swapChainBuffer1[i].Reset();
		swapChainBuffer2[i].Reset();
	}
	depthStencilBuffer1.Reset();
	depthStencilBuffer2.Reset();

	// ���µ�����̨��������Դ�Ĵ�С
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

	// ��̨��������������
	mCurrentBackBuffer = 0;

	CreateRTV();
	CreateDSV();
	// ִ��Resize����
	ThrowIfFailed(cmdList->Close());
	ID3D12CommandList* cmdsLists[] = { cmdList.Get() };
	cmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// �ȴ�Resize�������
	FlushCmdQueue();

	// �����ӿ�/ͶӰ������Ӧ���ڱ任
	CreateViewPortAndScissorRect();
}

void D3DApp::SetWindowInformation(int clientWidth, int clientHeight)
{
	graphicsWindowWidth = clientWidth / 5 * 2;
	graphicsWindowHeight = clientHeight;
	graphicsPosX = clientWidth / 5 ;
	graphicsPosY = 0;
}
