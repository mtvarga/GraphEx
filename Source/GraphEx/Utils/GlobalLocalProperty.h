#pragma once

#include "Standard.h"
#include "ProgramContext.h"
#include "../Serialization/Serialization.h"


#define GlocalInit(propertyName, defaultValue) \
    propertyName( \
        this, \
        (hasGlobalPropertySet() \
            ? getGlobalPropertySet()->getLocalValueFor(getGlobalPropertySet()->propertyName) \
            : defaultValue), \
        hasGlobalPropertySet() \
    )

#define GlocalGet(pPropertySet, propertyName) (\
    pPropertySet->hasGlobalPropertySet() \
        ? pPropertySet->shouldUseGlobalFor(pPropertySet->propertyName) \
            ? pPropertySet->getGlobalPropertySet()->getLocalValueFor(pPropertySet->getGlobalPropertySet()->propertyName) \
            : pPropertySet->getLocalValueFor(pPropertySet->propertyName) \
        : pPropertySet->getLocalValueFor(pPropertySet->propertyName) \
)

#define GlocalGetMy(propertyName) \
    GlocalGet(this, propertyName)


#define GlocalSet(pPropertySet, propertyName, value) \
    pPropertySet->setLocalValueFor(pPropertySet->propertyName, value)

#define GlocalSetMy(propertyName, value) \
    GlocalSet(this, propertyName, value)



namespace GraphEx
{

template<typename T>
struct GlocalProperty;


struct GRAPHEX_EXPORTABLE GlocalPropertySetBase
{
    virtual ~GlocalPropertySetBase() = default;

    template<typename PropertyValueT>
    void setUseGlobalFor(GlocalProperty<PropertyValueT>& property, bool useGlobal);

    template<typename PropertyValueT>
    void switchUseGlobalFor(GlocalProperty<PropertyValueT>& property);

    template<typename PropertyValueT>
    bool shouldUseGlobalFor(const GlocalProperty<PropertyValueT>& property) const;

    template<typename PropertyValueT>
    const PropertyValueT& getLocalValueFor(const GlocalProperty<PropertyValueT>& property) const;

    template<typename PropertyValueT>
    void setLocalValueFor(GlocalProperty<PropertyValueT>& property, const PropertyValueT& value);

    template<typename Archive>
    void serialize(Archive& ar);

    virtual bool hasGlobalPropertySet() const = 0;

protected:
    virtual void increasePropertyCountUsingGlobal();
    virtual void decreasePropertyCountUsingGlobal();

private:
    unsigned int mPropertyCountUsingGlobal = 0;

public:
    bool hasPropertiesUsingGlobal() const;

    template<typename T>
    friend struct GlocalProperty;
};


template<typename PropertyValueT>
void GlocalPropertySetBase::setUseGlobalFor(GlocalProperty<PropertyValueT>& property, bool useGlobal)
{
    if (property.getOwner() != this)
    {
        FALCOR_THROW("Attempted to set useGlobal for a property that does not belong to this GlocalPropertySet");
    }

    property.setUseGlobal(useGlobal);

    if (property.shouldUseGlobal())
    {
        increasePropertyCountUsingGlobal();
    }
    else
    {
        decreasePropertyCountUsingGlobal();
    }
}


template<typename PropertyValueT>
void GlocalPropertySetBase::switchUseGlobalFor(GlocalProperty<PropertyValueT>& property)
{
    if (property.getOwner() != this)
    {
        FALCOR_THROW("Attempted to switch useGlobal for a property that does not belong to this GlocalPropertySet");
    }

    property.switchUseGlobal();

    if (property.shouldUseGlobal())
    {
        increasePropertyCountUsingGlobal();
    }
    else
    {
        decreasePropertyCountUsingGlobal();
    }
}


template<typename PropertyValueT>
bool GlocalPropertySetBase::shouldUseGlobalFor(const GlocalProperty<PropertyValueT>& property) const
{
    if (property.getOwner() != this)
    {
        FALCOR_THROW("Attempted to get useGlobal for a property that does not belong to this GlocalPropertySet");
    }

    return property.shouldUseGlobal();
}


template<typename PropertyValueT>
const PropertyValueT& GlocalPropertySetBase::getLocalValueFor(const GlocalProperty<PropertyValueT>& property) const
{
    if (property.getOwner() != this)
    {
        FALCOR_THROW("Attempted to get value for a property that does not belong to this GlocalPropertySet");
    }

    return property.getValue();
}


template<typename PropertyValueT>
void GlocalPropertySetBase::setLocalValueFor(GlocalProperty<PropertyValueT>& property, const PropertyValueT& value)
{
    if (property.getOwner() != this)
    {
        FALCOR_THROW("Attempted to set value for a property that does not belong to this GlocalPropertySet");
    }

    property.setValue(value);
}


template<typename Archive>
void GlocalPropertySetBase::serialize(Archive& ar)
{
    ar(SerializeNamed<Archive>("propertyCountUsingGlobal", mPropertyCountUsingGlobal));
}


template<typename ValueT>
struct GlocalProperty
{
    static_assert(std::is_default_constructible_v<ValueT>, "GlobalLocalProperty requires the ValueT to be default constructible");

    using ValueType = ValueT;

    explicit GlocalProperty(GlocalPropertySetBase* pOwner, ValueType value = ValueType(), const bool useGlobal = true);
    virtual ~GlocalProperty();

    void renderGui(
        Falcor::Gui::Widgets& w,
        bool globalCheckboxSameLine,
        const std::function<void(Falcor::Gui::Widgets&)>& renderGui,
        const std::function<void()>& onUseGlobalChange = [] {}
    );

    template<typename Archive>
    void serialize(Archive& ar);

private:
    GlocalPropertySetBase* mpOwner;
    ValueType mValue;
    bool mUseGlobal = false;
    bool bUseGlobal = false;

    void setUseGlobal(bool value);
    void switchUseGlobal();
    bool hasGlobal() const;
    bool shouldUseGlobal() const;

    DEFAULT_CONST_GETTER_DEFINITION(Owner, mpOwner)
    DEFAULT_CONST_NONCONST_GETREF_SETTER_DEFINITIONS(Value, mValue)

    friend struct GlocalPropertySetBase;
};


template<typename ValueT>
GlocalProperty<ValueT>::GlocalProperty(GlocalPropertySetBase* pOwner, ValueType value, const bool useGlobal)
    : mpOwner(pOwner), mValue(std::move(value)), mUseGlobal(hasGlobal() ? useGlobal : false), bUseGlobal(hasGlobal() ? useGlobal : false)
{
    FALCOR_CHECK(mpOwner != nullptr, "GlobalLocalProperty: Attempted to construct without proper owning GlobalLocalPropertySet");

    if (shouldUseGlobal())
    {
        mpOwner->increasePropertyCountUsingGlobal();
    }
}


template<typename ValueT>
GlocalProperty<ValueT>::~GlocalProperty()
{
    if (shouldUseGlobal())
    {
        mpOwner->decreasePropertyCountUsingGlobal();
    }
}


template<typename T>
void GlocalProperty<T>::renderGui(
    Falcor::Gui::Widgets& w,
    const bool globalCheckboxSameLine,
    const std::function<void(Falcor::Gui::Widgets&)>& renderGui,
    const std::function<void()>& onUseGlobalChange
) {
    if (hasGlobal())
    {
        const auto checkboxLabel = "Global##" + std::to_string(reinterpret_cast<std::uintptr_t>(&mUseGlobal));

        if (w.checkbox(checkboxLabel.c_str(), bUseGlobal))
        {
            mpOwner->setUseGlobalFor(*this, bUseGlobal);
            onUseGlobalChange();
        }

        if (globalCheckboxSameLine)
        {
            ImGui::SameLine();
        }
    }

    if (shouldUseGlobal())
    {
        ImGui::BeginDisabled();
    }

    renderGui(w);

    if (shouldUseGlobal())
    {
        ImGui::EndDisabled();
    }
}


template<typename ValueT>
template<typename Archive>
void GlocalProperty<ValueT>::serialize(Archive& ar)
{
    ar(SerializeNamed<Archive>("value", mValue));
    ar(SerializeNamed<Archive>("useGlobal", mUseGlobal));

    if constexpr (IsInputArchive<Archive>())
    {
        bUseGlobal = mUseGlobal;
    }
}


template<typename ValueT>
bool GlocalProperty<ValueT>::hasGlobal() const
{
    return mpOwner->hasGlobalPropertySet();
}


template<typename ValueT>
bool GlocalProperty<ValueT>::shouldUseGlobal() const
{
    return hasGlobal() && mUseGlobal;
}


template<typename ValueT>
void GlocalProperty<ValueT>::setUseGlobal(const bool value)
{
    mUseGlobal = value;
    bUseGlobal = value;
}


template<typename ValueT>
void GlocalProperty<ValueT>::switchUseGlobal()
{
    mUseGlobal = !mUseGlobal;
    bUseGlobal = mUseGlobal;
}


template<typename SelfT>
struct GlocalPropertySet : GlocalPropertySetBase
{
    GlocalPropertySet() = default;
    explicit GlocalPropertySet(std::shared_ptr<SelfT> pGlobalPropertySet)
        : mpGlobalPropertySet(std::move(pGlobalPropertySet)) { }

    template<typename Archive>
    void serialize(Archive& ar);

    std::shared_ptr<SelfT> getGlobalPropertySet() const;
    bool hasGlobalPropertySet() const override;

protected:
    std::shared_ptr<SelfT> mpGlobalPropertySet;
};


template<typename SelfT>
template<typename Archive>
void GlocalPropertySet<SelfT>::serialize(Archive& ar)
{
    GlocalPropertySetBase::serialize(ar);
    ar(SerializeNamed<Archive>("globalPropertySet", mpGlobalPropertySet)); // SelfT must be registered with GRAPHEX_REGISTER_SERIALIZABLE
}


template<typename SelfT>
bool GlocalPropertySet<SelfT>::hasGlobalPropertySet() const
{
    return mpGlobalPropertySet != nullptr;
}


template<typename SelfT>
std::shared_ptr<SelfT> GlocalPropertySet<SelfT>::getGlobalPropertySet() const
{
    return mpGlobalPropertySet;
}


template<typename SelfT>
struct GlocalProgramContextProvider : BindableProgramContextProvider, GlocalPropertySet<SelfT>
{
    explicit GlocalProgramContextProvider(std::shared_ptr<SelfT> pGlobalPropertySet);

    void bindProgram(ProgramWrapper* pProgram, CycleGuard guard = nullptr) override;
    void associateProgram(ProgramWrapper* pProgram, CycleGuard guard = nullptr) override;
    void releaseProgram(ProgramWrapper* pProgram, CycleGuard guard = nullptr) override;

protected:
    void increasePropertyCountUsingGlobal() override;
    void decreasePropertyCountUsingGlobal() override;
};


template<typename SelfT>
GlocalProgramContextProvider<SelfT>::GlocalProgramContextProvider(std::shared_ptr<SelfT> pGlobalPropertySet)
    : GlocalPropertySet<SelfT>(std::move(pGlobalPropertySet)) {}


template<typename SelfT>
void GlocalProgramContextProvider<SelfT>::bindProgram(ProgramWrapper* pProgram, CycleGuard guard)
{
    BindableProgramContextProvider::bindProgram(pProgram, guard);

    if (this->hasPropertiesUsingGlobal())
    {
        // Reduce to association only
        this->mpGlobalPropertySet->associateProgram(pProgram, guard);
    }
}


template<typename SelfT>
void GlocalProgramContextProvider<SelfT>::associateProgram(ProgramWrapper* pProgram, CycleGuard guard)
{
    BindableProgramContextProvider::associateProgram(pProgram, guard);

    if (this->hasPropertiesUsingGlobal())
    {
        this->mpGlobalPropertySet->associateProgram(pProgram, guard);
    }
}


template<typename SelfT>
void GlocalProgramContextProvider<SelfT>::releaseProgram(ProgramWrapper* pProgram, CycleGuard guard)
{
    BindableProgramContextProvider::releaseProgram(pProgram, guard);

    if (this->hasPropertiesUsingGlobal())
    {
        this->mpGlobalPropertySet->releaseProgram(pProgram, guard);
    }
}


template<typename SelfT>
void GlocalProgramContextProvider<SelfT>::increasePropertyCountUsingGlobal()
{
    const auto old = this->hasPropertiesUsingGlobal();
    GlocalPropertySet<SelfT>::increasePropertyCountUsingGlobal();

    if (!old && this->hasPropertiesUsingGlobal())
    {
        for (const auto& [ pProgram, shouldRemove ] : this->getBoundPrograms())
        {
            this->mpGlobalPropertySet->associateProgram(pProgram);
        }
    }
}


template<typename SelfT>
void GlocalProgramContextProvider<SelfT>::decreasePropertyCountUsingGlobal()
{
    const auto old = this->hasPropertiesUsingGlobal();
    GlocalPropertySet<SelfT>::decreasePropertyCountUsingGlobal();

    if (old && !this->hasPropertiesUsingGlobal())
    {
        for (const auto& [ pProgram, shouldRemove ] : this->getBoundPrograms())
        {
            this->mpGlobalPropertySet->releaseProgram(pProgram);
        }
    }
}

} // namespace GraphEx
