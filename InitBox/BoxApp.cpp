#include "BoxApp.h"
#include <cmath>

//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd){
//    //��Ե��԰汾��������ʱ�ڴ���
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

    // reset�����б���Ϊ�����ʼ����׼��
    ThrowIfFailed(cmdList->Reset(cmdAllocator.Get(), nullptr));

    BuildRootSignature();
    BuildShadersAndInputLayout();
    //BuildGeometry(1);
    //BuildRenderItems(0, 0);
    BuildDescriptorHeaps(); 
    BuildConstantBuffers();
    BuildPSO();

    //ִ�г�ʼ������
    ThrowIfFailed(cmdList->Close());
    ID3D12CommandList* cmdLists[] = { cmdList.Get() };
    cmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

    //�ȴ�ֱ����ʼ���������
    FlushCmdQueue();

    return true;
}

//BoundingBox BoxApp::BuildBoundingBox(vector<Vertex> vertices)
//{
//    //��ʼ��AABB��vMin��vMax,infinityΪ32λfloat��������
//    XMFLOAT3 vMinF3 = XMFLOAT3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
//    XMFLOAT3 vMaxF3 = XMFLOAT3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);
//
//    //ת����XMVECTOR
//    XMVECTOR vMax = XMLoadFloat3(&vMaxF3);
//    XMVECTOR vMin = XMLoadFloat3(&vMinF3);
//
//    for (UINT i = 0; i < vertices.size(); i++)
//    {
//        XMVECTOR P = XMLoadFloat3(&vertices[i].Pos);
//
//        // ����AABB��Χ�������߽��,����vMax��vMin(����Min��Max����)
//        vMin = XMVectorMin(vMin, P);
//        vMax = XMVectorMax(vMax, P);
//    }
//
//    // ����BoundingBox
//    BoundingBox bounds;
//    XMStoreFloat3(&bounds.Center, 0.5f * (vMin + vMax));
//    XMStoreFloat3(&bounds.Extents, 0.5f * (vMax - vMin));
//
//    return bounds;
//}

 //����������
void BoxApp::BuildGeometry(int type) {
    FlushCmdQueue();
    ThrowIfFailed(cmdList->Reset(cmdAllocator.Get(), nullptr));

    GeometryGenerator geoGen;
    static UINT AllVertexOffset = 0; //ȫ�ֶ���ƫ��
    static UINT AllIndexOffset = 0;//ȫ������ƫ��

    static size_t totalVertexCount = 0; //ȫ�ֶ�������
    static std::vector<Vertex> vertices; //ȫ�ֶ�����������
    static std::vector<std::uint16_t> indices; //ȫ����������

    static UINT vbByteSize;
    static UINT ibByteSize;

    // ����MeshGeometryʵ��
    mGeo = std::make_unique<MeshGeometry>();
    mGeo->Name = "shapeGeo";

    //������Χ��
        //��ʼ��AABB��vMin��vMax,infinityΪ32λfloat��������(��MathHelper::Infinity��
        //����������  0x3f3f3f3f  ����)
    XMFLOAT3 vMinf3(0x3f3f3f3f, 0x3f3f3f3f, 0x3f3f3f3f);
    XMFLOAT3 vMaxf3(-0x3f3f3f3f, -0x3f3f3f3f, -0x3f3f3f3f);
        //ת����XMVECTOR
    XMVECTOR vMax = XMLoadFloat3(&vMaxf3);
    XMVECTOR vMin = XMLoadFloat3(&vMinf3);
        //��Χ��ʵ��
    BoundingBox bound;

    switch (type)
    {
    case 1: //������
    {
        int numSubdivisions = 3;
        //����
        GeometryGenerator::MeshData box = geoGen.CreateBox(1.5f, 0.5f, 1.5f, numSubdivisions); //������������ = ��2*2*2��^(numSubdivisions+1)
        //���㼸������ȫ�������µ�ƫ��
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

        //��ȫ�ֶ��������д�������
        struct Vertex a;
        for (size_t i = 0; i < box.Vertices.size(); ++i) {        
            a.Pos = box.Vertices[i].Position;
            a.Color = XMFLOAT4(DirectX::Colors::DarkGreen);
            vertices.push_back(a);

            //
            XMVECTOR P = XMLoadFloat3(&a.Pos);
            // ����AABB��Χ�������߽��,����vMax��vMin(����Min��Max����)
            vMin = XMVectorMin(vMin, P);
            vMax = XMVectorMax(vMax, P);
        }

        // ����BoundingBox
        XMStoreFloat3(&bound.Center, 0.5f * (vMin + vMax));
        XMStoreFloat3(&bound.Extents, 0.5f * (vMax - vMin));

        //��ȫ�����������д�������
        indices.insert(indices.end(), box.GetIndices16().begin(), box.GetIndices16().end());
        
        boxSubmesh.BoundingBox = bound;
        //�������������
        mGeo->DrawArgs["box"] = boxSubmesh;    

        break;
    }
        
    case 2: //����
    {
        int sliceCount = 20;
        int stackCount = 20;
        GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, sliceCount, stackCount); //������������ = sliceCount*2 //������
                                                                                                //          + (stackCount-2)*sliceCount*2 //�м��
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
            // ����AABB��Χ�������߽��,����vMax��vMin(����Min��Max����)
            vMin = XMVectorMin(vMin, P);
            vMax = XMVectorMax(vMax, P);
        }

        // ����BoundingBox
        XMStoreFloat3(&bound.Center, 0.5f * (vMin + vMax));
        XMStoreFloat3(&bound.Extents, 0.5f * (vMax - vMin));

        sphereSubmesh.BoundingBox = bound;

        indices.insert(indices.end(), sphere.GetIndices16().begin(), sphere.GetIndices16().end());

        mGeo->DrawArgs["sphere"] = sphereSubmesh;


        break;
    }

    case 3: //ƽ��
    {
        int m = 60;
        int n = 40;
        GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, m, n); //ƽ���������� = ��m-1��*��n-1��*2
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
            // ����AABB��Χ�������߽��,����vMax��vMin(����Min��Max����)
            vMin = XMVectorMin(vMin, P);
            vMax = XMVectorMax(vMax, P);
        }

        // ����BoundingBox
        XMStoreFloat3(&bound.Center, 0.5f * (vMin + vMax));
        XMStoreFloat3(&bound.Extents, 0.5f * (vMax - vMin));

        gridSubmesh.BoundingBox = bound;

        indices.insert(indices.end(), grid.GetIndices16().begin(), grid.GetIndices16().end());

        mGeo->DrawArgs["grid"] = gridSubmesh;
        break; 
    }


    case 4: //����
    {
        int sliceCount = 20;
        int stackCount = 20;
        GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, sliceCount, stackCount); //������������ = sliceCount*2 //������
                                                                                                                //          + stackCount*sliceCount*2 //�м��
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
            // ����AABB��Χ�������߽��,����vMax��vMin(����Min��Max����)
            vMin = XMVectorMin(vMin, P);
            vMax = XMVectorMax(vMax, P);
        }

        // ����BoundingBox
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

    //!!!!!!!!!!!!!!!!!!!!!! const��ʾ�����ε��������͵ı���������ֵ�ǲ��ܱ��ı�ġ�

    //// ����vertices��indices�ֽڴ�С
    //const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    //const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    // ����vertices��indices�ֽڴ�С
    vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    // �Ѷ���/���������ϴ���GPU
    ThrowIfFailed(D3DCreateBlob(vbByteSize, &mGeo->VertexBufferCPU));
    ThrowIfFailed(D3DCreateBlob(ibByteSize, &mGeo->IndexBufferCPU));
    CopyMemory(mGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
    CopyMemory(mGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
    mGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(d3dDevice.Get(),
        cmdList.Get(), vertices.data(), vbByteSize, mGeo->VertexBufferUploader);
    mGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(d3dDevice.Get(),
        cmdList.Get(), indices.data(), ibByteSize, mGeo->IndexBufferUploader);

    // ����MeshGeometryʵ������
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

    //// ����ÿ����������ȫ�������µ�ƫ��
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

    //// ����ȫ�ֶ�������
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

    //// ����ȫ����������
    //std::vector<std::uint16_t> indices;
    //indices.insert(indices.end(), box.GetIndices16().begin(), box.GetIndices16().end());
    //indices.insert(indices.end(), grid.GetIndices16().begin(), grid.GetIndices16().end());
    //indices.insert(indices.end(), sphere.GetIndices16().begin(), sphere.GetIndices16().end());
    //indices.insert(indices.end(), cylinder.GetIndices16().begin(), cylinder.GetIndices16().end());

    //// ����vertices��indices�ֽڴ�С
    //const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    //const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

    //// ����MeshGeometryʵ��
    //mGeo = std::make_unique<MeshGeometry>();
    //mGeo->Name = "shapeGeo";

    //// �Ѷ���/���������ϴ���GPU
    //ThrowIfFailed(D3DCreateBlob(vbByteSize, &mGeo->VertexBufferCPU));
    //ThrowIfFailed(D3DCreateBlob(ibByteSize, &mGeo->IndexBufferCPU));
    //CopyMemory(mGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
    //CopyMemory(mGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
    //mGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(d3dDevice.Get(),
    //    cmdList.Get(), vertices.data(), vbByteSize, mGeo->VertexBufferUploader);
    //mGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(d3dDevice.Get(),
    //    cmdList.Get(), indices.data(), ibByteSize, mGeo->IndexBufferUploader);

    //// ����MeshGeometryʵ������
    //mGeo->VertexByteStride = sizeof(Vertex);
    //mGeo->VertexBufferByteSize = vbByteSize;
    //mGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
    //mGeo->IndexBufferByteSize = ibByteSize;

    //// �������������
    //mGeo->DrawArgs["box"] = boxSubmesh;
    //mGeo->DrawArgs["grid"] = gridSubmesh;
    //mGeo->DrawArgs["sphere"] = sphereSubmesh;
    //mGeo->DrawArgs["cylinder"] = cylinderSubmesh;
}

// �����������ѣ������������ѣ�
void BoxApp::BuildDescriptorHeaps() {
    UINT objCount = 30;// (UINT)mOpaqueRitems.size();
    
    //����CBV��
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = objCount + 2; // ͼ�θ��� + ����passCBV
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;// �ɹ���ɫ������
    cbvHeapDesc.NodeMask = 0;// ����������
    ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mCbvHeap)));
}

// �������������������������ڰ���Դ�󶨵���ˮ���ϣ�
void BoxApp::BuildConstantBuffers() {
    // ��ȡ�����ϴ���1
    
    UINT objCount = 30;// (UINT)mOpaqueRitems.size();
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

        ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(d3dDevice.Get(), objCount, true);
  
        
        for (UINT i = 0; i < objCount; i++)
        {
            D3D12_GPU_VIRTUAL_ADDRESS cbAddress1 = ObjectCB->Resource()->GetGPUVirtualAddress();

            // ����������cbv��ַ����ƫ��
            cbAddress1 += i * objCBByteSize;

            // ����������cbv��heap�н���ƫ��
            int heapIndex = i;
            auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mCbvHeap->GetCPUDescriptorHandleForHeapStart());
            handle.Offset(heapIndex, cbv_srv_uavDescriptorSize);

            // ����cbv
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
            cbvDesc.BufferLocation = cbAddress1;
            cbvDesc.SizeInBytes = objCBByteSize;

            // ����cbv
            d3dDevice->CreateConstantBufferView(&cbvDesc, handle);
        }
    
        // ��ȡ�����ϴ���2��3������ͼ�δ�����ʾ��ͬ��������ӽǣ�
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
        //������������������������������������������������������������
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

// ������ǩ��(��û�к���Ⱦ��ˮ�߽��а󶨣������󶨵�����������Ⱦ��ˮ��״̬��)
void BoxApp::BuildRootSignature() {
    // ������������������������������������
    CD3DX12_ROOT_PARAMETER slotRootParameter[2];

    // ����������CBV����ɵ���������������������������Ŀ���ǽ�����һ��CBV����������󶨵������������Ĵ���0����HLSL�е�register��b0������
    CD3DX12_DESCRIPTOR_RANGE objCbvTable;
    objCbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,// ����������
        1,// ����������
        0);// ���������󶨵ļĴ����ۺ�
    slotRootParameter[0].InitAsDescriptorTable(1, &objCbvTable);

    CD3DX12_DESCRIPTOR_RANGE passCbvTable;
    passCbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
    slotRootParameter[1].InitAsDescriptorTable(1, &passCbvTable);

    // ��ǩ����һϵ�и��������(����������)
    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2,// ������������
        slotRootParameter,// ������ָ��
        0,// ��̬��������
        nullptr,// ��̬���������ṹ��
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);// ѡ���������� ��Ҫһ�鶥�㻺�����󶨵����벼��

    // �õ����Ĵ��������һ����ǩ�����ò�λָ��һ�������е�����������������������
    // �õ����л��ڴ�COM�ӿ�
    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc,// ����������ָ��
        D3D_ROOT_SIGNATURE_VERSION_1,// ������version
        serializedRootSig.GetAddressOf(),// [out]���л��ڴ��
        errorBlob.GetAddressOf());// [out]���л�������Ϣ

    if (errorBlob != nullptr) {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    // ������ǩ�������л��ڴ��
    ThrowIfFailed(d3dDevice->CreateRootSignature(
        0,// ����������
        serializedRootSig->GetBufferPointer(),// ��ǩ���󶨵����л��ڴ�ָ��
        serializedRootSig->GetBufferSize(),// ��ǩ���󶨵����л��ڴ�byte
        IID_PPV_ARGS(&mRootSignature)));// ��ǩ��COM ID

}

// �������벼�ֺͱ���Shader
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

// ������Ⱦ��ˮ��״̬
void BoxApp::BuildPSO() {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc; //��ˮ��״̬ϸ��
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC)); //��0�����һ���ڴ�����
    psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() }; //���벼������  
    psoDesc.pRootSignature = mRootSignature.Get(); // ���PSO�󶨵ĸ�ǩ��ָ��
    psoDesc.VS =
    {
        reinterpret_cast<BYTE*>(mVsByteCode->GetBufferPointer()),
        mVsByteCode->GetBufferSize()
    };// ���󶨵Ķ�����ɫ��
    psoDesc.PS =
    {
        reinterpret_cast<BYTE*>(mPsByteCode->GetBufferPointer()),
        mPsByteCode->GetBufferSize()
    };// ���󶨵�������ɫ��
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);// ��դ��״̬
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME; //D3D12_FILL_MODE_WIREFRAME; �߿�ģʽ D3D12_FILL_MODE_SOLID ʵ��ģʽ
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // D3D12_CULL_MODE_NONE�޳�ģʽ�����޳���D3D12_CULL_MODE_BACK �����޳�
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);// ���״̬
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);// ���/ģ�����״̬
    psoDesc.SampleMask = UINT_MAX;// ÿ��������Ĳ������
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;// ͼԪ��������
    psoDesc.NumRenderTargets = 1;// ��ȾĿ������
    psoDesc.RTVFormats[0] = mBackBufferFormat;// ��ȾĿ��ĸ�ʽ
    psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;// ���ز�������
    psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;// ���ز�������
    psoDesc.DSVFormat = mDepthStencilFormat;// ���/ģ�建������ʽ
    ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));
}

//������Ⱦ��
void BoxApp::BuildRenderItems(int num, int type, int whichWindow) {
    switch (type)
    {
    case 1: //������
    {
        // ����box��Ⱦ��
        auto boxRitem = std::make_unique<RenderItem>();
        // �����������
        XMFLOAT3 theWorld = ScreenToWorld(cursorPosX, cursorPosY, whichWindow);
        XMStoreFloat4x4(&boxRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(theWorld.x, theWorld.y, theWorld.z));
        // ������������ڳ��������е�����
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
        
    case 2: //����
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
        
    case 3: //ƽ��
    {
        // ����grid��Ⱦ��
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
        
    case 4: //����
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

    // ��䴫�ݸ�pso������
    for (auto& e : mAllRitems)
        mOpaqueRitems.push_back(e.get());

    //BuildDescriptorHeaps();
    //BuildConstantBuffers();

    //// ����box��Ⱦ��
    //auto boxRitem = std::make_unique<RenderItem>();
    //// �����������
    //XMStoreFloat4x4(&boxRitem->World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(0.0f, 0.5f, 0.0f));
    //// ������������ڳ��������е�����
    //boxRitem->ObjCBIndex = 0;
    //boxRitem->Geo = mGeo.get();
    //boxRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    //boxRitem->IndexCount = boxRitem->Geo->DrawArgs["box"].IndexCount;
    //boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["box"].StartIndexLocation;
    //boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["box"].BaseVertexLocation;
    //mAllRitems.push_back(std::move(boxRitem));

    //// ����grid��Ⱦ��
    //auto gridRitem = std::make_unique<RenderItem>();
    //gridRitem->World = MathHelper::Identity4x4();
    //gridRitem->ObjCBIndex = 1;
    //gridRitem->Geo = mGeo.get();
    //gridRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    //gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
    //gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
    //gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;
    //mAllRitems.push_back(std::move(gridRitem));

    //// ����cylinder��sphere��Ⱦ��
    //UINT objCBIndex = 2;// ����
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

    //// ��䴫�ݸ�pso������
    //for (auto& e : mAllRitems)
    //    mOpaqueRitems.push_back(e.get());
}

// ������Ⱦ��
void BoxApp::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems) {
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants)); 

    for (auto& ri : mAllRitems) {

        // ���ö��㻺����
        cmdList->IASetVertexBuffers(0, 1, &mGeo->VertexBufferView());
        // ��������������
        cmdList->IASetIndexBuffer(&mGeo->IndexBufferView());
        // ����ͼԪ����
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

        UINT cbvIndex = ri->ObjCBIndex;
        auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
        handle.Offset(cbvIndex, cbv_srv_uavDescriptorSize);

        // ���ø�ǩ����
        cmdList->SetGraphicsRootDescriptorTable(0, handle);

        // ���ƶ���ʵ��
        cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
    }
}

void BoxApp::Draw() {
    
    // �ظ�ʹ�ü�¼���������ڴ�
    ThrowIfFailed(cmdAllocator->Reset());
    // ���������б����ڴ�
    ThrowIfFailed(cmdList->Reset(cmdAllocator.Get(), mPSO.Get()));

    //�����ӿںͲü�����
    cmdList->RSSetViewports(1, &viewPort);
    cmdList->RSSetScissorRects(1, &scissorRect);

    //����̨������Դ�ӳ���״̬ת������ȾĿ��״̬����׼������ͼ����Ⱦ��
    UINT& ref_mCurrentBackBuffer = mCurrentBackBuffer;
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer1[ref_mCurrentBackBuffer].Get(),// ת����ԴΪ��̨��������Դ
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));// �ӳ��ֵ���ȾĿ��ת��
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer2[ref_mCurrentBackBuffer].Get(),// ת����ԴΪ��̨��������Դ
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));// �ӳ��ֵ���ȾĿ��ת��

    // Ȼ�������̨����������Ȼ�����������ֵ��
    // �������Ȼ�ö������������������ַ����
    // ��ͨ��ClearRenderTargetView������ClearDepthStencilView������  ���  ��  ��ֵ��
    // �������ǽ�RT��Դ����ɫ��ֵΪLightSteelBlue��
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle1 = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap1->GetCPUDescriptorHandleForHeapStart(), ref_mCurrentBackBuffer, rtvDescriptorSize);
    cmdList->ClearRenderTargetView(rtvHandle1, DirectX::Colors::LightSteelBlue, 0, nullptr);// ���RT����ɫΪ���죬���Ҳ����òü�����
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle2 = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap2->GetCPUDescriptorHandleForHeapStart(), ref_mCurrentBackBuffer, rtvDescriptorSize);
    cmdList->ClearRenderTargetView(rtvHandle2, DirectX::Colors::LightSteelBlue, 0, nullptr);// ���RT����ɫΪ���죬���Ҳ����òü�����

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle1 = dsvHeap1->GetCPUDescriptorHandleForHeapStart();
    cmdList->ClearDepthStencilView(dsvHandle1,	// DSV���������
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,	// FLAG
        1.0f,	// Ĭ�����ֵ
        0,	// Ĭ��ģ��ֵ
        0,	// �ü���������
        nullptr);	// �ü�����ָ��
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle2 = dsvHeap2->GetCPUDescriptorHandleForHeapStart();
    cmdList->ClearDepthStencilView(dsvHandle2,	D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        1.0f,	0,	0,	nullptr);	

     // ָ����Ҫ��Ⱦ�Ļ���������ָ��RTV��DSV����������ָ������1
    cmdList->OMSetRenderTargets(1, // ���󶨵�RTV����
        &rtvHandle1, // ָ��RTV�����ָ��
        true, // RTV�����ڶ��ڴ�����������ŵ�
        &dsvHandle1); // ָ��DSV��ָ��

    //����CBV��������
    ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() }; //ע������֮���������飬����Ϊ�����ܰ���SRV��UAV������������ֻ�õ���CBV
                                           //�����mCbvHeapΪ���������������ѣ�update�˵�mObjectCB
                                           //�����ݾ��ڳ�����������
    cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    //���ø�ǩ��
    cmdList->SetGraphicsRootSignature(mRootSignature.Get());

    // �ϴ�perPass����
    int passCbvIndex = 30;// (int)mAllRitems.size();
    auto passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
    passCbvHandle.Offset(passCbvIndex, cbv_srv_uavDescriptorSize);
    cmdList->SetGraphicsRootDescriptorTable(1, // ����������ʼ����
        passCbvHandle);
    
    DrawRenderItems(cmdList.Get(), mOpaqueRitems);

    // ָ������2
    cmdList->OMSetRenderTargets(1, &rtvHandle2, true, &dsvHandle2);

    passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
    passCbvHandle.Offset(passCbvIndex + 1, cbv_srv_uavDescriptorSize);
    cmdList->SetGraphicsRootDescriptorTable(1, passCbvHandle);

    DrawRenderItems(cmdList.Get(), mOpaqueRitems);

    // �ȵ���Ⱦ��ɣ�����̨��������״̬�ĳɳ���״̬��ʹ��֮���Ƶ�ǰ̨��������ʾ��
    // ���ˣ��ر������б��ȴ�����������С�
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer1[ref_mCurrentBackBuffer].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)); // ����ȾĿ�굽����
    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer2[ref_mCurrentBackBuffer].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)); // ����ȾĿ�굽����

    // �������ļ�¼�ر������б�
    ThrowIfFailed(cmdList->Close());

    // ��CPU�����׼���ú���Ҫ����ִ�е������б����GPU��������С�
    // ʹ�õ���ExecuteCommandLists������
    ID3D12CommandList* commandLists[] = { cmdList.Get() };// ���������������б�����
    cmdQueue->ExecuteCommandLists(_countof(commandLists), commandLists);// ������������б����������

    // Ȼ�󽻻�ǰ��̨������������������㷨��1��0��0��1��Ϊ���ú�̨������������ԶΪ0����
    ThrowIfFailed(swapChain1->Present(0, 0));
    ThrowIfFailed(swapChain2->Present(0, 0));
    mCurrentBackBuffer = (mCurrentBackBuffer + 1) % 2;

    // �������Χ��ֵ��ˢ��������У�ʹCPU��GPUͬ������δ����ڵ�һƪ������ϸ���ͣ�����ֱ�ӷ�װ��
    FlushCmdQueue();
}

// ÿһ֡�Ĳ���
void BoxApp::Update() {
    ObjectConstants objConstants;
    PassConstants passConstants;

    // ת����������
    float x1 = mRadius1 * sinf(mPhi1) * cosf(mTheta1);
    float z1 = mRadius1 * sinf(mPhi1) * sinf(mTheta1);
    float y1 = mRadius1 * cosf(mPhi1);
    float x2 = mRadius2 * sinf(mPhi2) * cosf(mTheta2);
    float z2 = mRadius2 * sinf(mPhi2) * sinf(mTheta2);
    float y2 = mRadius2 * cosf(mPhi2);

    // ������ͼ����
    XMVECTOR pos = XMVectorSet(x1, y1, z1, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX view1 = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView1, view1);

    pos = XMVectorSet(x2, y2, z2, 1.0f);
    XMMATRIX view2 = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView2, view2);

    XMMATRIX world;

    // ��������󴫵ݸ�GPU objConstants
            for (auto& t : mAllRitems) {
                world = XMLoadFloat4x4(&t->World);
                XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
                ObjectCB->CopyData(t->ObjCBIndex, objConstants);
            }

        
        // ����ͼͶӰ���󴫵ݸ�GPU passConstants
        XMMATRIX proj1 = XMLoadFloat4x4(&mProj1);
        XMMATRIX viewProj = view1 * proj1;
        XMStoreFloat4x4(&passConstants.viewProj, XMMatrixTranspose(viewProj));
        passCB->CopyData(0, passConstants);

        XMMATRIX proj2 = XMLoadFloat4x4(&mProj2);
        viewProj = view2 * proj2;
        XMStoreFloat4x4(&passConstants.viewProj, XMMatrixTranspose(viewProj));
        passCB->CopyData(1, passConstants); 
}

//������
void BoxApp::OnMouseDown(WPARAM btnState, int x, int y) {
    mLastMousePos.x = x;    // ���µ�ʱ���¼����x����
    mLastMousePos.y = y;	// ���µ�ʱ���¼����y����

    //SetCapture(graphicsWnd1);  // �����ڵ�ǰ�̵߳�ָ�������������겶��
}
void BoxApp::OnMouseUp(WPARAM btnState, int x, int y) {
    ReleaseCapture(); // ����̧����ͷ���겶��
}
void BoxApp::OnMouseMove(WPARAM btnState, int x, int y, int whichWindow) {
    if (graphicsWindowWidth - x <= 30 || x <= 30) //��ֹ�����ڽ��紦Ӱ��
        return;
    
    if ((btnState & MK_LBUTTON) != 0) //������������״̬ 
    {
        // �������ƶ����뻻��ɻ��ȣ�0.25Ϊ������ֵ
        float dx = XMConvertToRadians(static_cast<float>(mLastMousePos.x - x) * 0.25f);
        float dy = XMConvertToRadians(static_cast<float>(mLastMousePos.y - y) * 0.25f);

        if (whichWindow == 1)
        {
            // �������û���ɿ�ǰ���ۼƻ���
            mTheta1 += dx;
            mPhi1 += dy;

            // ���ƽǶ�phi�ķ�Χ�ڣ�0.1�� Pi-0.1��
            mPhi1 = MathHelper::Clamp(mPhi1, 0.1f, 3.1415f - 0.1f);
        }
        else if (whichWindow == 2)
        {
            // �������û���ɿ�ǰ���ۼƻ���
            mTheta2 += dx;
            mPhi2 += dy;

            // ���ƽǶ�phi�ķ�Χ�ڣ�0.1�� Pi-0.1��
            mPhi2 = MathHelper::Clamp(mPhi2, 0.1f, 3.1415f - 0.1f);
        }
    }
    else if ((btnState & MK_RBUTTON) != 0) //������Ҽ�����״̬
    {
        // �������ƶ����뻻������Ŵ�С��0.005Ϊ������ֵ
        float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
        float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);
        
        if (whichWindow == 1) {
            // ����������������������ӷ�Χ�뾶
            mRadius1 += dx - dy;
            //���ƿ��ӷ�Χ�뾶
            mRadius1 = MathHelper::Clamp(mRadius1, 1.0f, 50.0f);
        }
        else if (whichWindow == 2) {
            mRadius2 += dx - dy;
            mRadius2 = MathHelper::Clamp(mRadius2, 1.0f, 50.0f);
        }
        
    }
    // ����ǰ������긳ֵ������һ��������ꡱ��Ϊ��һ���������ṩ��ǰֵ
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
    XMMATRIX invView = XMMatrixInverse(&view, View);//�۲����������

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

    //��Ļ����ת��ΪͶӰ��������
    float xv = (2.f * x / graphicsWindowWidth - 1.f) / P(0, 0);
    float yv = (-2.f * y / graphicsWindowHeight + 1.f) / P(1, 1);

    bool Picked = false;
    //�ж�ͬʱ��⵽���
    int i = 0;
    float tmin;
    float triMin;

    //ѭ������ͼ��
    for (auto& ri : mAllRitems)
    {
        //����ԭ��ͷ���
        rayOrigin = XMVectorSet(0, 0, 0, 1.f);
        rayDir = XMVectorSet(xv, yv, 1.f, 0.f);
        //�۲���������
        XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(V), V);
        //������������
        XMMATRIX W = XMLoadFloat4x4(&ri->World);
        XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);
        // ��ʰȡ���߱任������ֲ��ռ� 
        //XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);
        //�ֱ������Ե���������б任(���Ա仯����)��
        rayOrigin = XMVector3TransformCoord(rayOrigin, invView);
        rayDir = XMVector3TransformNormal(rayDir, invView);

        rayDir = XMVector3Normalize(rayDir); // �淶��  
        
        if (i++ == 0)
        {           
            tmin = 0x3f3f3f3f;
            triMin = 0.0f;//���������������ж���ֵ
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
                //�жϵ����ͼ����� ��ʾͼ����������2��
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
                    nameText = L"����" + to_wstring(serialnum);
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
                    nameText = L"����" + to_wstring(serialnum);
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
                    nameText = L"ƽ��" + to_wstring(serialnum);
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
                    nameText = L"����" + to_wstring(serialnum);
                    break;
                }
                }

                wstring posx = L"x:  ";
                wstring positionText = L"�������꣺\n"
                    + posx + to_wstring(ri->World._41) + L"   \n"
                    + L"y:  " + to_wstring(ri->World._42) + L"   \n"
                    + L"z:  " + to_wstring(ri->World._43) + L"   \n";

                wstring faceText = L"������  " + to_wstring(ri->Faces);

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

    //����ͶӰ����
    XMMATRIX p1 = XMMatrixPerspectiveFovLH(0.25f * 3.1416f, static_cast<float>(graphicsWindowWidth) / graphicsWindowHeight, 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj1, p1);
    XMMATRIX p2 = XMMatrixPerspectiveFovLH(0.5f * 3.1416f, static_cast<float>(graphicsWindowWidth) / graphicsWindowHeight, 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj2, p2);
}