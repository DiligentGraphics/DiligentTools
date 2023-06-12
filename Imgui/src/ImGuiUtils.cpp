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

} // namespace ImGui
