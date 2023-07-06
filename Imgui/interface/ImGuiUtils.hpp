/*
 *  Copyright 2019-2023 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
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

#pragma once

#include <string>
#include <algorithm>
#include <limits>
#include <memory>
#include <vector>

#include "../../../DiligentCore/Platforms/Basic/interface/DebugUtilities.hpp"

namespace ImGui
{

typedef int ImGuiSliderFlags;

bool Checkbox(const char* label, bool* v);
bool SliderInt(const char* label, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags);
bool Combo(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items);
void PushID(const char* str_id);
void PushID(const void* ptr_id);
void PushID(int int_id);
void PopID();

class ScopedDisabler
{
public:
    explicit ScopedDisabler(bool Disable, float AlphaScale = 0.25f);
    ~ScopedDisabler();

private:
    const bool m_IsDisabled;
};


template <typename T>
inline bool Checkbox(const char* label, T* v)
{
    bool b = *v != 0;

    auto pressed = Checkbox(label, &b);
    if (pressed)
        *v = b ? 1 : 0;
    return pressed;
}


void HelpMarker(const char* desc, bool IsSameLine = true, const char* marker = "(?)");


template <typename T, typename = typename std::enable_if<std::numeric_limits<T>::is_integer>::type>
bool SliderIntT(const char* label, T* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0)
{
    int i = static_cast<int>(*v);

    auto value_changed = SliderInt(label, &i, v_min, v_max, format, flags);
    if (value_changed)
        *v = static_cast<T>(i);
    return value_changed;
}


namespace
{

inline const char* c_str(const std::string& str)
{
    return str.c_str();
}

inline const char* c_str(const char* str)
{
    return str;
}

} // namespace

template <typename ItemType, typename StrType>
bool Combo(const char* label, ItemType* current_item, const std::pair<ItemType, StrType> items[], int items_count, int popup_max_height_in_items = -1)
{
    int item_idx = 0;
    while (item_idx < items_count && items[item_idx].first != *current_item)
        ++item_idx;
    if (item_idx >= items_count)
    {
        UNEXPECTED("Current item was not found in the items list");
        return false;
    }
    auto names = std::make_unique<const char*[]>(items_count);
    for (int i = 0; i < items_count; ++i)
        names[i] = c_str(items[i].second);
    auto value_changed = Combo(label, &item_idx, names.get(), items_count, popup_max_height_in_items);
    if (value_changed)
        *current_item = items[item_idx].first;

    return value_changed;
}

template <typename IDType>
class ScopedID
{
public:
    ScopedID(IDType ID)
    {
        PushID(ID);
    }

    ~ScopedID()
    {
        PopID();
    }
};


class Plot
{
public:
    Plot(const char* Name, size_t Size, float Height) :
        m_Name{Name != nullptr ? Name : ""},
        m_Height{Height},
        m_Values(Size)
    {
    }

    void AddValue(float Value)
    {
        m_Values[m_FrameNum % m_Values.size()] = Value;
        ++m_FrameNum;
    }

    void Reset()
    {
        for (auto& Val : m_Values)
            Val = 0;
        m_FrameNum = 0;
    }

    void Render();

private:
    const std::string m_Name;
    const float       m_Height;

    std::vector<float> m_Values;
    size_t             m_FrameNum = 0;
};

void ApplyStyleColorsGamma(float Gamma, bool ApplyToAlpha = false);
void StyleColorsDiligent(float Gamma = 0.5f);

class LogWindow
{
public:
    ~LogWindow();
    LogWindow();

    void AddLog(const char* fmt, ...);
    void Draw(const char* title);
    void Clear();

private:
    std::unique_ptr<class LogWindowImpl> m_Impl;
};

} // namespace ImGui
