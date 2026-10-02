#pragma once
#include "Selectable.h"
#include "Text.h"

/** @brief Toggle を表すクラスです。 */
class Toggle : public Selectable, public IPointerClickHandler, public ISubmitHandler
{
	C_REFLECT(Toggle)
public:
	using Callback = std::function<void(bool)>;

	/**
	 * @brief Toggle を構築します。
	 */
	Toggle() = default;
	/**
	 * @brief Toggle を破棄します。
	 */
	~Toggle() override = default;

	/**
	 * @brief 初期化します。
	 */
	void Initialize() override;

	/**
	 * @brief Begin の処理を行います。
	 */
	void Begin(RenderContext* rtx) override;

#ifdef USE_IMGUI
	/**
	 * @brief 描画処理を行います。
	 */
	void DrawProperty(const PropertyDrawContext& context) override;
#endif // USE_IMGUI


	// シリアライズ
	/**
	 * @brief Serialize の処理を行います。
	 */
	json Serialize() const override;

	// デシリアライズ
	/**
	 * @brief Deserialize の処理を行います。
	 */
	void Deserialize(const json& j) override;

public:

	/**
	 * @brief IsOn を設定します。
	 */
	void SetIsOn(bool isOn);
	/**
	 * @brief IsOn の条件を満たすか判定します。
	 * @return 処理結果を返します。
	 */
	bool IsOn() const;

	/**
	 * @brief AddCallback の処理を行います。
	 */
	void AddCallback(std::function<void(bool)> func);
protected:
	/**
	 * @brief PointerClick イベントを処理します。
	 */
	void OnPointerClick(PointerEventData* eventData) override;
	/**
	 * @brief Submit イベントを処理します。
	 */
	void OnSubmit(BaseEventData* eventData) override;
private:
	/**
	 * @brief Notify の処理を行います。
	 */
	void Notify();
public:
	C_PROPERTY(CurryEngine::PropertyAttributes::ObjectReference("Image"))
	ObjectId checkMarkReference;

private:
	bool isOn = false;
	std::vector<Callback> callbacks;
};