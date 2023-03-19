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
//	ThrowIfFailed(cmdAllocator->Reset());// 重复使用记录命令的相关内存
//	ThrowIfFailed(cmdList->Reset(cmdAllocator.Get(), nullptr));// 复用命令列表及其内存
//	// 接着我们将后台缓冲资源从呈现状态转换到渲染目标状态（即准备接收图像渲染）
//	UINT& ref_mCurrentBackBuffer = mCurrentBackBuffer;
//	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer[ref_mCurrentBackBuffer].Get(),// 转换资源为后台缓冲区资源
//		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));// 从呈现到渲染目标转换
//
//   // 接下来设置视口和裁剪矩形。
//	cmdList->RSSetViewports(1, &viewPort);
//	cmdList->RSSetScissorRects(1, &scissorRect);
//
//	// 然后清除后台缓冲区和深度缓冲区，并赋值。
//	// 步骤是先获得堆中描述符句柄（即地址），
//	// 再通过ClearRenderTargetView函数和ClearDepthStencilView函数做清除和赋值。
//	// 这里我们将RT资源背景色赋值为LightSteelBlue。
//	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap->GetCPUDescriptorHandleForHeapStart(), ref_mCurrentBackBuffer, rtvDescriptorSize);
//	cmdList->ClearRenderTargetView(rtvHandle, DirectX::Colors::LightSteelBlue, 0, nullptr);// 清除RT背景色为暗红，并且不设置裁剪矩形
//	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
//	cmdList->ClearDepthStencilView(dsvHandle,	// DSV描述符句柄
//		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,	// FLAG
//		1.0f,	// 默认深度值
//		0,	// 默认模板值
//		0,	// 裁剪矩形数量
//		nullptr);	// 裁剪矩形指针
//
//	// 然后我们指定将要渲染的缓冲区，即指定RTV和DSV。
//	cmdList->OMSetRenderTargets(1,// 待绑定的RTV数量
//		&rtvHandle,	// 指向RTV数组的指针
//		true,	// RTV对象在堆内存中是连续存放的
//		&dsvHandle);	// 指向DSV的指针
//
//	// 等到渲染完成，我们要将后台缓冲区的状态改成呈现状态，使其之后推到前台缓冲区显示。
//	// 完了，关闭命令列表，等待传入命令队列。
//	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer[ref_mCurrentBackBuffer].Get(),
//		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));// 从渲染目标到呈现
//	// 完成命令的记录关闭命令列表
//	ThrowIfFailed(cmdList->Close());
//
//
//	// 等CPU将命令都准备好后，需要将待执行的命令列表加入GPU的命令队列。
//	// 使用的是ExecuteCommandLists函数。
//	ID3D12CommandList* commandLists[] = { cmdList.Get() };// 声明并定义命令列表数组
//	cmdQueue->ExecuteCommandLists(_countof(commandLists), commandLists);// 将命令从命令列表传至命令队列
//
//	// 然后交换前后台缓冲区索引（这里的算法是1变0，0变1，为了让后台缓冲区索引永远为0）。
//	ThrowIfFailed(swapChain->Present(0, 0));
//	ref_mCurrentBackBuffer = (ref_mCurrentBackBuffer + 1) % 2;
//
//	// 最后设置围栏值，刷新命令队列，使CPU和GPU同步，这段代码在第一篇中有详细解释，这里直接封装。
//	FlushCmdQueue();
//
//}
