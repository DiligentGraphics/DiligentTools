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

#include "OBJLoader.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "AdvancedMath.hpp"
#include "ParsingTools.hpp"

namespace Diligent
{
namespace OBJ
{
namespace
{

constexpr Uint32 MissingIndex = (std::numeric_limits<Uint32>::max)();

bool IsSpace(char Ch)
{
    return Ch == ' ' || Ch == '\t' || Ch == '\r' || Ch == '\n' || Ch == '\v' || Ch == '\f';
}

std::string_view Trim(std::string_view Text)
{
    while (!Text.empty() && IsSpace(Text.front()))
    {
        Text.remove_prefix(1);
    }
    while (!Text.empty() && IsSpace(Text.back()))
    {
        Text.remove_suffix(1);
    }
    return Text;
}

std::string_view ReadToken(std::string_view& Text)
{
    Text = Trim(Text);
    if (Text.empty())
    {
        return {};
    }
    // Quotes keep spaces inside a single token, such as a material filename.
    //  "paint maps.mtl" next.mtl
    //  ^
    //  Text
    size_t End = 0;
    if (Text.front() == '"' || Text.front() == '\'')
    {
        const char Quote = Text.front();
        End              = Text.find(Quote, 1);
        if (End != std::string_view::npos)
        {
            //  "paint maps.mtl" next.mtl
            //                 ^
            //                 End
            const std::string_view Token = Text.substr(1, End - 1);
            Text.remove_prefix(End + 1);
            return Token;
        }
    }
    End = 0;
    while (End < Text.size() && !IsSpace(Text[End]))
    {
        ++End;
    }
    // An unquoted token ends at the next whitespace character (or input end).
    //  1/2/3 4/5/6
    //       ^
    //       End
    const std::string_view Token = Text.substr(0, End);
    Text.remove_prefix(End);
    return Token;
}

std::string ReadName(std::string_view Text)
{
    Text = Trim(Text);
    // Names consume the entire remainder, so unquoted embedded spaces are kept.
    //  usemtl Painted Metal
    //         ^
    //         Text
    if (Text.size() >= 2 && (Text.front() == '"' || Text.front() == '\'') && Text.back() == Text.front())
    {
        Text.remove_prefix(1);
        Text.remove_suffix(1);
    }
    return std::string{Text};
}

// Ordinary lines borrow the source memory. Only continued logical lines use
// scratch storage, which is reused for the following line.
class LineReader
{
public:
    LineReader(const char* Data, size_t Size) :
        m_Data{Data},
        m_Size{Size}
    {
        // Skip an optional UTF-8 BOM before the first statement.
        if (Size >= 3 &&
            static_cast<unsigned char>(Data[0]) == 0xef &&
            static_cast<unsigned char>(Data[1]) == 0xbb &&
            static_cast<unsigned char>(Data[2]) == 0xbf)
        {
            m_Offset = 3;
        }
    }

    bool Next(std::string_view& Text, size_t& Line)
    {
        if (m_Offset == m_Size)
        {
            return false;
        }
        Line = m_Line;
        m_Scratch.clear();
        bool Continued = false;
        char Quote     = 0;
        do
        {
            // Read one physical line, stopping before the LF character.
            const size_t Begin = m_Offset;
            while (m_Offset < m_Size && m_Data[m_Offset] != '\n')
            {
                ++m_Offset;
            }
            Text = std::string_view{m_Data + Begin, m_Offset - Begin};
            if (m_Offset < m_Size)
            {
                ++m_Offset;
            }
            ++m_Line;
            for (size_t i = 0; i < Text.size(); ++i)
            {
                const char Ch = Text[i];
                if (Quote != 0)
                {
                    if (Ch == Quote)
                    {
                        Quote = 0;
                    }
                }
                else if ((Ch == '"' || Ch == '\'') && (i == 0 || IsSpace(Text[i - 1])))
                {
                    Quote = Ch;
                }
                else if (Ch == '#')
                {
                    // A '#' inside quotes belongs to the token; outside, it starts a comment.
                    //  mtllib "maps #1.mtl" # comment
                    //                       ^
                    //                       i
                    Text = Text.substr(0, i);
                    break;
                }
            }
            Text            = Trim(Text);
            const bool More = !Text.empty() && Text.back() == '\\';
            if (More)
            {
                // Remove the continuation marker and join the next physical line.
                //  f 1 2 \ <end of line>
                //        ^
                //        Text.back()
                //    3
                // These physical lines form one face statement.
                Text.remove_suffix(1);
            }
            if (Continued || More)
            {
                m_Scratch.append(Text.data(), Text.size());
                if (More && m_Offset < m_Size)
                {
                    m_Scratch.push_back(' ');
                }
            }
            if (!More || m_Offset == m_Size)
            {
                if (Continued || More)
                {
                    Text = m_Scratch;
                }
                return true;
            }
            Continued = true;
        } while (true);
    }

private:
    const char* m_Data   = nullptr;
    size_t      m_Size   = 0;
    size_t      m_Offset = 0;
    size_t      m_Line   = 1;
    std::string m_Scratch;
};

bool Fail(size_t Line, const std::string& Message, std::string& Error)
{
    Error = "Line " + std::to_string(Line) + ": " + Message;
    return false;
}

void Warn(size_t Line, const std::string& Message, std::string& Warnings)
{
    if (!Warnings.empty())
    {
        Warnings.push_back('\n');
    }
    Warnings += "Line " + std::to_string(Line) + ": " + Message;
}

bool ReadFloat(std::string_view Text, float& Value)
{
    // Numeric fields must consume the whole token: "1.5" is valid, "1.5x" is not.
    return !Text.empty() && Parsing::ReadFloat(Text.begin(), Text.end(), Value) == Text.end();
}

bool ReadUnsigned(std::string_view Text, Uint64& Value)
{
    // OBJ indices and MTL integer fields require a complete unsigned token.
    return !Text.empty() && Parsing::IsDigit(Text.front()) &&
        Parsing::ParseInteger(Text.begin(), Text.end(), Value) == Text.end();
}

bool ReadIndex(std::string_view Text, size_t Count, Uint32& Index)
{
    if (Text.empty())
    {
        return false;
    }
    // A negative index counts backward from the attributes defined so far.
    //  -3
    //  ^
    //  Text
    const bool Negative = Text.front() == '-';
    if (Text.front() == '+' || Negative)
    {
        Text.remove_prefix(1);
    }
    Uint64 Value = 0;
    if (!ReadUnsigned(Text, Value) || Value == 0 || Value > Count)
    {
        return false;
    }
    // Convert OBJ's one-based or relative index to a zero-based array index.
    // For Count == 5: 1 -> 0, 5 -> 4, -1 -> 4, -5 -> 0; zero is invalid.
    Index = static_cast<Uint32>(Negative ? Count - Value : Value - 1);
    return true;
}

bool ReadValues(std::string_view Text, float* Values, size_t Capacity, size_t& Count)
{
    Count = 0;
    Text  = Trim(Text);
    // Read whitespace-separated numeric components until the statement ends.
    //  v -0.5 1.0 2e-3
    //    ^
    //    Text
    while (!Text.empty())
    {
        if (Count == Capacity || !ReadFloat(ReadToken(Text), Values[Count]))
        {
            return false;
        }
        ++Count;
        Text = Trim(Text);
    }
    return true;
}

struct Corner
{
    Uint32 Position = MissingIndex;
    Uint32 TexCoord = MissingIndex;
    Uint32 Normal   = MissingIndex;
};

bool ReadCorner(std::string_view Text, size_t PositionCount, size_t TexCoordCount, size_t NormalCount, Corner& Result)
{
    // A face corner is v, v/vt, v//vn, or v/vt/vn. Each field indexes its own array.
    // First read the required position index, stopping at the first slash if present.
    //  12/4/7
    //  ^
    //  Text
    const size_t Slash = Text.find('/');
    if (!ReadIndex(Text.substr(0, Slash), PositionCount, Result.Position))
    {
        return false;
    }
    if (Slash == std::string_view::npos)
    {
        // "12": only the position index was supplied.
        return true;
    }
    Text.remove_prefix(Slash + 1);
    // The remaining text is "4", "4/7", or "/7".
    //  12/4/7
    //     ^
    //     Text
    const size_t SecondSlash = Text.find('/');
    if (SecondSlash == std::string_view::npos)
    {
        // "12/4": the remainder is the texture-coordinate index.
        return ReadIndex(Text, TexCoordCount, Result.TexCoord);
    }
    // "12/4/7" has a texture-coordinate index; "12//7" deliberately omits it.
    if (SecondSlash != 0 && !ReadIndex(Text.substr(0, SecondSlash), TexCoordCount, Result.TexCoord))
    {
        return false;
    }
    // The field after the second slash is the normal index in either form.
    //  12/4/7
    //       ^
    //       normal index
    return ReadIndex(Text.substr(SecondSlash + 1), NormalCount, Result.Normal);
}

struct VertexKey
{
    Uint32 Position;
    Uint32 TexCoord;
    Uint32 Normal;
    Uint32 GeneratedNormal;

    bool operator==(const VertexKey& Other) const
    {
        return Position == Other.Position && TexCoord == Other.TexCoord && Normal == Other.Normal && GeneratedNormal == Other.GeneratedNormal;
    }
};

struct VertexHash
{
    size_t operator()(const VertexKey& Key) const
    {
        return ComputeHash(Key.Position, Key.TexCoord, Key.Normal, Key.GeneratedNormal);
    }
};

struct SmoothKey
{
    Uint32 Position;
    Uint64 Group;

    bool operator==(const SmoothKey& Other) const
    {
        return Position == Other.Position && Group == Other.Group;
    }
};

struct SmoothHash
{
    size_t operator()(const SmoothKey& Key) const
    {
        return ComputeHash(Key.Position, Key.Group);
    }
};

struct GeneratedNormal
{
    double3 Sum{};
    double3 Fallback{};
    size_t  LastFace = 0;
};

float3 UnitNormal(const double3& Normal)
{
    const double Magnitude = length(Normal);
    return float3{static_cast<float>(Normal.x / Magnitude), static_cast<float>(Normal.y / Magnitude), static_cast<float>(Normal.z / Magnitude)};
}

double3 ToDouble(const float3& Value)
{
    return double3{Value.x, Value.y, Value.z};
}

bool ParseMap(std::string_view Text, TextureMap& Result, bool IsNormalMap, size_t Line, std::string& Error, std::string& Warnings)
{
    TextureMap Map;
    Text = Trim(Text);
    // Map options precede the filename.
    //  -s 2 3 -clamp on "painted metal.png"
    //  ^
    //  Text
    while (!Text.empty() && Text.front() == '-')
    {
        const std::string_view Option = ReadToken(Text);
        if (Option == "-o" || Option == "-s")
        {
            // -o u [v [w]] and -s u [v [w]] accept one to three numeric components.
            float3& Values = Option == "-s" ? Map.Scale : Map.Offset;
            size_t  Count  = 0;
            while (Count < 3)
            {
                // Probe without consuming the next option or filename if it is not numeric.
                // After reading two scale values, for example:
                //  -s 2 3 -clamp on "painted metal.png"
                //        ^
                //        Text
                std::string_view Remaining = Text;
                float            Value     = 0;
                if (!ReadFloat(ReadToken(Remaining), Value))
                {
                    break;
                }
                Values[Count++] = Value;
                Text            = Remaining;
            }
            if (Count == 0)
            {
                return Fail(Line, "Texture option requires a numeric argument.", Error);
            }
        }
        else if (Option == "-clamp")
        {
            //  -clamp on texture.png
            //        ^
            //        Text
            const std::string_view Value = ReadToken(Text);
            if (Value != "on" && Value != "off")
            {
                return Fail(Line, "Texture -clamp requires on or off.", Error);
            }
            Map.Clamp = Value == "on";
        }
        else if (Option == "-bm" && IsNormalMap)
        {
            // Normal-map strength is a single numeric argument.
            //  -bm 0.5 normal.png
            //     ^
            //     Text
            if (!ReadFloat(ReadToken(Text), Map.BumpMultiplier))
            {
                return Fail(Line, "Normal map -bm requires a numeric multiplier.", Error);
            }
        }
        else
        {
            Warn(Line, "Skipping texture map with unsupported option " + std::string{Option} + ".", Warnings);
            return true;
        }
        Text = Trim(Text);
    }
    // The remaining text is the filename, with optional surrounding quotes.
    //  -s 2 3 -clamp on "painted metal.png"
    //                   ^
    //                   Text
    Map.Name = ReadName(Text);
    if (Map.Name.empty())
    {
        return Fail(Line, "Texture map has no filename.", Error);
    }
    Result = std::move(Map);
    return true;
}

} // namespace

bool Parse(const char* Data, size_t Size, Document& Result, std::string& Error, std::string& Warnings)
{
    Result = Document{};
    Error.clear();
    Warnings.clear();
    if (Data == nullptr && Size != 0)
    {
        return Fail(1, "OBJ source is null.", Error);
    }

    Document                                          Output;
    std::vector<float3>                               SourcePositions;
    std::vector<float3>                               SourceNormals;
    std::vector<float2>                               SourceTexCoords;
    std::vector<float4>                               SourceColors;
    std::vector<bool>                                 SourceHasColor;
    std::vector<Corner>                               Face;
    std::vector<double3>                              Polygon;
    std::vector<Uint32>                               FaceVertices;
    std::vector<GeneratedNormal>                      GeneratedNormals;
    std::vector<Uint32>                               VertexGeneratedNormals;
    std::unordered_map<VertexKey, Uint32, VertexHash> Vertices;
    std::unordered_map<SmoothKey, Uint32, SmoothHash> SmoothNormals;
    Polygon3DTriangulator<Uint32, double>             Triangulator;
    std::string                                       MeshName = "Default";
    std::string                                       MaterialName;
    Uint64                                            SmoothingGroup = 0;
    size_t                                            FaceNumber     = 0;
    bool                                              NewMesh        = true;

    LineReader       Reader{Data, Size};
    std::string_view Text;
    size_t           Line = 0;
    while (Reader.Next(Text, Line))
    {
        const std::string_view Command = ReadToken(Text);
        if (Command.empty())
        {
            continue;
        }
        if (Command == "v" || Command == "vt" || Command == "vn")
        {
            // Parse the numeric components after the attribute command.
            //  v 1 2 3 0.2 0.4 0.6
            //   ^
            //   Text
            float  Values[7] = {};
            size_t Count     = 0;
            if (!ReadValues(Text, Values, 7, Count))
            {
                return Fail(Line, "Invalid numeric vertex attribute.", Error);
            }
            if (Command == "v")
            {
                // v x y z            : position
                // v x y z w          : homogeneous position, divide XYZ by W
                // v x y z r g b [a]  : position with RGB or RGBA color
                if (Count != 3 && Count != 4 && Count != 6 && Count != 7)
                {
                    return Fail(Line, "v requires XYZ, XYZW, XYZRGB or XYZRGBA.", Error);
                }
                if (SourcePositions.size() >= MissingIndex)
                {
                    return Fail(Line, "Too many source positions for 32-bit indices.", Error);
                }
                float3 Position{Values[0], Values[1], Values[2]};
                if (Count == 4)
                {
                    if (Values[3] == 0)
                    {
                        return Fail(Line, "Homogeneous vertex weight must not be zero.", Error);
                    }
                    Position /= Values[3];
                    if (!std::isfinite(Position.x) || !std::isfinite(Position.y) || !std::isfinite(Position.z))
                    {
                        return Fail(Line, "Homogeneous vertex is outside the finite coordinate range.", Error);
                    }
                }
                SourceHasColor.push_back(Count >= 6);
                if (Count >= 6 && SourceColors.empty())
                {
                    SourceColors.resize(SourcePositions.size(), float4{1, 1, 1, 1});
                }
                if (Count >= 6 || !SourceColors.empty())
                {
                    SourceColors.push_back(Count >= 6 ? float4{Values[3], Values[4], Values[5], Count == 7 ? Values[6] : 1.f} : float4{1, 1, 1, 1});
                }
                SourcePositions.push_back(Position);
            }
            else if (Command == "vt")
            {
                // vt u [v [w]]: V defaults to zero; W is accepted but not retained.
                if (Count < 1 || Count > 3)
                {
                    return Fail(Line, "vt requires one to three coordinates.", Error);
                }
                if (SourceTexCoords.size() >= MissingIndex)
                {
                    return Fail(Line, "Too many texture coordinates for 32-bit indices.", Error);
                }
                SourceTexCoords.emplace_back(Values[0], Values[1]);
            }
            else
            {
                // vn x y z: all three normal components are required.
                if (Count != 3)
                {
                    return Fail(Line, "vn requires three coordinates.", Error);
                }
                if (SourceNormals.size() >= MissingIndex)
                {
                    return Fail(Line, "Too many normals for 32-bit indices.", Error);
                }
                SourceNormals.emplace_back(Values[0], Values[1], Values[2]);
            }
        }
        else if (Command == "f")
        {
            Face.clear();
            Text = Trim(Text);
            // Read one independently indexed corner at a time.
            //  f 1/2/3 4//5 -1/6/7
            //    ^
            //    Text
            while (!Text.empty())
            {
                Corner Vertex;
                if (!ReadCorner(ReadToken(Text), SourcePositions.size(), SourceTexCoords.size(), SourceNormals.size(), Vertex))
                {
                    return Fail(Line, "Invalid face index: expected nonzero, in-range v, v/vt, v//vn or v/vt/vn.", Error);
                }
                Face.push_back(Vertex);
                Text = Trim(Text);
            }
            if (Face.size() < 3)
            {
                return Fail(Line, "A polygon face requires at least three vertices.", Error);
            }
            // The shared polygon triangulator uses int for its vertex count.
            if (Face.size() > static_cast<size_t>((std::numeric_limits<int>::max)()) ||
                Face.size() - 2 > (MissingIndex - Output.Indices.size()) / 3)
            {
                return Fail(Line, "Polygon exceeds the supported 32-bit index count.", Error);
            }
            ++FaceNumber;
            const double3 Origin = ToDouble(SourcePositions[Face.front().Position]);
            double3       FaceNormal{};
            for (size_t i = 1; i + 1 < Face.size(); ++i)
            {
                FaceNormal += cross(ToDouble(SourcePositions[Face[i].Position]) - Origin,
                                    ToDouble(SourcePositions[Face[i + 1].Position]) - Origin);
            }
            if (length(FaceNormal) == 0)
            {
                return Fail(Line, "Polygon has zero area and cannot be triangulated.", Error);
            }

            const Uint32  Triangle[3]        = {0, 1, 2};
            const Uint32* TriangleIndices    = Triangle;
            size_t        TriangleIndexCount = 3;
            if (Face.size() > 3)
            {
                Polygon.clear();
                Polygon.reserve(Face.size());
                for (const Corner& Vertex : Face)
                {
                    Polygon.push_back(ToDouble(SourcePositions[Vertex.Position]));
                }
                const std::vector<Uint32>& Triangles = Triangulator.Triangulate(Polygon);
                if (Triangulator.GetResult() != TRIANGULATE_POLYGON_RESULT_OK || Triangles.size() != (Face.size() - 2) * 3)
                {
                    return Fail(Line, "Polygon triangulation failed; expected a simple, nondegenerate polygon.", Error);
                }
                TriangleIndices    = Triangles.data();
                TriangleIndexCount = Triangles.size();
            }

            FaceVertices.clear();
            FaceVertices.reserve(Face.size());
            Uint32 FlatNormal = MissingIndex;
            for (const Corner& Vertex : Face)
            {
                Uint32 NormalIndex = MissingIndex;
                if (SmoothingGroup != 0)
                {
                    const SmoothKey Key{Vertex.Position, SmoothingGroup};
                    const auto      Found = SmoothNormals.find(Key);
                    if (Found == SmoothNormals.end())
                    {
                        if (GeneratedNormals.size() >= MissingIndex)
                        {
                            return Fail(Line, "Too many generated normal groups.", Error);
                        }
                        NormalIndex = static_cast<Uint32>(GeneratedNormals.size());
                        SmoothNormals.emplace(Key, NormalIndex);
                        GeneratedNormals.push_back(GeneratedNormal{FaceNormal, FaceNormal, FaceNumber});
                    }
                    else
                    {
                        NormalIndex             = Found->second;
                        GeneratedNormal& Normal = GeneratedNormals[NormalIndex];
                        if (Normal.LastFace != FaceNumber)
                        {
                            Normal.Sum += FaceNormal;
                            Normal.LastFace = FaceNumber;
                        }
                    }
                }
                else if (Vertex.Normal == MissingIndex)
                {
                    if (FlatNormal == MissingIndex)
                    {
                        if (GeneratedNormals.size() >= MissingIndex)
                        {
                            return Fail(Line, "Too many generated normal groups.", Error);
                        }
                        FlatNormal = static_cast<Uint32>(GeneratedNormals.size());
                        GeneratedNormals.push_back(GeneratedNormal{FaceNormal, FaceNormal, FaceNumber});
                    }
                    NormalIndex = FlatNormal;
                }
                if (Vertex.Normal != MissingIndex)
                {
                    NormalIndex = MissingIndex;
                }
                const VertexKey Key{Vertex.Position, Vertex.TexCoord, Vertex.Normal, NormalIndex};
                const auto      Found = Vertices.find(Key);
                if (Found != Vertices.end())
                {
                    FaceVertices.push_back(Found->second);
                    continue;
                }
                if (Output.Positions.size() >= MissingIndex)
                {
                    return Fail(Line, "Too many unified vertices for 32-bit indices.", Error);
                }
                const Uint32 Index = static_cast<Uint32>(Output.Positions.size());
                Vertices.emplace(Key, Index);
                FaceVertices.push_back(Index);
                if (Vertex.TexCoord != MissingIndex && Output.TexCoords.empty())
                {
                    Output.TexCoords.resize(Output.Positions.size());
                }
                if (Vertex.TexCoord != MissingIndex || !Output.TexCoords.empty())
                {
                    Output.TexCoords.push_back(Vertex.TexCoord != MissingIndex ? SourceTexCoords[Vertex.TexCoord] : float2{});
                }
                if (SourceHasColor[Vertex.Position] && Output.Colors.empty())
                {
                    Output.Colors.resize(Output.Positions.size(), float4{1, 1, 1, 1});
                }
                if (SourceHasColor[Vertex.Position] || !Output.Colors.empty())
                {
                    Output.Colors.push_back(SourceColors[Vertex.Position]);
                }
                Output.Positions.push_back(SourcePositions[Vertex.Position]);
                Output.Normals.push_back(Vertex.Normal != MissingIndex ? SourceNormals[Vertex.Normal] : float3{});
                VertexGeneratedNormals.push_back(NormalIndex);
            }

            if (NewMesh)
            {
                Output.Meshes.push_back(Mesh{MeshName, {}});
                NewMesh = false;
            }
            Mesh& CurrentMesh = Output.Meshes.back();
            if (CurrentMesh.Primitives.empty() || CurrentMesh.Primitives.back().MaterialName != MaterialName)
            {
                CurrentMesh.Primitives.push_back(Primitive{MaterialName, static_cast<Uint32>(Output.Indices.size()), 0});
            }
            Primitive& CurrentPrimitive = CurrentMesh.Primitives.back();
            for (size_t i = 0; i < TriangleIndexCount; ++i)
            {
                Output.Indices.push_back(FaceVertices[TriangleIndices[i]]);
            }
            CurrentPrimitive.IndexCount += static_cast<Uint32>(TriangleIndexCount);
        }
        else if (Command == "o" || Command == "g")
        {
            // The whole remaining name identifies the mesh for subsequent faces.
            //  o Front panel
            //   ^
            //   Text
            MeshName = ReadName(Text);
            if (MeshName.empty())
            {
                MeshName = "Default";
            }
            NewMesh = true;
        }
        else if (Command == "usemtl")
        {
            // Select the material for subsequent faces.
            //  usemtl Painted metal
            //        ^
            //        Text
            MaterialName = ReadName(Text);
        }
        else if (Command == "mtllib")
        {
            Text = Trim(Text);
            // Read separate library filenames; quotes keep spaces inside a filename.
            //  mtllib "painted parts.mtl" metal.mtl
            //         ^
            //         Text
            if (Text.empty())
            {
                return Fail(Line, "mtllib requires a filename.", Error);
            }
            while (!Text.empty())
            {
                Output.MaterialLibraries.emplace_back(ReadToken(Text));
                Text = Trim(Text);
            }
        }
        else if (Command == "s")
        {
            // off/0 disables smoothing; on selects group 1; an integer selects that group.
            //  s 7
            //   ^
            //   Text
            const std::string_view Group = ReadToken(Text);
            if (!Trim(Text).empty())
            {
                return Fail(Line, "s requires one smoothing group, on or off.", Error);
            }
            if (Group == "off" || Group == "0")
            {
                SmoothingGroup = 0;
            }
            else if (Group == "on")
            {
                SmoothingGroup = 1;
            }
            else if (!ReadUnsigned(Group, SmoothingGroup) || SmoothingGroup == 0)
            {
                return Fail(Line, "Invalid smoothing group.", Error);
            }
        }
        else
        {
            Warn(Line, "Ignoring unsupported OBJ statement " + std::string{Command} + ".", Warnings);
        }
    }
    if (Output.Indices.empty())
    {
        return Fail(Line == 0 ? 1 : Line, "OBJ source contains no polygon faces.", Error);
    }
    for (size_t i = 0; i < Output.Normals.size(); ++i)
    {
        if (VertexGeneratedNormals[i] != MissingIndex)
        {
            const GeneratedNormal& Normal = GeneratedNormals[VertexGeneratedNormals[i]];
            Output.Normals[i]             = UnitNormal(length(Normal.Sum) != 0 ? Normal.Sum : Normal.Fallback);
        }
    }
    Result = std::move(Output);
    return true;
}

bool ParseMaterials(const char* Data, size_t Size, std::vector<Material>& Result, std::string& Error, std::string& Warnings)
{
    Result.clear();
    Error.clear();
    Warnings.clear();
    if (Data == nullptr && Size != 0)
    {
        return Fail(1, "MTL source is null.", Error);
    }
    std::vector<Material> Output;
    bool                  HasDissolve = false;
    LineReader            Reader{Data, Size};
    std::string_view      Text;
    size_t                Line = 0;
    while (Reader.Next(Text, Line))
    {
        const std::string_view Command = ReadToken(Text);
        if (Command.empty())
        {
            continue;
        }
        if (Command == "newmtl")
        {
            // Start a material using the entire remaining name.
            //  newmtl Painted metal
            //        ^
            //        Text
            const std::string Name = ReadName(Text);
            if (Name.empty())
            {
                return Fail(Line, "newmtl requires a material name.", Error);
            }
            Output.emplace_back();
            Output.back().Name = Name;
            HasDissolve        = false;
            continue;
        }
        if (Output.empty())
        {
            Warn(Line, "Ignoring material property before newmtl.", Warnings);
            continue;
        }
        Material& Current = Output.back();
        if (Command == "Kd" || Command == "Ks" || Command == "Ke" || Command == "Ka")
        {
            // Read a scalar or RGB color; a scalar supplies all three channels.
            // Ambient color (Ka) is accepted but not retained.
            //  Kd 0.8 0.4 0.2
            //    ^
            //    Text
            float  Values[3] = {};
            size_t Count     = 0;
            if (!ReadValues(Text, Values, 3, Count) || (Count != 1 && Count != 3))
            {
                return Fail(Line, std::string{Command} + " requires one or three numeric color components.", Error);
            }
            const float3 Color = Count == 1 ? float3{Values[0]} : float3{Values[0], Values[1], Values[2]};
            if (Command == "Kd")
            {
                Current.Diffuse = Color;
            }
            else if (Command == "Ks")
            {
                Current.Specular = Color;
            }
            else if (Command == "Ke")
            {
                Current.Emissive = Color;
            }
        }
        else if (Command == "d" || Command == "Tr" || Command == "Ns" || Command == "Ni")
        {
            // Read opacity (d), transparency (Tr), shininess (Ns), or refraction (Ni).
            // d takes precedence over Tr; Ni is accepted but not retained.
            //  Tr 0.25
            //    ^
            //    Text
            float  Value = 0;
            size_t Count = 0;
            if (!ReadValues(Text, &Value, 1, Count) || Count != 1)
            {
                return Fail(Line, std::string{Command} + " requires one numeric value.", Error);
            }
            if (Command == "d")
            {
                Current.Opacity = Value;
                HasDissolve     = true;
            }
            else if (Command == "Tr" && !HasDissolve)
            {
                Current.Opacity = 1 - Value;
            }
            else if (Command == "Ns")
            {
                Current.Shininess = Value;
            }
        }
        else if (Command == "illum")
        {
            // Read one nonnegative illumination-model number.
            //  illum 2
            //       ^
            //       Text
            Uint64 Value = 0;
            if (!ReadUnsigned(ReadToken(Text), Value) || !Trim(Text).empty() || Value > static_cast<Uint64>((std::numeric_limits<int>::max)()))
            {
                return Fail(Line, "illum requires a nonnegative integer.", Error);
            }
            Current.IlluminationModel = static_cast<int>(Value);
        }
        else if (Command == "map_Kd" || Command == "map_Ks" || Command == "map_Ke" || Command == "norm")
        {
            // Read map options followed by the filename.
            //  map_Kd -s 2 3 -clamp on "painted metal.png"
            //        ^
            //        Text
            // clang-format off
            TextureMap& Map =
                Command == "map_Kd" ? Current.DiffuseMap :
                Command == "map_Ks" ? Current.SpecularMap :
                Command == "map_Ke" ? Current.EmissiveMap :
                                      Current.NormalMap;
            // clang-format on
            if (!ParseMap(Text, Map, Command == "norm", Line, Error, Warnings))
            {
                return false;
            }
        }
        else
        {
            Warn(Line, "Ignoring unsupported MTL statement " + std::string{Command} + ".", Warnings);
        }
    }
    Result = std::move(Output);
    return true;
}

} // namespace OBJ
} // namespace Diligent
