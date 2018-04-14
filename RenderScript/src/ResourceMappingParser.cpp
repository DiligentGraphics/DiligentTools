/*     Copyright 2015-2018 Egor Yusov
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

#include "pch.h"
#include "ResourceMappingParser.h"
#include "TextureViewParser.h"
#include "BufferParser.h"
#include "BufferViewParser.h"

namespace Diligent
{
    const Char* ResourceMappingParser::ResourceMappingLibName = "ResourceMapping";

    ResourceMappingParser::ResourceMappingParser( IRenderDevice *pRenderDevice, lua_State *L,
                                                    TextureViewParser *pTexViewParser,
                                                    BufferParser *pBuffParser,
                                                    BufferViewParser *pBuffViewParser ) :
        EngineObjectParserBase( pRenderDevice, L,  ResourceMappingLibName),
        m_pTexViewParser( pTexViewParser ),
        m_pBuffParser( pBuffParser ),
        m_pBuffViewParser( pBuffViewParser  ),
        m_MappedResourceMetatables( { pTexViewParser->GetMetatableName(), pBuffParser->GetMetatableName(), pBuffViewParser->GetMetatableName() } )
    {
        DEFINE_BINDER_EX( m_Bindings, ResourceMappingEntry, Name, const Char*, SkipValidationFunc<const Char*> );
        DEFINE_BINDER_EX( m_Bindings, ResourceMappingEntry, pObject, EngineObjectPtrLoader<IDeviceObject>, m_MappedResourceMetatables );
    };

    void ResourceMappingParser::CreateObj( lua_State *L )
    {
        std::vector<ResourceMappingEntry> Entries;
        
        ParseLuaArray( L, 1, &Entries, [ &]( void* pBasePointer, int StackIndex, int NewArrayIndex )
        {
            VERIFY( pBasePointer == &Entries, "Sanity check failed" );
            auto CurrIndex = Entries.size();
            if( static_cast<int>(CurrIndex) != NewArrayIndex - 1 )
                SCRIPT_PARSING_ERROR( L, "Explicit array indices are not allowed in resource mapping description.  Provided index ", NewArrayIndex - 1, " conflicts with actual index ", CurrIndex, "." );
            Entries.resize( CurrIndex + 1 );
            ParseLuaTable( L, StackIndex, &(Entries)[CurrIndex], m_Bindings );
        }
        );

        Entries.push_back( ResourceMappingEntry() );

        // Note that ResourceMappingEntry.Name is declared as const Char*, not as String.
        // This is not a problem as Lua guarantees that all pointers to string values 
        // remain valid as long as these strings are not popped from the stack
        ResourceMappingDesc ResourceMappingDesc;
        ResourceMappingDesc.pEntries = Entries.data();

        IResourceMapping **ppResourceMapping = reinterpret_cast<IResourceMapping**>( lua_newuserdata( L, sizeof( IResourceMapping* ) ) );
        *ppResourceMapping = nullptr;
        m_pRenderDevice->CreateResourceMapping( ResourceMappingDesc, ppResourceMapping );
        if( *ppResourceMapping == nullptr )
            SCRIPT_PARSING_ERROR(L, "Failed to create resource mapping")
    }
    
    void ResourceMappingParser::DestroyObj( void *pData )
    {
        if( pData != nullptr )
        {
            auto ppObject = reinterpret_cast<IResourceMapping**>(pData);
            if( *ppObject != nullptr )
                (*ppObject)->Release();
        }
    }

    void ResourceMappingParser::ReadField( lua_State *L, void *pData, const Char *Field )
    {
        auto *pResourceMapping = *reinterpret_cast<IResourceMapping**>(pData);
        Diligent::RefCntAutoPtr<IDeviceObject> pResource;
        pResourceMapping->GetResource( Field, &pResource );
        Diligent::RefCntAutoPtr<IObject> pTextureView;
        Diligent::RefCntAutoPtr<IObject> pBuffer;
        if( pResource )
        {
            pResource->QueryInterface( IID_TextureView, &pTextureView );
            if( pTextureView )
            {
                m_pTexViewParser->PushObject(L, pTextureView);
            }

            if( !pTextureView )
            {
                pResource->QueryInterface( IID_Buffer, &pBuffer );
                if( pBuffer )
                {
                    m_pBuffParser->PushObject( L, pBuffer );
                }
            }
        }

        if( !(pTextureView || pBuffer) )
        {
            lua_pushnil( L );       // -0 | +1 -> +1
        }
    }

    void ResourceMappingParser::UpdateField( lua_State *L, void *pData, const Char *Field )
    {
        auto *pResourceMapping = *reinterpret_cast<IResourceMapping**>(pData);
        if( lua_type( L, 3 ) == LUA_TNIL )
        {
            pResourceMapping->RemoveResourceByName( Field );
        }
        else
        {
            auto pResource = *GetUserData<IDeviceObject**>( L, 3, m_MappedResourceMetatables );
            pResourceMapping->AddResource( Field, pResource, false );
        }
    }

    void ResourceMappingParser::GetObjectByName( lua_State *L, const Char *ShaderName, IResourceMapping** ppObject )
    {
        auto pObject = *GetGlobalObject<IResourceMapping**>( L, ShaderName, m_MetatableRegistryName.c_str() );
        *ppObject = pObject;
        pObject->AddRef();
    }

    void ResourceMappingParser::PushExistingObject( lua_State *L, const void *pObject )
    {
        auto ppObject = reinterpret_cast<IResourceMapping**>(lua_newuserdata( L, sizeof( IResourceMapping* ) ));
        *ppObject = reinterpret_cast<IResourceMapping*>(const_cast<void*>(pObject));
        (*ppObject)->AddRef();
    }
}
