import GraphEx;


cbuffer PScb
{
    float3 color;
};


struct PsOut
{
    float4 col : SV_Target;
};


PsOut main()
{
    return { float4(color, 1.0) };
}
