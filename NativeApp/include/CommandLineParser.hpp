/*
 *  Copyright 2019-2022 Diligent Graphics LLC
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

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <cstdlib>

#include "HashUtils.hpp"
#include "DebugUtilities.hpp"
#include "StringTools.hpp"
#include "ParsingTools.hpp"

namespace Diligent
{

/// Simple command line parser.
///
/// Command line example:
///     --mode vk --width 1024 -h 768 --path=my/path --use_alpha true
///
/// Usage example:
///
///     CommandLineParser ArgsParser{argc, argv};
///
///     const std::vector<std::pair<const char*, RENDER_DEVICE_TYPE>> DeviceTypeEnumVals =
///         {
///             {"d3d11", RENDER_DEVICE_TYPE_D3D11},
///             {"d3d12", RENDER_DEVICE_TYPE_D3D12},
///             {"vk",    RENDER_DEVICE_TYPE_VULKAN}
///         };
///     RENDER_DEVICE_TYPE DeviceType = RENDER_DEVICE_TYPE_UNDEFINED;
///     ArgsParser.ParseEnum("mode", 'm', DeviceTypeEnumVals, DeviceType);
///
///     int Width = 0;
///     ArgsParser.Parse("width", 'w', Width);
///
///     int Height = 0;
///     ArgsParser.Parse("height", 'h', Height);
///
///     std::string Path;
///     ArgsParser.Parse("path", 'p', Path);
///
///     bool UseAlpha = false;
///     ArgsParser.Parse("use_alpha", 'a', UseAlpha);
///
///
/// \note
///     Strings pointed to by argv must remain valid while the parser is used.
class CommandLineParser
{
public:
    CommandLineParser(int                argc,
                      const char* const* argv,
                      const char*        ShortSeparator = "-",
                      const char*        LongSeparator  = "--") :
        m_ShortSeparator{ShortSeparator},
        m_LongSeparator{LongSeparator},
        m_Args{argv, argv + argc}
    {
        m_ParamNames.resize(m_Args.size());

        size_t arg = 0;
        while (arg < m_Args.size())
        {
            auto GetParameterName = [this](const char* Arg, bool& IsShort) -> HashMapStringKey {
                std::string Name;
                if (strncmp(Arg, m_LongSeparator.c_str(), m_LongSeparator.length()) == 0)
                {
                    //  --width
                    //  ^
                    Arg += m_LongSeparator.length();
                    //  --width
                    //    ^
                    IsShort = false;
                }
                else if (strncmp(Arg, m_ShortSeparator.c_str(), m_ShortSeparator.length()) == 0)
                {
                    //  -h
                    //  ^
                    Arg += m_ShortSeparator.length();
                    //  -h
                    //   ^
                    IsShort = true;
                }
                else
                {
                    // UnknownParameter
                    // ^
                    return {};
                }

                const auto* NameEnd = Parsing::SkipIdentifier(Arg, Arg + strlen(Arg));
                if (Arg == NameEnd || (IsShort && (NameEnd - Arg) != 1))
                {
                    // -10
                    // -InvalidShortName
                    return HashMapStringKey{};
                }

                return HashMapStringKey{std::string{Arg, NameEnd}};
            };

            const auto* Arg = argv[arg];

            bool IsShort   = false;
            auto ParamName = GetParameterName(Arg, IsShort);
            if (!ParamName)
            {
                // UnknownParameter
                // ^
                ++arg;
                continue;
            }

            // Set parameter name at current position
            m_ParamNames[arg] = ParamName.Clone();
            // m_Args:         --width   1024
            // m_ParamNames:     width

            const char* Value = nullptr;
            if (!IsShort)
            {
                const auto* EqSign = strchr(Arg + m_LongSeparator.length() + strlen(ParamName.GetStr()), '=');
                if (EqSign != nullptr)
                {
                    // --width=1024
                    //        ^
                    Value = EqSign + 1;
                }
            }

            ++arg;
            if (Value == nullptr && arg < static_cast<size_t>(argc))
            {
                if (!GetParameterName(argv[arg], IsShort))
                {
                    // --width 1024
                    //         ^
                    Value = argv[arg];

                    // Set parameter name for the value argument
                    m_ParamNames[arg] = ParamName.Clone();
                    // m_Args:         --width    1024
                    // m_ParamNames:     width   width

                    ++arg;
                }
                else
                {
                    // Missing value:
                    // --width --height
                    //         ^
                }
            }

            // "width" -> "1024"
            m_NameToValue[std::move(ParamName)] = Value;
        }
    }

    explicit CommandLineParser(const std::vector<const char*>& Args,
                               const char*                     ShortSeparator = "-",
                               const char*                     LongSeparator  = "--") :
        CommandLineParser{static_cast<int>(Args.size()), Args.data(), ShortSeparator, LongSeparator}
    {}

    const char* const* ArgV()
    {
        Prune();
        return !m_Args.empty() ? m_Args.data() : nullptr;
    }

    int ArgC()
    {
        Prune();
        return static_cast<int>(m_Args.size());
    }

    /// Parses command line argument with long name LongName and short name ShortName.
    /// If parameter is found, calls Handler passing the value string as parameter.
    /// If RemoveArgument is true, the argument will be removed from the arguments list if parsed
    /// successfully.
    /// Returns true if the argument was parsed successfully, and false otherwise.
    template <typename HandlerType>
    bool Parse(const char*   LongName,
               char          ShortName,
               HandlerType&& Handler,
               bool          RemoveArgument = true)
    {
        if (LongName == nullptr && ShortName == '\0')
            return false;

        auto it = m_NameToValue.end();

        const auto* Name = LongName;
        if (LongName != nullptr)
        {
            // --width
            //   ^
            it = m_NameToValue.find(LongName);
        }

        const char ShortNameStr[] = {ShortName, '\0'};
        if (it == m_NameToValue.end() && ShortName != '\0')
        {
            // -h
            //  ^
            it   = m_NameToValue.find(ShortNameStr);
            Name = ShortNameStr;
        }

        if (it == m_NameToValue.end())
            return false;

        const bool Parsed = Handler(it->second);
        if (RemoveArgument)
        {
            m_UsedArgs.emplace(it->first.Clone());
            if (LongName != nullptr && ShortName != '\0')
            {
                // Add both short and long names to the m_UsedArgs set
                if (Name == LongName)
                    m_UsedArgs.emplace(HashMapStringKey{ShortNameStr, true});
                else
                    m_UsedArgs.emplace(HashMapStringKey{LongName, true});
            }
            m_PruningRequired = true;
        }

        return Parsed;
    }

    /// Parses command line argument with long name LongName and short name ShortName as type ValType.
    /// If RemoveArgument is true, the argument will be removed from the arguments list if parsed
    /// successfully.
    /// Returns true if the argument was parsed successfully, and false otherwise.
    template <typename ValType>
    bool Parse(const char* LongName,
               char        ShortName,
               ValType&    Val,
               bool        RemoveArgument = true);

    /// Short version that only takes LongName parameter.
    template <typename ValType>
    bool Parse(const char* LongName,
               ValType&    Val,
               bool        RemoveArgument = true)
    {
        return Parse(LongName, 0, Val, RemoveArgument);
    }

    /// Parses command line argument with long name LongName and short name ShortName as enumeration with values EnumVals.
    /// If RemoveArgument is true, the argument will be removed from the arguments list if parsed
    /// successfully.
    /// Returns true if the argument was parsed successfully, and false otherwise.
    template <typename EnumType>
    bool ParseEnum(const char*                                          LongName,
                   char                                                 ShortName,
                   const std::vector<std::pair<const char*, EnumType>>& EnumVals,
                   EnumType&                                            Val,
                   bool                                                 CaseSensitive  = false,
                   bool                                                 RemoveArgument = true)
    {
        return Parse(
            LongName, ShortName,
            [&](const char* ValStr) //
            {
                if (ValStr == nullptr)
                    return false;

                for (const auto& EnumVal : EnumVals)
                {
                    if ((CaseSensitive && strcmp(EnumVal.first, ValStr) == 0) ||
                        (!CaseSensitive && StrCmpNoCase(EnumVal.first, ValStr) == 0))
                    {
                        Val = EnumVal.second;
                        return true;
                    }
                }

                std::stringstream ss;
                ss << '\'' << ValStr << "\' is not a valid value for argument ";
                if (LongName != nullptr)
                    ss << LongName;
                if (ShortName != '\0')
                {
                    if (LongName != nullptr)
                        ss << ", ";
                    ss << ShortName;
                }
                ss << ". Only the following values are allowed: ";
                for (size_t i = 0; i < EnumVals.size(); ++i)
                {
                    if (i > 0)
                        ss << ", ";
                    ss << EnumVals[i].first;
                }
                ss << '.';

                LOG_WARNING_MESSAGE(ss.str());

                return false;
            },
            RemoveArgument);
    }

private:
    void Prune()
    {
        if (!m_PruningRequired)
            return;

        VERIFY_EXPR(m_Args.size() == m_ParamNames.size());
        size_t i = 0;
        while (i < m_Args.size())
        {
            // Remove all arguments whose names are in m_UsedArgs
            if (m_UsedArgs.find(m_ParamNames[i]) != m_UsedArgs.end())
            {
                auto erase_range_end = i + 1;
                while (erase_range_end < m_Args.size() && m_UsedArgs.find(m_ParamNames[erase_range_end]) != m_UsedArgs.end())
                    ++erase_range_end;
                m_Args.erase(m_Args.begin() + i, m_Args.begin() + erase_range_end);
                m_ParamNames.erase(m_ParamNames.begin() + i, m_ParamNames.begin() + erase_range_end);
            }
            else
            {
                ++i;
            }
        }
        VERIFY_EXPR(m_Args.size() == m_ParamNames.size());

        m_PruningRequired = false;
    }

private:
    const std::string m_ShortSeparator;
    const std::string m_LongSeparator;

    std::vector<const char*> m_Args;

    // Parameter name -> value, mapping, e.g.:
    // m_Args:          "--mode", "vk", "-w", "10",   "--height=20"
    // m_NameToValue:    "mode"->"vk",   "w"->"10",  "height"->"20"
    std::unordered_map<HashMapStringKey, const char*> m_NameToValue;

    // Parameter names for each argument, e.g.
    // "--mode",   "vk",  "-w",  "10",  "--height=20", "UnknownArg"
    //   "mode", "mode",   "w",   "w",       "height",           ""
    std::vector<HashMapStringKey> m_ParamNames;

    // A hash set of used argument names
    std::unordered_set<HashMapStringKey> m_UsedArgs;

    bool m_PruningRequired = false;
};


template <>
inline bool CommandLineParser::Parse<bool>(const char* LongName, char ShortName, bool& Val, bool RemoveArgument)
{
    return Parse(
        LongName, ShortName,
        [&Val](const char* ValStr) //
        {
            if (ValStr != nullptr)
                Val = strcmp(ValStr, "1") == 0 || StrCmpNoCase(ValStr, "true") == 0;
            else
                Val = true; // Treat bool args without value as true (e.g. --help, -h)
            return true;
        },
        RemoveArgument);
}

template <>
inline bool CommandLineParser::Parse<int>(const char* LongName, char ShortName, int& Val, bool RemoveArgument)
{
    return Parse(
        LongName, ShortName,
        [&Val](const char* ValStr) //
        {
            if (ValStr == nullptr)
                return false;
            Val = atoi(ValStr);
            return true;
        },
        RemoveArgument);
}

template <>
inline bool CommandLineParser::Parse<unsigned int>(const char* LongName, char ShortName, unsigned int& Val, bool RemoveArgument)
{
    return Parse(
        LongName, ShortName,
        [&Val](const char* ValStr) //
        {
            if (ValStr == nullptr)
                return false;
            Val = static_cast<unsigned int>(strtoul(ValStr, nullptr, 10));
            return true;
        },
        RemoveArgument);
}

template <>
inline bool CommandLineParser::Parse<float>(const char* LongName, char ShortName, float& Val, bool RemoveArgument)
{
    return Parse(
        LongName, ShortName,
        [&Val](const char* ValStr) //
        {
            if (ValStr == nullptr)
                return false;
            Val = strtof(ValStr, nullptr);
            return true;
        },
        RemoveArgument);
}

template <>
inline bool CommandLineParser::Parse<double>(const char* LongName, char ShortName, double& Val, bool RemoveArgument)
{
    return Parse(
        LongName, ShortName,
        [&Val](const char* ValStr) //
        {
            if (ValStr == nullptr)
                return false;
            Val = atof(ValStr);
            return true;
        },
        RemoveArgument);
}

template <>
inline bool CommandLineParser::Parse<std::string>(const char* LongName, char ShortName, std::string& Val, bool RemoveArgument)
{
    return Parse(
        LongName, ShortName,
        [&Val](const char* ValStr) //
        {
            if (ValStr == nullptr)
                return false;
            Val = ValStr;
            return true;
        },
        RemoveArgument);
}

} // namespace Diligent
