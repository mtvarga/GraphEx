#pragma once

#include "Standard.h"


namespace GraphEx
{
class GRAPHEX_EXPORTABLE ProgramWrapper;


struct GRAPHEX_EXPORTABLE ProgramDefineProvider
{
    virtual ~ProgramDefineProvider() = default;
    virtual void addProgramDefines(Falcor::DefineList& defines) const;
};


struct GRAPHEX_EXPORTABLE ProgramIncludeProvider : ProgramDefineProvider
{
    struct IncludeInfo
    {
        std::string includeDefineName;
        std::string includeString;
    };

    virtual IncludeInfo getProgramIncludeInfo() const = 0;
    void addProgramDefines(Falcor::DefineList& defines) const override;
};


struct GRAPHEX_EXPORTABLE ProgramVarProvider
{
    virtual ~ProgramVarProvider() = default;
    virtual void setProgramVars(const Falcor::ShaderVar& var) const;

    static void trySetProgramVarsFor(const Falcor::ShaderVar& var, const std::string& memberName, const ProgramVarProvider& varProvider);

    template<typename T>
    static void trySetProgramVar(const Falcor::ShaderVar& var, const std::string& memberName, const T& memberValue);
};


template<typename T>
void ProgramVarProvider::trySetProgramVar(const Falcor::ShaderVar& var, const std::string& memberName, const T& memberValue)
{
    if (!var.hasMember(memberName))
    {
        return;
    }

    var[memberName] = memberValue;
}


struct GRAPHEX_EXPORTABLE ProgramContextProvider : ProgramDefineProvider, ProgramVarProvider {};
struct GRAPHEX_EXPORTABLE ProgramIncludedContextProvider : ProgramIncludeProvider, ProgramVarProvider {};


// I can be used if me or one of my direct or indirect descendants supports an operation that would cause the program whom we provide
// defines and vars for to have to recompile. If I have a member who is also a BindableProgramContextProvider, that would be a NESTED
// provider. If a nester provider provides defines and vars to the same program as I do, I can list it in the getNestedContextProviders()
// method. In the default implementation of binding, associating and releasing, providers returned by this method are to automatically
// bound/associated/released to/from the same program when that respective operation is done to me. If a nested provider isn't listed there,
// its binding/associating/releasing these must be done manually. These operations are customizable (maybe to customize their behaviors for
// some nested providers), but always make sure to call the base class implementation. See an example of this in
// GlobalLocalProgramContextProvider. When using getNestedContextProviders(), make sure to call the base class implementation too, and only
// append to the vector, don't overwrite it!
struct GRAPHEX_EXPORTABLE BindableProgramContextProvider : ProgramContextProvider
{
    using CycleGuard = std::shared_ptr<std::unordered_set<BindableProgramContextProvider*>>;

    ~BindableProgramContextProvider() override;

    virtual void bindProgram(ProgramWrapper* pProgram, CycleGuard guard = nullptr);
    virtual void associateProgram(ProgramWrapper* pProgram, CycleGuard guard = nullptr);
    virtual void releaseProgram(ProgramWrapper* pProgram, CycleGuard guard = nullptr);

    virtual void getNestedContextProviders(std::vector<BindableProgramContextProvider*>& contextProviders) { }

    virtual void setNeedsUpdatePrograms();

private:
    std::unordered_map<ProgramWrapper*, bool> mPrograms;

public:
    DEFAULT_CONST_GETREF_DEFINITION(BoundPrograms, mPrograms)
};

} // namespace GraphEx
