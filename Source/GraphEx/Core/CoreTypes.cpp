#include "CoreTypes.h"


using namespace GraphEx::Core;


constexpr auto SIGNED_TRANSFORM_SLIDER_LIMIT_LOWER = -10000.f;
constexpr auto TRANSFORM_SLIDER_LIMIT_UPPER = 10000.f;


constexpr auto MODEL_PROPERTIES_SHADER_VAR_NAME = "graphExModel";


void Transform::renderGui(Falcor::Gui::Widgets& w)
{
    auto position = mPosition;
    auto rotation = mRotation;
    auto anchorPoint = mAnchorPoint;
    auto scale = mScale;

    if (w.var("Position", position, SIGNED_TRANSFORM_SLIDER_LIMIT_LOWER, TRANSFORM_SLIDER_LIMIT_UPPER, 0.5f))
    {
        setPosition(position);
    }

    if (w.var("Rotation", rotation, SIGNED_TRANSFORM_SLIDER_LIMIT_LOWER, TRANSFORM_SLIDER_LIMIT_UPPER, 0.5f))
    {
        setRotation(rotation);
    }

    if (w.var("Anchor Point", anchorPoint, SIGNED_TRANSFORM_SLIDER_LIMIT_LOWER, TRANSFORM_SLIDER_LIMIT_UPPER, 0.5f))
    {
        setAnchorPoint(anchorPoint);
    }

    if (w.var("Scale", scale, 1e-3f, TRANSFORM_SLIDER_LIMIT_UPPER, 0.1f))
    {
        setScale(scale);
    }

    w.checkbox("Lock Scale Ratio", mLockScaleRatio);
}


void Transform::updateWorldMatrices()
{
    const auto anchorTranslation = Falcor::math::matrixFromTranslation(-mAnchorPoint);
    const auto inverseAnchorTranslation = Falcor::math::matrixFromTranslation(mAnchorPoint);

    mWorldMatrix = mul(mTranslationMatrix, mul(inverseAnchorTranslation, mul(mRotationMatrix, mul(mScaleMatrix, anchorTranslation))));
    mWorldMatrixInverse = Falcor::math::inverse(mWorldMatrix);
    mWorldMatrixInverseTranspose = Falcor::math::transpose(mWorldMatrixInverse);
}


void Transform::setPosition(const Falcor::float3& value)
{
    mPosition = value;
    mTranslationMatrix = Falcor::math::matrixFromTranslation(mPosition);
    updateWorldMatrices();
}


void Transform::setRotation(const Falcor::float3& value)
{
    mRotation = value;

    const auto rotationRad = Falcor::math::radians(mRotation);
    const auto rotation = Falcor::math::matrixFromRotationXYZ(rotationRad.x, rotationRad.y, rotationRad.z);

    mRotationMatrix = rotation;

    updateWorldMatrices();
}


void Transform::setAnchorPoint(const Falcor::float3& value)
{
    mAnchorPoint = value;
    setRotation(mRotation);
}


void Transform::setScale(const Falcor::float3& value)
{
    if (mLockScaleRatio)
    {
        Falcor::uint diffCount = 0;
        Falcor::uint differentIndex = 0;

        for (int i = 0; i < 3; i++)
        {
            if (mScale[i] != value[i])
            {
                diffCount++;
                differentIndex = i;
            }
        }

        if (diffCount == 1 && value[differentIndex] > 0.0f && mScale[differentIndex] > 0.0f)
        {
            const auto ratio = value[differentIndex] / mScale[differentIndex];
            mScale *= ratio;
            mScaleMatrix = Falcor::math::matrixFromScaling(mScale);
            updateWorldMatrices();
            return;
        }
    }

    mScale = value;
    mScaleMatrix = Falcor::math::matrixFromScaling(mScale);
    updateWorldMatrices();
}


void Material::renderGui(Falcor::Gui::Widgets& w)
{
    ImGui::PushItemWidth(200);
    w.rgbColor("Ambient Color", mAmbientColor);
    w.rgbColor("Diffuse Color", mDiffuseColor);
    w.rgbColor("Specular Color", mSpecularColor);
    ImGui::PopItemWidth();
    w.var("Shininess", mShininess, 0.0f, 100.0f, 0.001f);
}


std::filesystem::path Common::getShaderRootFolder()
{
    return GRAPHEX_SHADER_DIR;
}


std::filesystem::path Common::getBoundingBoxVertexShaderPath()
{
    return getShaderRootFolder() / "RenderBoundingBox.vs.slang";
}


std::filesystem::path Common::getBoundingBoxPixelShaderPath()
{
    return getShaderRootFolder() / "RenderBoundingBox.ps.slang";
}


std::filesystem::path Common::getAnchorPointVertexShaderPath()
{
    return getShaderRootFolder() / "RenderAnchorPoint.vs.slang";
}


std::filesystem::path Common::getAnchorPointPixelShaderPath()
{
    return getShaderRootFolder() / "RenderAnchorPoint.ps.slang";
}


void Transform::setProgramVars(const Falcor::ShaderVar& var) const
{
    trySetProgramVar(var, "_anchorPoint", mAnchorPoint);
    trySetProgramVar(var, "_translationMatrix", mTranslationMatrix);
    trySetProgramVar(var, "_rotationMatrix", mRotationMatrix);
    trySetProgramVar(var, "_scaleMatrix", mScaleMatrix);
    trySetProgramVar(var, "_worldMatrix", mWorldMatrix);
    trySetProgramVar(var, "_worldMatrixInverse", mWorldMatrixInverse);
    trySetProgramVar(var, "_worldMatrixInverseTranspose", mWorldMatrixInverseTranspose);
}


void Material::setProgramVars(const Falcor::ShaderVar& var) const
{
    trySetProgramVar(var, "_ambientColor", mAmbientColor);
    trySetProgramVar(var, "_diffuseColor", mDiffuseColor);
    trySetProgramVar(var, "_specularColor", mSpecularColor);
    trySetProgramVar(var, "_shininess", mShininess);
}


void Renderable::initRenderData(Falcor::RenderContext* pRenderContext) {}


void Renderable::preRender(
    const RenderManager& renderManager,
    Falcor::RenderContext* pRenderContext,
    const Falcor::ref<Falcor::Fbo>& pTargetFbo
) {}


void Renderable::render(
    const RenderManager& renderManager,
    Falcor::RenderContext* pRenderContext,
    const Falcor::ref<Falcor::Fbo>& pTargetFbo
) {}


void Renderable::postRender(
    const RenderManager& renderManager,
    Falcor::RenderContext* pRenderContext,
    const Falcor::ref<Falcor::Fbo>& pTargetFbo
) {}


bool Renderable::hasSettings() const
{
    return false;
}


void Renderable::renderSettingsUI(Falcor::Gui::Widgets& w) {}


SceneObject::SceneObject(std::string humanReadableName)
    : mHumanReadableName(std::move(humanReadableName)) {}


void SceneObject::setProgramVars(const Falcor::ShaderVar& var) const
{
    trySetProgramVarsFor(var, "_transform", mTransform);
    trySetProgramVarsFor(var, "_material", mMaterial);
}


void SceneObject::renderGui(Falcor::Gui::Widgets& w)
{
    if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Render Transform properties
        mTransform.renderGui(w);
        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("Material", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Render Material properties
        mMaterial.renderGui(w);
        ImGui::TreePop();
    }
}


void SceneObject::invalidate()
{
    mValid = false;
}


bool SceneObject::isSelected() const
{
    return mIsSelected;
}


bool SceneObject::isValid() const
{
    return mValid;
}


void Light::setProgramVars(const Falcor::ShaderVar& var) const
{
    trySetProgramVar(var, "_type", static_cast<Falcor::uint>(mType));
    trySetProgramVar(var, "_position", mPosition);
    trySetProgramVar(var, "_direction", mDirection);
    trySetProgramVar(var, "_ambientColor", mAmbientColor);
    trySetProgramVar(var, "_diffuseColor", mDiffuseColor);
    trySetProgramVar(var, "_specularColor", mSpecularColor);
    trySetProgramVar(var, "_constantAttenuation", mConstantAttenuation);
    trySetProgramVar(var, "_linearAttenuation", mLinearAttenuation);
    trySetProgramVar(var, "_quadraticAttenuation", mQuadraticAttenuation);
}


void Light::renderGui(Falcor::Gui::Widgets& w)
{
    w.dropdown("Type", mType);

    switch (mType)
    {
    case Type::Point:
        w.var("Position", mPosition, -10000.0f, 10000.0f, 0.5f);
        break;
    case Type::Directional:
        w.var("Direction", mDirection, -1.0f, 1.0f, 0.05f);
        break;
    default:
        break;
    }

    ImGui::PushItemWidth(200);
    w.rgbColor("Ambient Color", mAmbientColor);
    w.rgbColor("Diffuse Color", mDiffuseColor);
    w.rgbColor("Specular Color", mSpecularColor);
    ImGui::PopItemWidth();
    w.var("Constant Attenuation", mConstantAttenuation, 0.0f, 100.0f, 0.1f);
    w.var("Linear Attenuation", mLinearAttenuation, 0.0f, 100.0f, 0.1f);
    w.var("Quadratic Attenuation", mQuadraticAttenuation, 0.0f, 100.0f, 0.1f);
}