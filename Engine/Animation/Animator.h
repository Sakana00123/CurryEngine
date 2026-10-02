#pragma once
#include "Engine/Core/Component.h"
#include "Engine/Resources/AnimatorController.h"

/** @brief Animator を表すクラスです。 */
class Animator : public Component
{
	C_REFLECT(Animator)
public:
	/**
	 * @brief Animator を構築します。
	 */
	Animator() = default;
	/**
	 * @brief Animator を破棄します。
	 */
	virtual ~Animator() = default;

	/**
	 * @brief Awake の処理を行います。
	 */
	void Awake() override;

	/**
	 * @brief 状態を更新します。
	 */
	void Update(float deltaTime) override;

	/** @brief イベントの処理。*/
	void ProcessEvents(const std::vector<CurryEngine::Resources::FiredAnimationEvent>& events);

#ifdef USE_IMGUI
	/** @brief インスペクタ用プロパティ表示。*/
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI

	/**
	 * @brief SyncController の処理を行います。
	 */
	C_FUNCTION()
	void SyncController();

	/**
	 * @brief Float を設定します。
	 */
	C_FUNCTION()
	void SetFloat(const char* name, float value);
	/**
	 * @brief Int を設定します。
	 */
	C_FUNCTION()
	void SetInt(const char* name, int value);
	/**
	 * @brief Bool を設定します。
	 */
	C_FUNCTION()
	void SetBool(const char* name, bool value);
	/**
	 * @brief Trigger を設定します。
	 */
	C_FUNCTION()
	void SetTrigger(const char* name);

	/**
	 * @brief Play の処理を行います。
	 */
	C_FUNCTION()
	void Play(const char* name);

	/**
	 * @brief CrossFade の処理を行います。
	 */
	C_FUNCTION()
	void CrossFade(const char* name, float duration);

	/**
	 * @brief CrossFadeInFixedTime の処理を行います。
	 */
	C_FUNCTION()
	void CrossFadeInFixedTime(const char* name, float duration);

	// インデックスからステート名を取得する
	/**
	 * @brief GetStateNameFromIndex に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	C_FUNCTION()
	const char* GetStateNameFromIndex(int stateIndex) const;

	// ステート名からインデックスを取得する
	/**
	 * @brief GetStateIndexFromName に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	C_FUNCTION()
	int GetStateIndexFromName(const char* stateName) const;

	// 現在のステートインデックスを取得する
	/**
	 * @brief GetCurrentStateIndex に対応する値を取得します。
	 * @return 処理結果を返します。
	 */
	C_FUNCTION()
	int GetCurrentStateIndex() const;

	// シリアライズ
	/**
	 * @brief Serialize の処理を行います。
	 */
	json Serialize() const override;

	// デシリアライズ
	/**
	 * @brief Deserialize の処理を行います。
	 */
	void Deserialize(const json& jsonData) override;

	// デシリアライズ後の処理
	/**
	 * @brief PostDeserialize の処理を行います。
	 */
	void PostDeserialize() override;

private:

	std::shared_ptr<AnimatorController> controller;
	std::shared_ptr<RuntimeAnimatorController> runtimeController;

	C_PROPERTY(CurryEngine::PropertyAttributes::CustomDrawer("AssetId"), CurryEngine::PropertyAttributes::AssetTypeExtension(".controller"), CurryEngine::PropertyAttributes::OnPropertyChanged("SyncController"))
	CurryEngine::Resources::AssetId controllerAssetId; // AnimatorController の AssetId

	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("GltfModelRenderer"))
	ObjectId targetModelRendererId; // アニメーションを適用する対象の GltfModelRenderer の ObjectId

};
