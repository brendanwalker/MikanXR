Texture2D<float4> InputTexture : register(t0);
SamplerState samLinear : register(s0);

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
	return InputTexture.Sample(samLinear, input.uv);
}