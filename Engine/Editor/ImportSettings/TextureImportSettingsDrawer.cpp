#include "pch.h"
#include "TextureImportSettingsDrawer.h"
#include <Engine\Resources\Texture.h>
#include <Engine\Resources\ImportSettings\TextureImportSettings.h>

namespace CurryEngine::Resources
{
    void TextureImportSettingsDrawer::DrawPreview(const std::shared_ptr<Resource>& previewResource, RenderContext* context)
    {
        auto thumbnail = std::dynamic_pointer_cast<AssetTexture>(previewResource);
        if (!thumbnail) { ImGui::TextDisabled("No preview available."); return; }

        const D3D11_TEXTURE2D_DESC& desc = thumbnail->GetDesc();
        ImGui::Image(thumbnail->GetSRV(), ImVec2(180, 180));
        ImGui::Separator();
        ImGui::Text("%u x %u", desc.Width, desc.Height);
        ImGui::Text("Mips: %u", desc.MipLevels);
        ImGui::Text("Format: %s", DxgiFormatToString(desc.Format));
    }

    bool TextureImportSettingsDrawer::DrawSettingsFields(nlohmann::json& editingSettings, bool& isDirty)
    {
        auto settings = editingSettings.is_null()
            ? TextureImportSettings{}
        : editingSettings.get<TextureImportSettings>();

        bool changed = false;

		// インポーターの種類を選択するコンボボックス
		const char* textureTypeOptions[] = { "Default", "NormalMap", "GUI", "Sprite", "SpriteAtlas", "Cubemap" };
		int currentTextureTypeIndex = static_cast<int>(settings.textureType);

		// インポーターの種類を選択するコンボボックスを描画
        if (ImGui::Combo("Texture Type", &currentTextureTypeIndex, textureTypeOptions, IM_ARRAYSIZE(textureTypeOptions)))
        {
            settings.textureType = static_cast<TextureImporterType>(currentTextureTypeIndex);
			changed = true;
        }

        changed |= ImGui::Checkbox("Generate Mipmaps", &settings.generateMipmaps);

        const char* compressionOptions[] = { "None", "BC1", "BC3", "BC7" };
        int currentIndex = 0;
        for (int i = 0; i < IM_ARRAYSIZE(compressionOptions); ++i)
            if (settings.compression == compressionOptions[i]) { currentIndex = i; break; }

        if (ImGui::Combo("Compression", &currentIndex, compressionOptions, IM_ARRAYSIZE(compressionOptions)))
        {
            settings.compression = compressionOptions[currentIndex];
            changed = true;
        }

        if (changed)
        {
            editingSettings = settings;
            isDirty = true;
        }
		return changed;
    }

    nlohmann::json TextureImportSettingsDrawer::GetDefaultSettings() const
    {
		// デフォルトコンストラクタの値がそのままデフォルト設定になるようにしているため、特に値を指定せずに返す
		return TextureImportSettings{};
	}

    const char* TextureImportSettingsDrawer::DxgiFormatToString(DXGI_FORMAT format)
    {
        switch (format)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM:     return "RGBA8";
        case DXGI_FORMAT_BC1_UNORM:          return "BC1";
        case DXGI_FORMAT_BC3_UNORM:          return "BC3";
        case DXGI_FORMAT_BC7_UNORM:          return "BC7";
        case DXGI_FORMAT_R32G32B32A32_FLOAT: return "RGBA32F";
        case DXGI_FORMAT_R16G16B16A16_FLOAT: return "RGBA16F";
        default:                             return "Unknown";
        }
    }
}