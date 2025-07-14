#pragma once

#include "SerializationMacros.h"


namespace GraphEx
{

using SerializationAccess = cereal::access;

template<typename T>
using SerializeBase = cereal::base_class<T>;

template<typename T>
using SerializeVirtualBase = cereal::virtual_base_class<T>;

using InputArchive = cereal::JSONInputArchive;
using OutputArchive = cereal::JSONOutputArchive;


template<typename Archive, typename T>
auto SerializeNamed(const char* name, T&& value) -> decltype(auto)
{
    return cereal::make_nvp<Archive>(name, std::forward<T>(value));
}


template<typename T>
auto SaveNamed(const char* name, T&& value) -> decltype(auto)
{
    return SerializeNamed<OutputArchive>(name, std::forward<T>(value));
}


template<typename T>
auto LoadNamed(const char* name, T&& value) -> decltype(auto)
{
    return SerializeNamed<InputArchive>(name, std::forward<T>(value));
}


template<typename Archive>
constexpr bool IsOutputArchive()
{
    return std::is_same_v<OutputArchive, Archive>;
}


template<typename Archive>
constexpr bool IsInputArchive()
{
    return std::is_same_v<InputArchive, Archive>;
}


template<typename UnsafeWrappedT>
struct PolymorphicSafeAnchor
{
    static_assert(std::is_default_constructible_v<UnsafeWrappedT>, "SafeAnchor: Unsafe underlying type must be default constructible");
    static_assert(std::is_move_constructible_v<UnsafeWrappedT>, "SafeAnchor: Unsafe underlying type must be move constructible");
    static_assert(std::is_copy_assignable_v<UnsafeWrappedT>, "SafeAnchor: Unsafe underlying type must be copy assignable");

    PolymorphicSafeAnchor() = default;
    explicit PolymorphicSafeAnchor(UnsafeWrappedT value)
        : mAnchor(std::move(value)) {}

    template<typename Archive>
    void serialize(Archive& ar);

private:
    UnsafeWrappedT mAnchor;
    bool mSuccess = false;

public:
    DEFAULT_CONST_GETREF_DEFINITION(, mAnchor)

    bool success() const
    {
        return mSuccess;
    }
};


template<typename UnsafeWrappedT>
template<typename Archive>
void PolymorphicSafeAnchor<UnsafeWrappedT>::serialize(Archive& ar)
{
    try
    {
        ar(SerializeNamed<Archive>("polymorphic_safe_anchor", mAnchor));
        mSuccess = true;
    }
    catch (cereal::Exception& e)
    {
        Falcor::logWarning("Could not safely serialize an object: " + std::string(e.what()));

        // At least one node was interrupted, so calling this manually won't mess too much with the internal structure of the archive
        // If something went wrong inside the safe anchor, aka. the reason why we caught is not an UNREGISTERED_POLYMORPHIC_TYPE, we
        // are dealing with a corrupt archive
        ar.finishNode();
    }
}


template<typename TagT, typename UnsafeWrappedT>
struct TaggedPolymorphicSafeAnchor : PolymorphicSafeAnchor<UnsafeWrappedT>
{
    static_assert(std::is_default_constructible_v<TagT>, "TaggedSafeAnchor: If given, tag type must be default constructible");
    static_assert(std::is_move_constructible_v<TagT>, "TaggedSafeAnchor: If given, tag type must be move constructible");
    static_assert(std::is_copy_assignable_v<TagT>, "TaggedSafeAnchor: If given, tag type must be copy assignable");

    TaggedPolymorphicSafeAnchor() = default;
    TaggedPolymorphicSafeAnchor(TagT tag, UnsafeWrappedT value) : PolymorphicSafeAnchor<UnsafeWrappedT>(std::move(value)), mTag(std::move(tag)) {}

    template<typename Archive>
    void serialize(Archive& ar);

private:
    TagT mTag;

public:
    DEFAULT_CONST_GETTER_DEFINITION(, std::tie(this->mTag, PolymorphicSafeAnchor<UnsafeWrappedT>::get()))
};


template<typename TagT, typename UnsafeWrappedT>
template<typename Archive>
void TaggedPolymorphicSafeAnchor<TagT, UnsafeWrappedT>::serialize(Archive& ar)
{
    ar(SerializeNamed<Archive>("polymorphic_safe_anchor_tag", mTag));
    PolymorphicSafeAnchor<UnsafeWrappedT>::serialize(ar);
}

} // namespace GraphEx
