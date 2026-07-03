#pragma once
#include <cstdint>
#include <string>
#include <array>
#include <json.hpp>
using json = nlohmann::json;

constexpr int MAX_LAYERS = 32;

using Layer = int;      // 0~31のレイヤーID
using LayerMask = uint32_t; // ビットフラグ

// レイヤーID -> マスクに変換
inline constexpr LayerMask ToMask(Layer layer) {
    return (1u << layer);
}

// よく使うマスク定数
namespace LayerMasks {
    constexpr LayerMask Everything = 0xFFFFFFFF;
    constexpr LayerMask Nothing = 0x00000000;
	constexpr LayerMask Default = ToMask(0); // デフォルトレイヤー
	constexpr LayerMask TransparentFX = ToMask(1); // 透明FXレイヤー
	constexpr LayerMask IgnoreRaycast = ToMask(2); // Raycastを無視するレイヤー
	constexpr LayerMask UI = ToMask(3); // UIレイヤー
}


// レイヤー管理クラス
class LayerManager {
public:
    static LayerManager& Get() {
        static LayerManager instance;
        return instance;
    }

    // レイヤー名登録
    void SetLayerName(Layer layer, const std::string& name);

	// レイヤーIDから名前を取得
    Layer GetLayerByName(const std::string& name) const;

    // 衝突マトリクス設定（対称に自動設定）
    void SetLayerCollision(Layer a, Layer b, bool enabled);

    // レイヤー同士が衝突するか
	bool GetLayerCollision(Layer a, Layer b) const;

    // PhysXに渡すフィルターマスク取得
    LayerMask GetCollisionMask(Layer layer) const;

	// レイヤー名の配列を取得
	std::array<std::string, MAX_LAYERS> GetLayerNames() const { return m_layerNames; }

	// シリアライズ
    json Serialize() const;

	// デシリアライズ
    void Deserialize(const json& j);

	// レイヤー設定GUIの描画
	void DrawLayerSettingsGUI();

	// レイヤー設定GUIを開く
	void OpenLayerSettingsGUI() { m_isOpen = true; }

private:
	// コンストラクタはシングルトンのためprivate
    LayerManager();

	bool m_isOpen = false; // レイヤー設定GUIが開いているか

    std::array<std::string, MAX_LAYERS> m_layerNames;
	LayerMask m_collisionMatrix[MAX_LAYERS];
	Layer m_selectedLayer = -1; // GUIで選択されているレイヤーのID
	char m_renameBuffer[64] = { 0 }; // レイヤー名変更用のバッファ
	bool m_layerNameEditMode = false; // レイヤー名編集モードかどうか
	bool m_layerNameEditModeJustStarted = false; // レイヤー名編集モードが開始されたばかりかどうか
	Layer m_defaultLayer = 0; // デフォルトレイヤーID
	Layer m_transparentFXLayer = 1; // 透明FXレイヤーID
	Layer m_ignoreRaycastLayer = 2; // Raycastを無視するレイヤーID
	Layer m_uiLayer = 3; // UIレイヤーID
};