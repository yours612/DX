#pragma once
//#include <D:\DX\DragonBookCode\学习代码\MyDragonBook-main\04 DIrect3D的初始化：初步封装\Better Initialization\D3DApp.h>
//#include <windows.h>
//#include <wrl.h>


#include "stdafx.h"
#include "DxException.h"
using Microsoft::WRL::ComPtr;


class d3dUtil {
public:

	static UINT CalcConstantBufferByteSize(UINT byteSize) { return (byteSize + 255) & ~255; }

	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const void* initData,
		UINT64 byteSize,
		Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);


	static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
		const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target);
};