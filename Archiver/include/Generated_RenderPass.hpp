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

inline void to_json(nlohmann::json& Json, const RenderPassAttachmentDesc& Type)
{
    if (!(Type.Format == RenderPassAttachmentDesc{}.Format))
    {
        Json["Format"] = Type.Format;
    }

    if (!(Type.SampleCount == RenderPassAttachmentDesc{}.SampleCount))
    {
        Json["SampleCount"] = Type.SampleCount;
    }

    if (!(Type.LoadOp == RenderPassAttachmentDesc{}.LoadOp))
    {
        Json["LoadOp"] = Type.LoadOp;
    }

    if (!(Type.StoreOp == RenderPassAttachmentDesc{}.StoreOp))
    {
        Json["StoreOp"] = Type.StoreOp;
    }

    if (!(Type.StencilLoadOp == RenderPassAttachmentDesc{}.StencilLoadOp))
    {
        Json["StencilLoadOp"] = Type.StencilLoadOp;
    }

    if (!(Type.StencilStoreOp == RenderPassAttachmentDesc{}.StencilStoreOp))
    {
        Json["StencilStoreOp"] = Type.StencilStoreOp;
    }

    if (!(Type.InitialState == RenderPassAttachmentDesc{}.InitialState))
    {
        to_json_bitwise(Json["InitialState"], Type.InitialState);
    }

    if (!(Type.FinalState == RenderPassAttachmentDesc{}.FinalState))
    {
        to_json_bitwise(Json["FinalState"], Type.FinalState);
    }
}

inline void from_json(const nlohmann::json& Json, RenderPassAttachmentDesc& Type)
{
    if (Json.contains("Format"))
    {
        Json["Format"].get_to(Type.Format);
    }

    if (Json.contains("SampleCount"))
    {
        Json["SampleCount"].get_to(Type.SampleCount);
    }

    if (Json.contains("LoadOp"))
    {
        Json["LoadOp"].get_to(Type.LoadOp);
    }

    if (Json.contains("StoreOp"))
    {
        Json["StoreOp"].get_to(Type.StoreOp);
    }

    if (Json.contains("StencilLoadOp"))
    {
        Json["StencilLoadOp"].get_to(Type.StencilLoadOp);
    }

    if (Json.contains("StencilStoreOp"))
    {
        Json["StencilStoreOp"].get_to(Type.StencilStoreOp);
    }

    if (Json.contains("InitialState"))
    {
        from_json_bitwise(Json["InitialState"], Type.InitialState);
    }

    if (Json.contains("FinalState"))
    {
        from_json_bitwise(Json["FinalState"], Type.FinalState);
    }
}

inline void to_json(nlohmann::json& Json, const AttachmentReference& Type)
{
    if (!(Type.AttachmentIndex == AttachmentReference{}.AttachmentIndex))
    {
        Json["AttachmentIndex"] = Type.AttachmentIndex;
    }

    if (!(Type.State == AttachmentReference{}.State))
    {
        to_json_bitwise(Json["State"], Type.State);
    }
}

inline void from_json(const nlohmann::json& Json, AttachmentReference& Type)
{
    if (Json.contains("AttachmentIndex"))
    {
        Json["AttachmentIndex"].get_to(Type.AttachmentIndex);
    }

    if (Json.contains("State"))
    {
        from_json_bitwise(Json["State"], Type.State);
    }
}

inline void to_json(nlohmann::json& Json, const ShadingRateAttachment& Type)
{
    if (!(Type.Attachment == ShadingRateAttachment{}.Attachment))
    {
        Json["Attachment"] = Type.Attachment;
    }

    if (!(Type.TileSize == ShadingRateAttachment{}.TileSize))
    {
        Json["TileSize"] = Type.TileSize;
    }
}

inline void from_json(const nlohmann::json& Json, ShadingRateAttachment& Type)
{
    if (Json.contains("Attachment"))
    {
        Json["Attachment"].get_to(Type.Attachment);
    }

    if (Json.contains("TileSize"))
    {
        Json["TileSize"].get_to(Type.TileSize);
    }
}

inline void to_json(nlohmann::json& Json, const SubpassDesc& Type)
{
    if (!(Type.InputAttachmentCount == SubpassDesc{}.InputAttachmentCount))
    {
        Json["InputAttachmentCount"] = Type.InputAttachmentCount;
    }

    if (!(Type.pInputAttachments == SubpassDesc{}.pInputAttachments))
    {
        to_json_ptr(Json["pInputAttachments"], Type.pInputAttachments, Type.InputAttachmentCount);
    }

    if (!(Type.RenderTargetAttachmentCount == SubpassDesc{}.RenderTargetAttachmentCount))
    {
        Json["RenderTargetAttachmentCount"] = Type.RenderTargetAttachmentCount;
    }

    if (!(Type.pRenderTargetAttachments == SubpassDesc{}.pRenderTargetAttachments))
    {
        to_json_ptr(Json["pRenderTargetAttachments"], Type.pRenderTargetAttachments, Type.RenderTargetAttachmentCount);
    }

    if (!(Type.pResolveAttachments == SubpassDesc{}.pResolveAttachments))
    {
        to_json_ptr(Json["pResolveAttachments"], Type.pResolveAttachments, Type.PreserveAttachmentCount);
    }

    if (!(Type.pDepthStencilAttachment == SubpassDesc{}.pDepthStencilAttachment))
    {
        to_json_ptr(Json["pDepthStencilAttachment"], Type.pDepthStencilAttachment);
    }

    if (!(Type.PreserveAttachmentCount == SubpassDesc{}.PreserveAttachmentCount))
    {
        Json["PreserveAttachmentCount"] = Type.PreserveAttachmentCount;
    }

    if (!(Type.pPreserveAttachments == SubpassDesc{}.pPreserveAttachments))
    {
        to_json_ptr(Json["pPreserveAttachments"], Type.pPreserveAttachments, Type.PreserveAttachmentCount);
    }

    if (!(Type.pShadingRateAttachment == SubpassDesc{}.pShadingRateAttachment))
    {
        to_json_ptr(Json["pShadingRateAttachment"], Type.pShadingRateAttachment);
    }
}

inline void from_json(const nlohmann::json& Json, SubpassDesc& Type)
{
    if (Json.contains("InputAttachmentCount"))
    {
        Json["InputAttachmentCount"].get_to(Type.InputAttachmentCount);
    }

    if (Json.contains("pInputAttachments"))
    {
        from_json_ptr(Json["pInputAttachments"], remove_const(&Type.pInputAttachments), Json.at("InputAttachmentCount"));
    }

    if (Json.contains("RenderTargetAttachmentCount"))
    {
        Json["RenderTargetAttachmentCount"].get_to(Type.RenderTargetAttachmentCount);
    }

    if (Json.contains("pRenderTargetAttachments"))
    {
        from_json_ptr(Json["pRenderTargetAttachments"], remove_const(&Type.pRenderTargetAttachments), Json.at("RenderTargetAttachmentCount"));
    }

    if (Json.contains("pResolveAttachments"))
    {
        from_json_ptr(Json["pResolveAttachments"], remove_const(&Type.pResolveAttachments), Json.at("PreserveAttachmentCount"));
    }

    if (Json.contains("pDepthStencilAttachment"))
    {
        from_json_ptr(Json["pDepthStencilAttachment"], remove_const(&Type.pDepthStencilAttachment));
    }

    if (Json.contains("PreserveAttachmentCount"))
    {
        Json["PreserveAttachmentCount"].get_to(Type.PreserveAttachmentCount);
    }

    if (Json.contains("pPreserveAttachments"))
    {
        from_json_ptr(Json["pPreserveAttachments"], remove_const(&Type.pPreserveAttachments), Json.at("PreserveAttachmentCount"));
    }

    if (Json.contains("pShadingRateAttachment"))
    {
        from_json_ptr(Json["pShadingRateAttachment"], remove_const(&Type.pShadingRateAttachment));
    }
}

inline void to_json(nlohmann::json& Json, const SubpassDependencyDesc& Type)
{
    if (!(Type.SrcSubpass == SubpassDependencyDesc{}.SrcSubpass))
    {
        Json["SrcSubpass"] = Type.SrcSubpass;
    }

    if (!(Type.DstSubpass == SubpassDependencyDesc{}.DstSubpass))
    {
        Json["DstSubpass"] = Type.DstSubpass;
    }

    if (!(Type.SrcStageMask == SubpassDependencyDesc{}.SrcStageMask))
    {
        to_json_bitwise(Json["SrcStageMask"], Type.SrcStageMask);
    }

    if (!(Type.DstStageMask == SubpassDependencyDesc{}.DstStageMask))
    {
        to_json_bitwise(Json["DstStageMask"], Type.DstStageMask);
    }

    if (!(Type.SrcAccessMask == SubpassDependencyDesc{}.SrcAccessMask))
    {
        to_json_bitwise(Json["SrcAccessMask"], Type.SrcAccessMask);
    }

    if (!(Type.DstAccessMask == SubpassDependencyDesc{}.DstAccessMask))
    {
        to_json_bitwise(Json["DstAccessMask"], Type.DstAccessMask);
    }
}

inline void from_json(const nlohmann::json& Json, SubpassDependencyDesc& Type)
{
    if (Json.contains("SrcSubpass"))
    {
        Json["SrcSubpass"].get_to(Type.SrcSubpass);
    }

    if (Json.contains("DstSubpass"))
    {
        Json["DstSubpass"].get_to(Type.DstSubpass);
    }

    if (Json.contains("SrcStageMask"))
    {
        from_json_bitwise(Json["SrcStageMask"], Type.SrcStageMask);
    }

    if (Json.contains("DstStageMask"))
    {
        from_json_bitwise(Json["DstStageMask"], Type.DstStageMask);
    }

    if (Json.contains("SrcAccessMask"))
    {
        from_json_bitwise(Json["SrcAccessMask"], Type.SrcAccessMask);
    }

    if (Json.contains("DstAccessMask"))
    {
        from_json_bitwise(Json["DstAccessMask"], Type.DstAccessMask);
    }
}

inline void to_json(nlohmann::json& Json, const RenderPassDesc& Type)
{
    nlohmann::to_json(Json, static_cast<DeviceObjectAttribs>(Type));

    if (!(Type.AttachmentCount == RenderPassDesc{}.AttachmentCount))
    {
        Json["AttachmentCount"] = Type.AttachmentCount;
    }

    if (!(Type.pAttachments == RenderPassDesc{}.pAttachments))
    {
        to_json_ptr(Json["pAttachments"], Type.pAttachments, Type.AttachmentCount);
    }

    if (!(Type.SubpassCount == RenderPassDesc{}.SubpassCount))
    {
        Json["SubpassCount"] = Type.SubpassCount;
    }

    if (!(Type.pSubpasses == RenderPassDesc{}.pSubpasses))
    {
        to_json_ptr(Json["pSubpasses"], Type.pSubpasses, Type.SubpassCount);
    }

    if (!(Type.DependencyCount == RenderPassDesc{}.DependencyCount))
    {
        Json["DependencyCount"] = Type.DependencyCount;
    }

    if (!(Type.pDependencies == RenderPassDesc{}.pDependencies))
    {
        to_json_ptr(Json["pDependencies"], Type.pDependencies, Type.DependencyCount);
    }
}

inline void from_json(const nlohmann::json& Json, RenderPassDesc& Type)
{
    nlohmann::from_json(Json, static_cast<DeviceObjectAttribs&>(Type));

    if (Json.contains("AttachmentCount"))
    {
        Json["AttachmentCount"].get_to(Type.AttachmentCount);
    }

    if (Json.contains("pAttachments"))
    {
        from_json_ptr(Json["pAttachments"], remove_const(&Type.pAttachments), Json.at("AttachmentCount"));
    }

    if (Json.contains("SubpassCount"))
    {
        Json["SubpassCount"].get_to(Type.SubpassCount);
    }

    if (Json.contains("pSubpasses"))
    {
        from_json_ptr(Json["pSubpasses"], remove_const(&Type.pSubpasses), Json.at("SubpassCount"));
    }

    if (Json.contains("DependencyCount"))
    {
        Json["DependencyCount"].get_to(Type.DependencyCount);
    }

    if (Json.contains("pDependencies"))
    {
        from_json_ptr(Json["pDependencies"], remove_const(&Type.pDependencies), Json.at("DependencyCount"));
    }
}

} // namespace Diligent
