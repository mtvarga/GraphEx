#pragma once

#include "ProgramContext.h"

namespace GraphEx
{

class GRAPHEX_EXPORTABLE ProgramWrapper : public Falcor::Object
{
protected:
    explicit ProgramWrapper(Falcor::ref<Falcor::Device> pDevice);
    ~ProgramWrapper() override;

public:
    template<typename T>
    std::vector<T> readStructuredBuffer(const std::string& name);

    template<typename T>
    T* mapStructuredBuffer(const std::string& name) const;

    void* mapStructuredBufferRaw(const std::string& name) const;
    void  unmapStructuredBuffer(const std::string& name) const;
    void  allocateStructuredBuffer(const std::string& name, uint32_t nElements, const void* pInitData, size_t initDataSize);

    Falcor::ShaderVar operator[](const std::string& name);

    void addDefine(const std::string& name, const std::string& value);
    void addDefines(const Falcor::DefineList& defines);
    void clearDefines();

    void addContextProvider(BindableProgramContextProvider* pContextProvider);
    void removeContextProvider(BindableProgramContextProvider* pContextProvider);

    void updateDefines(bool force = false);
    void updateVars(Falcor::ShaderVar& vars) const;

    void setNeedsUpdateDefines();

    virtual void cacheProgramVars();

private:
    Falcor::ref<Falcor::Device> mpDevice;
    Falcor::ref<Falcor::Program> mpProgram;
    Falcor::ref<Falcor::ProgramVars> mpCachedVars;
    Falcor::DefineList mDefines;
    std::unordered_set<BindableProgramContextProvider*> mContextProviders;
    std::unordered_map<std::string, Falcor::ref<Falcor::Buffer>> mStructuredBuffers;

    bool mDirty = false;

public:
    DEFAULT_CONST_GETREF_DEFINITION(Device, mpDevice)
    DEFAULT_CONST_GETREF_DEFINITION(Program, mpProgram)
    DEFAULT_CONST_GETREF_DEFINITION(Defines, mDefines)
    DEFAULT_CONST_GETREF_DEFINITION(StructuredBuffers, mStructuredBuffers)

    const Falcor::ref<Falcor::ProgramVars>& getVars();
    Falcor::ShaderVar getRootVar();

protected:
    void setProgram(Falcor::ref<Falcor::Program> pProgram, Falcor::DefineList defines, bool createProgramVars = true);
};


template<typename T>
std::vector<T> ProgramWrapper::readStructuredBuffer(const std::string& name)
{
    FALCOR_CHECK(mStructuredBuffers.find(name) != mStructuredBuffers.end(), "ProgramWrapper: Couldn't find buffer by name: " + name);
    return mStructuredBuffers.at(name)->getElements<T>();
}


template<typename T>
T* ProgramWrapper::mapStructuredBuffer(const std::string& name) const
{
    return static_cast<T*>(mapStructuredBufferRaw(name));
}


struct GRAPHEX_EXPORTABLE ComputeProgramWrapper final : ProgramWrapper
{
    explicit ComputeProgramWrapper(Falcor::ref<Falcor::Device> pDevice);

    void runProgram(const Falcor::uint3& dimensions);
    void runProgram(Falcor::uint width = 1, Falcor::uint height = 1, Falcor::uint depth = 1);

    void recreateProgram(
        const std::filesystem::path& path,
        const std::string& csEntry = "main",
        const Falcor::DefineList& programDefines = { },
        Falcor::SlangCompilerFlags flags = Falcor::SlangCompilerFlags::None,
        Falcor::ShaderModel shaderModel = Falcor::ShaderModel::Unknown,
        bool createProgramVars = true
    );

    void recreateProgram(
        const Falcor::ProgramDesc& desc,
        const Falcor::DefineList& programDefines = Falcor::DefineList(),
        bool createProgramVars = true
    );

    void cacheProgramVars() override;

    static Falcor::ref<ComputeProgramWrapper> create(
        const Falcor::ref<Falcor::Device>& pDevice,
        const std::filesystem::path& path,
        const std::string& csEntry = "main",
        const Falcor::DefineList& programDefines = { },
        Falcor::SlangCompilerFlags flags = Falcor::SlangCompilerFlags::None,
        Falcor::ShaderModel shaderModel = Falcor::ShaderModel::Unknown,
        bool createProgramVars = true
    );

    static Falcor::ref<ComputeProgramWrapper> create(
        const Falcor::ref<Falcor::Device>& pDevice,
        const Falcor::ProgramDesc& desc,
        const Falcor::DefineList& programDefines = Falcor::DefineList(),
        bool createProgramVars = true
    );

private:
    Falcor::ref<Falcor::ComputeState> mpState;
    Falcor::uint3 mThreadGroupSize = { 0, 0, 0 };

public:
    DEFAULT_CONST_GETREF_DEFINITION(State, mpState)
    DEFAULT_CONST_GETTER_DEFINITION(ThreadGroupSize, mThreadGroupSize)
};


struct GRAPHEX_EXPORTABLE GraphicsProgramWrapper final : ProgramWrapper
{
    explicit GraphicsProgramWrapper(Falcor::ref<Falcor::Device> pDevice);

    Falcor::ref<Falcor::Vao> getVao() const;
    void setVao(const Falcor::ref<Falcor::Vao>& pVao) const;

    Falcor::ref<Falcor::Fbo> getFbo() const;
    void setFbo(const Falcor::ref<Falcor::Fbo>& pFbo, bool setVp0Sc0 = true) const;

    void bindStructuredBuffers();

    void draw(
        Falcor::RenderContext* pRenderContext,
        const Falcor::ref<Falcor::Fbo>& pFbo,
        Falcor::uint vertexCount,
        Falcor::uint startVertexLocation = 0
    );

    void drawIndexed(
        Falcor::RenderContext* pRenderContext,
        const Falcor::ref<Falcor::Fbo>& pFbo,
        Falcor::uint indexCount,
        Falcor::uint startIndexLocation = 0,
        int32_t baseVertexLocation = 0
    );

    void recreateProgram(
        const std::filesystem::path& vsPath,
        const std::filesystem::path& psPath,
        const std::string& vsEntry = "main",
        const std::string& psEntry = "main",
        const Falcor::DefineList& programDefines = { },
        bool createProgramVars = true
    );

    void recreateProgram(
        const Falcor::ProgramDesc& desc,
        const Falcor::DefineList& programDefines = { },
        bool createProgramVars = true
    );

    static Falcor::ref<GraphicsProgramWrapper> create(
        const Falcor::ref<Falcor::Device>& pDevice,
        const std::filesystem::path& vsPath,
        const std::filesystem::path& psPath,
        const std::string& vsEntry = "main",
        const std::string& psEntry = "main",
        const Falcor::DefineList& programDefines = { },
        bool createProgramVars = true
    );

    static Falcor::ref<GraphicsProgramWrapper> create(
        const Falcor::ref<Falcor::Device>& pDevice,
        const Falcor::ProgramDesc& desc,
        const Falcor::DefineList& programDefines = { },
        bool createProgramVars = true
    );

private:
    Falcor::ref<Falcor::GraphicsState> mpState;

public:
    DEFAULT_CONST_GETREF_DEFINITION(State, mpState)
};
} // namespace GraphEx