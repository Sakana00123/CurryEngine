#pragma once
#include "IEditorConfig.h"
#include "Engine/Core/Math/Vector3.h"

/** @brief スナップ設定構造体。*/
struct SnapParameters
{
	bool enabled = true; //!< スナップの有効化フラグ
	bool snapAllAxes = false; //!< スナップを全軸に適用するかどうかのフラグ
	Vector3 snapValue = { 1,1,1 }; //!< スナップ値
};

/** @brief シーンビューの設定構造体。*/
struct SceneViewConfig : public IEditorConfig
{
	int guizmoPivotMode; //!< ギズモのピボットモード（0: オブジェクト中心、1: ワールド原点）
	SnapParameters translationSnap; //!< ギズモの移動スナップ設定
	SnapParameters rotationSnap; //!< ギズモの回転スナップ設定
	SnapParameters scaleSnap; //!< ギズモのスケールスナップ設定

	SceneViewConfig() = default;

	/** @brief デフォルト設定にリセットします。*/
	void ResetToDefault() override;

	/** @brief 設定を保存します。*/
	json Serialize() const override;

	/** @brief 設定を読み込みます。*/
	void Deserialize(const json& j) override;
};