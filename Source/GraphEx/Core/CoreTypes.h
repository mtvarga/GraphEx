#pragma once

#include "../Serialization/Serialization.h"
#include "../Utils/ProgramContext.h"


namespace GraphEx::Core
{

struct GRAPHEX_EXPORTABLE Common
{
    static std::filesystem::path getShaderRootFolder();
    static std::filesystem::path getBoundingBoxVertexShaderPath();
    static std::filesystem::path getBoundingBoxPixelShaderPath();
    static std::filesystem::path getAnchorPointVertexShaderPath();
    static std::filesystem::path getAnchorPointPixelShaderPath();
};


struct GRAPHEX_EXPORTABLE Transform : ProgramVarProvider
{
    void setProgramVars(const Falcor::ShaderVar& var) const override;
    void renderGui(Falcor::Gui::Widgets& w);

    template<typename Archive>
    void serialize(Archive& ar);

private:
    void updateWorldMatrices();

    Falcor::float3 mPosition{0.0f};
    Falcor::float3 mRotation{0.0f};
    Falcor::float3 mAnchorPoint{0.0f};
    Falcor::float3 mScale{1.0f};
    bool mLockScaleRatio = false;

    Falcor::float4x4 mTranslationMatrix = Falcor::float4x4::identity();
    Falcor::float4x4 mRotationMatrix = Falcor::float4x4::identity();
    Falcor::float4x4 mScaleMatrix = Falcor::float4x4::identity();

    Falcor::float4x4 mWorldMatrix = Falcor::float4x4::identity();
    Falcor::float4x4 mWorldMatrixInverse = Falcor::float4x4::identity();
    Falcor::float4x4 mWorldMatrixInverseTranspose = Falcor::float4x4::identity();

public:
    DEFAULT_CONST_GETREF_DEFINITION(Position, mPosition)
    DEFAULT_CONST_GETREF_DEFINITION(Rotation, mRotation)
    DEFAULT_CONST_GETREF_DEFINITION(AnchorPoint, mAnchorPoint)
    DEFAULT_CONST_GETREF_DEFINITION(Scale, mScale)
    DEFAULT_CONST_GETTER_SETTER_DEFINITION(LockScaleRatio, mLockScaleRatio)

    void setPosition(const Falcor::float3& value);
    void setRotation(const Falcor::float3& value);
    void setAnchorPoint(const Falcor::float3& value);
    void setScale(const Falcor::float3& value);

    DEFAULT_CONST_GETREF_DEFINITION(WorldMatrix, mWorldMatrix)
    DEFAULT_CONST_GETREF_DEFINITION(WorldMatrixInverse, mWorldMatrixInverse)
    DEFAULT_CONST_GETREF_DEFINITION(WorldMatrixInverseTranspose, mWorldMatrixInverseTranspose)
};


template<typename Archive>
void Transform::serialize(Archive& ar)
{
    ar(SerializeNamed<Archive>("position", mPosition));
    ar(SerializeNamed<Archive>("rotation", mRotation));
    ar(SerializeNamed<Archive>("scale", mScale));
    ar(SerializeNamed<Archive>("lockScaleRatio", mLockScaleRatio));

    if constexpr (IsInputArchive<Archive>())
    {
        // Trigger matrix updates
        setPosition(mPosition);
        setRotation(mRotation);
        setScale(mScale);
    }
}


struct GRAPHEX_EXPORTABLE Material : ProgramVarProvider
{
    void setProgramVars(const Falcor::ShaderVar& var) const override;
    void renderGui(Falcor::Gui::Widgets& w);

    template<typename Archive>
    void serialize(Archive& ar);

private:
    Falcor::float3 mAmbientColor{0.0f};
    Falcor::float3 mDiffuseColor{1.0f};
    Falcor::float3 mSpecularColor{0.0f};
    float mShininess = 1.0f;

public:
    DEFAULT_CONST_GETREF_SETTER_DEFINITION(AmbientColor, mAmbientColor)
    DEFAULT_CONST_GETREF_SETTER_DEFINITION(DiffuseColor, mDiffuseColor)
    DEFAULT_CONST_GETREF_SETTER_DEFINITION(SpecularColor, mSpecularColor)
    DEFAULT_CONST_GETTER_SETTER_DEFINITION(Shininess, mShininess)
};

template<typename Archive>
void Material::serialize(Archive& ar)
{
    ar(SerializeNamed<Archive>("ambientColor", mAmbientColor));
    ar(SerializeNamed<Archive>("diffuseColor", mDiffuseColor));
    ar(SerializeNamed<Archive>("specularColor", mSpecularColor));
    ar(SerializeNamed<Archive>("shininess", mShininess));
}


struct GRAPHEX_EXPORTABLE RenderManager;


struct GRAPHEX_EXPORTABLE Renderable
{
    virtual ~Renderable() = default;

    virtual void initRenderData(Falcor::RenderContext* pRenderContext);

    virtual void preRender(
        const RenderManager& renderManager,
        Falcor::RenderContext* pRenderContext,
        const Falcor::ref<Falcor::Fbo>& pTargetFbo
    );

    virtual void render(
        const RenderManager& renderManager,
        Falcor::RenderContext* pRenderContext,
        const Falcor::ref<Falcor::Fbo>& pTargetFbo
    );

    virtual void postRender(
        const RenderManager& renderManager,
        Falcor::RenderContext* pRenderContext,
        const Falcor::ref<Falcor::Fbo>& pTargetFbo
    );

    virtual bool hasSettings() const;
    virtual void renderSettingsUI(Falcor::Gui::Widgets& w);
};


struct GRAPHEX_EXPORTABLE SceneObject : Renderable, ProgramVarProvider
{
    SceneObject() = default;
    explicit SceneObject(std::string humanReadableName);

    void setProgramVars(const Falcor::ShaderVar& var) const override;
    void renderGui(Falcor::Gui::Widgets& w);

    template<typename Archive>
    void serialize(Archive& ar);

    void invalidate();

private:
    std::string mHumanReadableName;
    Transform mTransform;
    Material mMaterial;
    bool mIsSelected = false;
    bool mValid = true;

public:
    DEFAULT_CONST_GETREF_SETTER_DEFINITION(HumanReadableName, mHumanReadableName)
    DEFAULT_CONST_NONCONST_GETREF_SETTER_DEFINITIONS(Transform, mTransform)
    DEFAULT_CONST_NONCONST_GETREF_SETTER_DEFINITIONS(Material, mMaterial)
    DEFAULT_SETTER_DEFINITION(Selected, mIsSelected)
    bool isSelected() const;
    bool isValid() const;
};


template<typename Archive>
void SceneObject::serialize(Archive& ar)
{
    ar(SerializeNamed<Archive>("humanReadableName", mHumanReadableName));
    ar(SerializeNamed<Archive>("transform", mTransform));
    ar(SerializeNamed<Archive>("material", mMaterial));
}


struct GRAPHEX_EXPORTABLE Light : Falcor::Object, ProgramVarProvider
{
    enum class Type : Falcor::uint
    {
        Point = 0,
        Directional = 1,
        MAX = 2
    };

    FALCOR_ENUM_INFO(Type, {{Type::Point, "Point"}, {Type::Directional, "Directional"}})

    Light() = default;

    explicit Light(Type type) : mType(type) {}

    void setProgramVars(const Falcor::ShaderVar& var) const override;
    void renderGui(Falcor::Gui::Widgets& w);

    template<typename Archive>
    void serialize(Archive& ar);

private:
    Type mType{Type::Point};
    Falcor::float3 mPosition{0.0f, 1.0f, 1.0f};
    Falcor::float3 mDirection{0.0f, 1.0f, 1.0f};
    Falcor::float3 mAmbientColor{0.125f};
    Falcor::float3 mDiffuseColor{1.0f};
    Falcor::float3 mSpecularColor{0.0f};
    float mConstantAttenuation = 1.0f;
    float mLinearAttenuation = 0.0f;
    float mQuadraticAttenuation = 0.0f;

public:
    DEFAULT_CONST_GETREF_SETTER_DEFINITION(Position, mPosition)
    DEFAULT_CONST_GETREF_SETTER_DEFINITION(Direction, mDirection)
    DEFAULT_CONST_GETREF_SETTER_DEFINITION(AmbientColor, mAmbientColor)
    DEFAULT_CONST_GETREF_SETTER_DEFINITION(DiffuseColor, mDiffuseColor)
    DEFAULT_CONST_GETREF_SETTER_DEFINITION(SpecularColor, mSpecularColor)
    DEFAULT_CONST_GETTER_SETTER_DEFINITION(ConstantAttenuation, mConstantAttenuation)
    DEFAULT_CONST_GETTER_SETTER_DEFINITION(LinearAttenuation, mLinearAttenuation)
    DEFAULT_CONST_GETTER_SETTER_DEFINITION(QuadraticAttenuation, mQuadraticAttenuation)
};


template<typename Archive>
void Light::serialize(Archive& ar)
{
    if constexpr (IsInputArchive<Archive>())
    {
        int type;
        ar(SerializeNamed<Archive>("type", type));

        if (type >= static_cast<int>(Type::MAX))
        {
            Falcor::logWarning("Unsupported light type was read from project file: '{}'. Resetting light to type 'Point'.", type);
            type = static_cast<int>(Type::Point);
        }

        mType = static_cast<Type>(type);
    }
    else
    {
        ar(SerializeNamed<Archive>("type", static_cast<int>(mType)));
    }

    ar(SerializeNamed<Archive>("position", mPosition));
    ar(SerializeNamed<Archive>("direction", mDirection));
    ar(SerializeNamed<Archive>("ambientColor", mAmbientColor));
    ar(SerializeNamed<Archive>("diffuseColor", mDiffuseColor));
    ar(SerializeNamed<Archive>("specularColor", mSpecularColor));
    ar(SerializeNamed<Archive>("constantAttenuation", mConstantAttenuation));
    ar(SerializeNamed<Archive>("linearAttenuation", mLinearAttenuation));
    ar(SerializeNamed<Archive>("quadraticAttenuation", mQuadraticAttenuation));
}


FALCOR_ENUM_REGISTER(Light::Type)


struct GRAPHEX_EXPORTABLE SerializedSceneObjectState : PolymorphicSafeAnchor<std::shared_ptr<SceneObject>>
{
    SerializedSceneObjectState() : PolymorphicSafeAnchor() {}

    explicit SerializedSceneObjectState(std::shared_ptr<SceneObject> value)
        : PolymorphicSafeAnchor(std::move(value)) {}

    template<typename Archive>
    void serialize(Archive& ar);
};


template<typename Archive>
void SerializedSceneObjectState::serialize(Archive& ar)
{
    // Will catch any serialization-related exceptions (like unregistered type, etc.)
    if constexpr (IsInputArchive<Archive>())
    {
        Internal::SerializationManager::get().pushInvalidityList();
    }

    PolymorphicSafeAnchor::serialize(ar);

    if constexpr (IsInputArchive<Archive>())
    {
        if (const auto pInvalidityList = Internal::SerializationManager::get().popInvalidityList();
            !pInvalidityList->empty())
        {
            // There was at least one semantic invalidity found -> warn the user
            std::ostringstream os;
            os << "Could not load scene object from project file. See reasons below:" << "\n";

            for (auto i = 0; i < pInvalidityList->size(); ++i)
            {
                os << pInvalidityList->at(i);

                if (i != pInvalidityList->size() - 1)
                {
                    os << "\n";
                }
            }

            msgBox("Warning", os.str(), Falcor::MsgBoxType::Ok, Falcor::MsgBoxIcon::Warning);
        }
    }

    if (!success())
    {
        Falcor::logWarning(
            "Scene object could not be serialized properly. Perhaps it is derived of a type that was not registered "
            "as serializable using GRAPHEX_REGISTER_SERIALIZABLE, or you didn't use GraphEx::SerializeBase() to "
            "serialize the base SceneObject class? Semi-compatible project file? All deriving classes of SceneObject "
            "should be serializable!"
        );
    }
}

} // namespace GraphEx::Core
