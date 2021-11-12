Texture2D<float4> tex  : register(t0);
Texture2D<float4> toon : register(t1);
SamplerState      samp : register(s0);
SamplerState      sampToon : register(s1);

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
	float diffuseB = saturate(dot(-light, vsout.normal));
	float4 toonDif = toon.Sample(sampToon, float2(0, 1.0f - diffuseB));
	//float brightness = max(dot(-light, vsout.normal), 0.0f);
	//brightness = min(brightness + 0.25f, 1.0f);
	float4 texColor = tex.Sample(samp, vsout.uv);

	return max(saturate(toonDif * texColor),float4(texColor * ambient,1));
}