struct VSOUT
{
    float4 svpos : SV_POSITION;
    float4 normal: NORMAL;
    //float4 color : COLORD;
    float2 uv    : TEXCOORD;
};

cbuffer cbuff:register(b0) {

    matrix transform;
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
   // pos = mul(bones[boneno], pos);
    vsout.svpos = mul(transform, pos);
    normal.w = 0;
    vsout.normal = mul(transform, normal);
    vsout.uv = uv;

    return vsout;
}
