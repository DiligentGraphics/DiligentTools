/*     Copyright 2015-2017 Egor Yusov
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

#include <memory>
#include "LuaWrappers.h"
#include "LuaFunctionBinding.h"
#include "RenderDevice.h"
#include "DeviceContext.h"
#include "RefCountedObjectImpl.h"
#include "RefCntAutoPtr.h"

namespace Diligent
{
    class ScriptParser : public Diligent::RefCountedObject<Diligent::IObject>
    {
    public:
        ScriptParser( IRenderDevice *pRenderDevice );
        ~ScriptParser();
        
        virtual void QueryInterface( const Diligent::INTERFACE_ID &IID, IObject **ppInterface );

        void Parse(const Char *pScript);
        
        template<typename... ArgsType>
        void Run(IDeviceContext *pContext, const ArgsType&... Args)
        {
            lua_pushstring( m_LuaState, DeviceContextRegistryKey );  // -0 | +1 -> +1
            lua_pushlightuserdata( m_LuaState, pContext );           // -0 | +1 -> +1
            lua_settable( m_LuaState, LUA_REGISTRYINDEX );           // -2 | +0 -> -2

            m_RunFunctionCaller(Args...);
        }
        template<typename... ArgsType>
        void Run( RefCntAutoPtr<IDeviceContext> &pContext, const ArgsType&... Args )
        {
            Run(pContext.RawPtr(), Args... );
        }

        template<typename... ArgsType>
        void Run( const ArgsType&... Args )
        {
            Run( static_cast<IDeviceContext *>(nullptr), Args... );
        }


        void GetSamplerByName( const Char *SamplerName, ISampler** ppSampler );
        void GetShaderByName( const Char *ShaderName, IShader** ppShader );
        void GetBufferByName( const Char *BufferName, IBuffer** ppBuffer );
        void GetTextureByName( const Char *TextureName, ITexture** ppTexture );
        void GetResourceMappingByName( const Char *ResourceMappingName, IResourceMapping** ppResourceMapping );
        void GetTextureViewByName( const Char *TextureViewName, ITextureView** ppTextureView );
        void GetBufferViewByName( const Char *BufferViewName, IBufferView** ppTextureView );
        void GetPipelineStateByName( const Char *PSOName, IPipelineState** ppPSO );
        void GetShaderVariableByName( const Char *ShaderVarName, IShaderVariable** ppShaderVar );

        template<typename ValType>
        void SetGlobalVariable( const Char *Name, ValType Var )
        {
            m_RunFunctionCaller.PushFuncStub( m_LuaState, Var );
            lua_setglobal( m_LuaState, Name );
        }

        static const Char* DeviceContextRegistryKey;

    private:
        class SpecialPushFuncs
        {
        public:
            SpecialPushFuncs() : m_pScriptParser( nullptr ){}
            void SetScriptParser( ScriptParser *pScriptParser ){ m_pScriptParser = pScriptParser; }

            void PushFuncStub( lua_State *L, const ISampler* pSampler );
            void PushFuncStub( lua_State *L, const RefCntAutoPtr<ISampler> &pSampler );
            void PushFuncStub( lua_State *L, const IShader* pShader );
            void PushFuncStub( lua_State *L, const RefCntAutoPtr<IShader> &pShader );
            void PushFuncStub( lua_State *L, const IBuffer* pBuffer );
            void PushFuncStub( lua_State *L, const RefCntAutoPtr<IBuffer> &pBuffer );
            void PushFuncStub( lua_State *L, const ITexture* pTexture );
            void PushFuncStub( lua_State *L, const RefCntAutoPtr<ITexture> &pTexture );
            void PushFuncStub( lua_State *L, const DrawAttribs &DrawAttribs );
            void PushFuncStub( lua_State *L, const IResourceMapping* pResourceMapping );
            void PushFuncStub( lua_State *L, const RefCntAutoPtr<IResourceMapping> &pResourceMapping );
            void PushFuncStub( lua_State *L, const ITextureView* pTextureView );
            void PushFuncStub( lua_State *L, const RefCntAutoPtr<ITextureView> &pTextureView );
            void PushFuncStub( lua_State *L, const IBufferView* pBufferView );
            void PushFuncStub( lua_State *L, const RefCntAutoPtr<IBufferView> &pBufferView );
            void PushFuncStub( lua_State *L, const IPipelineState* pPSO );
            void PushFuncStub( lua_State *L, const RefCntAutoPtr<IPipelineState> &pPSO );
            void PushFuncStub( lua_State *L, const Viewport &Viewport );
            void PushFuncStub( lua_State *L, const Rect &Rect );
            void PushFuncStub( lua_State *L, const IShaderVariable* pShaderVar );
            void PushFuncStub( lua_State *L, const RefCntAutoPtr<IShaderVariable> &pShaderVar );
            void PushFuncStub( lua_State *L, const IShaderResourceBinding* pShaderVar );
            void PushFuncStub( lua_State *L, const RefCntAutoPtr<IShaderResourceBinding> &pShaderVar );

        private:
            ScriptParser *m_pScriptParser;
        };
        LuaFunctionCaller<SpecialPushFuncs> m_RunFunctionCaller;

        void DefineGlobalConstants( lua_State *L );

        Diligent::RefCntAutoPtr<IRenderDevice> m_pRenderDevice;
        LuaState m_LuaState;
        std::unique_ptr<class SamplerParser> m_pSamplerParser;
        std::unique_ptr<class ShaderParser> m_pShaderParser;
        std::unique_ptr<class BufferParser> m_pBufferParser;
        std::unique_ptr<class TextureParser> m_pTextureParser;
        std::unique_ptr<class DrawAttribsParser> m_pDrawAttribsParser;
        std::unique_ptr<class ResourceMappingParser> m_pResourceMappingParser;
        std::unique_ptr<class TextureViewParser> m_pTextureViewParser;
        std::unique_ptr<class BufferViewParser> m_pBufferViewParser;
        std::unique_ptr<class PSODescParser> m_pPSOParser;
        std::unique_ptr<class DeviceContextFuncBindings> m_pDeviceCtxFuncBindings;
        std::unique_ptr<class ViewportParser> m_pViewportParser;
        std::unique_ptr<class ScissorRectParser> m_pScissorRectParser;
        std::unique_ptr<class ShaderVariableParser> m_pShaderVariableParser;
        std::unique_ptr<class ShaderResourceBindingParser> m_pShaderResBindingParser;
    };
}
