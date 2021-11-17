/*
 *  Copyright 2019-2021 Diligent Graphics LLC
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

#include "RenderPass.h"

namespace Diligent
{

NLOHMANN_JSON_SERIALIZE_ENUM(
    ATTACHMENT_LOAD_OP,
    {
        {ATTACHMENT_LOAD_OP_LOAD, "LOAD"},
        {ATTACHMENT_LOAD_OP_CLEAR, "CLEAR"},
        {ATTACHMENT_LOAD_OP_DISCARD, "DISCARD"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(
    ATTACHMENT_STORE_OP,
    {
        {ATTACHMENT_STORE_OP_STORE, "STORE"},
        {ATTACHMENT_STORE_OP_DISCARD, "DISCARD"},
    })

inline void Serialize(nlohmann::json& Json, const RenderPassAttachmentDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.Format == RenderPassAttachmentDesc{}.Format))
        Serialize(Json["Format"], Type.Format, pAllocator);

    if (!(Type.SampleCount == RenderPassAttachmentDesc{}.SampleCount))
        Serialize(Json["SampleCount"], Type.SampleCount, pAllocator);

    if (!(Type.LoadOp == RenderPassAttachmentDesc{}.LoadOp))
        Serialize(Json["LoadOp"], Type.LoadOp, pAllocator);

    if (!(Type.StoreOp == RenderPassAttachmentDesc{}.StoreOp))
        Serialize(Json["StoreOp"], Type.StoreOp, pAllocator);

    if (!(Type.StencilLoadOp == RenderPassAttachmentDesc{}.StencilLoadOp))
        Serialize(Json["StencilLoadOp"], Type.StencilLoadOp, pAllocator);

    if (!(Type.StencilStoreOp == RenderPassAttachmentDesc{}.StencilStoreOp))
        Serialize(Json["StencilStoreOp"], Type.StencilStoreOp, pAllocator);

    if (!(Type.InitialState == RenderPassAttachmentDesc{}.InitialState))
        SerializeBitwiseEnum(Json["InitialState"], Type.InitialState, pAllocator);

    if (!(Type.FinalState == RenderPassAttachmentDesc{}.FinalState))
        SerializeBitwiseEnum(Json["FinalState"], Type.FinalState, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, RenderPassAttachmentDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("Format"))
        Deserialize(Json["Format"], Type.Format, pAllocator);

    if (Json.contains("SampleCount"))
        Deserialize(Json["SampleCount"], Type.SampleCount, pAllocator);

    if (Json.contains("LoadOp"))
        Deserialize(Json["LoadOp"], Type.LoadOp, pAllocator);

    if (Json.contains("StoreOp"))
        Deserialize(Json["StoreOp"], Type.StoreOp, pAllocator);

    if (Json.contains("StencilLoadOp"))
        Deserialize(Json["StencilLoadOp"], Type.StencilLoadOp, pAllocator);

    if (Json.contains("StencilStoreOp"))
        Deserialize(Json["StencilStoreOp"], Type.StencilStoreOp, pAllocator);

    if (Json.contains("InitialState"))
        DeserializeBitwiseEnum(Json["InitialState"], Type.InitialState, pAllocator);

    if (Json.contains("FinalState"))
        DeserializeBitwiseEnum(Json["FinalState"], Type.FinalState, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const AttachmentReference& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.AttachmentIndex == AttachmentReference{}.AttachmentIndex))
        Serialize(Json["AttachmentIndex"], Type.AttachmentIndex, pAllocator);

    if (!(Type.State == AttachmentReference{}.State))
        SerializeBitwiseEnum(Json["State"], Type.State, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, AttachmentReference& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("AttachmentIndex"))
        Deserialize(Json["AttachmentIndex"], Type.AttachmentIndex, pAllocator);

    if (Json.contains("State"))
        DeserializeBitwiseEnum(Json["State"], Type.State, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const ShadingRateAttachment& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.Attachment == ShadingRateAttachment{}.Attachment))
        Serialize(Json["Attachment"], Type.Attachment, pAllocator);

    if (!CompareConstArray(Type.TileSize, ShadingRateAttachment{}.TileSize, _countof(Type.TileSize)))
        SerializeConstArray(Json["TileSize"], Type.TileSize, _countof(Type.TileSize), pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, ShadingRateAttachment& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("Attachment"))
        Deserialize(Json["Attachment"], Type.Attachment, pAllocator);

    if (Json.contains("TileSize"))
        DeserializeConstArray(Json["TileSize"], Type.TileSize, _countof(Type.TileSize), pAllocator);
}

inline void Serialize(nlohmann::json& Json, const SubpassDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.InputAttachmentCount == SubpassDesc{}.InputAttachmentCount))
        Serialize(Json["InputAttachmentCount"], Type.InputAttachmentCount, pAllocator);

    if (!(Type.pInputAttachments == SubpassDesc{}.pInputAttachments))
        SerializePtr(Json["pInputAttachments"], Type.pInputAttachments, Type.InputAttachmentCount, pAllocator);

    if (!(Type.RenderTargetAttachmentCount == SubpassDesc{}.RenderTargetAttachmentCount))
        Serialize(Json["RenderTargetAttachmentCount"], Type.RenderTargetAttachmentCount, pAllocator);

    if (!(Type.pRenderTargetAttachments == SubpassDesc{}.pRenderTargetAttachments))
        SerializePtr(Json["pRenderTargetAttachments"], Type.pRenderTargetAttachments, Type.RenderTargetAttachmentCount, pAllocator);

    if (!(Type.pResolveAttachments == SubpassDesc{}.pResolveAttachments))
        SerializePtr(Json["pResolveAttachments"], Type.pResolveAttachments, Type.PreserveAttachmentCount, pAllocator);

    if (!(Type.pDepthStencilAttachment == SubpassDesc{}.pDepthStencilAttachment))
        SerializePtr(Json["pDepthStencilAttachment"], Type.pDepthStencilAttachment, pAllocator);

    if (!(Type.PreserveAttachmentCount == SubpassDesc{}.PreserveAttachmentCount))
        Serialize(Json["PreserveAttachmentCount"], Type.PreserveAttachmentCount, pAllocator);

    if (!(Type.pPreserveAttachments == SubpassDesc{}.pPreserveAttachments))
        SerializePtr(Json["pPreserveAttachments"], Type.pPreserveAttachments, Type.PreserveAttachmentCount, pAllocator);

    if (!(Type.pShadingRateAttachment == SubpassDesc{}.pShadingRateAttachment))
        SerializePtr(Json["pShadingRateAttachment"], Type.pShadingRateAttachment, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, SubpassDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("InputAttachmentCount"))
        Deserialize(Json["InputAttachmentCount"], Type.InputAttachmentCount, pAllocator);

    if (Json.contains("pInputAttachments"))
        DeserializePtr(Json["pInputAttachments"], RemoveConst(&Type.pInputAttachments), Json.at("InputAttachmentCount"), pAllocator);

    if (Json.contains("RenderTargetAttachmentCount"))
        Deserialize(Json["RenderTargetAttachmentCount"], Type.RenderTargetAttachmentCount, pAllocator);

    if (Json.contains("pRenderTargetAttachments"))
        DeserializePtr(Json["pRenderTargetAttachments"], RemoveConst(&Type.pRenderTargetAttachments), Json.at("RenderTargetAttachmentCount"), pAllocator);

    if (Json.contains("pResolveAttachments"))
        DeserializePtr(Json["pResolveAttachments"], RemoveConst(&Type.pResolveAttachments), Json.at("PreserveAttachmentCount"), pAllocator);

    if (Json.contains("pDepthStencilAttachment"))
        DeserializePtr(Json["pDepthStencilAttachment"], RemoveConst(&Type.pDepthStencilAttachment), pAllocator);

    if (Json.contains("PreserveAttachmentCount"))
        Deserialize(Json["PreserveAttachmentCount"], Type.PreserveAttachmentCount, pAllocator);

    if (Json.contains("pPreserveAttachments"))
        DeserializePtr(Json["pPreserveAttachments"], RemoveConst(&Type.pPreserveAttachments), Json.at("PreserveAttachmentCount"), pAllocator);

    if (Json.contains("pShadingRateAttachment"))
        DeserializePtr(Json["pShadingRateAttachment"], RemoveConst(&Type.pShadingRateAttachment), pAllocator);
}

inline void Serialize(nlohmann::json& Json, const SubpassDependencyDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (!(Type.SrcSubpass == SubpassDependencyDesc{}.SrcSubpass))
        Serialize(Json["SrcSubpass"], Type.SrcSubpass, pAllocator);

    if (!(Type.DstSubpass == SubpassDependencyDesc{}.DstSubpass))
        Serialize(Json["DstSubpass"], Type.DstSubpass, pAllocator);

    if (!(Type.SrcStageMask == SubpassDependencyDesc{}.SrcStageMask))
        SerializeBitwiseEnum(Json["SrcStageMask"], Type.SrcStageMask, pAllocator);

    if (!(Type.DstStageMask == SubpassDependencyDesc{}.DstStageMask))
        SerializeBitwiseEnum(Json["DstStageMask"], Type.DstStageMask, pAllocator);

    if (!(Type.SrcAccessMask == SubpassDependencyDesc{}.SrcAccessMask))
        SerializeBitwiseEnum(Json["SrcAccessMask"], Type.SrcAccessMask, pAllocator);

    if (!(Type.DstAccessMask == SubpassDependencyDesc{}.DstAccessMask))
        SerializeBitwiseEnum(Json["DstAccessMask"], Type.DstAccessMask, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, SubpassDependencyDesc& Type, DeviceObjectReflection* pAllocator)
{
    if (Json.contains("SrcSubpass"))
        Deserialize(Json["SrcSubpass"], Type.SrcSubpass, pAllocator);

    if (Json.contains("DstSubpass"))
        Deserialize(Json["DstSubpass"], Type.DstSubpass, pAllocator);

    if (Json.contains("SrcStageMask"))
        DeserializeBitwiseEnum(Json["SrcStageMask"], Type.SrcStageMask, pAllocator);

    if (Json.contains("DstStageMask"))
        DeserializeBitwiseEnum(Json["DstStageMask"], Type.DstStageMask, pAllocator);

    if (Json.contains("SrcAccessMask"))
        DeserializeBitwiseEnum(Json["SrcAccessMask"], Type.SrcAccessMask, pAllocator);

    if (Json.contains("DstAccessMask"))
        DeserializeBitwiseEnum(Json["DstAccessMask"], Type.DstAccessMask, pAllocator);
}

inline void Serialize(nlohmann::json& Json, const RenderPassDesc& Type, DeviceObjectReflection* pAllocator)
{
    Serialize(Json, static_cast<DeviceObjectAttribs>(Type), pAllocator);

    if (!(Type.AttachmentCount == RenderPassDesc{}.AttachmentCount))
        Serialize(Json["AttachmentCount"], Type.AttachmentCount, pAllocator);

    if (!(Type.pAttachments == RenderPassDesc{}.pAttachments))
        SerializePtr(Json["pAttachments"], Type.pAttachments, Type.AttachmentCount, pAllocator);

    if (!(Type.SubpassCount == RenderPassDesc{}.SubpassCount))
        Serialize(Json["SubpassCount"], Type.SubpassCount, pAllocator);

    if (!(Type.pSubpasses == RenderPassDesc{}.pSubpasses))
        SerializePtr(Json["pSubpasses"], Type.pSubpasses, Type.SubpassCount, pAllocator);

    if (!(Type.DependencyCount == RenderPassDesc{}.DependencyCount))
        Serialize(Json["DependencyCount"], Type.DependencyCount, pAllocator);

    if (!(Type.pDependencies == RenderPassDesc{}.pDependencies))
        SerializePtr(Json["pDependencies"], Type.pDependencies, Type.DependencyCount, pAllocator);
}

inline void Deserialize(const nlohmann::json& Json, RenderPassDesc& Type, DeviceObjectReflection* pAllocator)
{
    Deserialize(Json, static_cast<DeviceObjectAttribs&>(Type), pAllocator);

    if (Json.contains("AttachmentCount"))
        Deserialize(Json["AttachmentCount"], Type.AttachmentCount, pAllocator);

    if (Json.contains("pAttachments"))
        DeserializePtr(Json["pAttachments"], RemoveConst(&Type.pAttachments), Json.at("AttachmentCount"), pAllocator);

    if (Json.contains("SubpassCount"))
        Deserialize(Json["SubpassCount"], Type.SubpassCount, pAllocator);

    if (Json.contains("pSubpasses"))
        DeserializePtr(Json["pSubpasses"], RemoveConst(&Type.pSubpasses), Json.at("SubpassCount"), pAllocator);

    if (Json.contains("DependencyCount"))
        Deserialize(Json["DependencyCount"], Type.DependencyCount, pAllocator);

    if (Json.contains("pDependencies"))
        DeserializePtr(Json["pDependencies"], RemoveConst(&Type.pDependencies), Json.at("DependencyCount"), pAllocator);
}

} // namespace Diligent
