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

#include "RenderStateMarkupParser.h"

namespace Diligent
{

inline void Serialize(nlohmann::json& Json, const GraphicsPipelineMarkup& Type, DynamicLinearAllocator& Allocator)
{
    if (!CompareStr(Type.pRenderPass, GraphicsPipelineMarkup{}.pRenderPass))
        Serialize(Json["pRenderPass"], Type.pRenderPass, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, GraphicsPipelineMarkup& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("pRenderPass"))
        Deserialize(Json["pRenderPass"], Type.pRenderPass, Allocator);
}

inline void Serialize(nlohmann::json& Json, const PipelineStateCreateMarkup& Type, DynamicLinearAllocator& Allocator)
{
    if (!(Type.ppResourceSignatures == PipelineStateCreateMarkup{}.ppResourceSignatures))
        Serialize(Json["ppResourceSignatures"], Type.ppResourceSignatures, Type.ResourceSignaturesCount, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, PipelineStateCreateMarkup& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("ppResourceSignatures"))
        Deserialize(Json["ppResourceSignatures"], Type.ppResourceSignatures, Type.ResourceSignaturesCount, Allocator);
}

inline void Serialize(nlohmann::json& Json, const GraphicsPipelineStateCreateMarkup& Type, DynamicLinearAllocator& Allocator)
{
    Serialize(Json, static_cast<const PipelineStateCreateMarkup&>(Type), Allocator);

    if (!(Type.GraphicsPipeline == GraphicsPipelineStateCreateMarkup{}.GraphicsPipeline))
        Serialize(Json["GraphicsPipeline"], Type.GraphicsPipeline, Allocator);

    if (!CompareStr(Type.pVS, GraphicsPipelineStateCreateMarkup{}.pVS))
        Serialize(Json["pVS"], Type.pVS, Allocator);

    if (!CompareStr(Type.pPS, GraphicsPipelineStateCreateMarkup{}.pPS))
        Serialize(Json["pPS"], Type.pPS, Allocator);

    if (!CompareStr(Type.pDS, GraphicsPipelineStateCreateMarkup{}.pDS))
        Serialize(Json["pDS"], Type.pDS, Allocator);

    if (!CompareStr(Type.pHS, GraphicsPipelineStateCreateMarkup{}.pHS))
        Serialize(Json["pHS"], Type.pHS, Allocator);

    if (!CompareStr(Type.pGS, GraphicsPipelineStateCreateMarkup{}.pGS))
        Serialize(Json["pGS"], Type.pGS, Allocator);

    if (!CompareStr(Type.pAS, GraphicsPipelineStateCreateMarkup{}.pAS))
        Serialize(Json["pAS"], Type.pAS, Allocator);

    if (!CompareStr(Type.pMS, GraphicsPipelineStateCreateMarkup{}.pMS))
        Serialize(Json["pMS"], Type.pMS, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, GraphicsPipelineStateCreateMarkup& Type, DynamicLinearAllocator& Allocator)
{
    Deserialize(Json, static_cast<PipelineStateCreateMarkup&>(Type), Allocator);

    if (Json.contains("GraphicsPipeline"))
        Deserialize(Json["GraphicsPipeline"], Type.GraphicsPipeline, Allocator);

    if (Json.contains("pVS"))
        Deserialize(Json["pVS"], Type.pVS, Allocator);

    if (Json.contains("pPS"))
        Deserialize(Json["pPS"], Type.pPS, Allocator);

    if (Json.contains("pDS"))
        Deserialize(Json["pDS"], Type.pDS, Allocator);

    if (Json.contains("pHS"))
        Deserialize(Json["pHS"], Type.pHS, Allocator);

    if (Json.contains("pGS"))
        Deserialize(Json["pGS"], Type.pGS, Allocator);

    if (Json.contains("pAS"))
        Deserialize(Json["pAS"], Type.pAS, Allocator);

    if (Json.contains("pMS"))
        Deserialize(Json["pMS"], Type.pMS, Allocator);
}

inline void Serialize(nlohmann::json& Json, const ComputePipelineStateCreateMarkup& Type, DynamicLinearAllocator& Allocator)
{
    Serialize(Json, static_cast<const PipelineStateCreateMarkup&>(Type), Allocator);

    if (!CompareStr(Type.pCS, ComputePipelineStateCreateMarkup{}.pCS))
        Serialize(Json["pCS"], Type.pCS, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, ComputePipelineStateCreateMarkup& Type, DynamicLinearAllocator& Allocator)
{
    Deserialize(Json, static_cast<PipelineStateCreateMarkup&>(Type), Allocator);

    if (Json.contains("pCS"))
        Deserialize(Json["pCS"], Type.pCS, Allocator);
}

inline void Serialize(nlohmann::json& Json, const TilePipelineStateCreateMarkup& Type, DynamicLinearAllocator& Allocator)
{
    Serialize(Json, static_cast<const PipelineStateCreateMarkup&>(Type), Allocator);

    if (!CompareStr(Type.pTS, TilePipelineStateCreateMarkup{}.pTS))
        Serialize(Json["pTS"], Type.pTS, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, TilePipelineStateCreateMarkup& Type, DynamicLinearAllocator& Allocator)
{
    Deserialize(Json, static_cast<PipelineStateCreateMarkup&>(Type), Allocator);

    if (Json.contains("pTS"))
        Deserialize(Json["pTS"], Type.pTS, Allocator);
}

inline void Serialize(nlohmann::json& Json, const RayTracingGeneralShaderGroupMarkup& Type, DynamicLinearAllocator& Allocator)
{
    if (!CompareStr(Type.pShader, RayTracingGeneralShaderGroupMarkup{}.pShader))
        Serialize(Json["pShader"], Type.pShader, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, RayTracingGeneralShaderGroupMarkup& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("pShader"))
        Deserialize(Json["pShader"], Type.pShader, Allocator);
}

inline void Serialize(nlohmann::json& Json, const RayTracingTriangleHitShaderGroupMarkup& Type, DynamicLinearAllocator& Allocator)
{
    if (!CompareStr(Type.pClosestHitShader, RayTracingTriangleHitShaderGroupMarkup{}.pClosestHitShader))
        Serialize(Json["pClosestHitShader"], Type.pClosestHitShader, Allocator);

    if (!CompareStr(Type.pAnyHitShader, RayTracingTriangleHitShaderGroupMarkup{}.pAnyHitShader))
        Serialize(Json["pAnyHitShader"], Type.pAnyHitShader, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, RayTracingTriangleHitShaderGroupMarkup& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("pClosestHitShader"))
        Deserialize(Json["pClosestHitShader"], Type.pClosestHitShader, Allocator);

    if (Json.contains("pAnyHitShader"))
        Deserialize(Json["pAnyHitShader"], Type.pAnyHitShader, Allocator);
}

inline void Serialize(nlohmann::json& Json, const RayTracingProceduralHitShaderGroupMarkup& Type, DynamicLinearAllocator& Allocator)
{
    if (!CompareStr(Type.pIntersectionShader, RayTracingProceduralHitShaderGroupMarkup{}.pIntersectionShader))
        Serialize(Json["pIntersectionShader"], Type.pIntersectionShader, Allocator);

    if (!CompareStr(Type.pClosestHitShader, RayTracingProceduralHitShaderGroupMarkup{}.pClosestHitShader))
        Serialize(Json["pClosestHitShader"], Type.pClosestHitShader, Allocator);

    if (!CompareStr(Type.pAnyHitShader, RayTracingProceduralHitShaderGroupMarkup{}.pAnyHitShader))
        Serialize(Json["pAnyHitShader"], Type.pAnyHitShader, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, RayTracingProceduralHitShaderGroupMarkup& Type, DynamicLinearAllocator& Allocator)
{
    if (Json.contains("pIntersectionShader"))
        Deserialize(Json["pIntersectionShader"], Type.pIntersectionShader, Allocator);

    if (Json.contains("pClosestHitShader"))
        Deserialize(Json["pClosestHitShader"], Type.pClosestHitShader, Allocator);

    if (Json.contains("pAnyHitShader"))
        Deserialize(Json["pAnyHitShader"], Type.pAnyHitShader, Allocator);
}

inline void Serialize(nlohmann::json& Json, const RayTracingPipelineStateCreateMarkup& Type, DynamicLinearAllocator& Allocator)
{
    Serialize(Json, static_cast<const PipelineStateCreateMarkup&>(Type), Allocator);

    if (!(Type.pGeneralShaders == RayTracingPipelineStateCreateMarkup{}.pGeneralShaders))
        Serialize(Json["pGeneralShaders"], Type.pGeneralShaders, Type.GeneralShaderCount, Allocator);

    if (!(Type.pTriangleHitShaders == RayTracingPipelineStateCreateMarkup{}.pTriangleHitShaders))
        Serialize(Json["pTriangleHitShaders"], Type.pTriangleHitShaders, Type.TriangleHitShaderCount, Allocator);

    if (!(Type.pProceduralHitShaders == RayTracingPipelineStateCreateMarkup{}.pProceduralHitShaders))
        Serialize(Json["pProceduralHitShaders"], Type.pProceduralHitShaders, Type.ProceduralHitShaderCount, Allocator);
}

inline void Deserialize(const nlohmann::json& Json, RayTracingPipelineStateCreateMarkup& Type, DynamicLinearAllocator& Allocator)
{
    Deserialize(Json, static_cast<PipelineStateCreateMarkup&>(Type), Allocator);

    if (Json.contains("pGeneralShaders"))
        Deserialize(Json["pGeneralShaders"], Type.pGeneralShaders, Type.GeneralShaderCount, Allocator);

    if (Json.contains("pTriangleHitShaders"))
        Deserialize(Json["pTriangleHitShaders"], Type.pTriangleHitShaders, Type.TriangleHitShaderCount, Allocator);

    if (Json.contains("pProceduralHitShaders"))
        Deserialize(Json["pProceduralHitShaders"], Type.pProceduralHitShaders, Type.ProceduralHitShaderCount, Allocator);
}

} // namespace Diligent
