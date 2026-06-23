#pragma once
#include <memory>
#include <unordered_map>
#include "Engine/Resources/Shader.h"
#include "Engine/Resources/Texture.h"
#include "Engine/Core/Color.h"
#include "Engine/Rendering/Pipeline/RenderState.h"
#include <json.hpp>

struct RenderContext;
using json = nlohmann::json;

/**
 * @file
 * @brief シェーダやテクスチャ、定数バッファを束ねて描画に適用するマテリアル。
 * @details シェーダ設定、テクスチャのスロット割り当て、各種値の定数バッファ反映、
 *          適用（バインド）処理やインスペクタ描画を提供します。
 */
class Material
{
public:
    /**
     * @brief シェーダを設定します。
     * @param device D3D11 デバイス。
     * @param shader 設定するシェーダ。
     */
    void SetShader(ID3D11Device* device, std::shared_ptr<Shader> shader);

    /**
     * @brief シェーダステージ別のシェーダを取得します。
     * @param type シェーダタイプ。
     * @return 指定タイプのシェーダ。設定されていない場合は nullptr。
     */
    std::shared_ptr<Shader> GetShader(ShaderType type);
    
	/** @brief シェーダを再読み込みします。*/
    void Reload(ID3D11Device* device);

	/** @brief シェーダのみ適用モードを設定。*/
	void SetShaderOnly() { shaderOnly = true; }

	/**
	 * @brief 指定名の定数バッファをバインドしないように設定します。
	 * @param names バインドを抑制する定数バッファ名の配列。
	 */
	void SetNotBindCBuffer(const std::vector<std::string>& names) { m_CBufferNotBindNames = names; }

	/** @brief バインドしない定数バッファ設定をクリアします。*/
	void ClearNotBindCBuffer() { m_CBufferNotBindNames.clear(); }

	/**
	 * @brief 名前でテクスチャを設定します。
	 * @param name テクスチャ変数名。
	 * @param texture 設定するテクスチャ。
	 */
	void SetTexture(const std::string& name, std::shared_ptr<Texture> texture);

	/** @brief 名前でテクスチャを取得します。設定されていなければ nullptr を返す。*/
	template<typename T = Texture>
	std::weak_ptr<T> GetTexture(const std::string& name)
	{
		// Shaderから変数情報を探す
		//for (ShaderBinding& binding : m_ShaderBindings)
		{
			// 対象のテクスチャ変数を探す
			auto it = textures.find(name);
			if (it != textures.end())
			{
				// 見つかったら返す
				return std::dynamic_pointer_cast<T>(it->second)/*.lock()*/;
			}
		}
		// 見つからなかった場合の警告
		//Console::LogWarning("Warning: Material::GetTexture failed. Texture variable " + name + " not found.");
		return {};
	}

	/**
	 * @brief 任意の値を名前で設定します（定数バッファへ反映）。
	 * @tparam T 値の型。
	 * @param name 値の識別名。
	 * @param value 設定する値。
	 * @details 内部で `SetValue(const std::string&, void*, size_t)` を呼び出して、値を定数バッファに反映します。
	 */
	template<typename T>
	void SetValue(const std::string& name, const T& value) {
		SetValue(name, (void*)&value, sizeof(T));
	}

	/**
	 * @brief 任意の値を名前で取得します（CPU 側コピーから）。
	 * @tparam T 値の型。
	 * @param name 値の識別名。
	 * @param value 出力先の値。
	 * @details 内部で `GetValue(const std::string&, void*, size_t)` を呼び出して、CPU 側コピーから値を取得します。
	 */
	template<typename T>
	void GetValue(const std::string& name, T& value) const {
		GetValue(name, (void*)&value, sizeof(T));
	}

    /**
     * @brief 任意の値を名前で設定します（定数バッファへ反映）。
     * @param name 値の識別名。
     * @param value 値の先頭ポインタ。
     * @param size バイトサイズ。
     */
    void SetValue(const std::string& name, void* value, size_t size);
    /**
     * @brief 任意の値を名前で取得します（CPU 側コピーから）。
     * @param name 値の識別名。
     * @param value 出力先バッファ。
     * @param size バイトサイズ。
     */
	void GetValue(const std::string& name, void* value, size_t size) const;

	/** @brief ブレンドステートを取得。*/
	BlendState GetBlendState() const { return blendState; }

	/** @brief ブレンドステートを設定。*/
	void SetBlendState(BlendState state) { blendState = state; }

	/** @brief ラスタライザステートを取得。*/
	RasterizerState GetRasterizerState() const { return rasterizerState; }

	/** @brief ラスタライザステートを設定。*/
	void SetRasterizerState(RasterizerState state) { rasterizerState = state; }

	/** @brief 深度ステンシルステートを取得。*/
	DepthStencilState GetDepthStencilState() const { return depthStencilState; }

	/** @brief 深度ステンシルステートを設定。*/
	void SetDepthStencilState(DepthStencilState state) { depthStencilState = state; }

	/**
	 * @brief マテリアルを描画コンテキストへ適用します（バインド）。
	 * @param rtx 描画コンテキスト。
	 */
	void Apply(RenderContext* rtx);

	/** @brief インスペクタ用のプロパティ描画。*/
	void DrawProperty();

	/** @brief エディタ上でテクスチャが変更されたかどうかを取得。*/
	bool IsTextureChanged() const { return m_IsTextureChanged; }

	/** @brief エディタ上でテクスチャが変更されたかどうかをクリア。*/
	void ClearTextureChangedFlag() { m_IsTextureChanged = false; }

	/**
	 * @brief マテリアルの設定を JSON 化します。
	 * @return マテリアル設定の JSON オブジェクト。
	 */
	json Serialize() const;

	/**
	 * @brief JSON からマテリアルの設定を復元します。
	 * @param j マテリアル設定の JSON オブジェクト。
	 * @return 成功で true。
	 */
	bool Deserialize(const json& j);

private:
	// シェーダセレクタの描画
	void DrawShaderSelector(ID3D11Device* device, size_t type);

	// 定数バッファ変数の描画
	void DrawCBufferVariables(size_t type);

	// テクスチャスロットの描画
	void DrawTextureSlots(size_t type);
	
	// サンプラースロットの描画
	void DrawSamplerSlots(size_t type);
	
	struct ShaderBinding;
	// 定数バッファの設定を更新
	void UpdateCBufferBindings(ID3D11Device* device, ShaderBinding& binding, const ShaderReflectionData& reflection);

	// テクスチャとサンプラーの設定を更新
	void UpdateTextureAndSamplerBindings(ID3D11Device* device, ShaderBinding& binding, const ShaderReflectionData& reflection);

private:
    /**
     * @brief 定数バッファの実体と CPU 側コピーをまとめた構造体。
     */
    struct CBufferData
    {
        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer; //!< GPU 側の定数バッファ
        std::vector<uint8_t> localData; // CPU 側のコピー
        bool dirty = false;             //!< 変更フラグ（true のとき GPU へ更新）
    };

    /**
     * @brief シェーダとその定数バッファ群をまとめた構造体。
	 */
    struct ShaderBinding
    {
		std::shared_ptr<Shader> shader = nullptr; //!< シェーダー
    };

	/** @brief シェーダステージ別のシェーダと定数バッファ群。*/
	ShaderBinding m_ShaderBindings[static_cast<size_t>(ShaderType::EnumCount)];
	
	std::unordered_map<std::string, CBufferData> cbuffers; //!< 名前をキーとする定数バッファ群
	std::unordered_map<std::string, std::shared_ptr<Texture>> textures; //!< 名前をキーとするテクスチャ群
	std::unordered_map<std::string, std::vector<uint8_t>> values; //!< 名前をキーとする任意の値群（CPU 側コピー）
	//bool m_isValuesDirty = false; //!< 任意の値が変更されたかどうか
	std::unordered_map<std::string, SamplerState> samplers; //!< 名前をキーとするサンプラー群

	/** @brief 深度ステンシルステート。*/
    DepthStencilState depthStencilState = DepthStencilState::EnumCount;
	/** @brief ブレンドステート。*/
    BlendState blendState = BlendState::EnumCount;
	/** @brief ラスタライザステート。*/
    RasterizerState rasterizerState = RasterizerState::EnumCount;

	/** @brief Apply時にバインドしない定数バッファ名一覧。*/
	std::vector<std::string> m_CBufferNotBindNames = {
		"SCENE_CONSTANT_BUFFER", "LIGHT_CONSTANT_BUFFER"
	};

private:
	bool m_IsTextureChanged = false; //!< テクスチャが変更されたかどうか
	bool shaderOnly = false; //!< シェーダのみのマテリアルかどうか
	//MaterialProperty m_Property; //!< マテリアルプロパティ
	//ImGuiTextFilter m_Filter; //!< インスペクタのフィルタ
	//bool m_IsOpen = true; //!< インスペクタが開いているかどうか
	//bool m_IsShaderSelectorOpen = false; //!< シェーダセレクタが開いているかどうか
	//size_t m_CurrentShaderType = 0; //!< 現在選択中のシェーダタイプ（インスペクタ用）
	//static std::shared_ptr<Shader> s_NullShader; //!< シェーダ未設定時に使用するダミーシェーダ
};