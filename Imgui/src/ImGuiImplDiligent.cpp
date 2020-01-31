/*
 *  Copyright 2019-2020 Diligent Graphics LLC
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

#include <cstddef>
#include "imgui.h"
#include "ImGuiImplDiligent.hpp"
#include "RenderDevice.h"
#include "DeviceContext.h"
#include "RefCntAutoPtr.hpp"
#include "BasicMath.hpp"
#include "MapHelper.hpp"

namespace Diligent
{

class ImGuiImplDiligent_Internal
{
public:
    ImGuiImplDiligent_Internal(IRenderDevice* pDevice,
                               TEXTURE_FORMAT BackBufferFmt,
                               TEXTURE_FORMAT DepthBufferFmt,
                               Uint32         InitialVertexBufferSize,
                               Uint32         InitialIndexBufferSize) :
        // clang-format off
        m_pDevice         {pDevice},
        m_BackBufferFmt   {BackBufferFmt},
        m_DepthBufferFmt  {DepthBufferFmt},
        m_VertexBufferSize{InitialVertexBufferSize},
        m_IndexBufferSize {InitialIndexBufferSize}
    // clang-format on
    {
        // Setup back-end capabilities flags
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io            = ImGui::GetIO();
        io.BackendRendererName = "ImGuiImplDiligent";
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset; // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
        io.IniFilename = nullptr;

        CreateDeviceObjects();
    }

    ~ImGuiImplDiligent_Internal()
    {
        ImGui::DestroyContext();
    }

    void NewFrame();
    void RenderDrawData(IDeviceContext* pCtx, ImDrawData* pDrawData);

    // Use if you want to reset your rendering device without losing ImGui state.
    void InvalidateDeviceObjects();
    void CreateDeviceObjects();

    void CreateFontsTexture();

private:
    RefCntAutoPtr<IRenderDevice>          m_pDevice;
    RefCntAutoPtr<IBuffer>                m_pVB;
    RefCntAutoPtr<IBuffer>                m_pIB;
    RefCntAutoPtr<IBuffer>                m_pVertexConstantBuffer;
    RefCntAutoPtr<IPipelineState>         m_pPSO;
    RefCntAutoPtr<ITextureView>           m_pFontSRV;
    RefCntAutoPtr<IShaderResourceBinding> m_pSRB;
    const TEXTURE_FORMAT                  m_BackBufferFmt;
    const TEXTURE_FORMAT                  m_DepthBufferFmt;
    Uint32                                m_VertexBufferSize = 0;
    Uint32                                m_IndexBufferSize  = 0;
};

void ImGuiImplDiligent_Internal::NewFrame()
{
    if (!m_pPSO)
        CreateDeviceObjects();
}

// Use if you want to reset your rendering device without losing ImGui state.
void ImGuiImplDiligent_Internal::InvalidateDeviceObjects()
{
    m_pVB.Release();
    m_pIB.Release();
    m_pVertexConstantBuffer.Release();
    m_pPSO.Release();
    m_pFontSRV.Release();
    m_pSRB.Release();
}

static const char* VertexShaderSource = R"(
cbuffer Constants
{
    float4x4 ProjectionMatrix;
}

struct VSInput
{
    float2 pos : ATTRIB0;
    float2 uv  : ATTRIB1;
    float4 col : ATTRIB2;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 uv  : TEXCOORD;
};

void main(in VSInput VSIn, out PSInput PSIn)
{
    PSIn.pos = mul(ProjectionMatrix, float4(VSIn.pos.xy, 0.0, 1.0));
    PSIn.col = VSIn.col;
    PSIn.uv  = VSIn.uv;
}
)";

static const char* PixelShaderSource = R"(
struct PSInput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 uv  : TEXCOORD;
};

Texture2D    Texture;
SamplerState Texture_sampler;

float4 main(in PSInput PSIn) : SV_Target
{
    return PSIn.col * Texture.Sample(Texture_sampler, PSIn.uv);
}
)";

void ImGuiImplDiligent_Internal::CreateDeviceObjects()
{
    InvalidateDeviceObjects();

    ShaderCreateInfo ShaderCI;
    ShaderCI.UseCombinedTextureSamplers = true;
    ShaderCI.SourceLanguage             = SHADER_SOURCE_LANGUAGE_HLSL;

    RefCntAutoPtr<IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        ShaderCI.Desc.Name       = "Imgui VS";
        ShaderCI.Source          = VertexShaderSource;
        m_pDevice->CreateShader(ShaderCI, &pVS);
    }

    RefCntAutoPtr<IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        ShaderCI.Desc.Name       = "Imgui PS";
        ShaderCI.Source          = PixelShaderSource;
        m_pDevice->CreateShader(ShaderCI, &pPS);
    }

    PipelineStateDesc PSODesc;
    PSODesc.Name           = "ImGUI PSO";
    auto& GraphicsPipeline = PSODesc.GraphicsPipeline;

    GraphicsPipeline.NumRenderTargets  = 1;
    GraphicsPipeline.RTVFormats[0]     = m_BackBufferFmt;
    GraphicsPipeline.DSVFormat         = m_DepthBufferFmt;
    GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    GraphicsPipeline.pVS = pVS;
    GraphicsPipeline.pPS = pPS;

    GraphicsPipeline.RasterizerDesc.CullMode      = CULL_MODE_NONE;
    GraphicsPipeline.RasterizerDesc.ScissorEnable = True;
    GraphicsPipeline.DepthStencilDesc.DepthEnable = False;

    auto& RT0                 = GraphicsPipeline.BlendDesc.RenderTargets[0];
    RT0.BlendEnable           = True;
    RT0.SrcBlend              = BLEND_FACTOR_SRC_ALPHA;
    RT0.DestBlend             = BLEND_FACTOR_INV_SRC_ALPHA;
    RT0.BlendOp               = BLEND_OPERATION_ADD;
    RT0.SrcBlendAlpha         = BLEND_FACTOR_INV_SRC_ALPHA;
    RT0.DestBlendAlpha        = BLEND_FACTOR_ZERO;
    RT0.BlendOpAlpha          = BLEND_OPERATION_ADD;
    RT0.RenderTargetWriteMask = COLOR_MASK_ALL;

    LayoutElement VSInputs[] //
        {
            {0, 0, 2, VT_FLOAT32},    // pos
            {1, 0, 2, VT_FLOAT32},    // uv
            {2, 0, 4, VT_UINT8, True} // col
        };
    GraphicsPipeline.InputLayout.NumElements    = _countof(VSInputs);
    GraphicsPipeline.InputLayout.LayoutElements = VSInputs;

    ShaderResourceVariableDesc Variables[] =
        {
            {SHADER_TYPE_PIXEL, "Texture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE} //
        };
    PSODesc.ResourceLayout.Variables    = Variables;
    PSODesc.ResourceLayout.NumVariables = _countof(Variables);

    SamplerDesc SamLinearWrap;
    SamLinearWrap.AddressU = TEXTURE_ADDRESS_WRAP;
    SamLinearWrap.AddressV = TEXTURE_ADDRESS_WRAP;
    SamLinearWrap.AddressW = TEXTURE_ADDRESS_WRAP;
    StaticSamplerDesc StaticSamplers[] =
        {
            {SHADER_TYPE_PIXEL, "Texture", SamLinearWrap} //
        };
    PSODesc.ResourceLayout.StaticSamplers    = StaticSamplers;
    PSODesc.ResourceLayout.NumStaticSamplers = _countof(StaticSamplers);

    m_pDevice->CreatePipelineState(PSODesc, &m_pPSO);

    {
        BufferDesc BuffDesc;
        BuffDesc.uiSizeInBytes  = sizeof(float4x4);
        BuffDesc.Usage          = USAGE_DYNAMIC;
        BuffDesc.BindFlags      = BIND_UNIFORM_BUFFER;
        BuffDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        m_pDevice->CreateBuffer(BuffDesc, nullptr, &m_pVertexConstantBuffer);
    }
    m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(m_pVertexConstantBuffer);

    CreateFontsTexture();
}

void ImGuiImplDiligent_Internal::CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO&       io     = ImGui::GetIO();
    unsigned char* pixels = nullptr;
    int            width = 0, height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    TextureDesc FontTexDesc;
    FontTexDesc.Name      = "Imgui font texture";
    FontTexDesc.Type      = RESOURCE_DIM_TEX_2D;
    FontTexDesc.Width     = static_cast<Uint32>(width);
    FontTexDesc.Height    = static_cast<Uint32>(height);
    FontTexDesc.Format    = TEX_FORMAT_RGBA8_UNORM;
    FontTexDesc.BindFlags = BIND_SHADER_RESOURCE;
    FontTexDesc.Usage     = USAGE_STATIC;

    TextureSubResData Mip0Data[] = {{pixels, FontTexDesc.Width * 4}};
    TextureData       InitData(Mip0Data, _countof(Mip0Data));

    RefCntAutoPtr<ITexture> pFontTex;
    m_pDevice->CreateTexture(FontTexDesc, &InitData, &pFontTex);
    m_pFontSRV = pFontTex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

    m_pSRB.Release();
    m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);
    m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "Texture")->Set(m_pFontSRV);

    // Store our identifier
    io.Fonts->TexID = (ImTextureID)m_pFontSRV;
}


void ImGuiImplDiligent_Internal::RenderDrawData(IDeviceContext* pCtx, ImDrawData* pDrawData)
{
    // Avoid rendering when minimized
    if (pDrawData->DisplaySize.x <= 0.0f || pDrawData->DisplaySize.y <= 0.0f)
        return;

    // Create and grow vertex/index buffers if needed
    if (!m_pVB || static_cast<int>(m_VertexBufferSize) < pDrawData->TotalVtxCount)
    {
        m_pVB.Release();
        while (static_cast<int>(m_VertexBufferSize) < pDrawData->TotalVtxCount)
            m_VertexBufferSize *= 2;

        BufferDesc VBDesc;
        VBDesc.Name           = "Imgui vertex buffer";
        VBDesc.BindFlags      = BIND_VERTEX_BUFFER;
        VBDesc.uiSizeInBytes  = m_VertexBufferSize * sizeof(ImDrawVert);
        VBDesc.Usage          = USAGE_DYNAMIC;
        VBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        m_pDevice->CreateBuffer(VBDesc, nullptr, &m_pVB);
    }

    if (!m_pIB || static_cast<int>(m_IndexBufferSize) < pDrawData->TotalIdxCount)
    {
        m_pIB.Release();
        while (static_cast<int>(m_IndexBufferSize) < pDrawData->TotalIdxCount)
            m_IndexBufferSize *= 2;

        BufferDesc IBDesc;
        IBDesc.Name           = "Imgui index buffer";
        IBDesc.BindFlags      = BIND_INDEX_BUFFER;
        IBDesc.uiSizeInBytes  = m_IndexBufferSize * sizeof(ImDrawIdx);
        IBDesc.Usage          = USAGE_DYNAMIC;
        IBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        m_pDevice->CreateBuffer(IBDesc, nullptr, &m_pIB);
    }

    {
        MapHelper<ImDrawVert> Verices(pCtx, m_pVB, MAP_WRITE, MAP_FLAG_DISCARD);
        MapHelper<ImDrawIdx>  Indices(pCtx, m_pIB, MAP_WRITE, MAP_FLAG_DISCARD);

        ImDrawVert* vtx_dst = Verices;
        ImDrawIdx*  idx_dst = Indices;
        for (int n = 0; n < pDrawData->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = pDrawData->CmdLists[n];
            memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtx_dst += cmd_list->VtxBuffer.Size;
            idx_dst += cmd_list->IdxBuffer.Size;
        }
    }


    // Setup orthographic projection matrix into our constant buffer
    // Our visible imgui space lies from pDrawData->DisplayPos (top left) to pDrawData->DisplayPos+data_data->DisplaySize (bottom right).
    // DisplayPos is (0,0) for single viewport apps.
    {
        MapHelper<float4x4> CBData(pCtx, m_pVertexConstantBuffer, MAP_WRITE, MAP_FLAG_DISCARD);

        float L = pDrawData->DisplayPos.x;
        float R = pDrawData->DisplayPos.x + pDrawData->DisplaySize.x;
        float T = pDrawData->DisplayPos.y;
        float B = pDrawData->DisplayPos.y + pDrawData->DisplaySize.y;
        // clang-format off
        *CBData = float4x4
        {
            2.0f / (R - L),                  0.0f,   0.0f,   0.0f,
            0.0f,                  2.0f / (T - B),   0.0f,   0.0f,
            0.0f,                            0.0f,   0.5f,   0.0f,
            (R + L) / (L - R),  (T + B) / (B - T),   0.5f,   1.0f
        };
        // clang-format on
    }

    auto DisplayWidth     = static_cast<Uint32>(pDrawData->DisplaySize.x);
    auto DisplayHeight    = static_cast<Uint32>(pDrawData->DisplaySize.y);
    auto SetupRenderState = [&]() //
    {
        // Setup shader and vertex buffers
        Uint32   Offsets[] = {0};
        IBuffer* pVBs[]    = {m_pVB};
        pCtx->SetVertexBuffers(0, 1, pVBs, Offsets, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
        pCtx->SetIndexBuffer(m_pIB, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        pCtx->SetPipelineState(m_pPSO);
        pCtx->CommitShaderResources(m_pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        const float blend_factor[4] = {0.f, 0.f, 0.f, 0.f};
        pCtx->SetBlendFactors(blend_factor);

        Viewport vp;
        vp.Width    = pDrawData->DisplaySize.x;
        vp.Height   = pDrawData->DisplaySize.y;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = vp.TopLeftY = 0;
        pCtx->SetViewports(1, &vp, DisplayWidth, DisplayHeight);
    };

    SetupRenderState();

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int global_idx_offset = 0;
    int global_vtx_offset = 0;

    ImVec2 clip_off = pDrawData->DisplayPos;
    for (int n = 0; n < pDrawData->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = pDrawData->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != NULL)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    SetupRenderState();
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // Apply scissor/clipping rectangle
                const Rect r =
                    {
                        static_cast<Int32>(pcmd->ClipRect.x - clip_off.x),
                        static_cast<Int32>(pcmd->ClipRect.y - clip_off.y),
                        static_cast<Int32>(pcmd->ClipRect.z - clip_off.x),
                        static_cast<Int32>(pcmd->ClipRect.w - clip_off.y) //
                    };
                pCtx->SetScissorRects(1, &r, DisplayWidth, DisplayHeight);

                // Bind texture, Draw
                auto* texture_srv = reinterpret_cast<ITextureView*>(pcmd->TextureId);
                VERIFY_EXPR(texture_srv == m_pFontSRV);
                (void)texture_srv;
                //ctx->PSSetShaderResources(0, 1, &texture_srv);
                DrawIndexedAttribs DrawAttrs(pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? VT_UINT16 : VT_UINT32, DRAW_FLAG_VERIFY_STATES);
                DrawAttrs.FirstIndexLocation = pcmd->IdxOffset + global_idx_offset;
                DrawAttrs.BaseVertex         = pcmd->VtxOffset + global_vtx_offset;
                pCtx->DrawIndexed(DrawAttrs);
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }
}



ImGuiImplDiligent::ImGuiImplDiligent(IRenderDevice* pDevice,
                                     TEXTURE_FORMAT BackBufferFmt,
                                     TEXTURE_FORMAT DepthBufferFmt,
                                     Uint32         InitialVertexBufferSize,
                                     Uint32         InitialIndexBufferSize) :
    m_pImpl(new ImGuiImplDiligent_Internal(pDevice, BackBufferFmt, DepthBufferFmt, InitialVertexBufferSize, InitialIndexBufferSize))
{
}

ImGuiImplDiligent::~ImGuiImplDiligent()
{
}

void ImGuiImplDiligent::NewFrame()
{
    m_pImpl->NewFrame();
    ImGui::NewFrame();
}

void ImGuiImplDiligent::EndFrame()
{
    ImGui::EndFrame();
}

void ImGuiImplDiligent::Render(IDeviceContext* pCtx)
{
    // No need to call ImGui::EndFrame as ImGui::Render calls it automatically
    ImGui::Render();
    m_pImpl->RenderDrawData(pCtx, ImGui::GetDrawData());
}

// Use if you want to reset your rendering device without losing ImGui state.
void ImGuiImplDiligent::InvalidateDeviceObjects()
{
    m_pImpl->InvalidateDeviceObjects();
}

void ImGuiImplDiligent::CreateDeviceObjects()
{
    m_pImpl->CreateDeviceObjects();
}

void ImGuiImplDiligent::UpdateFontsTexture()
{
    m_pImpl->CreateFontsTexture();
}


} // namespace Diligent
