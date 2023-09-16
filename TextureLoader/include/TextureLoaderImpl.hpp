/*
 *  Copyright 2019-2023 Diligent Graphics LLC
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

#include <vector>

#include "TextureLoader.h"
#include "RefCntAutoPtr.hpp"
#include "ObjectBase.hpp"

namespace Diligent
{

/// Implementation of ITextureLoader.
class TextureLoaderImpl final : public ObjectBase<ITextureLoader>
{
public:
    using TBase = ObjectBase<ITextureLoader>;

    TextureLoaderImpl(IReferenceCounters*        pRefCounters,
                      const TextureLoadInfo&     TexLoadInfo,
                      const Uint8*               pData,
                      size_t                     DataSize,
                      RefCntAutoPtr<IDataBlob>&& pDataBlob);

    TextureLoaderImpl(IReferenceCounters*    pRefCounters,
                      const TextureLoadInfo& TexLoadInfo,
                      Image*                 pImage);

    IMPLEMENT_QUERY_INTERFACE_IN_PLACE(IID_TextureLoader, TBase)

    virtual void DILIGENT_CALL_TYPE CreateTexture(IRenderDevice* pDevice,
                                                  ITexture**     ppTexture) override final;

    virtual const TextureDesc& DILIGENT_CALL_TYPE GetTextureDesc() const override final
    {
        return m_TexDesc;
    }

    virtual const TextureSubResData& DILIGENT_CALL_TYPE GetSubresourceData(Uint32 MipLevel,
                                                                           Uint32 ArraySlice) const override final
    {
        const auto Subres = ArraySlice * m_TexDesc.MipLevels + MipLevel;
        VERIFY_EXPR(Subres < m_SubResources.size());
        return m_SubResources[Subres];
    }

    virtual TextureData DILIGENT_CALL_TYPE GetTextureData() override final
    {
        return TextureData{m_SubResources.data(), static_cast<Uint32>(m_SubResources.size())};
    }

private:
    void LoadFromImage(const TextureLoadInfo& TexLoadInfo);
    void LoadFromKTX(const TextureLoadInfo& TexLoadInfo, const Uint8* pData, size_t DataSize);
    void LoadFromDDS(const TextureLoadInfo& TexLoadInfo, const Uint8* pData, size_t DataSize);

private:
    RefCntAutoPtr<IDataBlob> m_pDataBlob;
    RefCntAutoPtr<Image>     m_pImage;

    const std::string m_Name;
    TextureDesc       m_TexDesc;

    std::vector<TextureSubResData>  m_SubResources;
    std::vector<std::vector<Uint8>> m_Mips;
};

} // namespace Diligent
