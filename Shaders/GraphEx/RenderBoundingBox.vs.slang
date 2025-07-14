import GraphEx;


cbuffer VScb
{
    float3 modelScale;
    float3 modelTrans;
    float4x4 worldMatrix;
};


static const float3 P[] =
{
    {0,0,0}, {0,0,1}, {0,1,0}, {0,1,1},
    {1,0,0}, {1,0,1}, {1,1,0}, {1,1,1}
};


static const uint I[] =
{
    0,1, 0,2, 0,4,
    1,3, 1,5,
    2,3, 2,6,
    3,7,
    4,5, 4,6,
    5,7,
    6,7
};


struct VsOut
{
    float4 posH : SV_Position;
};


VsOut main(uint ID : SV_VertexID)
{
    let vert = P[I[ID]];
    return { mul(graphEx.activeCamera.viewProj, mul(worldMatrix, float4(vert * modelScale + modelTrans, 1.0))) };
}
