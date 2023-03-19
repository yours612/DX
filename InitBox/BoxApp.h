//#include <DirectXMath.h>
#include <array>
#include "D3DApp.h"
#include "DxException.h" //用""先从自定义的文件中找。
//#include <d3dcompiler.h> //用<>直接从编译器自带的函数库中寻找文件
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


//定义顶点结构体
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

    // 该几何体的世界矩阵
    XMFLOAT4X4 World = MathHelper::Identity4x4();

    // 该几何体的常量数据在objConstantBuffer中的索引
    UINT ObjCBIndex = -1;

    MeshGeometry* Geo = nullptr;

    BoundingBox bound; //包围盒

    INT Faces; //面数

    // 该几何体的图元拓扑类型
    D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // 该几何体的绘制三参数
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

    void BuildGeometry(int type)override;// 构建几何体          
    void BuildDescriptorHeaps();// 创建描述符堆（常量描述符堆） 
    void BuildConstantBuffers();// 创建常量描述符     
    void UpdateConstantBuffers(); //更新常量描述符 
    void BuildRootSignature();// 创建根签名                     
    void BuildShadersAndInputLayout();     //                   
    void BuildPSO();   // 构建渲染流水线状态                    
    void BuildRenderItems(int num, int type, int whichWindow)override; //创建各几何体的渲染项
    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);// 绘制渲染项

    // 鼠标事件
    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y, int whichWindow)override;
    XMFLOAT3 ScreenToWorld(int sx, int sy, int whichWindow);
    int Pick(int sx, int sy, int whichWindow)override;

    // 窗口尺寸更改事件
    virtual void OnResize() override;
private:

    std::unique_ptr<MeshGeometry> mGeo = nullptr;

    ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;// 常量缓冲描述符堆
    ComPtr<ID3D12DescriptorHeap> mCbvHeap1 = nullptr;

    std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;// 图形常量上传堆
    std::unique_ptr<UploadBuffer<PassConstants>> passCB = nullptr;// 常量上传堆2

    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;// 根签名

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;// 输入布局

    // 全部渲染项的集合
    std::vector<std::unique_ptr<RenderItem>> mAllRitems;

    // 传递给PSO的渲染项
    std::vector<RenderItem*> mOpaqueRitems;

    // 着色器字段
    ComPtr<ID3DBlob> mVsByteCode = nullptr;
    ComPtr<ID3DBlob> mPsByteCode = nullptr;

    ComPtr<ID3D12PipelineState> mPSO = nullptr;// 渲染流水线状态

    // mvp矩阵
    XMFLOAT4X4 mWorld = MathHelper::Identity4x4();
    XMFLOAT4X4 mView1 = MathHelper::Identity4x4();
    XMFLOAT4X4 mView2 = MathHelper::Identity4x4();
    XMFLOAT4X4 mProj1 = MathHelper::Identity4x4();
    XMFLOAT4X4 mProj2 = MathHelper::Identity4x4();

    // 球面坐标参数
    float mTheta1 = 1.5f * XM_PI;
    float mPhi1 = XM_PIDIV4;
    float mRadius1 = 10.0f;
    float mTheta2 = 1.5f * XM_PI;
    float mPhi2 = XM_PIDIV4;
    float mRadius2 = 10.0f;

    POINT mLastMousePos;// 记录按下鼠标位置
};
