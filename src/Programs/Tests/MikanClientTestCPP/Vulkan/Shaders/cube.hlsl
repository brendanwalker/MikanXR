// The cube shader of the Vulkan test harness, compiled to SPIR-V by dxc at build time.
// The matrix arrives as a GLM column-major mat4 through a push constant, so the transform is
// written column-vector style (mul(M, v)) rather than the DirectX path's row-vector style.

struct VS_INPUT
{
	[[vk::location(0)]] float4 pos : POSITION;
	[[vk::location(1)]] float4 col : COLOR;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	[[vk::location(0)]] float4 col : COLOR;
};

struct Constants
{
	float4x4 worldViewProj;
};
[[vk::push_constant]] Constants constants;

PS_INPUT vs_main(VS_INPUT input)
{
	PS_INPUT output= (PS_INPUT)0;

	output.pos= mul(constants.worldViewProj, input.pos);
	output.col= input.col;

	return output;
}

float4 ps_main(PS_INPUT input) : SV_TARGET
{
	return input.col;
}
