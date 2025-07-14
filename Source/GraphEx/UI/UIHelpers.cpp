#include "UIHelpers.h"

#include <imgui_internal.h>


using namespace GraphEx;


constexpr auto LEFT_WINDOW_WIDTH = 450;
constexpr auto RIGHT_WINDOW_WIDTH = LEFT_WINDOW_WIDTH;



std::string UIHelpers::to_string(const unsigned int number)
{
    std::ostringstream result;
    result << number;
    return result.str();
}


std::string UIHelpers::to_string(const float number, const int precision)
{
    std::ostringstream result;
    result << std::setprecision(precision) << number;
    return result.str();
}


std::string UIHelpers::to_string(const double number, const int precision)
{
    std::ostringstream result;
    result << std::setprecision(precision) << number;
    return result.str();
}


std::string UIHelpers::to_string(const bool value)
{
    std::ostringstream result;
    result << (value ? "true" : "false");
    return result.str();
}


std::pair<uint32_t, uint32_t> UIHelpers::getLeftWindowStart()
{
    return {ImGui::GetFrameHeight(), 2 * ImGui::GetFrameHeight()};
}


std::pair<uint32_t, uint32_t> UIHelpers::getRightWindowStart(const UI& ui)
{
    return { ui.getWindowSize().x - ImGui::GetFrameHeight() - RIGHT_WINDOW_WIDTH, 2 * ImGui::GetFrameHeight()};
}


std::pair<uint32_t, uint32_t> UIHelpers::getLeftWindowArea(const UI& ui)
{
    const auto windowHeight = ui.getWindowSize().y;
    return {LEFT_WINDOW_WIDTH, windowHeight - (2 + getLeftWindowCount()) * ImGui::GetFrameHeight()};
}

std::pair<uint32_t, uint32_t> UIHelpers::getRightWindowArea(const UI& ui)
{
    const auto windowHeight = ui.getWindowSize().y;
    return { RIGHT_WINDOW_WIDTH, windowHeight - (2 + getRightWindowCount()) * ImGui::GetFrameHeight() };
}


constexpr Falcor::uint UIHelpers::getLeftWindowCount()
{
    return 2;
}

constexpr Falcor::uint UIHelpers::getRightWindowCount()
{
    return 0;
}


void UIHelpers::DynamicButton::render(
    const std::string& label,
    const ImVec2& size,
    const std::function<void()>& buttonClickedCb,
    const std::function<void(const char*)>& labelChangedCb
)
{
    if (!mIsEditing)
    {
        if (ImGui::Button(label.c_str(), size))
        {
            buttonClickedCb();
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            mIsEditing = true;
            mJustStartedEditing = true;

            const auto labelEndPtr = ImGui::FindRenderedTextEnd(label.c_str());
            const auto labelSize = static_cast<size_t>(labelEndPtr - label.c_str());
#if FALCOR_WINDOWS
            strncpy_s(mInputBuffer, label.c_str(), labelSize);
#else
            strncpy(mInputBuffer, label.c_str(), labelSize);
#endif
        }
    }
    else
    {
        if (mJustStartedEditing)
        {
            ImGui::SetKeyboardFocusHere();
        }

        ImGui::SetNextItemWidth(size.x);

        if (ImGui::InputText(
                ("##" + label).c_str(),
                mInputBuffer,
                IM_ARRAYSIZE(mInputBuffer),
                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll
            ))
        {
            labelChangedCb(mInputBuffer);
            mIsEditing = false;
        }

        if (!ImGui::IsItemActive() && !mJustStartedEditing)
        {
            labelChangedCb(mInputBuffer);
            mIsEditing = false;
        }

        if (mJustStartedEditing)
        {
            mJustStartedEditing = false;
        }
    }
}
