cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
};
cbuffer cbPass : register(b1) {
	float4x4 gViewProj;
}


struct VertexIn
{
	float3 PosL  : POSITION;
	 float4 Color : COLOR; // nointerpolation����ƽ����ɫ
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// �任Ϊ��βü��ռ�
	float3 PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.PosH = mul(float4(PosW, 1.0f), gViewProj);

	// ֻ�轫������ɫ���ݵ�������ɫ��
	vout.Color = vin.Color;

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	return pin.Color;
}