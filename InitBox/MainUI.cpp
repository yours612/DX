#include "BoxApp.h"
#include <Winuser.h>

#define IDM_INSTRUCTION 0

HINSTANCE hInst;
int showCmd;

BoxApp theApp;

HWND MainWnd = 0;// �����ھ��
HWND buttonWnd = 0; //��ť����
HWND hwndButton[4];  //��ť
HWND textWnd = 0; //�ı�����
HMENU menuWnd = 0; //�˵����

//�����ڵĿ���
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

//��ť��Ϣ����
LRESULT CALLBACK MsgProc1(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	// ��Ϣ����
	switch (msg) {
	//��ֹ�϶�����
	case WM_NCLBUTTONDOWN:
	{
		theApp.createWhichGrapgic = 0;
		ResetCursor();

		switch (wParam)
		{
		case HTCAPTION: // HTCAPTION �ڱ������С�
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
			theApp.createWhichGrapgic = 1; //1��Ӧ�����壻
			//SetWindowText(hwndButton[0], L"�����ͼ��");
			break;
		case 102:
			SetHandCursor();
			theApp.createWhichGrapgic = 2; //2��Ӧ���壻
			break;
		case 103:
			SetHandCursor();
			theApp.createWhichGrapgic = 3; //3��Ӧƽ�棻
			break;
		case 104:
			SetHandCursor();
			theApp.createWhichGrapgic = 4; //4��Ӧ���壻
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
	// ������û�д������Ϣת����Ĭ�ϵĴ��ڹ���
	
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
	
}
//�ı���Ϣ����
LRESULT CALLBACK MsgProc2(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;  //�洢��ͼ�����������Ϣ
	RECT rect;

	wstring szText = theApp.InformationText;
	LPCWSTR str = szText.c_str();
	// ��Ϣ����
	switch (msg) {
		//��ֹ�϶�����
	case WM_NCLBUTTONDOWN:
	{
		theApp.createWhichGrapgic = 0;
		ResetCursor();

		switch (wParam)
		{
		case HTCAPTION: // HTCAPTION �ڱ������С�
			return 0;

		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}
	case WM_PAINT:
	{
			//��ʼ��ͼ�����ػ������
			hdc = BeginPaint(hwnd, &ps);
			GetClientRect(hwnd, &rect);
			//�������
			DrawText(hdc,str,-1,&rect, DT_LEFT);
			//������ͼ���ͷŻ������
			EndPaint(hwnd, &ps);
		
		return 0;
	}
	case WM_USER + 1:
		//��WM_PAINT������Ϣ
		GetClientRect(hwnd, &rect);//����Ҫ���µ����򣬿������Լ�������
		UpdateWindow(hwnd);
		RedrawWindow(hwnd, &rect, nullptr, RDW_INVALIDATE);// RDW_INTERNALPAINT: ��ʹ���ڲ�����Ч��Ҳ����Ͷ��һ��WM_PAINT��Ϣ
		break;
	case WM_MOUSEMOVE:
		if (theApp.hasCreateGrapgic) ResetCursor();
		break;
	default:
		break;
		// ������û�д������Ϣת����Ĭ�ϵĴ��ڹ���

	}
	return DefWindowProc(hwnd, msg, wParam, lParam);

}

// ���ڹ���
LRESULT CALLBACK MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	
	int lastWidth = clientWidth;
	int lastHeight = clientHeight;
	// ��Ϣ����
	switch (msg) {

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDM_INSTRUCTION:
			MessageBox(0, L"����������ť���ɶ�Ӧͼ�Σ�\n       ���ͼ����ʾ��Ϣ��\n       �϶���Ļ�任�ӽǣ�\n\n�Ҽ����϶���Ļ�����ӽ�", L"����˵��", 0);
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

		//�����������ᱨ��
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
		PostQuitMessage(0);	// ��ֹ��Ϣѭ����������WM_QUIT��Ϣ
		return 0;
		
	default:
		break;
	}
	// ������û�д������Ϣת����Ĭ�ϵĴ��ڹ���
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void CreateMyMenu()
{
	menuWnd = CreateMenu();
	if (!menuWnd) return;

	HMENU pop_instruction = CreatePopupMenu();
	AppendMenu(menuWnd, MF_POPUP, IDM_INSTRUCTION, L"˵��");
}

void InitHwndChild(HWND hwnd)
{
	// ���ڳ�ʼ�������ṹ��(WNDCLASS)
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;	// ����������߸ı䣬�����»��ƴ���
	wc.lpfnWndProc = MsgProc1;	// ָ�����ڹ���(һ�����ڹ���ֻ��ע��һ���ṹ��)
	wc.cbClsExtra = 0;	// �����������ֶ���Ϊ��ǰӦ�÷��������ڴ�ռ䣨���ﲻ���䣬������0��
	wc.cbWndExtra = 0;	// �����������ֶ���Ϊ��ǰӦ�÷��������ڴ�ռ䣨���ﲻ���䣬������0��
	wc.hInstance = hInst;	// Ӧ�ó���ʵ���������WinMain���룩
	wc.hIcon = LoadIcon(0, IDC_ARROW);	//ʹ��Ĭ�ϵ�Ӧ�ó���ͼ��
	wc.hCursor = LoadCursor(0, IDC_ARROW);	//ʹ�ñ�׼�����ָ����ʽ
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	//ָ���˰�ɫ������ˢ���
	wc.lpszMenuName = 0;	//û�в˵���
	wc.lpszClassName = L"StaticWnd";	//������
	// ������ע��ʧ��
	if (!RegisterClass(&wc)) {
		MessageBox(0, L"RegisterClass Failed", 0, 0);
		return ;
	}

	// ���ڳ�ʼ�������ṹ��(WNDCLASS)
	WNDCLASS wc1;
	wc1.style = CS_HREDRAW | CS_VREDRAW;	// ����������߸ı䣬�����»��ƴ���
	wc1.lpfnWndProc = MsgProc2;	// ָ�����ڹ���(һ�����ڹ���ֻ��ע��һ���ṹ��)
	wc1.cbClsExtra = 0;	// �����������ֶ���Ϊ��ǰӦ�÷��������ڴ�ռ䣨���ﲻ���䣬������0��
	wc1.cbWndExtra = 0;	// �����������ֶ���Ϊ��ǰӦ�÷��������ڴ�ռ䣨���ﲻ���䣬������0��
	wc1.hInstance = hInst;	// Ӧ�ó���ʵ���������WinMain���룩
	wc1.hIcon = LoadIcon(0, IDC_ARROW);	//ʹ��Ĭ�ϵ�Ӧ�ó���ͼ��
	wc1.hCursor = LoadCursor(0, IDC_ARROW);	//ʹ�ñ�׼�����ָ����ʽ
	wc1.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	//ָ���˰�ɫ������ˢ���
	wc1.lpszMenuName = 0;	//û�в˵���
	wc1.lpszClassName = L"TextWnd";	//������
	// ������ע��ʧ��
	if (!RegisterClass(&wc1)) {
		MessageBox(0, L"RegisterClass Failed", 0, 0);
		return;
	}

	//������ť����
	buttonWnd = CreateWindow(
		L"StaticWnd", L"��ť����",
		WS_CHILD | WS_VISIBLE | WS_CAPTION,
		0, 0, buttonWindowWidth, buttonWindowHeight, hwnd, (HMENU)100, hInst, 0);
	if (!buttonWnd)
		MessageBox(NULL, L"������ť����ʧ��", 0, 0);

	InitButton();

	//�����ı�����
	textWnd = CreateWindow(L"TextWnd", L"ͼ����Ϣ",
		WS_CHILD | WS_VISIBLE | WS_CAPTION,
		0, textWindowHeight, textWindowWidth, textWindowHeight, hwnd, (HMENU)200, hInst, 0);
	if (!textWnd)
		MessageBox(NULL, L"�����ı�����ʧ��", 0, 0);
}

void InitButton()
{
	hwndButton[0] = CreateWindow(L"button",         /*�Ӵ���������*/
			L"������",							/*��ť�ϵ�����*/
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,     /*��ť����*/
			buttonWindowWidth / 3, buttonWindowHeight / 8, buttonWindowWidth / 3,  buttonWindowHeight / 8,   /*λ�ü���С*/
			buttonWnd,                                 /*�����ھ��*/
			(HMENU)101,                                  /*��ťID,һ���Զ������ͳ���*/
			hInst, NULL);

	hwndButton[1] = CreateWindow(L"button",       /*�Ӵ���������*/
			L"����",									   /*��ť�ϵ�����*/
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,     /*��ť����*/
			buttonWindowWidth / 3, buttonWindowHeight / 8 * 2, buttonWindowWidth / 3, buttonWindowHeight / 8,   /*λ�ü���С*/
			buttonWnd,                                 /*�����ھ��*/
			(HMENU)102,                                  /*��ťID,һ���Զ������ͳ���*/
			hInst, NULL);

	hwndButton[2] = CreateWindow(L"button",       /*�Ӵ���������*/
		L"ƽ��",									   /*��ť�ϵ�����*/
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,     /*��ť����*/
		buttonWindowWidth / 3, buttonWindowHeight / 8 * 3, buttonWindowWidth / 3, buttonWindowHeight / 8,   /*λ�ü���С*/
		buttonWnd,                                 /*�����ھ��*/
		(HMENU)103,                                  /*��ťID,һ���Զ������ͳ���*/
		hInst, NULL);

	hwndButton[3] = CreateWindow(L"button",       /*�Ӵ���������*/
		L"����",									   /*��ť�ϵ�����*/
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,     /*��ť����*/
		buttonWindowWidth / 3, buttonWindowHeight / 8 * 4, buttonWindowWidth / 3, buttonWindowHeight / 8,   /*λ�ü���С*/
		buttonWnd,                                 /*�����ھ��*/
		(HMENU)104,                                  /*��ťID,һ���Զ������ͳ���*/
		hInst, NULL);

	for (int i = 0; i < 4; i++)
	{
		if (!hwndButton[i]) MessageBox(NULL, L"������ťʧ��", 0, 0);
	}
}

bool InitMainUI(HINSTANCE hInstance, int nShowCmd) {
	//�����˵�
	CreateMyMenu();

	// ���ڳ�ʼ�������ṹ��(WNDCLASS)
	WNDCLASS wc ;
	wc.style = CS_HREDRAW | CS_VREDRAW;	// ����������߸ı䣬�����»��ƴ���
	wc.lpfnWndProc = MsgProc;	// ָ�����ڹ���
	wc.cbClsExtra = 0;	// �����������ֶ���Ϊ��ǰӦ�÷��������ڴ�ռ䣨���ﲻ���䣬������0��
	wc.cbWndExtra = 0;	// �����������ֶ���Ϊ��ǰӦ�÷��������ڴ�ռ䣨���ﲻ���䣬������0��
	wc.hInstance = hInstance;	// Ӧ�ó���ʵ���������WinMain���룩
	wc.hIcon = LoadIcon(0, IDC_ARROW);	//ʹ��Ĭ�ϵ�Ӧ�ó���ͼ��
	wc.hCursor = LoadCursor(0, IDC_ARROW);	//ʹ�ñ�׼�����ָ����ʽ
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	//ָ���˰�ɫ������ˢ���
	wc.lpszMenuName = 0;	//û�в˵���
	wc.lpszClassName = L"MainWnd";	//������
	// ������ע��ʧ��
	if (!RegisterClass(&wc)) {
		MessageBox(0, L"RegisterClass Failed", 0, 0);
		return 0;
	}

	hInst = hInstance;
	showCmd = nShowCmd;

	// ������ע��ɹ�
	RECT R = { 0, 0, clientWidth, clientHeight };	// �ü�����
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);	//���ݴ��ڵĿͻ�����С���㴰�ڵĴ�С
	int width = R.right - R.left;
	int hight = R.bottom - R.top;

	// ��������,���ز���ֵ
	MainWnd = CreateWindow(L"MainWnd", L"DX12",
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, // & ~WS_MAXIMIZEBOX & ~WS_MINIMIZEBOX & ~WS_THICKFRAME,  //�������Ŵ���
		200, 100, width, hight, 0, menuWnd, hInstance, 0);
	//LoadMenu(hInstance, L"�˵�");
	// ���ڴ���ʧ��
	if (!MainWnd) {
		MessageBox(0, L"CreatWindow Failed", 0, 0);
		return 0;
	}
	// ���ڴ����ɹ�,����ʾ�����´���
	ShowWindow(MainWnd, nShowCmd);
	UpdateWindow(MainWnd);

	return true;
}

int Run(HINSTANCE hInstance, int nShowCmd, HWND hwnd) {
	// ������Ϣ�ṹ��
	MSG msg = { 0 };
	// ���PeekMessage����������0��˵��û�н��ܵ�WM_QUIT
	while (msg.message != WM_QUIT) {

		// ����д�����Ϣ�ͽ��д���
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) // PeekMessage�������Զ����msg�ṹ��Ԫ��
		{
			TranslateMessage(&msg);	// ���̰���ת�������������Ϣת��Ϊ�ַ���Ϣ
			DispatchMessage(&msg);	// ����Ϣ���ɸ���Ӧ�Ĵ��ڹ���
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
    //��Ե��԰汾��������ʱ�ڴ���
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