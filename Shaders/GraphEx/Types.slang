implementing GraphEx;

import Scene.Camera.CameraData;


namespace GraphEx
{

public struct Light
{
    public enum Type : uint
    {
        Point,
        Directional
    }

    private Type   _type;
    private float3 _position;
    private float3 _direction;
    private float3 _ambientColor;
    private float3 _diffuseColor;
    private float3 _specularColor;
    private float  _constantAttenuation;
    private float  _linearAttenuation;
    private float  _quadraticAttenuation;

    // Getters
    public property Type   type                 { get { return _type;                 } }
    public property float3 position             { get { return _position;             } }
    public property float3 direction            { get { return _direction;            } }
    public property float3 ambientColor         { get { return _ambientColor;         } }
    public property float3 diffuseColor         { get { return _diffuseColor;         } }
    public property float3 specularColor        { get { return _specularColor;        } }
    public property float  constantAttenuation  { get { return _constantAttenuation;  } }
    public property float  linearAttenuation    { get { return _linearAttenuation;    } }
    public property float  quadraticAttenuation { get { return _quadraticAttenuation; } }
}


public struct Scene
{
    private Light _globalLight;

    // Getters
    public property Light globalLight { get { return _globalLight; } }
}


public struct Transform
{
    private float3 _anchorPoint;

    private float4x4 _translationMatrix;
    private float4x4 _rotationMatrix;
    private float4x4 _scaleMatrix;

    private float4x4 _worldMatrix;
    private float4x4 _worldMatrixInverse;
    private float4x4 _worldMatrixInverseTranspose;

    // Getters
    public property float3 anchorPoint { get { return _anchorPoint; } }

    public property float4x4 translationMatrix { get { return _translationMatrix; } }
    public property float4x4 rotationMatrix    { get { return _rotationMatrix;    } }
    public property float4x4 scaleMatrix       { get { return _scaleMatrix;       } }

    public property float4x4 worldMatrix                 { get { return _worldMatrix;                 } }
    public property float4x4 worldMatrixInverse          { get { return _worldMatrixInverse;          } }
    public property float4x4 worldMatrixInverseTranspose { get { return _worldMatrixInverseTranspose; } }
}


public struct Material
{
    private float3 _ambientColor;
    private float3 _diffuseColor;
    private float3 _specularColor;
    private float  _shininess;

    // Getters
    public property float3 ambientColor  { get { return _ambientColor;  } }
    public property float3 diffuseColor  { get { return _diffuseColor;  } }
    public property float3 specularColor { get { return _specularColor; } }
    public property float  shininess     { get { return _shininess;     } }
}


public struct Model
{
    private Transform _transform;
    private Material  _material;

    // Getters
    public property Transform transform  { get { return _transform; } }
    public property Material  material   { get { return _material;  } }
}


public struct Camera
{
    private CameraData data;

    // Getters
    public property float3   position       { get { return data.posW;           } }
    public property float4x4 view           { get { return data.viewMat;        } }
    public property float4x4 proj           { get { return data.projMat;        } }
    public property float4x4 viewProj       { get { return data.viewProjMat;    } }
    public property float4x4 viewProjInv    { get { return data.invViewProj;    } }
    public property float    focalLength    { get { return data.focalLength;    } }
    public property float3   target         { get { return data.target;         } }
    public property float3   up             { get { return data.up;             } }
    public property float3   cameraU        { get { return data.cameraU;        } }
    public property float3   cameraV        { get { return data.cameraV;        } }
    public property float3   cameraW        { get { return data.cameraW;        } }
    public property float    nearZ          { get { return data.nearZ;          } }
    public property float    farZ           { get { return data.farZ;           } }
    public property float    aspectRatio    { get { return data.aspectRatio;    } }
    public property float    frameWidth     { get { return data.frameWidth;     } }
    public property float    frameHeight    { get { return data.frameHeight;    } }
    public property float    apertureRadius { get { return data.apertureRadius; } }
    public property float    shutterSpeed   { get { return data.shutterSpeed;   } }
    public property float    ISOSpeed       { get { return data.ISOSpeed;       } }
    public property float    jitterX        { get { return data.jitterX;        } }
    public property float    jitterY        { get { return data.jitterY;        } }
    public property float    focalDistance  { get { return data.focalDistance;  } }

}


public struct FrameData
{
    private Model  _model;
    private Scene  _scene;
    private Camera _activeCamera;

    // Getters
    public property Model  model         { get { return _model;        } }
    public property Scene  scene         { get { return _scene;        } }
    public property Camera activeCamera  { get { return _activeCamera; } }
}


typedef FrameData VertexData;
typedef FrameData PixelData;

}
