#include "BoxApp.h"
#include <Winuser.h>

#define IDM_INSTRUCTION 0

HINSTANCE hInst;
int showCmd;

BoxApp theApp;

HWND MainWnd = 0;// 主窗口句柄
HWND buttonWnd = 0; //按钮窗口
HWND hwndButton[4];  //按钮
HWND textWnd = 0; //文本窗口
HMENU menuWnd = 0; //菜单句柄

//各窗口的宽、高
int clientWidth = 1280;
int clientHeight = 720;
int buttonWindowWidth = clientWidth / 4;
int buttonWindowHeight = clientHeight / 2;
int textWindowWidth = clientWidth / 4;
int textWindowHeight = clientHeight / 2;

static int j = 0;

void CreateMyMenu();
void InitHwndChild(HWND hwnd);
void InitButton();
void OnResize(int clientWidth, int clientHeight);
LRESULT CALLBACK MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MsgProc1(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void SetHandCursor()
{
	SetClassLongPtr(hwndButton[0], GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_HAND));
	SetClassLongPtr(buttonWnd, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_HAND));
	SetClassLongPtr(theApp.graphicsWnd1, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_HAND));
	SetClassLongPtr(theApp.graphicsWnd2, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_HAND));
}
void ResetCursor()
{
	SetClassLongPtr(hwndButton[0], GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
	SetClassLongPtr(buttonWnd, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
	SetClassLongPtr(theApp.graphicsWnd1, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
	SetClassLongPtr(theApp.graphicsWnd2, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
}

//按钮消息窗口
LRESULT CALLBACK MsgProc1(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	// 消息处理
	switch (msg) {
	//禁止拖动窗口
	case WM_NCLBUTTONDOWN:
	{
		theApp.createWhichGrapgic = 0;
		ResetCursor();

		switch (wParam)
		{
		case HTCAPTION: // HTCAPTION 在标题条中。
			return 0;

		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case 101:
			SetHandCursor();
			theApp.createWhichGrapgic = 1; //1对应正方体；
			//SetWindowText(hwndButton[0], L"请放置图形");
			break;
		case 102:
			SetHandCursor();
			theApp.createWhichGrapgic = 2; //2对应球体；
			break;
		case 103:
			SetHandCursor();
			theApp.createWhichGrapgic = 3; //3对应平面；
			break;
		case 104:
			SetHandCursor();
			theApp.createWhichGrapgic = 4; //4对应柱体；
			break;
		default:
			break;
		}
		theApp.hasCreateGrapgic = false;
	}

	case WM_MOUSEMOVE:
		if (theApp.hasCreateGrapgic) ResetCursor();
		break;
	default:
		break;
	// 将上面没有处理的消息转发给默认的窗口过程
	
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
	
}
//文本消息窗口
LRESULT CALLBACK MsgProc2(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;  //存储绘图环境的相关信息
	RECT rect;

	wstring szText = theApp.InformationText;
	LPCWSTR str = szText.c_str();
	// 消息处理
	switch (msg) {
		//禁止拖动窗口
	case WM_NCLBUTTONDOWN:
	{
		theApp.createWhichGrapgic = 0;
		ResetCursor();

		switch (wParam)
		{
		case HTCAPTION: // HTCAPTION 在标题条中。
			return 0;

		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}
	case WM_PAINT:
	{
			//开始绘图并返回环境句柄
			hdc = BeginPaint(hwnd, &ps);
			GetClientRect(hwnd, &rect);
			//输出文字
			DrawText(hdc,str,-1,&rect, DT_LEFT);
			//结束绘图并释放环境句柄
			EndPaint(hwnd, &ps);
		
		return 0;
	}
	case WM_USER + 1:
		//向WM_PAINT发送消息
		GetClientRect(hwnd, &rect);//设置要更新的区域，可以是自己给定的
		UpdateWindow(hwnd);
		RedrawWindow(hwnd, &rect, nullptr, RDW_INVALIDATE);// RDW_INTERNALPAINT: 即使窗口并非无效，也向其投递一条WM_PAINT消息
		break;
	case WM_MOUSEMOVE:
		if (theApp.hasCreateGrapgic) ResetCursor();
		break;
	default:
		break;
		// 将上面没有处理的消息转发给默认的窗口过程

	}
	return DefWindowProc(hwnd, msg, wParam, lParam);

}

// 窗口过程
LRESULT CALLBACK MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	
	int lastWidth = clientWidth;
	int lastHeight = clientHeight;
	// 消息处理
	switch (msg) {

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDM_INSTRUCTION:
			MessageBox(0, L"左键：点击按钮生成对应图形，\n       点击图形显示信息，\n       拖动屏幕变换视角；\n\n右键：拖动屏幕缩放视角", L"操作说明", 0);
			break;
		}
		break;
	}
	case WM_CREATE:
		InitHwndChild(hwnd);
		break;
	case WM_SIZE:
		clientWidth = LOWORD(lParam);
		clientHeight = HIWORD(lParam);

		//不加这两个会报错
		if (wParam == SIZE_MINIMIZED) {
			ShowWindow(theApp.graphicsWnd1, SW_SHOWMINIMIZED);
			ShowWindow(theApp.graphicsWnd2, SW_SHOWMINIMIZED);
		}
		if (wParam == SIZE_RESTORED)
		{
			if (theApp.mMinimized)
			{
				ShowWindow(theApp.graphicsWnd1, SW_SHOWMAXIMIZED);
				ShowWindow(theApp.graphicsWnd2, SW_SHOWMAXIMIZED);
			}
				
		}

		if (lastHeight != clientHeight || lastWidth != clientWidth)
			OnResize(clientWidth, clientHeight);
		
		break;

	case WM_DESTROY:
		PostQuitMessage(0);	// 终止消息循环，并发出WM_QUIT消息
		return 0;
		
	default:
		break;
	}
	// 将上面没有处理的消息转发给默认的窗口过程
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void CreateMyMenu()
{
	menuWnd = CreateMenu();
	if (!menuWnd) return;

	HMENU pop_instruction = CreatePopupMenu();
	AppendMenu(menuWnd, MF_POPUP, IDM_INSTRUCTION, L"说明");
}

void InitHwndChild(HWND hwnd)
{
	// 窗口初始化描述结构体(WNDCLASS)
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;	// 当工作区宽高改变，则重新绘制窗口
	wc.lpfnWndProc = MsgProc1;	// 指定窗口过程(一个窗口过程只能注册一个结构体)
	wc.cbClsExtra = 0;	// 借助这两个字段来为当前应用分配额外的内存空间（这里不分配，所以置0）
	wc.cbWndExtra = 0;	// 借助这两个字段来为当前应用分配额外的内存空间（这里不分配，所以置0）
	wc.hInstance = hInst;	// 应用程序实例句柄（由WinMain传入）
	wc.hIcon = LoadIcon(0, IDC_ARROW);	//使用默认的应用程序图标
	wc.hCursor = LoadCursor(0, IDC_ARROW);	//使用标准的鼠标指针样式
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	//指定了白色背景画刷句柄
	wc.lpszMenuName = 0;	//没有菜单栏
	wc.lpszClassName = L"StaticWnd";	//窗口名
	// 窗口类注册失败
	if (!RegisterClass(&wc)) {
		MessageBox(0, L"RegisterClass Failed", 0, 0);
		return ;
	}

	// 窗口初始化描述结构体(WNDCLASS)
	WNDCLASS wc1;
	wc1.style = CS_HREDRAW | CS_VREDRAW;	// 当工作区宽高改变，则重新绘制窗口
	wc1.lpfnWndProc = MsgProc2;	// 指定窗口过程(一个窗口过程只能注册一个结构体)
	wc1.cbClsExtra = 0;	// 借助这两个字段来为当前应用分配额外的内存空间（这里不分配，所以置0）
	wc1.cbWndExtra = 0;	// 借助这两个字段来为当前应用分配额外的内存空间（这里不分配，所以置0）
	wc1.hInstance = hInst;	// 应用程序实例句柄（由WinMain传入）
	wc1.hIcon = LoadIcon(0, IDC_ARROW);	//使用默认的应用程序图标
	wc1.hCursor = LoadCursor(0, IDC_ARROW);	//使用标准的鼠标指针样式
	wc1.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	//指定了白色背景画刷句柄
	wc1.lpszMenuName = 0;	//没有菜单栏
	wc1.lpszClassName = L"TextWnd";	//窗口名
	// 窗口类注册失败
	if (!RegisterClass(&wc1)) {
		MessageBox(0, L"RegisterClass Failed", 0, 0);
		return;
	}

	//创建按钮窗口
	buttonWnd = CreateWindow(
		L"StaticWnd", L"按钮窗口",
		WS_CHILD | WS_VISIBLE | WS_CAPTION,
		0, 0, buttonWindowWidth, buttonWindowHeight, hwnd, (HMENU)100, hInst, 0);
	if (!buttonWnd)
		MessageBox(NULL, L"创建按钮窗口失败", 0, 0);

	InitButton();

	//创建文本窗口
	textWnd = CreateWindow(L"TextWnd", L"图形信息",
		WS_CHILD | WS_VISIBLE | WS_CAPTION,
		0, textWindowHeight, textWindowWidth, textWindowHeight, hwnd, (HMENU)200, hInst, 0);
	if (!textWnd)
		MessageBox(NULL, L"创建文本窗口失败", 0, 0);
}

void InitButton()
{
	hwndButton[0] = CreateWindow(L"button",         /*子窗口类名称*/
			L"正方体",							/*按钮上的文字*/
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,     /*按钮类型*/
			buttonWindowWidth / 3, buttonWindowHeight / 8, buttonWindowWidth / 3,  buttonWindowHeight / 8,   /*位置及大小*/
			buttonWnd,                                 /*父窗口句柄*/
			(HMENU)101,                                  /*按钮ID,一个自定义整型常量*/
			hInst, NULL);

	hwndButton[1] = CreateWindow(L"button",       /*子窗口类名称*/
			L"球体",									   /*按钮上的文字*/
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,     /*按钮类型*/
			buttonWindowWidth / 3, buttonWindowHeight / 8 * 2, buttonWindowWidth / 3, buttonWindowHeight / 8,   /*位置及大小*/
			buttonWnd,                                 /*父窗口句柄*/
			(HMENU)102,                                  /*按钮ID,一个自定义整型常量*/
			hInst, NULL);

	hwndButton[2] = CreateWindow(L"button",       /*子窗口类名称*/
		L"平面",									   /*按钮上的文字*/
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,     /*按钮类型*/
		buttonWindowWidth / 3, buttonWindowHeight / 8 * 3, buttonWindowWidth / 3, buttonWindowHeight / 8,   /*位置及大小*/
		buttonWnd,                                 /*父窗口句柄*/
		(HMENU)103,                                  /*按钮ID,一个自定义整型常量*/
		hInst, NULL);

	hwndButton[3] = CreateWindow(L"button",       /*子窗口类名称*/
		L"柱体",									   /*按钮上的文字*/
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,     /*按钮类型*/
		buttonWindowWidth / 3, buttonWindowHeight / 8 * 4, buttonWindowWidth / 3, buttonWindowHeight / 8,   /*位置及大小*/
		buttonWnd,                                 /*父窗口句柄*/
		(HMENU)104,                                  /*按钮ID,一个自定义整型常量*/
		hInst, NULL);

	for (int i = 0; i < 4; i++)
	{
		if (!hwndButton[i]) MessageBox(NULL, L"创建按钮失败", 0, 0);
	}
}

bool InitMainUI(HINSTANCE hInstance, int nShowCmd) {
	//创建菜单
	CreateMyMenu();

	// 窗口初始化描述结构体(WNDCLASS)
	WNDCLASS wc ;
	wc.style = CS_HREDRAW | CS_VREDRAW;	// 当工作区宽高改变，则重新绘制窗口
	wc.lpfnWndProc = MsgProc;	// 指定窗口过程
	wc.cbClsExtra = 0;	// 借助这两个字段来为当前应用分配额外的内存空间（这里不分配，所以置0）
	wc.cbWndExtra = 0;	// 借助这两个字段来为当前应用分配额外的内存空间（这里不分配，所以置0）
	wc.hInstance = hInstance;	// 应用程序实例句柄（由WinMain传入）
	wc.hIcon = LoadIcon(0, IDC_ARROW);	//使用默认的应用程序图标
	wc.hCursor = LoadCursor(0, IDC_ARROW);	//使用标准的鼠标指针样式
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	//指定了白色背景画刷句柄
	wc.lpszMenuName = 0;	//没有菜单栏
	wc.lpszClassName = L"MainWnd";	//窗口名
	// 窗口类注册失败
	if (!RegisterClass(&wc)) {
		MessageBox(0, L"RegisterClass Failed", 0, 0);
		return 0;
	}

	hInst = hInstance;
	showCmd = nShowCmd;

	// 窗口类注册成功
	RECT R = { 0, 0, clientWidth, clientHeight };	// 裁剪矩形
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);	//根据窗口的客户区大小计算窗口的大小
	int width = R.right - R.left;
	int hight = R.bottom - R.top;

	// 创建窗口,返回布尔值
	MainWnd = CreateWindow(L"MainWnd", L"DX12",
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, // & ~WS_MAXIMIZEBOX & ~WS_MINIMIZEBOX & ~WS_THICKFRAME,  //不能缩放窗口
		200, 100, width, hight, 0, menuWnd, hInstance, 0);
	//LoadMenu(hInstance, L"菜单");
	// 窗口创建失败
	if (!MainWnd) {
		MessageBox(0, L"CreatWindow Failed", 0, 0);
		return 0;
	}
	// 窗口创建成功,则显示并更新窗口
	ShowWindow(MainWnd, nShowCmd);
	UpdateWindow(MainWnd);

	return true;
}

int Run(HINSTANCE hInstance, int nShowCmd, HWND hwnd) {
	// 定义消息结构体
	MSG msg = { 0 };
	// 如果PeekMessage函数不等于0，说明没有接受到WM_QUIT
	while (msg.message != WM_QUIT) {

		// 如果有窗口消息就进行处理
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) // PeekMessage函数会自动填充msg结构体元素
		{
			TranslateMessage(&msg);	// 键盘按键转换，将虚拟键消息转换为字符消息
			DispatchMessage(&msg);	// 把消息分派给相应的窗口过程
		}
		else {
			if (j == 0)
			{
				
				theApp.SetWindowInformation(clientWidth, clientHeight);
				if (!theApp.Init(hInstance, nShowCmd, hwnd))
					return 0;
				OnResize(clientWidth, clientHeight);
				return theApp.Run(textWnd);

				j++;
			}
		}
		

	}
	return (int)msg.wParam;
}

void OnResize(int clientWidth, int clientHeight)
{
	buttonWindowWidth = clientWidth / 5;
	buttonWindowHeight = clientHeight / 2;
	textWindowWidth = clientWidth / 5;
	textWindowHeight = clientHeight / 2;

	MoveWindow(buttonWnd, 0, 0, buttonWindowWidth, buttonWindowHeight, true);

	for (int i = 0; i < 4; i++)
	{
		MoveWindow(hwndButton[i], buttonWindowWidth / 3, buttonWindowHeight / 8 * (i + 1),
			buttonWindowWidth / 3, buttonWindowHeight / 8, true);
	}

	MoveWindow(textWnd, 0, buttonWindowHeight, textWindowWidth, textWindowHeight, true);
	MoveWindow(theApp.graphicsWnd1, buttonWindowWidth, 0, clientWidth / 5 * 2, clientHeight, true);
	MoveWindow(theApp.graphicsWnd2, buttonWindowWidth+clientWidth / 5 * 2, 0, clientWidth / 5 * 2, clientHeight, true);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd) {
    //针对调试版本开启运行时内存检测
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif 
    try
    {
		MSG msg;
		if (!InitMainUI(hInstance, showCmd)) {
			return 0;
		}
		return Run(hInstance, showCmd, MainWnd);
    }
    catch (DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }

}