#pragma once

#include "SerializationMacros.h"
#include "SerializationTemplates.h"


namespace GraphEx::Internal
{

struct GRAPHEX_EXPORTABLE SerializationManager
{
    using InvalidityMessage = std::string;
    using InvalidityList = std::vector<InvalidityMessage>;

    std::optional<InputArchive> beginLoad(std::istream& is);
    std::optional<OutputArchive> beginSave(std::ostream& os);

    void finish();

    void addTrackedFalcorRef(uintptr_t address, Falcor::ref<Falcor::Object> ref);
    bool isFalcorRefTracked(uintptr_t address) const;
    Falcor::ref<Falcor::Object> getTrackedFalcorRef(uintptr_t address) const;

    void releaseTrackedRefs();

    void pushInvalidityList();
    std::shared_ptr<InvalidityList> popInvalidityList();
    void logInvalidity(const InvalidityMessage& message);

    static SerializationManager& get();

private:
    std::stack<std::shared_ptr<InvalidityList>> mInvalidityListStack;
    std::unordered_map<uintptr_t, Falcor::ref<Falcor::Object>> mTrackedRefs;
};

}  // namespace GraphEx::Internal
