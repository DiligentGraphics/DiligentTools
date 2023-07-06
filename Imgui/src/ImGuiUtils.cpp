/*
 *  Copyright 2019-2023 Diligent Graphics LLC
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  In no event and under no legal theory, whether in tort (including negligence),
 *  contract, or otherwise, unless required by applicable law (such as deliberate
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental,
 *  or consequential damages of any character arising as a result of this License or
 *  out of the use or inability to use the software (including but not limited to damages
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and
 *  all other commercial damages or losses), even if such Contributor has been advised
 *  of the possibility of such damages.
 */

#include "ImGuiUtils.hpp"

#include <cmath>
#include <mutex>

// NOTE: don't use relative paths to ThirdParty as they will not work with custom DEAR_IMGUI_PATH
#include "imgui.h"
#include "imgui_internal.h"

namespace ImGui
{

ScopedDisabler::ScopedDisabler(bool Disable, float AlphaScale) :
    m_IsDisabled{Disable}
{
    if (m_IsDisabled)
    {
        PushItemFlag(ImGuiItemFlags_Disabled, true);
        PushStyleVar(ImGuiStyleVar_Alpha, GetStyle().Alpha * AlphaScale);
    }
}

ScopedDisabler::~ScopedDisabler()
{
    if (m_IsDisabled)
    {
        PopItemFlag();
        PopStyleVar();
    }
}

void HelpMarker(const char* desc, bool IsSameLine, const char* marker)
{
    if (IsSameLine)
        SameLine();

    TextDisabled("%s", marker);
    if (IsItemHovered())
    {
        BeginTooltip();
        PushTextWrapPos(GetFontSize() * 35.0f);
        TextUnformatted(desc);
        PopTextWrapPos();
        EndTooltip();
    }
}

void Plot::Render()
{
    float MaxVal  = -FLT_MAX;
    float MeanVal = 0;
    for (const auto& Val : m_Values)
    {
        MaxVal = std::max(MaxVal, Val);
        MeanVal += Val;
    }
    MeanVal /= static_cast<float>(m_Values.size());

    char overlay[128];
    snprintf(overlay, sizeof(overlay),
             "avg: %5.1f\n"
             "max: %5.1f",
             MeanVal, MaxVal);

    ImGui::PlotLines(m_Name.c_str(), m_Values.data(), static_cast<int>(m_Values.size()),
                     static_cast<int>(m_FrameNum % m_Values.size()), overlay, 0, FLT_MAX,
                     ImVec2(static_cast<float>(m_Values.size()), m_Height));
}

void ApplyStyleColorsGamma(float Gamma, bool ApplyToAlpha)
{
    auto& Colors = ImGui::GetStyle().Colors;
    for (int i = 0; i < ImGuiCol_COUNT; ++i)
    {
        auto& Col = Colors[i];
        Col.x     = Col.x > 0 ? std::pow(Col.x, Gamma) : 0;
        Col.y     = Col.y > 0 ? std::pow(Col.y, Gamma) : 0;
        Col.z     = Col.z > 0 ? std::pow(Col.z, Gamma) : 0;
        if (ApplyToAlpha)
            Col.w = Col.w > 0 ? std::pow(Col.w, Gamma) : 0;
    }
}

void StyleColorsDiligent(float Gamma)
{
    StyleColorsDark();
    ApplyStyleColorsGamma(Gamma, true);
    auto& Colors                = ImGui::GetStyle().Colors;
    Colors[ImGuiCol_WindowBg].w = 0.75f;
    Colors[ImGuiCol_PlotLines]  = {1.00f, 1.00f, 1.00f, 1.00f};
}


class LogWindowImpl
{
public:
    LogWindowImpl()
    {
        Clear();
    }

    void AddLog(const char* fmt, va_list args)
    {
        std::lock_guard<std::mutex> Guard{m_Mtx};

        int old_size = Buf.size();
        Buf.appendfv(fmt, args);
        for (int new_size = Buf.size(); old_size < new_size; old_size++)
            if (Buf[old_size] == '\n')
                LineOffsets.push_back(old_size + 1);
    }

    void Draw(const char* title)
    {
        std::lock_guard<std::mutex> Guard{m_Mtx};

        if (!ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
        {
            ImGui::End();
            return;
        }

        bool clear = ImGui::Button("Clear");
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        Filter.Draw("Filter", -150.0f);
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &AutoScroll);

        ImGui::Separator();
        ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        if (clear)
            UnsafeClear();
        if (copy)
            ImGui::LogToClipboard();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        const char* buf     = Buf.begin();
        const char* buf_end = Buf.end();
        if (Filter.IsActive())
        {
            // In this example we don't use the clipper when Filter is enabled.
            // This is because we don't have a random access on the result on our filter.
            // A real application processing logs with ten of thousands of entries may want to store the result of search/filter.
            // especially if the filtering function is not trivial (e.g. reg-exp).
            for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
            {
                const char* line_start = buf + LineOffsets[line_no];
                const char* line_end   = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                if (Filter.PassFilter(line_start, line_end))
                    ImGui::TextUnformatted(line_start, line_end);
            }
        }
        else
        {
            // The simplest and easy way to display the entire buffer:
            //   ImGui::TextUnformatted(buf_begin, buf_end);
            // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward to skip non-visible lines.
            // Here we instead demonstrate using the clipper to only process lines that are within the visible area.
            // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them on your side is recommended.
            // Using ImGuiListClipper requires A) random access into your data, and B) items all being the  same height,
            // both of which we can handle since we an array pointing to the beginning of each line of text.
            // When using the filter (in the block of code above) we don't have random access into the data to display anymore, which is why we don't use the clipper.
            // Storing or skimming through the search result would make it possible (and would be recommended if you want to search through tens of thousands of entries)
            ImGuiListClipper clipper;
            clipper.Begin(LineOffsets.Size);
            while (clipper.Step())
            {
                for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                {
                    const char* line_start = buf + LineOffsets[line_no];
                    const char* line_end   = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                    ImGui::TextUnformatted(line_start, line_end);
                }
            }
            clipper.End();
        }
        ImGui::PopStyleVar();

        if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
        ImGui::End();
    }

    void Clear()
    {
        std::lock_guard<std::mutex> Guard{m_Mtx};

        UnsafeClear();
    }

private:
    void UnsafeClear()
    {
        // Must be called under mutex
        Buf.clear();
        LineOffsets.clear();
        LineOffsets.push_back(0);
    }

    std::mutex m_Mtx;

    ImGuiTextBuffer Buf;
    ImGuiTextFilter Filter;
    ImVector<int>   LineOffsets;       // Index to lines offset. We maintain this with AddLog() calls, allowing us to have a random access on lines
    bool            AutoScroll = true; // Keep scrolling if already at the bottom
};

LogWindow::LogWindow() :
    m_Impl{std::make_unique<LogWindowImpl>()}
{
}

LogWindow::~LogWindow()
{
}

void LogWindow::AddLog(const char* fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    m_Impl->AddLog(fmt, argptr);
    va_end(argptr);
}

void LogWindow::Draw(const char* title)
{
    m_Impl->Draw(title);
}

void LogWindow::Clear()
{
    m_Impl->Clear();
}

} // namespace ImGui
