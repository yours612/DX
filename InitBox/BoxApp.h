//#include <DirectXMath.h>
#include <array>
#include "D3DApp.h"
#include "DxException.h" //��""�ȴ��Զ�����ļ����ҡ�
//#include <d3dcompiler.h> //��<>ֱ�Ӵӱ������Դ��ĺ�������Ѱ���ļ�
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "MeshGeometry.h"
#include "GeometryGenerator.h"


#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;
using namespace std;


//���嶥��ṹ��
struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
};

struct ObjectConstants {
    XMFLOAT4X4 World = MathHelper::Identity4x4();
};
struct PassConstants {
    XMFLOAT4X4 viewProj = MathHelper::Identity4x4();
};

struct RenderItem
{
    RenderItem() = default;

    // �ü�������������
    XMFLOAT4X4 World = MathHelper::Identity4x4();

    // �ü�����ĳ���������objConstantBuffer�е�����
    UINT ObjCBIndex = -1;

    MeshGeometry* Geo = nullptr;

    BoundingBox bound; //��Χ��

    INT Faces; //����

    // �ü������ͼԪ��������
    D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // �ü�����Ļ���������
    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    UINT BaseVertexLocation = 0;
};

class BoxApp : public D3DApp
{
public:

    BoxApp();
    ~BoxApp();

    virtual bool Init(HINSTANCE hInstance, int nShowCmd, HWND hwnd) override;

    wstring InformationText;

private:
    virtual void Draw() override;
    virtual void Update() override;

    void BuildGeometry(int type)override;// ����������          
    void BuildDescriptorHeaps();// �����������ѣ������������ѣ� 
    void BuildConstantBuffers();// ��������������     
    void UpdateConstantBuffers(); //���³��������� 
    void BuildRootSignature();// ������ǩ��                     
    void BuildShadersAndInputLayout();     //                   
    void BuildPSO();   // ������Ⱦ��ˮ��״̬                    
    void BuildRenderItems(int num, int type, int whichWindow)override; //���������������Ⱦ��
    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);// ������Ⱦ��

    // ����¼�
    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y, int whichWindow)override;
    XMFLOAT3 ScreenToWorld(int sx, int sy, int whichWindow);
    int Pick(int sx, int sy, int whichWindow)override;

    // ���ڳߴ�����¼�
    virtual void OnResize() override;
private:

    std::unique_ptr<MeshGeometry> mGeo = nullptr;

    ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;// ����������������
    ComPtr<ID3D12DescriptorHeap> mCbvHeap1 = nullptr;

    std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;// ͼ�γ����ϴ���
    std::unique_ptr<UploadBuffer<PassConstants>> passCB = nullptr;// �����ϴ���2

    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;// ��ǩ��

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;// ���벼��

    // ȫ����Ⱦ��ļ���
    std::vector<std::unique_ptr<RenderItem>> mAllRitems;

    // ���ݸ�PSO����Ⱦ��
    std::vector<RenderItem*> mOpaqueRitems;

    // ��ɫ���ֶ�
    ComPtr<ID3DBlob> mVsByteCode = nullptr;
    ComPtr<ID3DBlob> mPsByteCode = nullptr;

    ComPtr<ID3D12PipelineState> mPSO = nullptr;// ��Ⱦ��ˮ��״̬

    // mvp����
    XMFLOAT4X4 mWorld = MathHelper::Identity4x4();
    XMFLOAT4X4 mView1 = MathHelper::Identity4x4();
    XMFLOAT4X4 mView2 = MathHelper::Identity4x4();
    XMFLOAT4X4 mProj1 = MathHelper::Identity4x4();
    XMFLOAT4X4 mProj2 = MathHelper::Identity4x4();

    // �����������
    float mTheta1 = 1.5f * XM_PI;
    float mPhi1 = XM_PIDIV4;
    float mRadius1 = 10.0f;
    float mTheta2 = 1.5f * XM_PI;
    float mPhi2 = XM_PIDIV4;
    float mRadius2 = 10.0f;

    POINT mLastMousePos;// ��¼�������λ��
};
