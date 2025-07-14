#include "ProgramContext.h"

#include "ProgramWrapper.h"


using namespace GraphEx;


void ProgramDefineProvider::addProgramDefines(Falcor::DefineList& defines) const {}
void ProgramVarProvider::setProgramVars(const Falcor::ShaderVar& var) const {}


void ProgramVarProvider::trySetProgramVarsFor(
    const Falcor::ShaderVar& var,
    const std::string& memberName,
    const ProgramVarProvider& varProvider
) {
    if (!var.hasMember(memberName))
    {
        return;
    }

    varProvider.setProgramVars(var[memberName]);
}


BindableProgramContextProvider::~BindableProgramContextProvider()
{
    for (const auto& [ pProgram, shouldRemove ] : mPrograms)
    {
        if (shouldRemove)
        {
            pProgram->removeContextProvider(this);
        }
    }
}


void BindableProgramContextProvider::bindProgram(ProgramWrapper* pProgram, CycleGuard guard)
{
    if (!pProgram)
    {
        return;
    }

    if (!guard)
    {
        guard = std::make_shared<std::unordered_set<BindableProgramContextProvider*>>();
    }

    if (guard->find(this) != guard->end())
    {
        FALCOR_THROW("Circular nesting of BindableProgramContextProvider was detected.");
    }

    guard->insert(this);

    pProgram->addContextProvider(this);
    mPrograms.emplace(pProgram, true);

    // Bind nested context providers
    std::vector<BindableProgramContextProvider*> nestedContextProviders;
    getNestedContextProviders(nestedContextProviders);

    for (const auto pNestedContextProvider : nestedContextProviders)
    {
        pNestedContextProvider->bindProgram(pProgram, guard);
    }

    guard->erase(this);
}


void BindableProgramContextProvider::associateProgram(ProgramWrapper* pProgram, CycleGuard guard)
{
    if (!pProgram)
    {
        return;
    }

    if (!guard)
    {
        guard = std::make_shared<std::unordered_set<BindableProgramContextProvider*>>();
    }

    if (guard->find(this) != guard->end())
    {
        FALCOR_THROW("Circular nesting of BindableProgramContextProvider was detected.");
    }

    guard->insert(this);

    mPrograms.emplace(pProgram, false);

    // Associate nested context providers
    std::vector<BindableProgramContextProvider*> nestedContextProviders;
    getNestedContextProviders(nestedContextProviders);

    for (const auto pNestedContextProvider : nestedContextProviders)
    {
        pNestedContextProvider->associateProgram(pProgram, guard);
    }

    guard->erase(this);
}


void BindableProgramContextProvider::releaseProgram(ProgramWrapper* pProgram, CycleGuard guard)
{
    if (!guard)
    {
        guard = std::make_shared<std::unordered_set<BindableProgramContextProvider*>>();
    }

    if (guard->find(this) != guard->end())
    {
        FALCOR_THROW("Circular nesting of BindableProgramContextProvider was detected.");
    }

    guard->insert(this);

    const auto it = mPrograms.find(pProgram);

    if (it == mPrograms.end())
    {
        return;
    }

    if (it->second)
    {
        pProgram->removeContextProvider(this);
    }

    mPrograms.erase(it);

    // Release nested context providers
    std::vector<BindableProgramContextProvider*> nestedContextProviders;
    getNestedContextProviders(nestedContextProviders);

    for (const auto pNestedContextProvider : nestedContextProviders)
    {
        pNestedContextProvider->releaseProgram(pProgram, guard);
    }

    guard->erase(this);
}


void BindableProgramContextProvider::setNeedsUpdatePrograms()
{
    for (const auto& [ pProgram, shouldRemove ] : mPrograms)
    {
        pProgram->setNeedsUpdateDefines();
    }
}


void ProgramIncludeProvider::addProgramDefines(Falcor::DefineList& defines) const
{
    const auto& [ includeDefineName, includeString ] = getProgramIncludeInfo();
    defines[includeDefineName] = includeString;
}
