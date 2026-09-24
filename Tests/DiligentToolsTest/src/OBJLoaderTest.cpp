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
 *  In no event and under no legal theory, whether in tort (including
 * negligence), contract, or otherwise, unless required by applicable law (such
 * as deliberate and grossly negligent acts) or agreed to in writing, shall any
 * Contributor be liable for any damages, including any direct, indirect,
 * special, incidental, or consequential damages of any character arising as a
 * result of this License or out of the use or inability to use the software
 * (including but not limited to damages for loss of goodwill, work stoppage,
 * computer failure or malfunction, or any and all other commercial damages or
 * losses), even if such Contributor has been advised of the possibility of such
 * damages.
 */

#include "OBJLoader.hpp"

#include "gtest/gtest.h"

#include <cmath>
#include <string>
#include <vector>

using namespace Diligent;

namespace
{

void ExpectFloat2(const float2& Actual, const float2& Expected)
{
    EXPECT_NEAR(Actual.x, Expected.x, 1e-6f);
    EXPECT_NEAR(Actual.y, Expected.y, 1e-6f);
}

void ExpectFloat3(const float3& Actual, const float3& Expected)
{
    EXPECT_NEAR(Actual.x, Expected.x, 1e-6f);
    EXPECT_NEAR(Actual.y, Expected.y, 1e-6f);
    EXPECT_NEAR(Actual.z, Expected.z, 1e-6f);
}

void ExpectFloat4(const float4& Actual, const float4& Expected)
{
    EXPECT_NEAR(Actual.x, Expected.x, 1e-6f);
    EXPECT_NEAR(Actual.y, Expected.y, 1e-6f);
    EXPECT_NEAR(Actual.z, Expected.z, 1e-6f);
    EXPECT_NEAR(Actual.w, Expected.w, 1e-6f);
}

class OBJLoaderTest : public ::testing::Test
{
protected:
    bool Parse(const std::string& Source)
    {
        return OBJ::Parse(Source.data(), Source.size(), Result, Error, Warnings);
    }

    bool ParseMaterials(const std::string& Source)
    {
        return OBJ::ParseMaterials(Source.data(), Source.size(), Materials, Error,
                                   Warnings);
    }

    OBJ::Document              Result;
    std::vector<OBJ::Material> Materials;
    std::string                Error;
    std::string                Warnings;
};

TEST_F(OBJLoaderTest, IndependentAttributeIndicesAndSeams)
{
    ASSERT_TRUE(Parse("v 0 0 2\n"
                      "v 2 0 2\n"
                      "v 0 3 2\n"
                      "vt 0.25 0.75\n"
                      "vt 0.5 0.125\n"
                      "vt 0.875 0.625\n"
                      "vn 0 0 -1\n"
                      "vn 0 1 0\n"
                      "f 1/3/2 2/1/2 3/2/2\n"
                      "f 1/1/1 3/2/1 2/3/1\n"))
        << Error;

    ASSERT_EQ(Result.Indices.size(), 6u);
    ASSERT_EQ(Result.Positions.size(), 6u);
    ASSERT_EQ(Result.TexCoords.size(), Result.Positions.size());
    ASSERT_EQ(Result.Normals.size(), Result.Positions.size());
    const float3 Positions[] = {{0, 0, 2}, {2, 0, 2}, {0, 3, 2}, {0, 0, 2}, {0, 3, 2}, {2, 0, 2}};
    const float2 TexCoords[] = {{0.875f, 0.625f}, {0.25f, 0.75f}, {0.5f, 0.125f}, {0.25f, 0.75f}, {0.5f, 0.125f}, {0.875f, 0.625f}};
    for (size_t Corner = 0; Corner < Result.Indices.size(); ++Corner)
    {
        const Uint32 Index = Result.Indices[Corner];
        ASSERT_LT(Index, Result.Positions.size());
        ExpectFloat3(Result.Positions[Index], Positions[Corner]);
        ExpectFloat2(Result.TexCoords[Index], TexCoords[Corner]);
        ExpectFloat3(Result.Normals[Index],
                     Corner < 3 ? float3{0, 1, 0} : float3{0, 0, -1});
    }
}

TEST_F(OBJLoaderTest, NegativeIndicesAreRelativeToEachAttributeAtTheFace)
{
    ASSERT_TRUE(Parse("v 0 0 0\n"
                      "v 1 0 0\n"
                      "v 0 1 0\n"
                      "vt 0 0\n"
                      "vt 0.2 0.3\n"
                      "vt 0.4 0.5\n"
                      "vt 0.6 0.7\n"
                      "vn 0 0 1\n"
                      "f -3/-2/-1 -2/-1/-1 -1/-3/-1\n"
                      "v 1 1 0\n"
                      "vn 0 0 -1\n"
                      "f -3/-4/-1 -1/-1/-1 -2/-2/-1\n"))
        << Error;

    ASSERT_EQ(Result.Indices.size(), 6u);
    ASSERT_EQ(Result.Normals.size(), Result.Positions.size());
    ASSERT_EQ(Result.TexCoords.size(), Result.Positions.size());
    const float3 Positions[] = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}};
    const float2 TexCoords[] = {{0.4f, 0.5f}, {0.6f, 0.7f}, {0.2f, 0.3f}, {0, 0}, {0.6f, 0.7f}, {0.4f, 0.5f}};
    for (size_t Corner = 0; Corner < Result.Indices.size(); ++Corner)
    {
        const Uint32 Index = Result.Indices[Corner];
        ASSERT_LT(Index, Result.Positions.size());
        ExpectFloat3(Result.Positions[Index], Positions[Corner]);
        ExpectFloat2(Result.TexCoords[Index], TexCoords[Corner]);
        ExpectFloat3(Result.Normals[Index],
                     Corner < 3 ? float3{0, 0, 1} : float3{0, 0, -1});
    }
}

TEST_F(OBJLoaderTest, OptionalTextureCoordinatesAndNormals)
{
    struct TestCase
    {
        const char* Face;
        bool        HasUVs;
        bool        HasNormals;
    };
    const TestCase Cases[] = {
        {"f 1 2 3\n", false, false},
        {"f 1/1 2/1 3/1\n", true, false},
        {"f 1//1 2//1 3//1\n", false, true},
        {"f 1/1/1 2/1/1 3/1/1\n", true, true},
    };
    for (const TestCase& Case : Cases)
    {
        SCOPED_TRACE(Case.Face);
        const std::string Source =
            std::string{"v 0 0 0\nv 1 0 0\nv 0 1 0\nvt 0.25 0.75\nvn 0 1 0\n"} +
            Case.Face;
        ASSERT_TRUE(Parse(Source)) << Error;
        ASSERT_EQ(Result.Positions.size(), 3u);
        ASSERT_EQ(Result.Normals.size(), Result.Positions.size());
        EXPECT_TRUE(Result.Colors.empty());
        ASSERT_EQ(Result.TexCoords.size(),
                  Case.HasUVs ? Result.Positions.size() : 0u);
        for (size_t Vertex = 0; Vertex < Result.Positions.size(); ++Vertex)
        {
            ExpectFloat3(Result.Normals[Vertex],
                         Case.HasNormals ? float3{0, 1, 0} : float3{0, 0, 1});
            if (Case.HasUVs)
            {
                ExpectFloat2(Result.TexCoords[Vertex], float2{0.25f, 0.75f});
            }
        }
    }
}

TEST_F(OBJLoaderTest, MixedMissingAttributesRemainAligned)
{
    ASSERT_TRUE(Parse("v 0 0 0\nv 1 0 0\nv 0 1 0\n"
                      "vt 0.25 0.75\nvn 0 1 0\n"
                      "f 1/1/1 2//1 3\n"))
        << Error;
    ASSERT_EQ(Result.Indices.size(), 3u);
    ASSERT_EQ(Result.Normals.size(), Result.Positions.size());
    ASSERT_EQ(Result.TexCoords.size(), Result.Positions.size());
    for (size_t Corner = 0; Corner < Result.Indices.size(); ++Corner)
    {
        const Uint32 Index = Result.Indices[Corner];
        ASSERT_LT(Index, Result.Positions.size());
        ExpectFloat2(Result.TexCoords[Index],
                     Corner == 0 ? float2{0.25f, 0.75f} : float2{0, 0});
        ExpectFloat3(Result.Normals[Index],
                     Corner < 2 ? float3{0, 1, 0} : float3{0, 0, 1});
    }
}

TEST_F(OBJLoaderTest, LateTextureCoordinatesAndColorsFillEarlierVertices)
{
    // The first face creates vertices before either optional array exists.
    // The second face introduces both attributes on its second corner.
    ASSERT_TRUE(Parse("v 0 0 0\nv 1 0 0\nv 0 1 0\n"
                      "f 1 2 3\n"
                      "v 1 1 0 0.2 0.4 0.6 0.8\nvt 0.25 0.75\n"
                      "f 2 4/1 3\n"
                      "v 2 0 0\nf 2 5 4\n"))
        << Error;
    ASSERT_EQ(Result.Indices.size(), 9u);
    ASSERT_EQ(Result.Positions.size(), 9u);
    ASSERT_EQ(Result.TexCoords.size(), Result.Positions.size());
    ASSERT_EQ(Result.Colors.size(), Result.Positions.size());
    const float3 Positions[] = {
        {0, 0, 0},
        {1, 0, 0},
        {0, 1, 0},
        {1, 0, 0},
        {1, 1, 0},
        {0, 1, 0},
        {1, 0, 0},
        {2, 0, 0},
        {1, 1, 0},
    };
    for (size_t Corner = 0; Corner < Result.Indices.size(); ++Corner)
    {
        const Uint32 Index = Result.Indices[Corner];
        ASSERT_LT(Index, Result.Positions.size());
        ExpectFloat3(Result.Positions[Index], Positions[Corner]);
        ExpectFloat2(Result.TexCoords[Index], Corner == 4 ? float2{0.25f, 0.75f} : float2{});
        ExpectFloat4(Result.Colors[Index],
                     Corner == 4 || Corner == 8 ? float4{0.2f, 0.4f, 0.6f, 0.8f} : float4{1, 1, 1, 1});
    }
}

TEST_F(OBJLoaderTest, ConcavePolygonsPreserveAreaAndWinding)
{
    // The fixed triangles tile this L shape. Reversing the face also reverses
    // the unified vertex order, which is checked separately from the indices.
    // 6---5
    // |   |
    // |   4---3
    // |       |
    // 1-------2
    struct TestCase
    {
        const char*         Face;
        std::vector<float3> Positions;
        std::vector<Uint32> Indices;
        float3              Normal;
    };
    const TestCase Cases[] = {
        {"f 1 2 3 4 5 6\n",
         {{0, 0, 5}, {2, 0, 5}, {2, 1, 5}, {1, 1, 5}, {1, 2, 5}, {0, 2, 5}},
         {0, 1, 2, 0, 2, 3, 5, 0, 3, 3, 4, 5},
         {0, 0, 1}},
        {"f 6 5 4 3 2 1\n",
         {{0, 2, 5}, {1, 2, 5}, {1, 1, 5}, {2, 1, 5}, {2, 0, 5}, {0, 0, 5}},
         {5, 0, 1, 5, 1, 2, 5, 2, 3, 3, 4, 5},
         {0, 0, -1}},
    };
    for (const TestCase& Case : Cases)
    {
        SCOPED_TRACE(Case.Face);
        const std::string Source =
            std::string{"v 0 0 5\nv 2 0 5\nv 2 1 5\nv 1 1 5\nv 1 2 5\nv 0 2 5\n"} + Case.Face;
        ASSERT_TRUE(Parse(Source)) << Error;
        EXPECT_EQ(Result.Indices, Case.Indices);
        ASSERT_EQ(Result.Positions.size(), Case.Positions.size());
        ASSERT_EQ(Result.Normals.size(), Case.Positions.size());
        for (size_t Vertex = 0; Vertex < Case.Positions.size(); ++Vertex)
        {
            ExpectFloat3(Result.Positions[Vertex], Case.Positions[Vertex]);
            ExpectFloat3(Result.Normals[Vertex], Case.Normal);
        }
    }
}


TEST_F(OBJLoaderTest, ObliqueQuadUsesUnifiedVertexAndPrimitiveOffsets)
{
    // XY outline, lifted onto z = x + 3*y - 2:
    // 4---3
    // |   |
    // 1---2
    // A preceding triangle makes the quad's vertex and primitive offsets nonzero.
    ASSERT_TRUE(Parse("v 2 0 0\nv 4 0 2\nv 4 1 5\nv 2 1 3\n"
                      "usemtl Triangle\nf 1 2 3\n"
                      "usemtl Quad\nf 1 2 3 4\n"))
        << Error;
    const std::vector<Uint32> Indices{0, 1, 2, 6, 3, 4, 4, 5, 6};
    EXPECT_EQ(Result.Indices, Indices);
    ASSERT_EQ(Result.Positions.size(), 7u);
    ASSERT_EQ(Result.Normals.size(), Result.Positions.size());
    const float3 Positions[] = {
        {2, 0, 0},
        {4, 0, 2},
        {4, 1, 5},
        {2, 0, 0},
        {4, 0, 2},
        {4, 1, 5},
        {2, 1, 3},
    };
    const float  InverseNormalLength = 1.0f / std::sqrt(11.0f);
    const float3 Normal{-InverseNormalLength, -3.0f * InverseNormalLength, InverseNormalLength};
    for (size_t Vertex = 0; Vertex < Result.Positions.size(); ++Vertex)
    {
        ExpectFloat3(Result.Positions[Vertex], Positions[Vertex]);
        ExpectFloat3(Result.Normals[Vertex], Normal);
    }
    ASSERT_EQ(Result.Meshes.size(), 1u);
    ASSERT_EQ(Result.Meshes[0].Primitives.size(), 2u);
    const OBJ::Primitive& Triangle = Result.Meshes[0].Primitives[0];
    const OBJ::Primitive& Quad     = Result.Meshes[0].Primitives[1];
    EXPECT_EQ(Triangle.MaterialName, "Triangle");
    EXPECT_EQ(Triangle.FirstIndex, 0u);
    EXPECT_EQ(Triangle.IndexCount, 3u);
    EXPECT_EQ(Quad.MaterialName, "Quad");
    EXPECT_EQ(Quad.FirstIndex, 3u);
    EXPECT_EQ(Quad.IndexCount, 6u);
}

TEST_F(OBJLoaderTest, FlatNormalsKeepHardEdges)
{
    ASSERT_TRUE(Parse("v 0 0 0\nv 1 0 0\nv 0 1 0\nv 0 0 1\n"
                      "s off\nf 1 2 3\nf 1 4 2\n"))
        << Error;
    ASSERT_EQ(Result.Indices.size(), 6u);
    ASSERT_EQ(Result.Positions.size(), 6u);
    ASSERT_EQ(Result.Normals.size(), Result.Positions.size());
    for (size_t Corner = 0; Corner < Result.Indices.size(); ++Corner)
    {
        const Uint32 Index = Result.Indices[Corner];
        ASSERT_LT(Index, Result.Positions.size());
        ExpectFloat3(Result.Normals[Index],
                     Corner < 3 ? float3{0, 0, 1} : float3{0, 1, 0});
    }
}

TEST_F(OBJLoaderTest, SmoothingGroupsAverageSharedPositions)
{
    ASSERT_TRUE(Parse("v 0 0 0\nv 1 0 0\nv 0 1 0\nv 0 0 1\n"
                      "s 7\nf 1 2 3\nf 1 4 2\n"))
        << Error;
    ASSERT_EQ(Result.Indices.size(), 6u);
    ASSERT_EQ(Result.Positions.size(), 4u);
    ASSERT_EQ(Result.Normals.size(), Result.Positions.size());
    const float Component = 1.0f / std::sqrt(2.0f);
    for (size_t Vertex = 0; Vertex < Result.Positions.size(); ++Vertex)
    {
        const float3& Position = Result.Positions[Vertex];

        // clang-format off
        const float3 Expected =
            Position.y == 1 ? float3{0, 0, 1} :
            Position.z == 1 ? float3{0, 1, 0} :
                              float3{0, Component, Component};
        // clang-format on
        ExpectFloat3(Result.Normals[Vertex], Expected);
    }
}

TEST_F(OBJLoaderTest, SmoothingContinuesAcrossTextureCoordinateSeams)
{
    ASSERT_TRUE(Parse("v 0 0 0\nv 1 0 0\nv 0 1 0\nv 0 0 1\n"
                      "vt 0 0\nvt 1 0\nvt 0 1\nvt 0.5 0.5\n"
                      "s 1\nf 1/1 2/2 3/3\nf 1/4 4/3 2/1\n"))
        << Error;
    ASSERT_EQ(Result.Positions.size(), 6u);
    ASSERT_EQ(Result.Normals.size(), Result.Positions.size());
    ASSERT_EQ(Result.TexCoords.size(), Result.Positions.size());
    const float Component    = 1.0f / std::sqrt(2.0f);
    size_t      SeamVertices = 0;
    for (size_t Vertex = 0; Vertex < Result.Positions.size(); ++Vertex)
    {
        const float3& Position = Result.Positions[Vertex];
        if (Position.y == 0 && Position.z == 0)
        {
            ExpectFloat3(Result.Normals[Vertex], float3{0, Component, Component});
            ++SeamVertices;
        }
    }
    EXPECT_EQ(SeamVertices, 4u);
}

TEST_F(OBJLoaderTest, DistinctSmoothingGroupsKeepHardEdges)
{
    ASSERT_TRUE(Parse("v 0 0 0\nv 1 0 0\nv 0 1 0\nv 0 0 1\n"
                      "s 7\nf 1 2 3\ns 8\nf 1 4 2\n"))
        << Error;
    ASSERT_EQ(Result.Indices.size(), 6u);
    ASSERT_EQ(Result.Positions.size(), 6u);
    ASSERT_EQ(Result.Normals.size(), Result.Positions.size());

    // The faces share positions 1 and 2, but different smoothing groups
    // require separate vertices with the individual +Z and +Y normals.
    EXPECT_NE(Result.Indices[0], Result.Indices[3]);
    EXPECT_NE(Result.Indices[1], Result.Indices[5]);
    for (size_t Corner = 0; Corner < Result.Indices.size(); ++Corner)
    {
        const Uint32 Index = Result.Indices[Corner];
        ASSERT_LT(Index, Result.Positions.size());
        ExpectFloat3(Result.Normals[Index],
                     Corner < 3 ? float3{0, 0, 1} : float3{0, 1, 0});
    }
}

TEST_F(OBJLoaderTest, SmoothingWeightsNormalsByFaceArea)
{
    ASSERT_TRUE(Parse("v 0 0 0\nv 1 0 0\nv 0 1 0\nv 0 0 2\n"
                      "s 7\nf 1 2 3\nf 1 4 2\n"))
        << Error;
    ASSERT_EQ(Result.Indices.size(), 6u);
    ASSERT_EQ(Result.Positions.size(), 4u);
    ASSERT_EQ(Result.Normals.size(), Result.Positions.size());
    EXPECT_EQ(Result.Indices[0], Result.Indices[3]);
    EXPECT_EQ(Result.Indices[1], Result.Indices[5]);

    // The +Y face has twice the area of the +Z face, so the shared normal
    // is normalize(0, 2, 1), rather than the average of the unit normals.
    const float  InvLength = 1.0f / std::sqrt(5.0f);
    const float3 SharedNormal{0, 2 * InvLength, InvLength};
    const float3 ExpectedNormals[] = {
        SharedNormal, SharedNormal, {0, 0, 1}, SharedNormal, {0, 1, 0}, SharedNormal};
    for (size_t Corner = 0; Corner < Result.Indices.size(); ++Corner)
    {
        const Uint32 Index = Result.Indices[Corner];
        ASSERT_LT(Index, Result.Positions.size());
        ExpectFloat3(Result.Normals[Index], ExpectedNormals[Corner]);
    }
}

TEST_F(OBJLoaderTest, CancellingSmoothedNormalsUseFirstFace)
{
    for (bool ReverseOrder : {false, true})
    {
        SCOPED_TRACE(ReverseOrder);
        const std::string Source =
            std::string{"v 0 0 0\nv 1 0 0\nv 0 1 0\ns 7\n"} +
            (ReverseOrder ? "f 1 3 2\nf 1 2 3\n" : "f 1 2 3\nf 1 3 2\n");
        ASSERT_TRUE(Parse(Source)) << Error;
        ASSERT_EQ(Result.Indices.size(), 6u);
        ASSERT_EQ(Result.Positions.size(), 3u);
        ASSERT_EQ(Result.Normals.size(), Result.Positions.size());
        EXPECT_EQ(Result.Indices[0], Result.Indices[3]);
        EXPECT_EQ(Result.Indices[1], Result.Indices[5]);
        EXPECT_EQ(Result.Indices[2], Result.Indices[4]);

        // Opposite windings give equal and opposite normal contributions.
        // The first face supplies a finite unit normal when the sum is zero.
        const float3 ExpectedNormal{0, 0, ReverseOrder ? -1.0f : 1.0f};
        for (Uint32 Index : Result.Indices)
        {
            ASSERT_LT(Index, Result.Positions.size());
            const float3& Normal = Result.Normals[Index];
            EXPECT_TRUE(std::isfinite(Normal.x));
            EXPECT_TRUE(std::isfinite(Normal.y));
            EXPECT_TRUE(std::isfinite(Normal.z));
            ExpectFloat3(Normal, ExpectedNormal);
        }
    }
}

TEST_F(OBJLoaderTest, SmoothingPreservesAuthoredNormals)
{
    ASSERT_TRUE(Parse("v 0 0 0\nv 1 0 0\nv 0 1 0\nv 0 0 1\n"
                      "vn 0 0 -2\ns 7\nf 1//1 2 3\nf 1 4 2\n"))
        << Error;
    ASSERT_EQ(Result.Indices.size(), 6u);
    ASSERT_EQ(Result.Positions.size(), 5u);
    ASSERT_EQ(Result.Normals.size(), Result.Positions.size());
    EXPECT_NE(Result.Indices[0], Result.Indices[3]);
    EXPECT_EQ(Result.Indices[1], Result.Indices[5]);

    // The authored normal is preserved verbatim, including its length.
    // Generated normals at both shared positions use the face geometry:
    // the equal-area +Z and +Y faces contribute even at the authored corner.
    const float  Component = 1.0f / std::sqrt(2.0f);
    const float3 SharedNormal{0, Component, Component};
    const float3 ExpectedNormals[] = {
        {0, 0, -2}, SharedNormal, {0, 0, 1}, SharedNormal, {0, 1, 0}, SharedNormal};
    for (size_t Corner = 0; Corner < Result.Indices.size(); ++Corner)
    {
        const Uint32 Index = Result.Indices[Corner];
        ASSERT_LT(Index, Result.Positions.size());
        ExpectFloat3(Result.Normals[Index], ExpectedNormals[Corner]);
    }
}

TEST_F(OBJLoaderTest, VertexColorsAndHomogeneousPositions)
{
    ASSERT_TRUE(Parse("v .0 0e0 -0.0 +2e-1 3.e-1 .4\n"
                      "v +2.0e+0 0 0 2\n"
                      "v 0 1. 0 0.5 0.6 0.7 0.8\n"
                      "f 1 2 3\n"))
        << Error;
    ASSERT_EQ(Result.Indices.size(), 3u);
    ASSERT_EQ(Result.Colors.size(), Result.Positions.size());
    const float3 Positions[] = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}};
    const float4 Colors[]    = {
        {0.2f, 0.3f, 0.4f, 1}, {1, 1, 1, 1}, {0.5f, 0.6f, 0.7f, 0.8f}};
    for (size_t Corner = 0; Corner < Result.Indices.size(); ++Corner)
    {
        const Uint32 Index = Result.Indices[Corner];
        ASSERT_LT(Index, Result.Positions.size());
        ExpectFloat3(Result.Positions[Index], Positions[Corner]);
        ExpectFloat4(Result.Colors[Index], Colors[Corner]);
    }
    EXPECT_TRUE(std::signbit(Result.Positions[Result.Indices[0]].z));
}

TEST_F(OBJLoaderTest, ObjectGroupAndMaterialRanges)
{
    ASSERT_TRUE(Parse("v 0 0 0\nv 1 0 0\nv 0 1 0\n"
                      "o Object\nusemtl red\nf 1 2 3\n"
                      "usemtl unresolved\nf 1 2 3\n"
                      "usemtl red\nf 1 2 3\n"
                      "g Group\nf 1 2 3\n"))
        << Error;
    ASSERT_EQ(Result.Meshes.size(), 2u);
    EXPECT_EQ(Result.Meshes[0].Name, "Object");
    EXPECT_EQ(Result.Meshes[1].Name, "Group");
    ASSERT_EQ(Result.Meshes[0].Primitives.size(), 3u);
    ASSERT_EQ(Result.Meshes[1].Primitives.size(), 1u);
    EXPECT_EQ(Result.Meshes[0].Primitives[0].MaterialName, "red");
    EXPECT_EQ(Result.Meshes[0].Primitives[1].MaterialName, "unresolved");
    EXPECT_EQ(Result.Meshes[0].Primitives[2].MaterialName, "red");
    EXPECT_EQ(Result.Meshes[1].Primitives[0].MaterialName, "red");
    Uint32 NextIndex = 0;
    for (const OBJ::Mesh& Mesh : Result.Meshes)
    {
        for (const OBJ::Primitive& Primitive : Mesh.Primitives)
        {
            EXPECT_EQ(Primitive.FirstIndex, NextIndex);
            EXPECT_EQ(Primitive.IndexCount, 3u);
            NextIndex += Primitive.IndexCount;
        }
    }
    EXPECT_EQ(NextIndex, Result.Indices.size());
}

TEST_F(OBJLoaderTest, BoundedInputCommentsContinuationsAndMaterialLibraries)
{
    const std::string Source = "\xef\xbb\xbf# header\r\n"
                               "mtllib materials/base.mtl \"materials/extra "
                               "library.mtl\" # libraries\r\n"
                               "v 0 0 0\r\nv 1 0 0\r\nv 0 1 0\r\n"
                               "f 1 2 \\\r\n 3 # continued face\r\n";
    const std::string Buffer =
        Source + "f invalid data outside the source span\n";
    Error = "stale error";
    ASSERT_TRUE(OBJ::Parse(Buffer.data(), Source.size(), Result, Error, Warnings))
        << Error;
    EXPECT_TRUE(Error.empty());
    EXPECT_EQ(Result.Indices.size(), 3u);
    ASSERT_EQ(Result.MaterialLibraries.size(), 2u);
    EXPECT_EQ(Result.MaterialLibraries[0], "materials/base.mtl");
    EXPECT_EQ(Result.MaterialLibraries[1], "materials/extra library.mtl");
}

TEST_F(OBJLoaderTest, InvalidGeometryReportsLineAndClearsOutput)
{
    const char* InvalidLines[] = {
        "f 0 2 3\n",
        // In-range indices can still describe zero-area triangles or polygons.
        "f 1 1 3\n",
        "f 1 2 2 1\n",
        "f 1 2 4\n",
        "f -4 2 3\n",
        "f 1/0/1 2/1/1 3/1/1\n",
        "f 1/2/1 2/1/1 3/1/1\n",
        "f 1//0 2//1 3//1\n",
        "f 1//2 2//1 3//1\n",
        "f 1/1/1/1 2/1/1 3/1/1\n",
        "f 1x 2 3\n",
        "f 99999999999999999999999 2 3\n",
        "f 1 2\n",
        "f 1 2 \\\n4\n",
        "v nan 0 0\n",
        "v 1 inf 0\n",
        "v 1e1000 0 0\n",
        "v 0x1p0 0 0\n",
        "v 1e+ 0 0\n",
        "vt -inf 0\n",
        "vn 0 0 nan\n",
        "v 0 0\n",
        "v 0 0 0 0\n",
    };
    const std::string Prefix = "v 0 0 0\nv 1 0 0\nv 0 1 0\nvt 0 0\nvn 0 0 1\n";
    for (const char* InvalidLine : InvalidLines)
    {
        SCOPED_TRACE(InvalidLine);
        ASSERT_TRUE(Parse(Prefix + "mtllib previous.mtl\nf 1/1/1 2/1/1 3/1/1\n"))
            << Error;
        ASSERT_FALSE(Parse(Prefix + InvalidLine));
        EXPECT_NE(Error.find("Line 6:"), std::string::npos) << Error;
        EXPECT_TRUE(Result.Positions.empty());
        EXPECT_TRUE(Result.Normals.empty());
        EXPECT_TRUE(Result.TexCoords.empty());
        EXPECT_TRUE(Result.Colors.empty());
        EXPECT_TRUE(Result.Indices.empty());
        EXPECT_TRUE(Result.Meshes.empty());
        EXPECT_TRUE(Result.MaterialLibraries.empty());
    }
}

TEST_F(OBJLoaderTest, MaterialValuesAndDissolvePrecedence)
{
    ASSERT_TRUE(
        ParseMaterials("newmtl Painted Metal\n"
                       "Kd 0.2 0.3 0.4\nKs 0.5 0.6 0.7\nKe 0.01 0.02 0.03\n"
                       "Ns 64\nillum 3\nd 0.7\nTr 0.8\n"
                       "newmtl Second\nTr 0.9\nd 0.6\n"
                       "newmtl Transparent\nTr 0.25\n"))
        << Error;
    ASSERT_EQ(Materials.size(), 3u);
    const OBJ::Material& First = Materials[0];
    EXPECT_EQ(First.Name, "Painted Metal");
    ExpectFloat3(First.Diffuse, float3{0.2f, 0.3f, 0.4f});
    ExpectFloat3(First.Specular, float3{0.5f, 0.6f, 0.7f});
    ExpectFloat3(First.Emissive, float3{0.01f, 0.02f, 0.03f});
    EXPECT_FLOAT_EQ(First.Shininess, 64.0f);
    EXPECT_EQ(First.IlluminationModel, 3);
    EXPECT_NEAR(First.Opacity, 0.7f, 1e-6f);
    EXPECT_NEAR(Materials[1].Opacity, 0.6f, 1e-6f);
    EXPECT_NEAR(Materials[2].Opacity, 0.75f, 1e-6f);
    ExpectFloat3(Materials[2].Diffuse, float3{1, 1, 1});
    ExpectFloat3(Materials[2].Specular, float3{0, 0, 0});
    ExpectFloat3(Materials[2].Emissive, float3{0, 0, 0});
}

TEST_F(OBJLoaderTest, ScalarMaterialColors)
{
    ASSERT_TRUE(ParseMaterials("newmtl Scalar\n"
                               "Kd 0.2\nKs 0.4\nKe 0.6\n"
                               "Ns 32\nillum 3\nd 0.75\nKa 0.9\n"
                               "newmtl Next\nKd 0.3 0.5 0.7\nKs 0.8\n"))
        << Error;
    ASSERT_EQ(Materials.size(), 2u);
    EXPECT_TRUE(Error.empty());
    EXPECT_TRUE(Warnings.empty());

    // Ignored ambient color must not alter previously parsed material properties.
    const OBJ::Material& First = Materials[0];
    EXPECT_EQ(First.Name, "Scalar");
    ExpectFloat3(First.Diffuse, float3{0.2f, 0.2f, 0.2f});
    ExpectFloat3(First.Specular, float3{0.4f, 0.4f, 0.4f});
    ExpectFloat3(First.Emissive, float3{0.6f, 0.6f, 0.6f});
    EXPECT_FLOAT_EQ(First.Shininess, 32.0f);
    EXPECT_EQ(First.IlluminationModel, 3);
    EXPECT_FLOAT_EQ(First.Opacity, 0.75f);

    const OBJ::Material& Next = Materials[1];
    EXPECT_EQ(Next.Name, "Next");
    ExpectFloat3(Next.Diffuse, float3{0.3f, 0.5f, 0.7f});
    ExpectFloat3(Next.Specular, float3{0.8f, 0.8f, 0.8f});
}

TEST_F(OBJLoaderTest, MaterialMapTransformsAndFilenames)
{
    ASSERT_TRUE(ParseMaterials(
        "\xef\xbb\xbfnewmtl Maps\r\n"
        "map_Kd -s 2 3 4 -o 0.1 0.2 0.3 -clamp on \\\r\n"
        " \"textures/albedo #color.png\" # diffuse\r\n"
        "map_Ks textures/specular color.png\r\n"
        "map_Ke \"textures/emissive color.png\"\r\n"
        "norm -bm 0.35 -o -1 2 3 -clamp off textures/tangent normal.png\r\n"))
        << Error;
    ASSERT_EQ(Materials.size(), 1u);
    const OBJ::Material& Material = Materials[0];
    EXPECT_EQ(Material.DiffuseMap.Name, "textures/albedo #color.png");
    ExpectFloat3(Material.DiffuseMap.Scale, float3{2, 3, 4});
    ExpectFloat3(Material.DiffuseMap.Offset, float3{0.1f, 0.2f, 0.3f});
    EXPECT_TRUE(Material.DiffuseMap.Clamp);
    EXPECT_EQ(Material.SpecularMap.Name, "textures/specular color.png");
    ExpectFloat3(Material.SpecularMap.Scale, float3{1, 1, 1});
    ExpectFloat3(Material.SpecularMap.Offset, float3{0, 0, 0});
    EXPECT_FALSE(Material.SpecularMap.Clamp);
    EXPECT_EQ(Material.EmissiveMap.Name, "textures/emissive color.png");
    EXPECT_EQ(Material.NormalMap.Name, "textures/tangent normal.png");
    ExpectFloat3(Material.NormalMap.Offset, float3{-1, 2, 3});
    EXPECT_NEAR(Material.NormalMap.BumpMultiplier, 0.35f, 1e-6f);
    EXPECT_FALSE(Material.NormalMap.Clamp);
}

TEST_F(OBJLoaderTest, MaterialMapPartialTransformsPreserveFollowingTokens)
{
    ASSERT_TRUE(ParseMaterials("newmtl Partial transforms\n"
                               "map_Kd -s 2 -o -0.25 diffuse.png\n"
                               "map_Ks -o 0.5 0.75 -clamp on -s 3 4 specular.png\n"))
        << Error;
    ASSERT_EQ(Materials.size(), 1u);
    EXPECT_TRUE(Error.empty());
    EXPECT_TRUE(Warnings.empty());

    const OBJ::TextureMap& Diffuse = Materials[0].DiffuseMap;
    EXPECT_EQ(Diffuse.Name, "diffuse.png");
    ExpectFloat3(Diffuse.Scale, float3{2, 1, 1});
    ExpectFloat3(Diffuse.Offset, float3{-0.25f, 0, 0});
    EXPECT_FALSE(Diffuse.Clamp);

    const OBJ::TextureMap& Specular = Materials[0].SpecularMap;
    EXPECT_EQ(Specular.Name, "specular.png");
    ExpectFloat3(Specular.Scale, float3{3, 4, 1});
    ExpectFloat3(Specular.Offset, float3{0.5f, 0.75f, 0});
    EXPECT_TRUE(Specular.Clamp);
}

TEST_F(OBJLoaderTest, InvalidMaterialMapArgumentsReportLineAndClearOutput)
{
    const char* InvalidLines[] = {
        "map_Kd\n",
        "map_Ks -s 2\n",
        "map_Ke -o 0.5 0.75\n",
        "norm -bm 0.5\n",
        "map_Kd -s\n",
        "map_Kd -s diffuse.png\n",
        "map_Ks -o\n",
        "map_Ks -o -clamp on specular.png\n",
        "norm -clamp\n",
        "norm -clamp invalid normal.png\n",
        "norm -bm\n",
        "norm -bm invalid normal.png\n",
    };
    for (const char* InvalidLine : InvalidLines)
    {
        SCOPED_TRACE(InvalidLine);
        ASSERT_TRUE(ParseMaterials("newmtl Previous\nmap_Kd previous.png\n")) << Error;
        ASSERT_FALSE(ParseMaterials(std::string{"newmtl Invalid\n"} + InvalidLine));
        EXPECT_NE(Error.find("Line 2:"), std::string::npos) << Error;
        EXPECT_TRUE(Materials.empty());
    }
}

TEST_F(OBJLoaderTest, UnsupportedMaterialMapOptionPreservesPreviousMap)
{
    ASSERT_TRUE(ParseMaterials("newmtl Maps\n"
                               "map_Kd -s 2 3 -o 0.25 0.5 -clamp on original.png\n"
                               "map_Kd -o 9 8 -mm 0 1 skipped.png\n"
                               "Kd 0.1 0.2 0.3\n"
                               "map_Ks specular.png\n"
                               "newmtl Next\nKe 0.4 0.5 0.6\n"))
        << Error;
    ASSERT_EQ(Materials.size(), 2u);
    EXPECT_TRUE(Error.empty());
    EXPECT_NE(Warnings.find("Line 3:"), std::string::npos) << Warnings;
    EXPECT_NE(Warnings.find("-mm"), std::string::npos) << Warnings;

    const OBJ::Material& First = Materials[0];
    EXPECT_EQ(First.Name, "Maps");
    EXPECT_EQ(First.DiffuseMap.Name, "original.png");
    ExpectFloat3(First.DiffuseMap.Scale, float3{2, 3, 1});
    ExpectFloat3(First.DiffuseMap.Offset, float3{0.25f, 0.5f, 0});
    EXPECT_TRUE(First.DiffuseMap.Clamp);
    ExpectFloat3(First.Diffuse, float3{0.1f, 0.2f, 0.3f});
    EXPECT_EQ(First.SpecularMap.Name, "specular.png");
    EXPECT_EQ(Materials[1].Name, "Next");
    ExpectFloat3(Materials[1].Emissive, float3{0.4f, 0.5f, 0.6f});
}

TEST_F(OBJLoaderTest, HeightMapsWarnWithoutBecomingNormalMaps)
{
    ASSERT_TRUE(ParseMaterials("newmtl Height\n"
                               "bump -bm 0.25 textures/height.png\n"
                               "map_bump textures/other_height.png\n"))
        << Error;
    ASSERT_EQ(Materials.size(), 1u);
    EXPECT_TRUE(Materials[0].NormalMap.Name.empty());
    EXPECT_TRUE(Error.empty());
    EXPECT_NE(Warnings.find("Line 2:"), std::string::npos) << Warnings;
    EXPECT_NE(Warnings.find("Line 3:"), std::string::npos) << Warnings;
}

TEST_F(OBJLoaderTest, InvalidMaterialsReportLineAndClearOutput)
{
    const char* InvalidLines[] = {
        "Kd\n",
        "Kd 0.1 0.2 0.3 0.4\n",
        "Ka invalid\n",
        "Ka 0.2 0.3\n",
        "Kd 0.2 0.3 invalid\n",
        "Ks 0.2 0.3\n",
        "Ke 0 nan 0\n",
        "d inf\n",
        "Tr invalid\n",
        "Ns nan\n",
        "illum invalid\n",
    };
    for (const char* InvalidLine : InvalidLines)
    {
        SCOPED_TRACE(InvalidLine);
        ASSERT_TRUE(ParseMaterials("newmtl Previous\nKd 0.2 0.3 0.4\n")) << Error;
        ASSERT_FALSE(ParseMaterials(std::string{"newmtl Invalid\n"} + InvalidLine));
        EXPECT_NE(Error.find("Line 2:"), std::string::npos) << Error;
        EXPECT_TRUE(Materials.empty());
    }
}

} // namespace
