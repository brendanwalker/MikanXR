Texture2D<float> InputTexture : register(t0);
SamplerState samLinear : register(s0);

cbuffer ConstantBuffer : register(b0)
{
	float zNear;
	float zFar;
};

struct VS_INPUT
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

PS_INPUT vs_main(VS_INPUT input)
{
	PS_INPUT output;
	output.pos = float4(input.position, 1.0f);
	output.uv = input.texCoord;
	return output;
}

float4 ps_main(PS_INPUT input) : SV_TARGET
{
	float depth = InputTexture.Sample(samLinear, input.uv).r;
	float eyeDepth = zFar * zNear / ((zNear - zFar) * depth + zFar);
	float zNorm = (eyeDepth - zNear) / (zFar - zNear);

	return float4(zNorm, zNorm, zNorm, 1.0);
}