#pragma once

#include "SerializationMacros.h"
#include "SerializationTemplates.h"


GRAPHEX_SERIALIZATION_SPECIFY_BEGIN

GRAPHEX_SERIALIZATION_SAVE_FUNCTION(ar, Falcor::Camera, camera)
{
    const auto name = camera.getName();
    const auto focalLength = camera.getFocalLength();
    const auto aspectRatio = camera.getAspectRatio();
    const auto focalDistance = camera.getFocalDistance();
    const auto apertureRadius = camera.getApertureRadius();
    const auto shutterSpeed = camera.getShutterSpeed();
    const auto isoSpeed = camera.getISOSpeed();
    const auto nearPlane = camera.getNearPlane();
    const auto farPlane = camera.getFarPlane();
    const auto position = camera.getPosition();
    const auto target = camera.getTarget();
    const auto upVector = camera.getUpVector();

    ar(GRAPHEX_SERIALIZE_WITH_NAME(name));
    ar(GRAPHEX_SERIALIZE_WITH_NAME(focalLength));
    ar(GRAPHEX_SERIALIZE_WITH_NAME(aspectRatio));
    ar(GRAPHEX_SERIALIZE_WITH_NAME(focalDistance));
    ar(GRAPHEX_SERIALIZE_WITH_NAME(apertureRadius));
    ar(GRAPHEX_SERIALIZE_WITH_NAME(shutterSpeed));
    ar(GRAPHEX_SERIALIZE_WITH_NAME(isoSpeed));
    ar(GRAPHEX_SERIALIZE_WITH_NAME(nearPlane));
    ar(GRAPHEX_SERIALIZE_WITH_NAME(farPlane));
    ar(GRAPHEX_SERIALIZE_WITH_NAME(position));
    ar(GRAPHEX_SERIALIZE_WITH_NAME(target));
    ar(GRAPHEX_SERIALIZE_WITH_NAME(upVector));
}


GRAPHEX_SERIALIZATION_LOAD_FUNCTION(ar, Falcor::Camera, camera)
{
    std::string name;        ar(GRAPHEX_SERIALIZE_WITH_NAME(name));
    float focalLength;       ar(GRAPHEX_SERIALIZE_WITH_NAME(focalLength));
    float aspectRatio;       ar(GRAPHEX_SERIALIZE_WITH_NAME(aspectRatio));
    float focalDistance;     ar(GRAPHEX_SERIALIZE_WITH_NAME(focalDistance));
    float apertureRadius;    ar(GRAPHEX_SERIALIZE_WITH_NAME(apertureRadius));
    float shutterSpeed;      ar(GRAPHEX_SERIALIZE_WITH_NAME(shutterSpeed));
    float isoSpeed;          ar(GRAPHEX_SERIALIZE_WITH_NAME(isoSpeed));
    float nearPlane;         ar(GRAPHEX_SERIALIZE_WITH_NAME(nearPlane));
    float farPlane;          ar(GRAPHEX_SERIALIZE_WITH_NAME(farPlane));
    Falcor::float3 position; ar(GRAPHEX_SERIALIZE_WITH_NAME(position));
    Falcor::float3 target;   ar(GRAPHEX_SERIALIZE_WITH_NAME(target));
    Falcor::float3 upVector; ar(GRAPHEX_SERIALIZE_WITH_NAME(upVector));

    camera.setName(name);
    camera.setFocalLength(focalLength);
    camera.setAspectRatio(aspectRatio);
    camera.setFocalDistance(focalDistance);
    camera.setApertureRadius(apertureRadius);
    camera.setShutterSpeed(shutterSpeed);
    camera.setISOSpeed(isoSpeed);
    camera.setNearPlane(nearPlane);
    camera.setFarPlane(farPlane);
    camera.setPosition(position);
    camera.setTarget(target);
    camera.setUpVector(upVector);
}


GRAPHEX_SERIALIZATION_SPECIFY_CONSTRUCT_BEGIN(Falcor::Camera)

GRAPHEX_SERIALIZATION_CONSTRUCT_FUNCTION(ar, Falcor::Camera, constructCamera)
{
    constructCamera("");
    load(ar, *constructCamera.ptr());
}

GRAPHEX_SERIALIZATION_SPECIFY_CONSTRUCT_END

GRAPHEX_SERIALIZATION_SPECIFY_END
