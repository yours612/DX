//#include "InitD3D12App.h"
//
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int nShowCmd) {
//#if defined(DEBUG) | defined(_DEBUG)
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//#endif
//	try {
//		InitD3D12App theApp;
//		if (!theApp.Init(hInstance, nShowCmd))
//			return 0;
//
//		return theApp.Run();
//	}
//	catch (DxException& e) {
//		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
//		return 0;
//	}
//}
//
//InitD3D12App::InitD3D12App() {
//
//}
//
//InitD3D12App::~InitD3D12App() {
//
//}
//
//bool InitD3D12App::Init(HINSTANCE hInstance, int nShowCmd) {
//	if (!D3DApp::Init(hInstance, nShowCmd))
//		return false;
//	return true;
//}
//
//void InitD3D12App::Draw() {
//	ThrowIfFailed(cmdAllocator->Reset());// �ظ�ʹ�ü�¼���������ڴ�
//	ThrowIfFailed(cmdList->Reset(cmdAllocator.Get(), nullptr));// ���������б����ڴ�
//	// �������ǽ���̨������Դ�ӳ���״̬ת������ȾĿ��״̬����׼������ͼ����Ⱦ��
//	UINT& ref_mCurrentBackBuffer = mCurrentBackBuffer;
//	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer[ref_mCurrentBackBuffer].Get(),// ת����ԴΪ��̨��������Դ
//		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));// �ӳ��ֵ���ȾĿ��ת��
//
//   // �����������ӿںͲü����Ρ�
//	cmdList->RSSetViewports(1, &viewPort);
//	cmdList->RSSetScissorRects(1, &scissorRect);
//
//	// Ȼ�������̨����������Ȼ�����������ֵ��
//	// �������Ȼ�ö������������������ַ����
//	// ��ͨ��ClearRenderTargetView������ClearDepthStencilView����������͸�ֵ��
//	// �������ǽ�RT��Դ����ɫ��ֵΪLightSteelBlue��
//	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap->GetCPUDescriptorHandleForHeapStart(), ref_mCurrentBackBuffer, rtvDescriptorSize);
//	cmdList->ClearRenderTargetView(rtvHandle, DirectX::Colors::LightSteelBlue, 0, nullptr);// ���RT����ɫΪ���죬���Ҳ����òü�����
//	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
//	cmdList->ClearDepthStencilView(dsvHandle,	// DSV���������
//		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,	// FLAG
//		1.0f,	// Ĭ�����ֵ
//		0,	// Ĭ��ģ��ֵ
//		0,	// �ü���������
//		nullptr);	// �ü�����ָ��
//
//	// Ȼ������ָ����Ҫ��Ⱦ�Ļ���������ָ��RTV��DSV��
//	cmdList->OMSetRenderTargets(1,// ���󶨵�RTV����
//		&rtvHandle,	// ָ��RTV�����ָ��
//		true,	// RTV�����ڶ��ڴ�����������ŵ�
//		&dsvHandle);	// ָ��DSV��ָ��
//
//	// �ȵ���Ⱦ��ɣ�����Ҫ����̨��������״̬�ĳɳ���״̬��ʹ��֮���Ƶ�ǰ̨��������ʾ��
//	// ���ˣ��ر������б��ȴ�����������С�
//	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer[ref_mCurrentBackBuffer].Get(),
//		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));// ����ȾĿ�굽����
//	// �������ļ�¼�ر������б�
//	ThrowIfFailed(cmdList->Close());
//
//
//	// ��CPU�����׼���ú���Ҫ����ִ�е������б����GPU��������С�
//	// ʹ�õ���ExecuteCommandLists������
//	ID3D12CommandList* commandLists[] = { cmdList.Get() };// ���������������б�����
//	cmdQueue->ExecuteCommandLists(_countof(commandLists), commandLists);// ������������б����������
//
//	// Ȼ�󽻻�ǰ��̨������������������㷨��1��0��0��1��Ϊ���ú�̨������������ԶΪ0����
//	ThrowIfFailed(swapChain->Present(0, 0));
//	ref_mCurrentBackBuffer = (ref_mCurrentBackBuffer + 1) % 2;
//
//	// �������Χ��ֵ��ˢ��������У�ʹCPU��GPUͬ������δ����ڵ�һƪ������ϸ���ͣ�����ֱ�ӷ�װ��
//	FlushCmdQueue();
//
//}
