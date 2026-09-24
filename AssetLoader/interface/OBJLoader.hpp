/*
 *  Copyright 2026 Diligent Graphics LLC
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

#include <cstddef>
#include <string>
#include <vector>

#include "../../../DiligentCore/Common/interface/BasicMath.hpp"

namespace Diligent
{
namespace OBJ
{

struct Primitive
{
    /// Material name referenced by usemtl, or empty for the default material.
    std::string MaterialName;
    /// Contiguous range in Document::Indices, expressed in indices, not bytes.
    Uint32 FirstIndex = 0;
    Uint32 IndexCount = 0;
};

struct Mesh
{
    /// Consecutive face range introduced by o or g; empty ranges are omitted.
    std::string            Name;
    std::vector<Primitive> Primitives;
};

/// CPU geometry with one shared index for all vertex attributes.
/// Normals are always populated; optional texture coordinates and colors are
/// either empty or have the same length as Positions. Missing values are zero
/// UVs and white colors. Winding and texture V are not transformed; homogeneous
/// positions are divided by W, with no further coordinate conversion.
struct Document
{
    std::vector<float3> Positions;
    std::vector<float3> Normals;
    std::vector<float2> TexCoords;
    std::vector<float4> Colors;
    /// Triangle indices into the unified Positions/Normals/optional arrays.
    std::vector<Uint32> Indices;
    std::vector<Mesh>   Meshes;
    /// Filenames from mtllib in source order; loading is the caller's responsibility.
    std::vector<std::string> MaterialLibraries;
};

/// Parses a bounded OBJ source without retaining an intermediate face model.
/// Supports polygonal f, v (XYZ, XYZW, XYZRGB or XYZRGBA), vt, vn,
/// o, g, s, usemtl and mtllib. Faces are triangulated and unified as read.
/// Each face must be a simple (non-self-intersecting) polygon whose vertices
/// lie in a single plane.
/// Missing normals are generated flat or by position and smoothing group;
/// authored normals are preserved. Unsupported geometry produces warnings.
/// Error and warning messages include the logical line's first source line.
/// Result is empty on failure; Data need not be zero-terminated.
bool Parse(const char* Data, size_t Size, Document& Result, std::string& Error, std::string& Warnings);

struct TextureMap
{
    /// Filename relative to the containing material library.
    std::string Name;
    /// Texture coordinate transform: UV * Scale.xy + Offset.xy.
    float3 Scale{1, 1, 1};
    float3 Offset{};
    /// True selects clamped addressing, false selects wrapping.
    bool Clamp = false;
    /// Normal map strength from norm -bm; height maps are not supported.
    float BumpMultiplier = 1;
};

struct Material
{
    std::string Name;
    float3      Diffuse{1, 1, 1};
    float3      Specular{};
    float3      Emissive{};
    /// Dissolve d, or 1 - Tr when no d statement was supplied.
    float Opacity = 1;
    /// Specular exponent Ns and source illum model; conversion is left to the caller.
    float      Shininess         = 0;
    int        IlluminationModel = 2;
    TextureMap DiffuseMap;
    TextureMap SpecularMap;
    TextureMap EmissiveMap;
    TextureMap NormalMap;
};

/// Supports newmtl, Kd/Ks/Ke, d/Tr (d takes precedence), Ns, illum,
/// map_Kd/map_Ks/map_Ke and norm. Maps support -o/-s/-clamp (plus -bm for norm) and quoted
/// or unquoted filenames containing spaces. Ka and Ni are accepted and ignored.
/// Unsupported maps (including height-map bump/map_bump) produce warnings.
/// Both parsers support comments, CRLF and backslash line continuations.
/// Result is empty on failure; Data need not be zero-terminated.
bool ParseMaterials(const char* Data, size_t Size, std::vector<Material>& Result, std::string& Error, std::string& Warnings);

} // namespace OBJ
} // namespace Diligent
