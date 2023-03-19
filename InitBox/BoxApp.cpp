#include "BoxApp.h"
#include <cmath>

//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd){
//    //针对调试版本开启运行时内存检测
//#if defined(DEBUG) | defined(_DEBUG)
//    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//#endif 
//    try
//    {
//        BoxApp theApp;
//        if (!theApp.Init(hInstance, showCmd))
//            return 0;
//        return theApp.Run();
//    }
//    catch (DxException& e)
//    {
//        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
//        return 0;
//    }
//
//}
int i = 0;

BoxApp::BoxApp() {
    
}

BoxApp::~BoxApp() {
    
}

bool BoxApp::Init(HINSTANCE hInstance, int nShowCmd, HWND hwnd) {
    if (!D3DApp::Init(hInstance, nShowCmd, hwnd))
        return false;

    // reset命令列表离为后面初始化做准备
    ThrowIfFailed(cmdList->Reset(cmdAllocator.Get(), nullptr));

    BuildRootSignature();
    BuildShadersAndInputLayout();
    //BuildGeometry(1);
    //BuildRenderItems(0, 0);
    BuildDescriptorHeaps(); 
    BuildConstantBuffers();
    BuildPSO();

    //执行初始化命令
    ThrowIfFailed(cmdList->Close());
    ID3D12CommandList* cmdLists[] = { cmdList.Get() };
    cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

    //等待直到初始化命令完成
    FlushCmdQueue();

    return true;
}

//BoundingBox BoxApp::BuildBoundingBox(vector<Vertex> vertices)
//{
//    //初始化AABB的vMin和vMax,infinity为32位float中最大的数
//    XMFLOAT3 vMinF3 = XMFLOAT3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
//    XMFLOAT3 vMaxF3 = XMFLOAT3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);
//
//    //转换到XMVECTOR
//    XMVECTOR vMax = XMLoadFloat3(&vMaxF3);
//    XMVECTOR vMin = XMLoadFloat3(&vMinF3);
//
//    for (UINT i = 0; i < vertices.size(); i++)
//    {
//        XMVECTOR P = XMLoadFloat3(&vertices[i].Pos);
//
//        // 计算AABB包围盒两个边界点,计算vMax和vMin(类似Min和Max函数)
//        vMin = XMVectorMin(vMin, P);
//        vMax = XMVectorMax(vMax, P);
//    }
//
//    // 构建BoundingBox
//    BoundingBox bounds;
//    XMStoreFloat3(&bounds.Center, 0.5f * (vMin + vMax));
//    XMStoreFloat3(&bounds.Extents, 0.5f * (vMax - vMin));
//
//    return bounds;
//}

 //创建几何体
void BoxApp::BuildGeometry(int type) {
    FlushCmdQueue();
    ThrowIfFailed(cmdList->Reset(cmdAllocator.Get(), nullptr));

    GeometryGenerator geoGen;
    static UINT AllVertexOffset = 0; //全局顶点偏移
    static UINT AllIndexOffset = 0;//全局索引偏移

    static size_t totalVertexCount = 0; //全局顶点数据
    static std::vector<Vertex> vertices; //全局顶点数据数组
    static std::vector<std::uint16_t> indices; //全局索引数据

    static UINT vbByteSize;
    static UINT ibByteSize;

    // 创建MeshGeometry实例
    mGeo = std::make_unique<MeshGeometry>();
    mGeo->Name = "shapeGeo";

    //创建包围盒
        //初始化AABB的vMin和vMax,infinity为32位float中最大的数(用MathHelper::Infinity会
        //报错，所以用  0x3f3f3f3f  代替)
    XMFLOAT3 vMinf3(0x3f3f3f3f, 0x3f3f3f3f, 0x3f3f3f3f);
    XMFLOAT3 vMaxf3(-0x3f3f3f3f, -0x3f3f3f3f, -0x3f3f3f3f);
        //转换到XMVECTOR
    XMVECTOR vMax = XMLoadFloat3(&vMaxf3);
    XMVECTOR vMin = XMLoadFloat3(&vMinf3);
        //包围盒实例
    BoundingBox bound;

    switch (type)
    {
    case 1: //长方体
    {
        int numSubdivisions = 3;
        //创建
        GeometryGenerator::MeshData box = geoGen.CreateBox(1.5f, 0.5f, 1.5f, numSubdivisions); //矩形三角面数 = （2*2*2）^(numSubdivisions+1)
        //计算几何体在全局索引下的偏移
        UINT boxVertexOffset = AllVertexOffset;
        AllVertexOffset += (UINT)box.Vertices.size();

        UINT boxIndexOffset = AllIndexOffset;
        AllIndexOffset += (UINT)box.Indices32.size();

        SubmeshGeometry boxSubmesh;
        boxSubmesh.IndexCount = (UINT)box.Indices32.size();
        boxSubmesh.StartIndexLocation = boxIndexOffset;
        boxSubmesh.BaseVertexLocation = boxVertexOffset;
        boxSubmesh.Faces = pow(8, numSubdivisions+1);

        totalVertexCount += box.Vertices.size();

        //向全局顶点数组中存入数据
        struct Vertex a;
        for (size_t i = 0; i < box.Vertices.size(); ++i) {        
            a.Pos = box.Vertices[i].Position;
            a.Color = XMFLOAT4(DirectX::Colors::DarkGreen);
            vertices.push_back(a);

            //
            XMVECTOR P = XMLoadFloat3(&a.Pos);
            // 计算AABB包围盒两个边界点,计算vMax和vMin(类似Min和Max函数)
            vMin = XMVectorMin(vMin, P);
            vMax = XMVectorMax(vMax, P);
        }

        // 构建BoundingBox
        XMStoreFloat3(&bound.Center, 0.5f * (vMin + vMax));
        XMStoreFloat3(&bound.Extents, 0.5f * (vMax - vMin));

        //向全局索引数组中存入数据
        indices.insert(indices.end(), box.GetIndices16().begin(), box.GetIndices16().end());
        
        boxSubmesh.BoundingBox = bound;
        //创建子物体关联
        mGeo->DrawArgs["box"] = boxSubmesh;    

        break;
    }
        
    case 2: //球体
    {
        int sliceCount = 20;
        int stackCount = 20;
        GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, sliceCount, stackCount); //球体三角面数 = sliceCount*2 //上下面
                                                                                                //          + (stackCount-2)*sliceCount*2 //中间层
        UINT sphereVertexOffset = AllVertexOffset;
        AllVertexOffset += (UINT)sphere.Vertices.size();

        UINT sphereIndexOffset = AllIndexOffset;
        AllIndexOffset += (UINT)sphere.Indices32.size();

        SubmeshGeometry sphereSubmesh;
        sphereSubmesh.IndexCount = (UINT)sphere.Indices32.size();
        sphereSubmesh.StartIndexLocation = sphereIndexOffset;
        sphereSubmesh.BaseVertexLocation = sphereVertexOffset;
        sphereSubmesh.Faces = sliceCount * 2 + (stackCount - 2) * sliceCount * 2;

        totalVertexCount += sphere.Vertices.size();
        
        struct Vertex a;
        for (size_t i = 0; i < sphere.Vertices.size(); ++i) {     
            a.Pos = sphere.Vertices[i].Position;
            a.Color = XMFLOAT4(DirectX::Colors::Crimson);
            vertices.push_back(a);

            //
            XMVECTOR P = XMLoadFloat3(&a.Pos);
            // 计算AABB包围盒两个边界点,计算vMax和vMin(类似Min和Max函数)
            vMin = XMVectorMin(vMin, P);
            vMax = XMVectorMax(vMax, P);
        }

        // 构建BoundingBox
        XMStoreFloat3(&bound.Center, 0.5f * (vMin + vMax));
        XMStoreFloat3(&bound.Extents, 0.5f * (vMax - vMin));

        sphereSubmesh.BoundingBox = bound;

        indices.insert(indices.end(), sphere.GetIndices16().begin(), sphere.GetIndices16().end());

        mGeo->DrawArgs["sphere"] = sphereSubmesh;


        break;
    }

    case 3: //平面
    {
        int m = 60;
        int n = 40;
        GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, m, n); //平面三角面数 = （m-1）*（n-1）*2
        UINT gridVertexOffset = AllVertexOffset;
        AllVertexOffset += (UINT)grid.Vertices.size();

        UINT gridIndexOffset = AllIndexOffset;
        AllIndexOffset += (UINT)grid.Indices32.size();

        SubmeshGeometry gridSubmesh;
        gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
        gridSubmesh.StartIndexLocation = gridIndexOffset;
        gridSubmesh.BaseVertexLocation = gridVertexOffset;
        gridSubmesh.Faces = (m - 1) * (n - 1) * 2;

        totalVertexCount += grid.Vertices.size();

        struct Vertex a;
        for (size_t i = 0; i < grid.Vertices.size(); ++i) {
            a.Pos = grid.Vertices[i].Position;
            a.Color = XMFLOAT4(DirectX::Colors::ForestGreen);
            vertices.push_back(a);
            
            //
            XMVECTOR P = XMLoadFloat3(&a.Pos);
            // 计算AABB包围盒两个边界点,计算vMax和vMin(类似Min和Max函数)
            vMin = XMVectorMin(vMin, P);
            vMax = XMVectorMax(vMax, P);
        }

        // 构建BoundingBox
        XMStoreFloat3(&bound.Center, 0.5f * (vMin + vMax));
        XMStoreFloat3(&bound.Extents, 0.5f * (vMax - vMin));

        gridSubmesh.BoundingBox = bound;

        indices.insert(indices.end(), grid.GetIndices16().begin(), grid.GetIndices16().end());

        mGeo->DrawArgs["grid"] = gridSubmesh;
        break; 
    }


    case 4: //柱体
    {
        int sliceCount = 20;
        int stackCount = 20;
        GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, sliceCount, stackCount); //柱体三角面数 = sliceCount*2 //上下面
                                                                                                                //          + stackCount*sliceCount*2 //中间层
        UINT cylinderVertexOffset = AllVertexOffset;
        AllVertexOffset += (UINT)cylinder.Vertices.size();

        UINT cylinderIndexOffset = AllIndexOffset;
        AllIndexOffset += (UINT)cylinder.Indices32.size();

        SubmeshGeometry cylinderSubmesh;
        cylinderSubmesh.IndexCount = (UINT)cylinder.Indices32.size();
        cylinderSubmesh.StartIndexLocation = cylinderIndexOffset;
        cylinderSubmesh.BaseVertexLocation = cylinderVertexOffset;
        cylinderSubmesh.Faces = sliceCount * 2 + stackCount * sliceCount * 2;

        totalVertexCount += cylinder.Vertices.size();

        struct Vertex a;
        for (size_t i = 0; i < cylinder.Vertices.size(); ++i) {
            a.Pos = cylinder.Vertices[i].Position;
            a.Color = XMFLOAT4(DirectX::Colors::SteelBlue);
            vertices.push_back(a);
            //
            XMVECTOR P = XMLoadFloat3(&a.Pos);
            // 计算AABB包围盒两个边界点,计算vMax和vMin(类似Min和Max函数)
            vMin = XMVectorMin(vMin, P);
            vMax = XMVectorMax(vMax, P);
        }

        // 构建BoundingBox
        XMStoreFloat3(&bound.Center, 0.5f * (vMin + vMax));
        XMStoreFloat3(&bound.Extents, 0.5f * (vMax - vMin));

        cylinderSubmesh.BoundingBox = bound;

        indices.insert(indices.end(), cylinder.GetIndices16().begin(), cylinder.GetIndices16().end());

        mGeo->DrawArgs["cylinder"] = cylinderSubmesh;
        break;
    }

    default:
        break;
    }

    //!!!!!!!!!!!!!!!!!!!!!! const表示所修饰的数据类型的变量或对象的值是不能被改变的。

    //// 计算vertices和indices字节大小
    //const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    //const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    // 计算vertices和indices字节大小
    vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    // 把顶点/索引数据上传到GPU
    ThrowIfFailed(D3DCreateBlob(vbByteSize, &mGeo->VertexBufferCPU));
    ThrowIfFailed(D3DCreateBlob(ibByteSize, &mGeo->IndexBufferCPU));
    CopyMemory(mGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
    CopyMemory(mGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
    mGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(d3dDevice.Get(),
        cmdList.Get(), vertices.data(), vbByteSize, mGeo->VertexBufferUploader);
    mGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(d3dDevice.Get(),
        cmdList.Get(), indices.data(), ibByteSize, mGeo->IndexBufferUploader);

    // 更新MeshGeometry实例数据
    mGeo->VertexByteStride = sizeof(Vertex);
    mGeo->VertexBufferByteSize = vbByteSize;
    mGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
    mGeo->IndexBufferByteSize = ibByteSize;

    ThrowIfFailed(cmdList->Close());
    ID3D12CommandList* cmdLists[] = { cmdList.Get() };
    cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
 
    FlushCmdQueue();

    //GeometryGenerator geoGen;
    //GeometryGenerator::MeshData box = geoGen.CreateBox(1.5f, 0.5f, 1.5f, 3);
    //GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
    //GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
    //GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);

    //// 计算每个几何体在全局索引下的偏移
    //UINT boxVertexOffset = 0;
    //UINT gridVertexOffset = (UINT)box.Vertices.size();
    //UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
    //UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();

    //UINT boxIndexOffset = 0;
    //UINT gridIndexOffset = (UINT)box.Indices32.size();
    //UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
    //UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();

    //SubmeshGeometry boxSubmesh;
    //boxSubmesh.IndexCount = (UINT)box.Indices32.size();
    //boxSubmesh.StartIndexLocation = boxIndexOffset;
    //boxSubmesh.BaseVertexLocation = boxVertexOffset;

    //SubmeshGeometry gridSubmesh;
    //gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
    //gridSubmesh.StartIndexLocation = gridIndexOffset;
    //gridSubmesh.BaseVertexLocation = gridVertexOffset;

    //SubmeshGeometry sphereSubmesh;
    //sphereSubmesh.IndexCount = (UINT)sphere.Indices32.size();
    //sphereSubmesh.StartIndexLocation = sphereIndexOffset;
    //sphereSubmesh.BaseVertexLocation = sphereVertexOffset;

    //SubmeshGeometry cylinderSubmesh;
    //cylinderSubmesh.IndexCount = (UINT)cylinder.Indices32.size();
    //cylinderSubmesh.StartIndexLocation = cylinderIndexOffset;
    //cylinderSubmesh.BaseVertexLocation = cylinderVertexOffset;

    //// 创建全局顶点数据
    //size_t totalVertexCount = box.Vertices.size() + grid.Vertices.size() + sphere.Vertices.size() + cylinder.Vertices.size();
    //std::vector<Vertex> vertices(totalVertexCount);

    //UINT k = 0;
    //for (size_t i = 0; i < box.Vertices.size(); ++i, ++k) {
    //    vertices[k].Pos = box.Vertices[i].Position;
    //    vertices[k].Color = XMFLOAT4(DirectX::Colors::DarkGreen);
    //}

    //for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k) {
    //    vertices[k].Pos = grid.Vertices[i].Position;
    //    vertices[k].Color = XMFLOAT4(DirectX::Colors::ForestGreen);
    //}

    //for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k) {
    //    vertices[k].Pos = sphere.Vertices[i].Position;
    //    vertices[k].Color = XMFLOAT4(DirectX::Colors::Crimson);
    //}

    //for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k) {
    //    vertices[k].Pos = cylinder.Vertices[i].Position;
    //    vertices[k].Color = XMFLOAT4(DirectX::Colors::SteelBlue);
    //}

    //// 创建全局索引数据
    //std::vector<std::uint16_t> indices;
    //indices.insert(indices.end(), box.GetIndices16().begin(), box.GetIndices16().end());
    //indices.insert(indices.end(), grid.GetIndices16().begin(), grid.GetIndices16().end());
    //indices.insert(indices.end(), sphere.GetIndices16().begin(), sphere.GetIndices16().end());
    //indices.insert(indices.end(), cylinder.GetIndices16().begin(), cylinder.GetIndices16().end());

    //// 计算vertices和indices字节大小
    //const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    //const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    //// 创建MeshGeometry实例
    //mGeo = std::make_unique<MeshGeometry>();
    //mGeo->Name = "shapeGeo";

    //// 把顶点/索引数据上传到GPU
    //ThrowIfFailed(D3DCreateBlob(vbByteSize, &mGeo->VertexBufferCPU));
    //ThrowIfFailed(D3DCreateBlob(ibByteSize, &mGeo->IndexBufferCPU));
    //CopyMemory(mGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
    //CopyMemory(mGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
    //mGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(d3dDevice.Get(),
    //    cmdList.Get(), vertices.data(), vbByteSize, mGeo->VertexBufferUploader);
    //mGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(d3dDevice.Get(),
    //    cmdList.Get(), indices.data(), ibByteSize, mGeo->IndexBufferUploader);

    //// 更新MeshGeometry实例数据
    //mGeo->VertexByteStride = sizeof(Vertex);
    //mGeo->VertexBufferByteSize = vbByteSize;
    //mGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
    //mGeo->IndexBufferByteSize = ibByteSize;

    //// 创建子物体关联
    //mGeo->DrawArgs["box"] = boxSubmesh;
    //mGeo->DrawArgs["grid"] = gridSubmesh;
    //mGeo->DrawArgs["sphere"] = sphereSubmesh;
    //mGeo->DrawArgs["cylinder"] = cylinderSubmesh;
}

// 创建描述符堆（常量描述符堆）
void BoxApp::BuildDescriptorHeaps() {
    UINT objCount = 30;// (UINT)mOpaqueRitems.size();
    
    //创建CBV堆
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = objCount + 2; // 图形个数 + 两个passCBV
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;// 可供着色器访问
    cbvHeapDesc.NodeMask = 0;// 单个适配器
    ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mCbvHeap)));
}

// 创建常量描述符（描述符用于把资源绑定到流水线上）
void BoxApp::BuildConstantBuffers() {
    // 获取常量上传堆1
    
    UINT objCount = 30;// (UINT)mOpaqueRitems.size();
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

        ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(d3dDevice.Get(), objCount, true);
  
        
        for (UINT i = 0; i < objCount; i++)
        {
            D3D12_GPU_VIRTUAL_ADDRESS cbAddress1 = ObjectCB->Resource()->GetGPUVirtualAddress();

            // 根据索引对cbv地址进行偏移
            cbAddress1 += i * objCBByteSize;

            // 根据索引对cbv在heap中进行偏移
            int heapIndex = i;
            auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
            handle.Offset(heapIndex, cbv_srv_uavDescriptorSize);

            // 描述cbv
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
            cbvDesc.BufferLocation = cbAddress1;
            cbvDesc.SizeInBytes = objCBByteSize;

            // 创建cbv
            d3dDevice->CreateConstantBufferView(&cbvDesc, handle);
        }
    
        // 获取常量上传堆2、3（用于图形窗口显示不同的摄像机视角）
        passCB = std::make_unique<UploadBuffer<PassConstants>>(d3dDevice.Get(), 2, true);
        UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
        D3D12_GPU_VIRTUAL_ADDRESS cbAddress2 = passCB->Resource()->GetGPUVirtualAddress();

        int passCbElementIndex = 0;
        cbAddress2 += passCbElementIndex * passCBByteSize;

        int passCBufIndex = objCount;
        auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
        handle.Offset(passCBufIndex, cbv_srv_uavDescriptorSize);

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc2;
        cbvDesc2.BufferLocation = cbAddress2;
        cbvDesc2.SizeInBytes = passCBByteSize;
        d3dDevice->CreateConstantBufferView(&cbvDesc2, handle);
        //——————————————————————————————
        D3D12_GPU_VIRTUAL_ADDRESS cbAddress3 = passCB->Resource()->GetGPUVirtualAddress();

        passCbElementIndex = 1;
        cbAddress3 += passCbElementIndex * passCBByteSize;

        passCBufIndex = objCount + 1;
        handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
        handle.Offset(passCBufIndex, cbv_srv_uavDescriptorSize);

        cbvDesc2.BufferLocation = cbAddress3;
        cbvDesc2.SizeInBytes = passCBByteSize;
        d3dDevice->CreateConstantBufferView(&cbvDesc2, handle);
}

// 创建根签名(并没有和渲染流水线进行绑定，真正绑定的是在设置渲染流水线状态中)
void BoxApp::BuildRootSignature() {
    // 根参数可以是描述符表、根描述符、根常量
    CD3DX12_ROOT_PARAMETER slotRootParameter[2];

    // 创建由两个CBV所组成的描述符表（即创建两个根参数，目的是将含有一个CBV的描述符表绑定到常量缓冲区寄存器0，即HLSL中的register（b0）））
    CD3DX12_DESCRIPTOR_RANGE objCbvTable;
    objCbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,// 描述符类型
        1,// 描述符数量
        0);// 描述符所绑定的寄存器槽号
    slotRootParameter[0].InitAsDescriptorTable(1, &objCbvTable);

    CD3DX12_DESCRIPTOR_RANGE passCbvTable;
    passCbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
    slotRootParameter[1].InitAsDescriptorTable(1, &passCbvTable);

    // 根签名由一系列根参数组成(这里是两个)
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2,// 根参数的数量
        slotRootParameter,// 根参数指针
        0,// 静态采样数量
        nullptr,// 静态采样描述结构体
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);// 选择输入汇编器 需要一组顶点缓冲区绑定的输入布局

    // 用单个寄存槽来创建一个根签名，该槽位指向一个仅含有单个常量缓冲区描述符区域
    // 得到序列化内存COM接口
    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc,// 根参数描述指针
        D3D_ROOT_SIGNATURE_VERSION_1,// 根参数version
        serializedRootSig.GetAddressOf(),// [out]序列化内存块
        errorBlob.GetAddressOf());// [out]序列化错误信息

    if (errorBlob != nullptr) {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    // 创建根签名与序列化内存绑定
    ThrowIfFailed(d3dDevice->CreateRootSignature(
        0,// 适配器数量
        serializedRootSig->GetBufferPointer(),// 根签名绑定的序列化内存指针
        serializedRootSig->GetBufferSize(),// 根签名绑定的序列化内存byte
        IID_PPV_ARGS(&mRootSignature)));// 根签名COM ID

}

// 创建输入布局和编译Shader
void BoxApp::BuildShadersAndInputLayout() {
    HRESULT hr = S_OK;

    mVsByteCode = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr,
                                  "VS", "vs_5_0" );
    mPsByteCode = d3dUtil::CompileShader(L"Shaders\\color.hlsl", nullptr,
        "PS", "ps_5_0");

    mInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

// 构建渲染流水线状态
void BoxApp::BuildPSO() {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc; //流水线状态细节
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC)); //用0来填充一块内存区域
    psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() }; //输入布局描述  
    psoDesc.pRootSignature = mRootSignature.Get(); // 与此PSO绑定的根签名指针
    psoDesc.VS =
    {
        reinterpret_cast<BYTE*>(mVsByteCode->GetBufferPointer()),
        mVsByteCode->GetBufferSize()
    };// 待绑定的顶点着色器
    psoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mPsByteCode->GetBufferPointer()),
        mPsByteCode->GetBufferSize()
    };// 待绑定的像素着色器
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);// 光栅化状态
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME; //D3D12_FILL_MODE_WIREFRAME; 线框模式 D3D12_FILL_MODE_SOLID 实体模式
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // D3D12_CULL_MODE_NONE剔除模式：不剔除；D3D12_CULL_MODE_BACK 背面剔除
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);// 混合状态
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);// 深度/模板测试状态
    psoDesc.SampleMask = UINT_MAX;// 每个采样点的采样情况
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;// 图元拓扑类型
    psoDesc.NumRenderTargets = 1;// 渲染目标数量
    psoDesc.RTVFormats[0] = mBackBufferFormat;// 渲染目标的格式
    psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;// 多重采样数量
    psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;// 多重采样级别
    psoDesc.DSVFormat = mDepthStencilFormat;// 深度/模板缓冲区格式
    ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));
}

//创建渲染项
void BoxApp::BuildRenderItems(int num, int type, int whichWindow) {
    switch (type)
    {
    case 1: //正方体
    {
        // 创建box渲染项
        auto boxRitem = std::make_unique<RenderItem>();
        // 设置世界矩阵
        XMFLOAT3 theWorld = ScreenToWorld(cursorPosX, cursorPosY, whichWindow);
        XMStoreFloat4x4(&boxRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(theWorld.x, theWorld.y, theWorld.z));
        // 设置世界矩阵在常量数据中的索引
        boxRitem->ObjCBIndex = num;
        boxRitem->Geo = mGeo.get();
        boxRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        boxRitem->IndexCount = boxRitem->Geo->DrawArgs["box"].IndexCount;
        boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["box"].StartIndexLocation;
        boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["box"].BaseVertexLocation;
        boxRitem->bound = boxRitem->Geo->DrawArgs["box"].BoundingBox;
        boxRitem->Faces = boxRitem->Geo->DrawArgs["box"].Faces;
        mAllRitems.push_back(std::move(boxRitem));

        break;
    }
        
    case 2: //球体
    {
        auto sphereRitem = std::make_unique<RenderItem>();
        XMFLOAT3 theWorld = ScreenToWorld(cursorPosX, cursorPosY, whichWindow);
        XMStoreFloat4x4(&sphereRitem->World, XMMatrixTranslation(theWorld.x, theWorld.y, theWorld.z));
        sphereRitem->ObjCBIndex = num;
        sphereRitem->Geo = mGeo.get();
        sphereRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        sphereRitem->IndexCount = sphereRitem->Geo->DrawArgs["sphere"].IndexCount;
        sphereRitem->StartIndexLocation = sphereRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
        sphereRitem->BaseVertexLocation = sphereRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;
        sphereRitem->bound = sphereRitem->Geo->DrawArgs["sphere"].BoundingBox;
        sphereRitem->Faces = sphereRitem->Geo->DrawArgs["sphere"].Faces;
        mAllRitems.push_back(std::move(sphereRitem));

        break;
    }
        
    case 3: //平面
    {
        // 创建grid渲染项
        auto gridRitem = std::make_unique<RenderItem>();
        XMFLOAT3 theWorld = ScreenToWorld(cursorPosX, cursorPosY, whichWindow);
        gridRitem->World = MathHelper::Identity4x4();
        gridRitem->ObjCBIndex = num;
        gridRitem->Geo = mGeo.get();
        gridRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
        gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
        gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;
        gridRitem->bound = gridRitem->Geo->DrawArgs["grid"].BoundingBox;
        gridRitem->Faces = gridRitem->Geo->DrawArgs["grid"].Faces;
        mAllRitems.push_back(std::move(gridRitem));

        break;
    }
        
    case 4: //柱体
    {
        auto cylRitem = std::make_unique<RenderItem>();
        XMFLOAT3 theWorld = ScreenToWorld(cursorPosX, cursorPosY, whichWindow);
        XMStoreFloat4x4(&cylRitem->World, XMMatrixTranslation(theWorld.x, theWorld.y, theWorld.z));
        cylRitem->ObjCBIndex = num;
        cylRitem->Geo = mGeo.get();
        cylRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        cylRitem->IndexCount = cylRitem->Geo->DrawArgs["cylinder"].IndexCount;
        cylRitem->StartIndexLocation = cylRitem->Geo->DrawArgs["cylinder"].StartIndexLocation;
        cylRitem->BaseVertexLocation = cylRitem->Geo->DrawArgs["cylinder"].BaseVertexLocation;
        cylRitem->bound = cylRitem->Geo->DrawArgs["cylinder"].BoundingBox;
        cylRitem->Faces = cylRitem->Geo->DrawArgs["cylinder"].Faces;
        mAllRitems.push_back(std::move(cylRitem));

        break;
    }       
    default:
        break;
    }

    // 填充传递给pso的容器
    for (auto& e : mAllRitems)
        mOpaqueRitems.push_back(e.get());

    //BuildDescriptorHeaps();
    //BuildConstantBuffers();

    //// 创建box渲染项
    //auto boxRitem = std::make_unique<RenderItem>();
    //// 设置世界矩阵
    //XMStoreFloat4x4(&boxRitem->World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(0.0f, 0.5f, 0.0f));
    //// 设置世界矩阵在常量数据中的索引
    //boxRitem->ObjCBIndex = 0;
    //boxRitem->Geo = mGeo.get();
    //boxRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    //boxRitem->IndexCount = boxRitem->Geo->DrawArgs["box"].IndexCount;
    //boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["box"].StartIndexLocation;
    //boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["box"].BaseVertexLocation;
    //mAllRitems.push_back(std::move(boxRitem));

    //// 创建grid渲染项
    //auto gridRitem = std::make_unique<RenderItem>();
    //gridRitem->World = MathHelper::Identity4x4();
    //gridRitem->ObjCBIndex = 1;
    //gridRitem->Geo = mGeo.get();
    //gridRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    //gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
    //gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
    //gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;
    //mAllRitems.push_back(std::move(gridRitem));

    //// 创建cylinder和sphere渲染项
    //UINT objCBIndex = 2;// 索引
    //for (int i = 0; i < 5; ++i) {
    //    auto leftCylRitem = std::make_unique<RenderItem>();
    //    auto rightCylRitem = std::make_unique<RenderItem>();
    //    auto leftSphereRitem = std::make_unique<RenderItem>();
    //    auto rightSphereRitem = std::make_unique<RenderItem>();

    //    XMMATRIX leftCylWorld = XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f);
    //    XMMATRIX rightCylWorld = XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f);

    //    XMMATRIX leftSphereWorld = XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f);
    //    XMMATRIX rightSphereWorld = XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f);

    //    XMStoreFloat4x4(&leftCylRitem->World, rightCylWorld);
    //    leftCylRitem->ObjCBIndex = objCBIndex++;
    //    leftCylRitem->Geo = mGeo.get();
    //    leftCylRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    //    leftCylRitem->IndexCount = leftCylRitem->Geo->DrawArgs["cylinder"].IndexCount;
    //    leftCylRitem->StartIndexLocation = leftCylRitem->Geo->DrawArgs["cylinder"].StartIndexLocation;
    //    leftCylRitem->BaseVertexLocation = leftCylRitem->Geo->DrawArgs["cylinder"].BaseVertexLocation;

    //    XMStoreFloat4x4(&rightCylRitem->World, leftCylWorld);
    //    rightCylRitem->ObjCBIndex = objCBIndex++;
    //    rightCylRitem->Geo = mGeo.get();
    //    rightCylRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    //    rightCylRitem->IndexCount = rightCylRitem->Geo->DrawArgs["cylinder"].IndexCount;
    //    rightCylRitem->StartIndexLocation = rightCylRitem->Geo->DrawArgs["cylinder"].StartIndexLocation;
    //    rightCylRitem->BaseVertexLocation = rightCylRitem->Geo->DrawArgs["cylinder"].BaseVertexLocation;

    //    XMStoreFloat4x4(&leftSphereRitem->World, leftSphereWorld);
    //    leftSphereRitem->ObjCBIndex = objCBIndex++;
    //    leftSphereRitem->Geo = mGeo.get();
    //    leftSphereRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    //    leftSphereRitem->IndexCount = leftSphereRitem->Geo->DrawArgs["sphere"].IndexCount;
    //    leftSphereRitem->StartIndexLocation = leftSphereRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
    //    leftSphereRitem->BaseVertexLocation = leftSphereRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;

    //    XMStoreFloat4x4(&rightSphereRitem->World, rightSphereWorld);
    //    rightSphereRitem->ObjCBIndex = objCBIndex++;
    //    rightSphereRitem->Geo = mGeo.get();
    //    rightSphereRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    //    rightSphereRitem->IndexCount = rightSphereRitem->Geo->DrawArgs["sphere"].IndexCount;
    //    rightSphereRitem->StartIndexLocation = rightSphereRitem->Geo->DrawArgs["sphere"].StartIndexLocation;
    //    rightSphereRitem->BaseVertexLocation = rightSphereRitem->Geo->DrawArgs["sphere"].BaseVertexLocation;

    //    mAllRitems.push_back(std::move(leftCylRitem));
    //    mAllRitems.push_back(std::move(rightCylRitem));
    //    mAllRitems.push_back(std::move(leftSphereRitem));
    //    mAllRitems.push_back(std::move(rightSphereRitem));
    //}

    //// 填充传递给pso的容器
    //for (auto& e : mAllRitems)
    //    mOpaqueRitems.push_back(e.get());
}

// 绘制渲染项
void BoxApp::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems) {
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants)); 

    for (auto& ri : mAllRitems) {

        // 设置顶点缓冲区
        cmdList->IASetVertexBuffers(0, 1, &mGeo->VertexBufferView());
        // 设置索引缓冲区
        cmdList->IASetIndexBuffer(&mGeo->IndexBufferView());
        // 设置图元拓扑
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

        UINT cbvIndex = ri->ObjCBIndex;
        auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
        handle.Offset(cbvIndex, cbv_srv_uavDescriptorSize);

        // 设置根签名表
        cmdList->SetGraphicsRootDescriptorTable(0, handle);

        // 绘制顶点实例
        cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
    }
}

void BoxApp::Draw() {
    
    // 重复使用记录命令的相关内存
    ThrowIfFailed(cmdAllocator->Reset());
    // 复用命令列表及其内存
    ThrowIfFailed(cmdList->Reset(cmdAllocator.Get(), mPSO.Get()));

    //设置视口和裁剪矩形
    cmdList->RSSetViewports(1, &viewPort);
    cmdList->RSSetScissorRects(1, &scissorRect);

    //将后台缓冲资源从呈现状态转换到渲染目标状态（即准备接收图像渲染）
    UINT& ref_mCurrentBackBuffer = mCurrentBackBuffer;
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer1[ref_mCurrentBackBuffer].Get(),// 转换资源为后台缓冲区资源
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));// 从呈现到渲染目标转换
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer2[ref_mCurrentBackBuffer].Get(),// 转换资源为后台缓冲区资源
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));// 从呈现到渲染目标转换

    // 然后清除后台缓冲区和深度缓冲区，并赋值。
    // 步骤是先获得堆中描述符句柄（即地址），
    // 再通过ClearRenderTargetView函数和ClearDepthStencilView函数做  清除  和  赋值。
    // 这里我们将RT资源背景色赋值为LightSteelBlue。
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle1 = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap1->GetCPUDescriptorHandleForHeapStart(), ref_mCurrentBackBuffer, rtvDescriptorSize);
    cmdList->ClearRenderTargetView(rtvHandle1, DirectX::Colors::LightSteelBlue, 0, nullptr);// 清除RT背景色为暗红，并且不设置裁剪矩形
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle2 = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap2->GetCPUDescriptorHandleForHeapStart(), ref_mCurrentBackBuffer, rtvDescriptorSize);
    cmdList->ClearRenderTargetView(rtvHandle2, DirectX::Colors::LightSteelBlue, 0, nullptr);// 清除RT背景色为暗红，并且不设置裁剪矩形

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle1 = dsvHeap1->GetCPUDescriptorHandleForHeapStart();
    cmdList->ClearDepthStencilView(dsvHandle1,	// DSV描述符句柄
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,	// FLAG
        1.0f,	// 默认深度值
        0,	// 默认模板值
        0,	// 裁剪矩形数量
        nullptr);	// 裁剪矩形指针
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle2 = dsvHeap2->GetCPUDescriptorHandleForHeapStart();
    cmdList->ClearDepthStencilView(dsvHandle2,	D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        1.0f,	0,	0,	nullptr);	

     // 指定将要渲染的缓冲区，即指定RTV和DSV————先指定窗口1
    cmdList->OMSetRenderTargets(1, // 待绑定的RTV数量
        &rtvHandle1, // 指向RTV数组的指针
        true, // RTV对象在堆内存中是连续存放的
        &dsvHandle1); // 指向DSV的指针

    //设置CBV描述符堆
    ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() }; //注意这里之所以是数组，是因为还可能包含SRV和UAV，而这里我们只用到了CBV
                                           //这里的mCbvHeap为常量缓冲描述符堆，update了的mObjectCB
                                           //的数据就在常量缓冲区里
    cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    //设置根签名
    cmdList->SetGraphicsRootSignature(mRootSignature.Get());

    // 上传perPass常量
    int passCbvIndex = 30;// (int)mAllRitems.size();
    auto passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
    passCbvHandle.Offset(passCbvIndex, cbv_srv_uavDescriptorSize);
    cmdList->SetGraphicsRootDescriptorTable(1, // 根参数的起始索引
        passCbvHandle);
    
    DrawRenderItems(cmdList.Get(), mOpaqueRitems);

    // 指定窗口2
    cmdList->OMSetRenderTargets(1, &rtvHandle2, true, &dsvHandle2);

    passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
    passCbvHandle.Offset(passCbvIndex + 1, cbv_srv_uavDescriptorSize);
    cmdList->SetGraphicsRootDescriptorTable(1, passCbvHandle);

    DrawRenderItems(cmdList.Get(), mOpaqueRitems);

    // 等到渲染完成，将后台缓冲区的状态改成呈现状态，使其之后推到前台缓冲区显示。
    // 完了，关闭命令列表，等待传入命令队列。
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer1[ref_mCurrentBackBuffer].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)); // 从渲染目标到呈现
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer2[ref_mCurrentBackBuffer].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)); // 从渲染目标到呈现

    // 完成命令的记录关闭命令列表
    ThrowIfFailed(cmdList->Close());

    // 等CPU将命令都准备好后，需要将待执行的命令列表加入GPU的命令队列。
    // 使用的是ExecuteCommandLists函数。
    ID3D12CommandList* commandLists[] = { cmdList.Get() };// 声明并定义命令列表数组
    cmdQueue->ExecuteCommandLists(_countof(commandLists), commandLists);// 将命令从命令列表传至命令队列

    // 然后交换前后台缓冲区索引（这里的算法是1变0，0变1，为了让后台缓冲区索引永远为0）。
    ThrowIfFailed(swapChain1->Present(0, 0));
    ThrowIfFailed(swapChain2->Present(0, 0));
    mCurrentBackBuffer = (mCurrentBackBuffer + 1) % 2;

    // 最后设置围栏值，刷新命令队列，使CPU和GPU同步，这段代码在第一篇中有详细解释，这里直接封装。
    FlushCmdQueue();
}

// 每一帧的操作
void BoxApp::Update() {
    ObjectConstants objConstants;
    PassConstants passConstants;

    // 转换球面坐标
    float x1 = mRadius1 * sinf(mPhi1) * cosf(mTheta1);
    float z1 = mRadius1 * sinf(mPhi1) * sinf(mTheta1);
    float y1 = mRadius1 * cosf(mPhi1);
    float x2 = mRadius2 * sinf(mPhi2) * cosf(mTheta2);
    float z2 = mRadius2 * sinf(mPhi2) * sinf(mTheta2);
    float y2 = mRadius2 * cosf(mPhi2);

    // 构建视图矩阵
    XMVECTOR pos = XMVectorSet(x1, y1, z1, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX view1 = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView1, view1);

    pos = XMVectorSet(x2, y2, z2, 1.0f);
    XMMATRIX view2 = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView2, view2);

    XMMATRIX world;

    // 将世界矩阵传递给GPU objConstants
            for (auto& t : mAllRitems) {
                world = XMLoadFloat4x4(&t->World);
                XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
                ObjectCB->CopyData(t->ObjCBIndex, objConstants);
            }

        
        // 将视图投影矩阵传递给GPU passConstants
        XMMATRIX proj1 = XMLoadFloat4x4(&mProj1);
        XMMATRIX viewProj = view1 * proj1;
        XMStoreFloat4x4(&passConstants.viewProj, XMMatrixTranspose(viewProj));
        passCB->CopyData(0, passConstants);

        XMMATRIX proj2 = XMLoadFloat4x4(&mProj2);
        viewProj = view2 * proj2;
        XMStoreFloat4x4(&passConstants.viewProj, XMMatrixTranspose(viewProj));
        passCB->CopyData(1, passConstants); 
}

//鼠标控制
void BoxApp::OnMouseDown(WPARAM btnState, int x, int y) {
    mLastMousePos.x = x;    // 按下的时候记录坐标x分量
    mLastMousePos.y = y;	// 按下的时候记录坐标y分量

    //SetCapture(graphicsWnd1);  // 在属于当前线程的指定窗口里，设置鼠标捕获
}
void BoxApp::OnMouseUp(WPARAM btnState, int x, int y) {
    ReleaseCapture(); // 按键抬起后释放鼠标捕获
}
void BoxApp::OnMouseMove(WPARAM btnState, int x, int y, int whichWindow) {
    if (graphicsWindowWidth - x <= 30 || x <= 30) //防止两窗口交界处影响
        return;
    
    if ((btnState & MK_LBUTTON) != 0) //如果在左键按下状态 
    {
        // 将鼠标的移动距离换算成弧度，0.25为调节阈值
        float dx = XMConvertToRadians(static_cast<float>(mLastMousePos.x - x) * 0.25f);
        float dy = XMConvertToRadians(static_cast<float>(mLastMousePos.y - y) * 0.25f);

        if (whichWindow == 1)
        {
            // 计算鼠标没有松开前的累计弧度
            mTheta1 += dx;
            mPhi1 += dy;

            // 限制角度phi的范围在（0.1， Pi-0.1）
            mPhi1 = MathHelper::Clamp(mPhi1, 0.1f, 3.1415f - 0.1f);
        }
        else if (whichWindow == 2)
        {
            // 计算鼠标没有松开前的累计弧度
            mTheta2 += dx;
            mPhi2 += dy;

            // 限制角度phi的范围在（0.1， Pi-0.1）
            mPhi2 = MathHelper::Clamp(mPhi2, 0.1f, 3.1415f - 0.1f);
        }
    }
    else if ((btnState & MK_RBUTTON) != 0) //如果在右键按下状态
    {
        // 将鼠标的移动距离换算成缩放大小，0.005为调节阈值
        float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
        float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);
        
        if (whichWindow == 1) {
            // 根据鼠标输入更新摄像机可视范围半径
            mRadius1 += dx - dy;
            //限制可视范围半径
            mRadius1 = MathHelper::Clamp(mRadius1, 1.0f, 50.0f);
        }
        else if (whichWindow == 2) {
            mRadius2 += dx - dy;
            mRadius2 = MathHelper::Clamp(mRadius2, 1.0f, 50.0f);
        }
        
    }
    // 将当前鼠标坐标赋值给“上一次鼠标坐标”，为下一次鼠标操作提供先前值
    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

XMFLOAT3 BoxApp::ScreenToWorld(int x, int y, int whichWindow)
{
    XMFLOAT4X4 P;
    XMMATRIX View;
    if (whichWindow == 1)
    {
        P = mProj1;
        View = XMLoadFloat4x4(&mView1);
    }
        
    else if (whichWindow == 2)
    {
        P = mProj2;
        View = XMLoadFloat4x4(&mView2);
    }

    float xv = 10 *(2.f * x / graphicsWindowWidth - 1.f) / P(0, 0);
    float yv = 10 *(-2.f * y / graphicsWindowHeight + 1.f) / P(1, 1);
    float zv = 10;

    XMVECTOR A = XMVectorSet(xv, yv, zv, 1.0);

    auto view = XMMatrixDeterminant(View);
    XMMATRIX invView = XMMatrixInverse(&view, View);//观察矩阵的逆矩阵

    A = XMVector3TransformCoord(A, invView);

    XMFLOAT3 Pos;

    XMStoreFloat3(&Pos, A);

    return Pos;
}
int BoxApp::Pick(int x, int y, int whichWindow)
{
    XMVECTOR rayOrigin;
    XMVECTOR rayDir;

    XMFLOAT4X4 P;
    XMMATRIX V;
    if (whichWindow == 1)
    {
        P = mProj1;
        V = XMLoadFloat4x4(&mView1);
    }
    else if (whichWindow == 2)
    {
        P = mProj2;
        V = XMLoadFloat4x4(&mView2);
    }

    //屏幕坐标转换为投影窗口坐标
    float xv = (2.f * x / graphicsWindowWidth - 1.f) / P(0, 0);
    float yv = (-2.f * y / graphicsWindowHeight + 1.f) / P(1, 1);

    bool Picked = false;
    //判断同时检测到多个
    int i = 0;
    float tmin;
    float triMin;

    //循环所有图形
    for (auto& ri : mAllRitems)
    {
        //射线原点和方向
        rayOrigin = XMVectorSet(0, 0, 0, 1.f);
        rayDir = XMVectorSet(xv, yv, 1.f, 0.f);
        //观察矩阵逆矩阵
        XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(V), V);
        //世界矩阵逆矩阵
        XMMATRIX W = XMLoadFloat4x4(&ri->World);
        XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);
        // 将拾取射线变换到网格局部空间 
        //XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);
        //分别用来对点和向量进行变换(乘以变化矩阵)。
        rayOrigin = XMVector3TransformCoord(rayOrigin, invView);
        rayDir = XMVector3TransformNormal(rayDir, invView);

        rayDir = XMVector3Normalize(rayDir); // 规范化  
        
        if (i++ == 0)
        {           
            tmin = 0x3f3f3f3f;
            triMin = 0.0f;//离射线最近物体的判断阈值
        }
        

        BoundingBox Bounds;
        XMVECTOR boundPos1 = XMLoadFloat3(&ri->bound.Center);
        boundPos1 = XMVector3TransformCoord(boundPos1, W);
        XMStoreFloat3(&Bounds.Center, boundPos1);
        XMVECTOR boundPos2 = XMLoadFloat3(&ri->bound.Extents);
        XMStoreFloat3(&Bounds.Extents, boundPos2);

        if (Bounds.Intersects(rayOrigin, rayDir, triMin))
        {          
            if (triMin < tmin)
            {
                tmin = triMin;

                wstring nameText;
                //判断点击的图形序号 显示图形名（矩形2）
                switch (allGraphics[ri->ObjCBIndex + 1])
                {
                case 1:
                {
                    int serialnum = 1;
                    for (int j = 1; j < ri->ObjCBIndex + 1; j++)
                    {
                        if (allGraphics[j] == 1)
                            serialnum++;
                    }
                    nameText = L"矩形" + to_wstring(serialnum);
                    break;
                }
                case 2:
                {
                    int serialnum = 1;
                    for (int j = 1; j < ri->ObjCBIndex + 1; j++)
                    {
                        if (allGraphics[j] == 2)
                            serialnum++;
                    }
                    nameText = L"球体" + to_wstring(serialnum);
                    break;
                }
                case 3:
                {
                    int serialnum = 1;
                    for (int j = 1; j < ri->ObjCBIndex + 1; j++)
                    {
                        if (allGraphics[j] == 3)
                            serialnum++;
                    }
                    nameText = L"平面" + to_wstring(serialnum);
                    break;
                }
                case 4:
                {
                    int serialnum = 1;
                    for (int j = 1; j < ri->ObjCBIndex + 1; j++)
                    {
                        if (allGraphics[j] == 4)
                            serialnum++;
                    }
                    nameText = L"柱体" + to_wstring(serialnum);
                    break;
                }
                }

                wstring posx = L"x:  ";
                wstring positionText = L"世界坐标：\n"
                    + posx + to_wstring(ri->World._41) + L"   \n"
                    + L"y:  " + to_wstring(ri->World._42) + L"   \n"
                    + L"z:  " + to_wstring(ri->World._43) + L"   \n";

                wstring faceText = L"面数：  " + to_wstring(ri->Faces);

                InformationText = nameText + L"\n\n" + positionText + L"\n" + faceText + L"\n";
                LPCWSTR str = InformationText.c_str();

                Picked = true;
            }  
            
        }
    }

    if (Picked) return 1;
    else return 0;
}

void BoxApp::OnResize() {
    D3DApp::OnResize();

    //构建投影矩阵
    XMMATRIX p1 = XMMatrixPerspectiveFovLH(0.25f * 3.1416f, static_cast<float>(graphicsWindowWidth) / graphicsWindowHeight, 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj1, p1);
    XMMATRIX p2 = XMMatrixPerspectiveFovLH(0.5f * 3.1416f, static_cast<float>(graphicsWindowWidth) / graphicsWindowHeight, 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj2, p2);
}