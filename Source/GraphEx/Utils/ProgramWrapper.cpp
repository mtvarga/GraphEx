#include "ProgramWrapper.h"


using namespace GraphEx;


ProgramWrapper::ProgramWrapper(Falcor::ref<Falcor::Device> pDevice)
    : mpDevice(std::move(pDevice)) {}


ProgramWrapper::~ProgramWrapper()
{
    while (!mContextProviders.empty())
    {
        (*mContextProviders.begin())->releaseProgram(this);
    }
}


void ProgramWrapper::allocateStructuredBuffer(const std::string& name, const uint32_t nElements, const void* pInitData, size_t initDataSize)
{
    FALCOR_CHECK(mpCachedVars, "ProgramWrapper: ProgramVars was not created");
    mStructuredBuffers[name] = mpDevice->createStructuredBuffer(mpCachedVars->getRootVar()[name], nElements);

    if (!pInitData)
    {
        return;
    }

    const auto& buffer = mStructuredBuffers[name];
    const auto expectedDataSize = buffer->getStructSize() * buffer->getElementCount();

    if (initDataSize == 0)
    {
        initDataSize = expectedDataSize;
    }
    else if (initDataSize != expectedDataSize)
    {
        FALCOR_THROW("ProgramWrapper: StructuredBuffer '" + name + "' initial data size mismatch.");
    }

    buffer->setBlob(pInitData, 0, initDataSize);
}


Falcor::ShaderVar ProgramWrapper::operator[](const std::string& name)
{
    return getVars()->getRootVar()[name];
}


void ProgramWrapper::addDefine(const std::string& name, const std::string& value)
{
    mDefines.add(name, value);
    setNeedsUpdateDefines();
}


void ProgramWrapper::addDefines(const Falcor::DefineList& defines)
{
    mDefines.add(defines);
    setNeedsUpdateDefines();
}


void ProgramWrapper::clearDefines()
{
    mDefines.clear();
    setNeedsUpdateDefines();
}


void ProgramWrapper::addContextProvider(BindableProgramContextProvider* pContextProvider)
{
    mContextProviders.insert(pContextProvider);
    setNeedsUpdateDefines();
}


void ProgramWrapper::removeContextProvider(BindableProgramContextProvider* pContextProvider)
{
    mContextProviders.erase(pContextProvider);
    setNeedsUpdateDefines();
}


void ProgramWrapper::updateDefines(const bool force)
{
    if (!force && !mDirty)
    {
        return;
    }

    auto defines = mDefines;

    for (const auto& pContextProvider : mContextProviders)
    {
        pContextProvider->addProgramDefines(defines);
    }

    mpProgram->setDefines(defines);
    mDirty = false;

    // Must recache program vars. New defines may have added new shader components that define new vars
    cacheProgramVars();
}


void ProgramWrapper::updateVars(Falcor::ShaderVar& vars) const
{
    for (const auto& pContextProvider : mContextProviders)
    {
        pContextProvider->setProgramVars(vars);
    }
}


void ProgramWrapper::setNeedsUpdateDefines()
{
    mDirty = true;
}


void ProgramWrapper::cacheProgramVars()
{
    mpCachedVars = Falcor::ProgramVars::create(mpDevice, mpProgram->getReflector());
}


const Falcor::ref<Falcor::ProgramVars>& ProgramWrapper::getVars()
{
    updateDefines(); // This may recache the reflector of the program vars
    return mpCachedVars;
}


Falcor::ShaderVar ProgramWrapper::getRootVar()
{
    return getVars()->getRootVar();
}


void ProgramWrapper::setProgram(Falcor::ref<Falcor::Program> pProgram, Falcor::DefineList defines, const bool createProgramVars)
{
    mpProgram = std::move(pProgram);
    mDefines = std::move(defines);

    if (createProgramVars)
    {
        this->cacheProgramVars();
    }
}


void ProgramWrapper::unmapStructuredBuffer(const std::string& name) const
{
    FALCOR_CHECK(mStructuredBuffers.find(name) != mStructuredBuffers.end(), "ProgramWrapper: Couldn't find buffer by name: " + name);
    mStructuredBuffers.at(name)->unmap();
}


void* ProgramWrapper::mapStructuredBufferRaw(const std::string& name) const
{
    FALCOR_CHECK(mStructuredBuffers.find(name) != mStructuredBuffers.end(), "ProgramWrapper: Couldn't find buffer by name: " + name);
    return mStructuredBuffers.at(name)->map();
}


ComputeProgramWrapper::ComputeProgramWrapper(Falcor::ref<Falcor::Device> pDevice)
    : ProgramWrapper(std::move(pDevice)), mpState(Falcor::ComputeState::create(getDevice())) {}


void ComputeProgramWrapper::runProgram(const Falcor::uint3& dimensions)
{
    auto& pDevice = getDevice();
    auto& pVars = getVars();

    FALCOR_CHECK(pVars, "ComputeProgramWrapper: Attempted to run compute program, but ProgramVars were not created for it");

    auto rootVar = pVars->getRootVar();
    updateVars(rootVar);

    for (const auto& [ name, pBuffer ] : getStructuredBuffers())
    {
        pVars->setBuffer(name, pBuffer);
    }

    const auto groups = Falcor::div_round_up(dimensions, mThreadGroupSize);

    if (any(groups > pDevice->getLimits().maxComputeDispatchThreadGroups))
    {
        FALCOR_THROW("ComputeProgramWrapper: Attempted to run compute program, but dispatch dimensions exceed maximum");
    }

    pDevice->getRenderContext()->dispatch(mpState.get(), pVars.get(), groups);
}


void ComputeProgramWrapper::runProgram(const Falcor::uint width, const Falcor::uint height, const Falcor::uint depth)
{
    runProgram({ width, height, depth });
}


void ComputeProgramWrapper::recreateProgram(
    const std::filesystem::path& path,
    const std::string& csEntry,
    const Falcor::DefineList& programDefines,
    const Falcor::SlangCompilerFlags flags,
    const Falcor::ShaderModel shaderModel,
    const bool createProgramVars
) {
    const auto& pDevice = getDevice();
    const auto  pProgram = Falcor::Program::createCompute(pDevice, path, csEntry, { }, flags, shaderModel);

    setProgram(pProgram, programDefines, createProgramVars);
    mpState->setProgram(pProgram);
}


void ComputeProgramWrapper::recreateProgram(
    const Falcor::ProgramDesc& desc,
    const Falcor::DefineList& programDefines,
    const bool createProgramVars
) {
    const auto& pDevice = getDevice();
    const auto  pProgram = Falcor::Program::create(pDevice, desc, { });

    setProgram(pProgram, programDefines, createProgramVars);
    mpState->setProgram(pProgram);
}


void ComputeProgramWrapper::cacheProgramVars()
{
    ProgramWrapper::cacheProgramVars();

    mThreadGroupSize = getProgram()->getReflector()->getThreadGroupSize();
    FALCOR_CHECK(
        mThreadGroupSize.x >= 1 && mThreadGroupSize.y >= 1 && mThreadGroupSize.z >= 1, "ComputeProgramWrapper: Invalid thread group size"
    );
}


Falcor::ref<ComputeProgramWrapper> ComputeProgramWrapper::create(
    const Falcor::ref<Falcor::Device>& pDevice,
    const std::filesystem::path& path,
    const std::string& csEntry,
    const Falcor::DefineList& programDefines,
    const Falcor::SlangCompilerFlags flags,
    const Falcor::ShaderModel shaderModel,
    const bool createProgramVars
) {
    const auto pResult = Falcor::make_ref<ComputeProgramWrapper>(pDevice);
    pResult->recreateProgram(path, csEntry, programDefines, flags, shaderModel, createProgramVars);
    return pResult;
}


Falcor::ref<ComputeProgramWrapper> ComputeProgramWrapper::create(
    const Falcor::ref<Falcor::Device>& pDevice,
    const Falcor::ProgramDesc& desc,
    const Falcor::DefineList& programDefines,
    const bool createProgramVars
) {
    const auto pResult = Falcor::make_ref<ComputeProgramWrapper>(pDevice);
    pResult->recreateProgram(desc, programDefines, createProgramVars);
    return pResult;
}


GraphicsProgramWrapper::GraphicsProgramWrapper(Falcor::ref<Falcor::Device> pDevice)
    : ProgramWrapper(std::move(pDevice)), mpState(Falcor::GraphicsState::create(getDevice())) {}


Falcor::ref<Falcor::Vao> GraphicsProgramWrapper::getVao() const
{
    return mpState->getVao();
}


void GraphicsProgramWrapper::setVao(const Falcor::ref<Falcor::Vao>& pVao) const
{
    mpState->setVao(pVao);
}

Falcor::ref<Falcor::Fbo> GraphicsProgramWrapper::getFbo() const
{
    return mpState->getFbo();
}


void GraphicsProgramWrapper::setFbo(const Falcor::ref<Falcor::Fbo>& pFbo, const bool setVp0Sc0) const
{
    mpState->setFbo(pFbo, setVp0Sc0);
}


void GraphicsProgramWrapper::bindStructuredBuffers()
{
    const auto& pVars = getVars();
    FALCOR_CHECK(
        pVars,
        "GraphicsProgramWrapper: Attempted to bind structured buffers to graphics program, but ProgramVars were not created for it"
    );

    for (const auto& [ name, pBuffer ] : getStructuredBuffers())
    {
        pVars->setBuffer(name, pBuffer);
    }
}


void GraphicsProgramWrapper::draw(
    Falcor::RenderContext* pRenderContext,
    const Falcor::ref<Falcor::Fbo>& pFbo,
    const Falcor::uint vertexCount,
    const Falcor::uint startVertexLocation
) {
    const auto& pVars = getVars();

    FALCOR_CHECK(pVars, "GraphicsProgramWrapper: Attempted to draw with graphics program, but ProgramVars were not created for it");

    auto rootVar = pVars->getRootVar();

    updateVars(rootVar);
    bindStructuredBuffers();

    if (pFbo)
    {
        setFbo(pFbo, true);
    }

    pRenderContext->draw(mpState.get(), pVars.get(), vertexCount, startVertexLocation);
}


void GraphicsProgramWrapper::drawIndexed(
    Falcor::RenderContext* pRenderContext,
    const Falcor::ref<Falcor::Fbo>& pFbo,
    const Falcor::uint indexCount,
    const Falcor::uint startIndexLocation,
    const int32_t baseVertexLocation
) {
    const auto& pVars = getVars();

    FALCOR_CHECK(pVars, "GraphicsProgramWrapper: Attempted to draw with graphics program, but ProgramVars were not created for it");

    auto rootVar = pVars->getRootVar();
    updateVars(rootVar);
    bindStructuredBuffers();

    if (pFbo)
    {
        setFbo(pFbo, true);
    }

    pRenderContext->drawIndexed(mpState.get(), pVars.get(), indexCount, startIndexLocation, baseVertexLocation);
}


void GraphicsProgramWrapper::recreateProgram(
    const std::filesystem::path& vsPath,
    const std::filesystem::path& psPath,
    const std::string& vsEntry,
    const std::string& psEntry,
    const Falcor::DefineList& programDefines,
    const bool createProgramVars
) {
    const auto desc = Falcor::ProgramDesc()
        .addShaderLibrary(vsPath).vsEntry(vsEntry)
        .addShaderLibrary(psPath).psEntry(psEntry);

    recreateProgram(desc, programDefines, createProgramVars);
}


void GraphicsProgramWrapper::recreateProgram(
    const Falcor::ProgramDesc& desc,
    const Falcor::DefineList& programDefines,
    const bool createProgramVars
) {
    const auto& pDevice = getDevice();
    const auto  pProgram = Falcor::Program::create(pDevice, desc, { });

    setProgram(pProgram, programDefines, createProgramVars);
    mpState->setProgram(pProgram);
}


Falcor::ref<GraphicsProgramWrapper> GraphicsProgramWrapper::create(
    const Falcor::ref<Falcor::Device>& pDevice,
    const std::filesystem::path& vsPath,
    const std::filesystem::path& psPath,
    const std::string& vsEntry,
    const std::string& psEntry,
    const Falcor::DefineList& programDefines,
    const bool createProgramVars
) {
    const auto pResult = Falcor::make_ref<GraphicsProgramWrapper>(pDevice);
    pResult->recreateProgram(vsPath, psPath, vsEntry, psEntry, programDefines, createProgramVars);
    return pResult;
}


Falcor::ref<GraphicsProgramWrapper> GraphicsProgramWrapper::create(
    const Falcor::ref<Falcor::Device>& pDevice,
    const Falcor::ProgramDesc& desc,
    const Falcor::DefineList& programDefines,
    const bool createProgramVars
) {
    const auto pResult = Falcor::make_ref<GraphicsProgramWrapper>(pDevice);
    pResult->recreateProgram(desc, programDefines, createProgramVars);
    return pResult;
}

