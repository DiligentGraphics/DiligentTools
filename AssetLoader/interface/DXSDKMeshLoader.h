/*     Copyright 2015-2019 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF ANY PROPRIETARY RIGHTS.
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

#include "../../../DiligentCore/Primitives/interface/BasicTypes.h"
#include "../../../DiligentCore/Common/interface/BasicMath.h"
#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/Buffer.h"
#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/Texture.h"
#include "../../../DiligentCore/Graphics/GraphicsEngine/interface/TextureView.h"

namespace Diligent
{

//--------------------------------------------------------------------------------------
// Hard Defines for the various structures
//--------------------------------------------------------------------------------------
#define DXSDKMESH_FILE_VERSION  101

#define INVALID_FRAME           ((Uint32)-1)
#define INVALID_MESH            ((Uint32)-1)
#define INVALID_MATERIAL        ((Uint32)-1)
#define INVALID_SUBSET          ((Uint32)-1)
#define INVALID_ANIMATION_DATA  ((Uint32)-1)
#define INVALID_SAMPLER_SLOT    ((Uint32)-1)
#define ERROR_RESOURCE_VALUE    1

//--------------------------------------------------------------------------------------
// Enumerated Types.
//--------------------------------------------------------------------------------------
enum DXSDKMESH_PRIMITIVE_TYPE
{
    PT_TRIANGLE_LIST = 0,
    PT_TRIANGLE_STRIP,
    PT_LINE_LIST,
    PT_LINE_STRIP,
    PT_POINT_LIST,
    PT_TRIANGLE_LIST_ADJ,
    PT_TRIANGLE_STRIP_ADJ,
    PT_LINE_LIST_ADJ,
    PT_LINE_STRIP_ADJ,
    PT_QUAD_PATCH_LIST,
    PT_TRIANGLE_PATCH_LIST,
};

enum DXSDKMESH_INDEX_TYPE
{
    IT_16BIT = 0,
    IT_32BIT,
};

enum FRAME_TRANSFORM_TYPE
{
    FTT_RELATIVE = 0,
    FTT_ABSOLUTE,    //This is not currently used but is here to support absolute transformations in the future
};

//--------------------------------------------------------------------------------------
// Structures.  Unions with pointers are forced to 64bit.
//--------------------------------------------------------------------------------------
struct DXSDKMESH_HEADER
{
    //Basic Info and sizes
    Uint32 Version;
    Uint8  IsBigEndian;
    Uint64 HeaderSize;
    Uint64 NonBufferDataSize;
    Uint64 BufferDataSize;

    //Stats
    Uint32 NumVertexBuffers;
    Uint32 NumIndexBuffers;
    Uint32 NumMeshes;
    Uint32 NumTotalSubsets;
    Uint32 NumFrames;
    Uint32 NumMaterials;

    //Offsets to Data
    Uint64 VertexStreamHeadersOffset;
    Uint64 IndexStreamHeadersOffset;
    Uint64 MeshDataOffset;
    Uint64 SubsetDataOffset;
    Uint64 FrameDataOffset;
    Uint64 MaterialDataOffset;
};

struct DXSDKMESH_VERTEX_ELEMENT
{
    Uint16    Stream;     // Stream index
    Uint16    Offset;     // Offset in the stream in Uint8s
    Uint8     Type;       // Data type
    Uint8     Method;     // Processing method
    Uint8     Usage;      // Semantics
    Uint8     UsageIndex; // Semantic index
};

struct DXSDKMESH_VERTEX_BUFFER_HEADER
{
    Uint64 NumVertices;
    Uint64 SizeUint8s;
    Uint64 StrideUint8s;

    static constexpr size_t MaxVertexElements = 32;
    DXSDKMESH_VERTEX_ELEMENT Decl[MaxVertexElements];
    union
    {
        Uint64   DataOffset;        //(This also forces the union to 64bits)
        IBuffer* pVB;
    };
};

struct DXSDKMESH_INDEX_BUFFER_HEADER
{
    Uint64 NumIndices;
    Uint64 SizeUint8s;
    Uint32 IndexType;
    union
    {
        Uint64   DataOffset;        //(This also forces the union to 64bits)
        IBuffer* pIB;
    };
};

struct DXSDKMESH_MESH
{
    static constexpr size_t MaxMeshName = 100;
    char   Name[MaxMeshName];
    Uint8  NumVertexBuffers;
    static constexpr size_t MaxVertexStreams = 16;
    Uint32 VertexBuffers[MaxVertexStreams];
    Uint32 IndexBuffer;
    Uint32 NumSubsets;
    Uint32 NumFrameInfluences; //aka bones

    float3 BoundingBoxCenter;
    float3 BoundingBoxExtents;

    union
    {
        Uint64 SubsetOffset;  //Offset to list of subsets (This also forces the union to 64bits)
        Uint32* pSubsets;      //Pointer to list of subsets
    };
    union
    {
        Uint64 FrameInfluenceOffset;  //Offset to list of frame influences (This also forces the union to 64bits)
        Uint32* pFrameInfluences;      //Pointer to list of frame influences
    };
};

struct DXSDKMESH_SUBSET
{
    static constexpr size_t MaxSubsetName = 100;
    char Name[MaxSubsetName];
    Uint32 MaterialID;
    Uint32 PrimitiveType;
    Uint64 IndexStart;
    Uint64 IndexCount;
    Uint64 VertexStart;
    Uint64 VertexCount;
};

struct DXSDKMESH_FRAME
{
    static constexpr size_t MaxFrameName = 100;
    char Name[MaxFrameName];
    Uint32 Mesh;
    Uint32 ParentFrame;
    Uint32 ChildFrame;
    Uint32 SiblingFrame;
    float4x4 Matrix;
    Uint32 AnimationDataIndex;    //Used to index which set of keyframes transforms this frame
};

struct DXSDKMESH_MATERIAL
{
    static constexpr size_t MaxMaterialName = 100;
    char    Name[MaxMaterialName];

    // Use MaterialInstancePath
    static constexpr size_t MaxMaterialPath = 260;
    char    MaterialInstancePath[MaxMaterialPath];

    // Or fall back to d3d8-type materials
    static constexpr size_t MaxTextureName = 260;
    char    DiffuseTexture[MaxTextureName];
    char    NormalTexture[MaxTextureName];
    char    SpecularTexture[MaxTextureName];

    float4 Diffuse;
    float4 Ambient;
    float4 Specular;
    float4 Emissive;
    float Power;

    union
    {
        Uint64 Force64_1;      //Force the union to 64bits
        ITexture* pDiffuseTexture;
    };
    union
    {
        Uint64 Force64_2;      //Force the union to 64bits
        ITexture* pNormalTexture;
    };
    union
    {
        Uint64 Force64_3;      //Force the union to 64bits
        ITexture* pSpecularTexture;
    };

    union
    {
        Uint64 Force64_4;      //Force the union to 64bits
        ITextureView* pDiffuseRV;
    };
    union
    {
        Uint64 Force64_5;        //Force the union to 64bits
        ITextureView* pNormalRV;
    };
    union
    {
        Uint64 Force64_6;      //Force the union to 64bits
        ITextureView* pSpecularRV;
    };

};

struct SDKANIMATION_FILE_HEADER
{
    Uint32 Version;
    Uint8 IsBigEndian;
    Uint32 FrameTransformType;
    Uint32 NumFrames;
    Uint32 NumAnimationKeys;
    Uint32 AnimationFPS;
    Uint64 AnimationDataSize;
    Uint64 AnimationDataOffset;
};

struct SDKANIMATION_DATA
{
    float3 Translation;
    float4 Orientation;
    float3 Scaling;
};

struct SDKANIMATION_FRAME_DATA
{
    static constexpr size_t MaxFrameName = 100;
    char FrameName[MaxFrameName];
    union
    {
        Uint64 DataOffset;
        SDKANIMATION_DATA* pAnimationData;
    };
};


//--------------------------------------------------------------------------------------
// CDXUTDXSDKMesh class.  This class reads the DXSDKMesh file format for use by the samples
//--------------------------------------------------------------------------------------
class DXSDKMesh
{
private:
    Uint32              m_NumOutstandingResources = 0;
    bool                m_bLoading                = false;
    std::vector<Uint8*> m_MappedPointers;

protected:
    //These are the pointers to the two chunks of data loaded in from the mesh file
    std::vector<Uint8>      m_StaticMeshData;
    Uint8*  m_pAnimationData    = nullptr;
    std::vector<Uint8*>     m_ppVertices;
    std::vector<Uint8*>     m_ppIndices;

    //General mesh info
    DXSDKMESH_HEADER*                 m_pMeshHeader        = nullptr;
    DXSDKMESH_VERTEX_BUFFER_HEADER*   m_pVertexBufferArray = nullptr;
    DXSDKMESH_INDEX_BUFFER_HEADER*    m_pIndexBufferArray  = nullptr;
    DXSDKMESH_MESH*                   m_pMeshArray         = nullptr;
    DXSDKMESH_SUBSET*                 m_pSubsetArray       = nullptr;
    DXSDKMESH_FRAME*                  m_pFrameArray        = nullptr;
    DXSDKMESH_MATERIAL*               m_pMaterialArray     = nullptr;

    // Adjacency information (not part of the m_pStaticMeshData, so it must be created and destroyed separately )
    DXSDKMESH_INDEX_BUFFER_HEADER* m_pAdjacencyIndexBufferArray = nullptr;

    //Animation (TODO: Add ability to load/track multiple animation sets)
    SDKANIMATION_FILE_HEADER*   m_pAnimationHeader          = nullptr;
    SDKANIMATION_FRAME_DATA*    m_pAnimationFrameData       = nullptr;
    float4x4*                   m_pBindPoseFrameMatrices    = nullptr;
    float4x4*                   m_pTransformedFrameMatrices = nullptr;
    float4x4*                   m_pWorldPoseFrameMatrices   = nullptr;

protected:
    virtual bool                    CreateFromFile( const char* szFileName,
                                                    bool bCreateAdjacencyIndices);

    virtual bool                    CreateFromMemory( Uint8* pData,
                                                      Uint32 DataUint8s,
                                                      bool bCreateAdjacencyIndices,
                                                      bool bCopyStatic );
public:
    virtual                         ~DXSDKMesh();

    virtual bool                    Create( const Char* szFileName, bool bCreateAdjacencyIndices = false );
    virtual bool                    Create( Uint8* pData, Uint32 DataUint8s,
                                            bool bCreateAdjacencyIndices = false, bool bCopyStatic = false );
    virtual void                    Destroy();


    //Helpers (D3D11 specific)
    static PRIMITIVE_TOPOLOGY       GetPrimitiveType( DXSDKMESH_PRIMITIVE_TYPE PrimType );
    VALUE_TYPE                      GetIBFormat( Uint32 iMesh );
    DXSDKMESH_INDEX_TYPE            GetIndexType( Uint32 iMesh );

    //Helpers (general)
    Uint32                          GetNumMeshes();
    Uint32                          GetNumMaterials();
    Uint32                          GetNumVBs();
    Uint32                          GetNumIBs();

    Uint8*                          GetRawVerticesAt( Uint32 iVB );
    Uint8*                          GetRawIndicesAt( Uint32 iIB );
    DXSDKMESH_MATERIAL*             GetMaterial( Uint32 iMaterial );
    DXSDKMESH_MESH*                 GetMesh( Uint32 iMesh );
    Uint32                          GetNumSubsets( Uint32 iMesh );
    DXSDKMESH_SUBSET*               GetSubset( Uint32 iMesh, Uint32 iSubset );
    Uint32                          GetVertexStride( Uint32 iMesh, Uint32 iVB );
    Uint32                          GetNumFrames();
    DXSDKMESH_FRAME*                GetFrame( Uint32 iFrame );
    DXSDKMESH_FRAME*                FindFrame( char* pszName );
    Uint64                          GetNumVertices( Uint32 iMesh, Uint32 iVB );
    Uint64                          GetNumIndices( Uint32 iMesh );

    const DXSDKMESH_VERTEX_ELEMENT*   VBElements( Uint32 iVB ) { return m_pVertexBufferArray[0].Decl; }
};

}