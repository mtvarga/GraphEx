import GraphEx;


cbuffer VScb
{
    float3 anchorPoint;
    float screenScaling;
    uint2 screenSize;
    float pointSize;
}


struct VsOut
{
    float4 posH : SV_Position;
};


static const float2 P[] =
{
    { -1, -1 }, { -1, 1 }, { 1, 1 }, { 1, -1 }
};


static const uint I[] =
{
    0, 2, 1,
    2, 3, 0
};


VsOut main(uint ID : SV_VertexID)
{
    var vertex = P[I[ID]];
    var anchorPointH =
        mul(graphEx.activeCamera.viewProj,
            mul(graphEx.model.transform.worldMatrix, float4(anchorPoint, 1.0)));

    var pixelSize = 2.0 / screenSize;

    anchorPointH.x += vertex.x * (pointSize / 2.0) * pixelSize.x * screenScaling * anchorPointH.w;
    anchorPointH.y += vertex.y * (pointSize / 2.0) * pixelSize.y * screenScaling * anchorPointH.w;

    return { anchorPointH };
}

