struct VSOUT
{
    float4 svpos : SV_POSITION;
    float4 normal: NORMAL;
    //float4 color : COLORD;
    float2 uv    : TEXCOORD;
};

cbuffer cbuff:register(b0) {

    matrix world;
    matrix view;
    matrix proj;
    matrix bones[512];
}

VSOUT BasicVS(
    float4 pos : POSITION,
    float4 normal : NORMAL,
    float2 uv : TEXCOORD,
    int boneno : BONENO
   /* int boneno1 : BONENO1,
    int boneno2 : BONENO2,
    int boneno3 : BONENO3,
    float  weight : WEIGHT*/)
{

    VSOUT vsout;

    vsout.svpos = mul(world, pos);
    vsout.svpos = mul(view, vsout.svpos);
    vsout.svpos = mul(proj, vsout.svpos);

    vsout.normal = mul(world, normal);
    vsout.uv = uv;

    return vsout;
}
