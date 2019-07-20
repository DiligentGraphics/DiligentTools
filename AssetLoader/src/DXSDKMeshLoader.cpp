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

#include "DXSDKMeshLoader.h"
#include "DataBlobImpl.h"
#include "RefCntAutoPtr.h"
#include "FileWrapper.h"

namespace Diligent
{


//--------------------------------------------------------------------------------------
bool DXSDKMesh::CreateFromFile( const char* szFileName, bool bCreateAdjacencyIndices)
{
    FileWrapper File;
    File.Open(FileOpenAttribs{szFileName});
    if (!File)
    {
        LOG_ERROR("Failed to open SDK Mesh file ", szFileName);
        return false;
    }

    RefCntAutoPtr<IDataBlob> pFileData(MakeNewRCObj<DataBlobImpl>()(0));
    File->Read(pFileData);

    File.Close();

    auto res = CreateFromMemory( reinterpret_cast<Uint8*>(pFileData->GetDataPtr()),
                                 static_cast<Uint32>(pFileData->GetSize()),
                                 bCreateAdjacencyIndices,
                                 false);
   
    return res;
}


bool DXSDKMesh::CreateFromMemory( Uint8* pData,
                                  Uint32 DataUint8s,
                                  bool   bCreateAdjacencyIndices,
                                  bool   bCopyStatic )
{
    // Set outstanding resources to zero
    m_NumOutstandingResources = 0;

    m_StaticMeshData.resize(DataUint8s);
    memcpy(m_StaticMeshData.data(), pData, DataUint8s);

    // Pointer fixup
    auto* pStaticMeshData = m_StaticMeshData.data();
    m_pMeshHeader        = reinterpret_cast<DXSDKMESH_HEADER*>              (pStaticMeshData);
    m_pVertexBufferArray = reinterpret_cast<DXSDKMESH_VERTEX_BUFFER_HEADER*>(pStaticMeshData + m_pMeshHeader->VertexStreamHeadersOffset);
    m_pIndexBufferArray  = reinterpret_cast<DXSDKMESH_INDEX_BUFFER_HEADER*> (pStaticMeshData + m_pMeshHeader->IndexStreamHeadersOffset);
    m_pMeshArray         = reinterpret_cast<DXSDKMESH_MESH*>                (pStaticMeshData + m_pMeshHeader->MeshDataOffset);
    m_pSubsetArray       = reinterpret_cast<DXSDKMESH_SUBSET*>              (pStaticMeshData + m_pMeshHeader->SubsetDataOffset);
    m_pFrameArray        = reinterpret_cast<DXSDKMESH_FRAME*>               (pStaticMeshData + m_pMeshHeader->FrameDataOffset);
    m_pMaterialArray     = reinterpret_cast<DXSDKMESH_MATERIAL*>            (pStaticMeshData + m_pMeshHeader->MaterialDataOffset);

    // Setup subsets
    for( Uint32 i = 0; i < m_pMeshHeader->NumMeshes; i++ )
    {
        m_pMeshArray[i].pSubsets         = reinterpret_cast<Uint32*>(pStaticMeshData + m_pMeshArray[i].SubsetOffset);
        m_pMeshArray[i].pFrameInfluences = reinterpret_cast<Uint32*>(pStaticMeshData + m_pMeshArray[i].FrameInfluenceOffset);
    }

    // error condition
    if( m_pMeshHeader->Version != DXSDKMESH_FILE_VERSION )
    {
        LOG_ERROR("Unexpected SDK mesh file version");
        return false;
    }

    // Setup buffer data pointer
    Uint8* pBufferData = pData + m_pMeshHeader->HeaderSize + m_pMeshHeader->NonBufferDataSize;

    // Get the start of the buffer data
    Uint64 BufferDataStart = m_pMeshHeader->HeaderSize + m_pMeshHeader->NonBufferDataSize;

    // Create VBs
    m_ppVertices.resize(m_pMeshHeader->NumVertexBuffers);
    for( Uint32 i = 0; i < m_pMeshHeader->NumVertexBuffers; i++ )
    {
        Uint8* pVertices = NULL;
        pVertices = reinterpret_cast<Uint8*>(pBufferData + ( m_pVertexBufferArray[i].DataOffset - BufferDataStart));

        m_ppVertices[i] = pVertices;
    }

    // Create IBs
    m_ppIndices.resize(m_pMeshHeader->NumIndexBuffers);
    for( Uint32 i = 0; i < m_pMeshHeader->NumIndexBuffers; i++ )
    {
        Uint8* pIndices = NULL;
        pIndices = ( Uint8* )( pBufferData + ( m_pIndexBufferArray[i].DataOffset - BufferDataStart ) );

        m_ppIndices[i] = pIndices;
    }

    return true;
}

//#define MAX_D3D11_VERTEX_STREAMS D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT

//--------------------------------------------------------------------------------------
DXSDKMesh::~DXSDKMesh()
{
    Destroy();
}

//--------------------------------------------------------------------------------------
bool DXSDKMesh::Create( const Char* szFileName, bool bCreateAdjacencyIndices )
{
    return CreateFromFile( szFileName, bCreateAdjacencyIndices );
}

//--------------------------------------------------------------------------------------
bool DXSDKMesh::Create( Uint8* pData, Uint32 DataUint8s, bool bCreateAdjacencyIndices, bool bCopyStatic)
{
    return CreateFromMemory( pData, DataUint8s, bCreateAdjacencyIndices, bCopyStatic );
}

//--------------------------------------------------------------------------------------
void DXSDKMesh::Destroy()
{
    delete[] m_pAdjacencyIndexBufferArray; m_pAdjacencyIndexBufferArray = nullptr;

    m_StaticMeshData.clear();
    delete[] m_pAnimationData;              m_pAnimationData            = nullptr;
    delete[] m_pBindPoseFrameMatrices;      m_pBindPoseFrameMatrices    = nullptr;
    delete[] m_pTransformedFrameMatrices;   m_pTransformedFrameMatrices = nullptr;
    delete[] m_pWorldPoseFrameMatrices;     m_pWorldPoseFrameMatrices   = nullptr;

    m_ppVertices.clear();
    m_ppIndices.clear();

    m_pMeshHeader        = nullptr;
    m_pVertexBufferArray = nullptr;
    m_pIndexBufferArray  = nullptr;
    m_pMeshArray         = nullptr;
    m_pSubsetArray       = nullptr;
    m_pFrameArray        = nullptr;
    m_pMaterialArray     = nullptr;

    m_pAnimationHeader    = nullptr;
    m_pAnimationFrameData = nullptr;
}


//--------------------------------------------------------------------------------------
PRIMITIVE_TOPOLOGY DXSDKMesh::GetPrimitiveType( DXSDKMESH_PRIMITIVE_TYPE PrimType )
{
    switch( PrimType )
    {
        case PT_TRIANGLE_LIST:
            return PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            
        case PT_TRIANGLE_STRIP:
            return PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            
        case PT_LINE_LIST:
            return PRIMITIVE_TOPOLOGY_LINE_LIST;
            
        case PT_LINE_STRIP:
            return PRIMITIVE_TOPOLOGY_UNDEFINED;
            
        case PT_POINT_LIST:
            return PRIMITIVE_TOPOLOGY_POINT_LIST;
            
        case PT_TRIANGLE_LIST_ADJ:
            UNEXPECTED("Unsupported primitive topolgy type");
            return PRIMITIVE_TOPOLOGY_UNDEFINED;// PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
            
        case PT_TRIANGLE_STRIP_ADJ:
            UNEXPECTED("Unsupported primitive topolgy type");
            return PRIMITIVE_TOPOLOGY_UNDEFINED;// PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
            
        case PT_LINE_LIST_ADJ:
            UNEXPECTED("Unsupported primitive topolgy type");
            return PRIMITIVE_TOPOLOGY_UNDEFINED;// PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
            
        case PT_LINE_STRIP_ADJ:
            UNEXPECTED("Unsupported primitive topolgy type");
            return PRIMITIVE_TOPOLOGY_UNDEFINED;// D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
            
        default:
            UNEXPECTED("Unknown primitive topolgy type");
            return PRIMITIVE_TOPOLOGY_UNDEFINED;
    }
}

//--------------------------------------------------------------------------------------
VALUE_TYPE DXSDKMesh::GetIBFormat( Uint32 iMesh )
{
    switch( m_pIndexBufferArray[ m_pMeshArray[ iMesh ].IndexBuffer ].IndexType )
    {
        case IT_16BIT:
            return VT_UINT16;
        case IT_32BIT:
            return VT_UINT32;
        default:
            UNEXPECTED("Unexpected IB format");
            return VT_UINT32;
    }
}

//--------------------------------------------------------------------------------------
Uint32 DXSDKMesh::GetNumMeshes()
{
    if( !m_pMeshHeader )
        return 0;
    return m_pMeshHeader->NumMeshes;
}

//--------------------------------------------------------------------------------------
Uint32 DXSDKMesh::GetNumMaterials()
{
    if( !m_pMeshHeader )
        return 0;
    return m_pMeshHeader->NumMaterials;
}

//--------------------------------------------------------------------------------------
Uint32 DXSDKMesh::GetNumVBs()
{
    if( !m_pMeshHeader )
        return 0;
    return m_pMeshHeader->NumVertexBuffers;
}

//--------------------------------------------------------------------------------------
Uint32 DXSDKMesh::GetNumIBs()
{
    if( !m_pMeshHeader )
        return 0;
    return m_pMeshHeader->NumIndexBuffers;
}

//--------------------------------------------------------------------------------------
Uint8* DXSDKMesh::GetRawVerticesAt( Uint32 iVB )
{
    return m_ppVertices[iVB];
}

//--------------------------------------------------------------------------------------
Uint8* DXSDKMesh::GetRawIndicesAt( Uint32 iIB )
{
    return m_ppIndices[iIB];
}

//--------------------------------------------------------------------------------------
DXSDKMESH_MATERIAL* DXSDKMesh::GetMaterial( Uint32 iMaterial )
{
    return &m_pMaterialArray[ iMaterial ];
}

//--------------------------------------------------------------------------------------
DXSDKMESH_MESH* DXSDKMesh::GetMesh( Uint32 iMesh )
{
    return &m_pMeshArray[ iMesh ];
}

//--------------------------------------------------------------------------------------
Uint32 DXSDKMesh::GetNumSubsets( Uint32 iMesh )
{
    return m_pMeshArray[ iMesh ].NumSubsets;
}

//--------------------------------------------------------------------------------------
DXSDKMESH_SUBSET* DXSDKMesh::GetSubset( Uint32 iMesh, Uint32 iSubset )
{
    return &m_pSubsetArray[ m_pMeshArray[ iMesh ].pSubsets[iSubset] ];
}

//--------------------------------------------------------------------------------------
Uint32 DXSDKMesh::GetVertexStride( Uint32 iMesh, Uint32 iVB )
{
    return ( Uint32 )m_pVertexBufferArray[ m_pMeshArray[ iMesh ].VertexBuffers[iVB] ].StrideUint8s;
}

//--------------------------------------------------------------------------------------
Uint64 DXSDKMesh::GetNumVertices( Uint32 iMesh, Uint32 iVB )
{
    return m_pVertexBufferArray[ m_pMeshArray[ iMesh ].VertexBuffers[iVB] ].NumVertices;
}

//--------------------------------------------------------------------------------------
Uint64 DXSDKMesh::GetNumIndices( Uint32 iMesh )
{
    return m_pIndexBufferArray[ m_pMeshArray[ iMesh ].IndexBuffer ].NumIndices;
}

DXSDKMESH_INDEX_TYPE DXSDKMesh::GetIndexType( Uint32 iMesh )
{
    return ( DXSDKMESH_INDEX_TYPE ) m_pIndexBufferArray[m_pMeshArray[ iMesh ].IndexBuffer].IndexType;
}


}