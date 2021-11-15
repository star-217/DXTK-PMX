Texture2D<float4> tex  : register(t0);
Texture2D<float4> sph  : register(t1);
SamplerState      samp : register(s0);

struct VSOUT {
	float4 svpos : SV_POSITION;
	float4 normal: NORMAL;
	float2 uv    : TEXCOORD;
	//float4 color : COLORD;

};

cbuffer Material : register(b1)
{
	float4 diffuse;
	float4 specular;
	float3 ambient;
}

float4 BasicPS(VSOUT vsout) : SV_TARGET
{
	float3 light = normalize(float3(1,-1,1));
	float brightness = max(dot(-light, vsout.normal), 0.0f);
	brightness = min(brightness + 0.25f, 1.0f);

	float2 normalUV = (vsout.normal.xy + float2(1, 1)) * float2(0.5, 0.5);

	return float4(brightness, brightness, brightness, 1.0f) * diffuse * tex.Sample(samp, vsout.uv) * sph.Sample(samp, normalUV);
}