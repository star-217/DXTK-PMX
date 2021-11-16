Texture2D<float4> tex  : register(t0);
Texture2D<float4> sph  : register(t1);
Texture2D<float4> toon : register(t2);
SamplerState      samp : register(s0);
SamplerState	  toonsamp : register(s1);

struct VSOUT {
	float4 svpos : SV_POSITION;
	float4 normal: NORMAL;
	float4 vnormal : NORMAL1;
	float3 ray : VECTOR;
	float2 uv    : TEXCOORD;

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

	float3 lightColor = float3(1, 1, 1);

	float diffuseB = saturate(dot(-light, vsout.normal));

	float brightness = max(dot(-light, vsout.normal), 0.0f);
	brightness = min(brightness + 0.25f, 1.0f);

	float2 sphereMapUV = vsout.vnormal.xy;
	float4 texColor = tex.Sample(samp, vsout.uv);

	//float3 refLight = normalize(reflect(light, vsout.normal.xyz));
	//float  specularB = pow(saturate(dot(refLight, -vsout.ray)), specular.a);

	float4 toonDif = toon.Sample(toonsamp, float2(0, 1.0 - diffuseB));

	return	saturate(
			toonDif
			* diffuse
			* texColor
			* sph.Sample(samp, sphereMapUV));
}