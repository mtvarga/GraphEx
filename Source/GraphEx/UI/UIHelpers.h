#pragma once

#include "../Utils/GlobalLocalProperty.h"
#include "UI.h"


namespace GraphEx
{

struct GRAPHEX_EXPORTABLE UIHelpers final
{
    template<typename T, int N>
    static std::string to_string(const Falcor::math::vector<T, N>& vec, int precision = 4);
    static std::string to_string(unsigned int number);
    static std::string to_string(float number, int precision = 4);
    static std::string to_string(double number, int precision = 4);
    static std::string to_string(bool value);

    static std::pair<uint32_t, uint32_t> getLeftWindowStart();
    static std::pair<uint32_t, uint32_t> getRightWindowStart(const UI& ui);

    static std::pair<uint32_t, uint32_t> getLeftWindowArea(const UI& ui);
    static std::pair<uint32_t, uint32_t> getRightWindowArea(const UI& ui);

    static constexpr Falcor::uint getLeftWindowCount();
    static constexpr Falcor::uint getRightWindowCount();

    class DynamicButton
    {
    public:
        void render(
            const std::string& label,
            const ImVec2& size,
            const std::function<void()>& buttonClickedCb,
            const std::function<void(const char*)>& labelChangedCb
        );

    private:
        char mInputBuffer[256] = {};
        bool mIsEditing = false;
        bool mJustStartedEditing = false;
    };

    template<typename HumanReadableT>
    static Falcor::Gui::DropdownList getDropdownListForHumanReadables(const std::vector<HumanReadableT>& list);
};


template<typename T, int N>
std::string UIHelpers::to_string(const Falcor::math::vector<T, N>& vec, const int precision)
{
    std::ostringstream result;
    result << std::setprecision(precision);

    for (size_t i = 0; i < vec.length(); ++i)
    {
        result << vec[i];

        if (i < vec.length() - 1)
        {
            result << ", ";
        }
    }

    return result.str();
}


template<typename HumanReadableT>
Falcor::Gui::DropdownList UIHelpers::getDropdownListForHumanReadables(const std::vector<HumanReadableT>& list)
{
    Falcor::Gui::DropdownList result;
    result.reserve(list.size());

    for (const auto& item : list)
    {
        result.emplace_back(Falcor::Gui::DropdownValue{ static_cast<Falcor::uint>(result.size()), item->getHumanReadableName() });
    }

    return result;
}

} // namespace GraphEx
